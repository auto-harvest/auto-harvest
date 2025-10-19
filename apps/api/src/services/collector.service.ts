// services/CollectorService.ts
import mongoose from 'mongoose';
import cron from 'node-cron';
import sensorLogModel from '../models/sensorLog.model';
import SquashedSensorLog from '../models/squashedSensorLog.model';

async function withLock<T>(
  key: string,
  fn: () => Promise<T>
): Promise<T | undefined> {
  try {
    await mongoose.connection
      .collection('locks')
      .insertOne({ _id: key as any, createdAt: new Date() });
  } catch (e: any) {
    if (e.code === 11000) return; // already running
    throw e;
  }
  try {
    return await fn();
  } finally {
    await mongoose.connection
      .collection('locks')
      .deleteOne({ _id: key as any });
  }
}

class CollectorService {
  private useAdvancedAggregation = false;

  constructor() {
    this.checkMongoDBVersion();
    this.setupCronJobs();
  }

  private async checkMongoDBVersion() {
    try {
      const admin = mongoose.connection.db.admin();
      const info = await admin.serverInfo();
      const version = info.version.split('.').map(Number);
      // Check if MongoDB version is 5.2 or higher
      this.useAdvancedAggregation =
        version[0] > 5 || (version[0] === 5 && version[1] >= 2);
      console.log(
        `MongoDB ${info.version} detected. Advanced aggregation: ${this.useAdvancedAggregation}`
      );
    } catch (error) {
      console.warn(
        'Could not detect MongoDB version, using fallback aggregation'
      );
      this.useAdvancedAggregation = false;
    }
  }

  /**
   * Store raw sensor log and immediately update squashed logs (event sourcing)
   */
  public async storeSensorLog(log: {
    type: string;
    value: number;
    controller: string;
  }) {
    const timestamp = new Date();
    const newLog = new sensorLogModel({ ...log, timestamp });

    // Store raw log
    await newLog.save();

    // Immediately update squashed logs for all intervals (event sourcing)
    // This runs in background - errors are logged but don't block the main flow
    Promise.all([
      this.updateSquashedLog(
        log.type,
        log.value,
        log.controller,
        timestamp,
        'minute'
      ),
      this.updateSquashedLog(
        log.type,
        log.value,
        log.controller,
        timestamp,
        'hour'
      ),
      this.updateSquashedLog(
        log.type,
        log.value,
        log.controller,
        timestamp,
        'day'
      ),
      this.updateSquashedLog(
        log.type,
        log.value,
        log.controller,
        timestamp,
        'week'
      ),
    ]).catch((error) => {
      console.error('Error updating squashed logs in real-time:', error);
      // Cron jobs will pick up any missed updates
    });
  }

  /**
   * Real-time update of squashed log using incremental statistics
   */
  private async updateSquashedLog(
    type: string,
    value: number,
    controller: string,
    timestamp: Date,
    interval: 'minute' | 'hour' | 'day' | 'week'
  ) {
    try {
      const intervalTimestamp = this.getIntervalTimestamp(timestamp, interval);

      // Find existing squashed log for this interval
      const existing = await SquashedSensorLog.findOne({
        type,
        controller,
        interval,
        timestamp: intervalTimestamp,
      });

      if (existing) {
        // Update existing entry using incremental statistics
        const newDatapoints = existing.datapoints + 1;

        // Incremental mean: new_mean = old_mean + (new_value - old_mean) / new_count
        const newAverage =
          existing.average + (value - existing.average) / newDatapoints;

        // Update variance using Welford's online algorithm
        const delta = value - existing.average;
        const delta2 = value - newAverage;
        const m2 =
          existing.variance * existing.variance * existing.datapoints +
          delta * delta2;
        const newVariance = Math.sqrt(m2 / newDatapoints);

        // Update min/max
        const newMin =
          value < existing.min.value
            ? { value, date: timestamp }
            : existing.min;
        const newMax =
          value > existing.max.value
            ? { value, date: timestamp }
            : existing.max;

        // Approximate median update (simple approach - can be improved)
        // For now, use average of old median and new value weighted by counts
        const newMedian =
          (existing.median * existing.datapoints + value) / newDatapoints;

        await SquashedSensorLog.updateOne(
          { _id: existing._id },
          {
            $set: {
              averageValue: newAverage,
              average: newAverage,
              variance: newVariance,
              median: newMedian,
              datapoints: newDatapoints,
              min: newMin,
              max: newMax,
            },
          }
        );
      } else {
        // Create new entry for this interval
        await SquashedSensorLog.create({
          type,
          controller,
          interval,
          timestamp: intervalTimestamp,
          averageValue: value,
          average: value,
          variance: 0, // Single datapoint has no variance
          median: value,
          datapoints: 1,
          min: { value, date: timestamp },
          max: { value, date: timestamp },
        });
      }
    } catch (error: any) {
      // Duplicate key errors are expected if multiple writes happen simultaneously
      if (error.code !== 11000) {
        console.error(
          `Error updating squashed log for ${interval}:`,
          error.message
        );
        throw error;
      }
      // If duplicate, another process already created it - that's fine
    }
  }

  /**
   * Get the timestamp for the start of an interval
   */
  private getIntervalTimestamp(
    timestamp: Date,
    interval: 'minute' | 'hour' | 'day' | 'week'
  ): Date {
    const date = new Date(timestamp);

    switch (interval) {
      case 'minute':
        date.setSeconds(0, 0);
        break;
      case 'hour':
        date.setMinutes(0, 0, 0);
        break;
      case 'day':
        date.setHours(0, 0, 0, 0);
        break;
      case 'week':
        // eslint-disable-next-line no-case-declarations
        const day = date.getDay();
        date.setDate(date.getDate() - day);
        date.setHours(0, 0, 0, 0);
        break;
    }

    return date;
  }

  /**
   * Get the timestamp for the end of an interval (start of next interval)
   */
  private getNextInterval(timestamp: Date, interval: string): Date {
    const date = new Date(timestamp);

    switch (interval) {
      case 'minute':
        date.setMinutes(date.getMinutes() + 1);
        break;
      case 'hour':
        date.setHours(date.getHours() + 1);
        break;
      case 'day':
        date.setDate(date.getDate() + 1);
        break;
      case 'week':
        date.setDate(date.getDate() + 7);
        break;
    }

    return date;
  }

  /**
   * Find and process only missed intervals (gaps in squashed logs)
   * This reduces the burden on cron jobs since real-time updates handle most cases
   */
  private async groupLogsByInterval(interval: string, dateFormat: string) {
    console.log(`🔍 Checking for missed ${interval} intervals...`);

    // Find the time range to check for gaps
    const now = new Date();
    const lookbackPeriod = this.getLookbackPeriod(interval);
    const startTime = new Date(now.getTime() - lookbackPeriod);

    // Get all raw sensor logs grouped by controller and type
    const rawLogIntervals = await sensorLogModel.aggregate([
      {
        $match: {
          timestamp: { $gte: startTime, $lt: now },
        },
      },
      {
        $group: {
          _id: {
            type: '$type',
            controller: '$controller',
            interval: {
              $dateToString: { format: dateFormat, date: '$timestamp' },
            },
          },
        },
      },
    ]);

    if (rawLogIntervals.length === 0) {
      console.log(`✅ No raw logs found for ${interval} interval check`);
      return;
    }

    // Check which intervals are missing squashed logs
    const missedIntervals = [];
    for (const rawInterval of rawLogIntervals) {
      const exists = await SquashedSensorLog.findOne({
        type: rawInterval._id.type,
        controller: rawInterval._id.controller,
        interval,
        timestamp: new Date(rawInterval._id.interval),
      });

      if (!exists) {
        missedIntervals.push(rawInterval._id);
      }
    }

    if (missedIntervals.length === 0) {
      console.log(`✅ No missed ${interval} intervals found`);
      return;
    }

    console.log(
      `⚠️  Found ${missedIntervals.length} missed ${interval} intervals, processing...`
    );
    console.log(
      `   Missing intervals:`,
      missedIntervals
        .map((m) => `${m.controller}/${m.type}@${m.interval}`)
        .join(', ')
    );

    // Process only the missed intervals
    if (this.useAdvancedAggregation) {
      return this.processMissedIntervalsAdvanced(
        missedIntervals,
        interval,
        dateFormat
      );
    }
    return this.processMissedIntervalsFallback(
      missedIntervals,
      interval,
      dateFormat
    );
  }

  /**
   * Get lookback period for checking missed intervals
   */
  private getLookbackPeriod(interval: string): number {
    switch (interval) {
      case 'minute':
        return 2 * 60 * 60 * 1000; // 2 hours
      case 'hour':
        return 48 * 60 * 60 * 1000; // 48 hours
      case 'day':
        return 14 * 24 * 60 * 60 * 1000; // 14 days
      case 'week':
        return 8 * 7 * 24 * 60 * 60 * 1000; // 8 weeks
      default:
        return 24 * 60 * 60 * 1000; // 24 hours default
    }
  }

  /**
   * MongoDB 5.2+ version - process specific missed intervals
   */
  private async processMissedIntervalsAdvanced(
    missedIntervals: Array<{
      type: string;
      controller: string;
      interval: string;
    }>,
    interval: string,
    dateFormat: string
  ) {
    // Create match conditions for only the missed intervals
    const matchConditions = missedIntervals.map((mi) => ({
      type: mi.type,
      controller: mi.controller,
      timestamp: {
        $gte: new Date(mi.interval),
        $lt: this.getNextInterval(new Date(mi.interval), interval),
      },
    }));

    const logs = await sensorLogModel.aggregate([
      {
        $match: {
          $or: matchConditions,
        },
      },
      {
        $group: {
          _id: { type: '$type', controller: '$controller', bucket: '$bucket' },
          averageValue: { $avg: '$value' },
          variance: { $stdDevPop: '$value' },
          datapoints: { $sum: 1 },
          minValue: { $min: '$value' },
          maxValue: { $max: '$value' },
          // MongoDB 5.2+ native median (approximate) - using any to bypass TypeScript limitation
          // eslint-disable-next-line @typescript-eslint/no-explicit-any
          median: {
            $median: { input: '$value', method: 'approximate' },
          } as any,
          // Collect all docs for min/max lookup
          allDocs: { $push: { value: '$value', date: '$timestamp' } },
        },
      },
      {
        $project: {
          _id: 1,
          averageValue: 1,
          variance: 1,
          datapoints: 1,
          minValue: 1,
          maxValue: 1,
          median: 1,
          // Filter to only keep min/max docs
          minMaxDocs: {
            $filter: {
              input: '$allDocs',
              as: 'doc',
              cond: {
                $or: [
                  { $eq: ['$$doc.value', '$minValue'] },
                  { $eq: ['$$doc.value', '$maxValue'] },
                ],
              },
            },
          },
        },
      },
    ] as any); // Type assertion for MongoDB 5.2+ features

    if (logs.length === 0) return;

    const bulkOps = logs.map(
      (log: {
        _id: { type: string; controller: string; interval: string };
        averageValue: number;
        variance: number;
        datapoints: number;
        minValue: number;
        maxValue: number;
        median: number;
        minMaxDocs: Array<{ value: number; date: Date }>;
      }) => {
        const minEntry = log.minMaxDocs.find((v) => v.value === log.minValue);
        const maxEntry = log.minMaxDocs.find((v) => v.value === log.maxValue);

        return {
          insertOne: {
            document: {
              type: log._id.type,
              interval: interval,
              timestamp: new Date(log._id.interval),
              averageValue: log.averageValue,
              controller: log._id.controller,
              variance: log.variance,
              median: log.median,
              average: log.averageValue,
              datapoints: log.datapoints,
              min: { date: minEntry.date, value: log.minValue },
              max: { date: maxEntry.date, value: log.maxValue },
            },
          },
        };
      }
    );

    await SquashedSensorLog.bulkWrite(bulkOps, { ordered: false });
    console.log(
      `✅ Processed ${logs.length} missed ${interval} intervals successfully`
    );
  }

  /**
   * Fallback version - process specific missed intervals
   */
  private async processMissedIntervalsFallback(
    missedIntervals: Array<{
      type: string;
      controller: string;
      interval: string;
    }>,
    interval: string,
    dateFormat: string
  ) {
    // Create match conditions for only the missed intervals
    const matchConditions = missedIntervals.map((mi) => ({
      type: mi.type,
      controller: mi.controller,
      timestamp: {
        $gte: new Date(mi.interval),
        $lt: this.getNextInterval(new Date(mi.interval), interval),
      },
    }));

    const logs = await sensorLogModel.aggregate([
      {
        $match: {
          $or: matchConditions,
        },
      },
      {
        $sort: { value: 1 }, // Sort by value for percentile calculation
      },
      {
        $group: {
          _id: {
            type: '$type',
            controller: '$controller',
            interval: {
              $dateToString: { format: dateFormat, date: '$timestamp' },
            },
          },
          averageValue: { $avg: '$value' },
          variance: { $stdDevPop: '$value' },
          datapoints: { $sum: 1 },
          minValue: { $min: '$value' },
          maxValue: { $max: '$value' },
          // Collect sorted values for median (MongoDB will keep sort order)
          sortedValues: { $push: { value: '$value', date: '$timestamp' } },
          // Get first occurrence of min/max for dates
          minDoc: { $first: '$$ROOT' },
          maxDoc: { $last: '$$ROOT' },
        },
      },
      {
        $addFields: {
          // Calculate median index
          medianIndex: {
            $floor: { $divide: ['$datapoints', 2] },
          },
          isEvenCount: {
            $eq: [{ $mod: ['$datapoints', 2] }, 0],
          },
        },
      },
    ]);

    // Use bulkWrite for better performance
    if (logs.length === 0) return;

    const bulkOps = logs.map(
      (log: {
        _id: { type: string; controller: string; interval: string };
        averageValue: number;
        variance: number;
        datapoints: number;
        minValue: number;
        maxValue: number;
        medianIndex: number;
        isEvenCount: boolean;
        sortedValues: Array<{ value: number; date: Date }>;
      }) => {
        // Calculate median from sorted values
        const medianIdx = log.medianIndex;
        let median: number;

        if (log.isEvenCount && log.sortedValues.length > 1) {
          // Even number of values: average of two middle values
          median =
            (log.sortedValues[medianIdx - 1].value +
              log.sortedValues[medianIdx].value) /
            2;
        } else {
          // Odd number of values: middle value
          median = log.sortedValues[medianIdx].value;
        }

        // Find min/max entries efficiently
        const minEntry = log.sortedValues.find((v) => v.value === log.minValue);
        const maxEntry = log.sortedValues.find((v) => v.value === log.maxValue);

        return {
          insertOne: {
            document: {
              type: log._id.type,
              interval: interval,
              timestamp: new Date(log._id.interval),
              averageValue: log.averageValue,
              controller: log._id.controller,
              variance: log.variance,
              median: median,
              average: log.averageValue,
              datapoints: log.datapoints,
              min: { date: minEntry.date, value: log.minValue },
              max: { date: maxEntry.date, value: log.maxValue },
            },
          },
        };
      }
    );

    await SquashedSensorLog.bulkWrite(bulkOps, { ordered: false });
    console.log(
      `✅ Processed ${logs.length} missed ${interval} intervals successfully`
    );
  }
  private async squashForward(interval: Interval) {
    const last = await (SquashedSensorLog as any)
      .findOne({ interval })
      .sort({ timestamp: -1 })
      .select({ timestamp: 1 });

    const lastTime = last ? last.timestamp : new Date(0);
    const endBound = floorUTC(new Date(), interval); // exclude current open bucket
    if (lastTime >= endBound) {
      console.log(`[squash:${interval}] up-to-date`);
      return;
    }

    const pipeline: any[] = [
      { $match: { timestamp: { $gt: lastTime, $lt: endBound } } }, // strict > to avoid redoing last bucket
      { $addFields: { bucket: bucketExpr(interval) } },
      {
        $group: {
          _id: { type: '$type', controller: '$controller', bucket: '$bucket' },
          averageValue: { $avg: '$value' },
          variance: { $stdDevPop: '$value' },
          datapoints: { $sum: 1 },
          values: { $push: { value: '$value', date: '$timestamp' } },
        },
      },
    ];

    const logs = await (sensorLogModel as any)
      .aggregate(pipeline)
      .allowDiskUse(true);
    if (!logs.length) return;

    const ops = logs.map((log: any) => {
      const values = log.values
        .map((v: any) => v.value)
        .sort((a: number, b: number) => a - b);
      const median =
        values.length % 2 === 0
          ? (values[values.length / 2 - 1] + values[values.length / 2]) / 2
          : values[Math.floor(values.length / 2)];
      const min = log.values.reduce(
        (m: any, v: any) => (v.value < m.value ? v : m),
        log.values[0]
      );
      const max = log.values.reduce(
        (m: any, v: any) => (v.value > m.value ? v : m),
        log.values[0]
      );

      const filter = {
        interval,
        type: log._id.type,
        controller: log._id.controller,
        timestamp: log._id.bucket,
      };
      const update = {
        $set: {
          averageValue: log.averageValue,
          average: log.averageValue,
          variance: log.variance,
          median,
          datapoints: log.datapoints,
          min: { date: min.date, value: min.value },
          max: { date: max.date, value: max.value },
        },
      };
      return { updateOne: { filter, update, upsert: true } };
    });

    const res = await (SquashedSensorLog as any).bulkWrite(ops, {
      ordered: false,
    });
    console.log(
      `[squash:${interval}] upserted=${res.upsertedCount ?? 0} modified=${
        res.modifiedCount ?? 0
      }`
    );
  }
  private setupCronJobs() {
    cron.schedule('* * * * *', async () =>
      withLock('cron:squash:minute', () => this.squashForward('minute'))
    );
    cron.schedule('0 * * * *', async () =>
      withLock('cron:squash:hour', () => this.squashForward('hour'))
    );
    cron.schedule('0 0 * * *', async () =>
      withLock('cron:squash:day', () => this.squashForward('day'))
    );
    // Monday 00:00 UTC (ISO week)
    cron.schedule('0 0 * * 1', async () =>
      withLock('cron:squash:week', async () => {
        await this.squashForward('week');
        // optional retention of raw logs
        const weekAgo = new Date();
        weekAgo.setUTCDate(weekAgo.getUTCDate() - 7);
        await (sensorLogModel as any).deleteMany({
          timestamp: { $lt: weekAgo },
        });
      })
    );
  }
}
// utils/bucket.ts
import { Expression } from 'mongoose'; // type helper if you want

export type Interval = 'minute' | 'hour' | 'day' | 'week';

/** Returns a MongoDB aggregation expression that yields the bucket start Date (UTC). */
export function bucketExpr(interval: Interval): any {
  if (interval === 'minute') {
    return {
      $dateFromParts: {
        year: { $year: { date: '$timestamp', timezone: 'UTC' } },
        month: { $month: { date: '$timestamp', timezone: 'UTC' } },
        day: { $dayOfMonth: { date: '$timestamp', timezone: 'UTC' } },
        hour: { $hour: { date: '$timestamp', timezone: 'UTC' } },
        minute: { $minute: { date: '$timestamp', timezone: 'UTC' } },
        timezone: 'UTC',
      },
    };
  }

  if (interval === 'hour') {
    return {
      $dateFromParts: {
        year: { $year: { date: '$timestamp', timezone: 'UTC' } },
        month: { $month: { date: '$timestamp', timezone: 'UTC' } },
        day: { $dayOfMonth: { date: '$timestamp', timezone: 'UTC' } },
        hour: { $hour: { date: '$timestamp', timezone: 'UTC' } },
        minute: 0,
        second: 0,
        millisecond: 0,
        timezone: 'UTC',
      },
    };
  }

  if (interval === 'day') {
    return {
      $dateFromParts: {
        year: { $year: { date: '$timestamp', timezone: 'UTC' } },
        month: { $month: { date: '$timestamp', timezone: 'UTC' } },
        day: { $dayOfMonth: { date: '$timestamp', timezone: 'UTC' } },
        hour: 0,
        minute: 0,
        second: 0,
        millisecond: 0,
        timezone: 'UTC',
      },
    };
  }

  // ISO week start (Monday 00:00:00.000)
  return {
    $dateFromParts: {
      isoWeekYear: { $isoWeekYear: { date: '$timestamp', timezone: 'UTC' } },
      isoWeek: { $isoWeek: { date: '$timestamp', timezone: 'UTC' } },
      isoDayOfWeek: 1, // Monday
      hour: 0,
      minute: 0,
      second: 0,
      millisecond: 0,
      timezone: 'UTC',
    },
  };
}

export function floorUTC(date: Date, interval: Interval): Date {
  const d = new Date(date);
  if (interval === 'minute') {
    d.setUTCSeconds(0, 0);
  } else if (interval === 'hour') {
    d.setUTCMinutes(0, 0, 0);
  } else if (interval === 'day') {
    d.setUTCHours(0, 0, 0, 0);
  } else if (interval === 'week') {
    // floor to Monday 00:00 UTC
    const day = d.getUTCDay() || 7; // Sun=0→7
    d.setUTCDate(d.getUTCDate() - (day - 1));
    d.setUTCHours(0, 0, 0, 0);
  }
  return d;
}
