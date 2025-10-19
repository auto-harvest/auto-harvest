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
    await mongoose.connection.collection('locks').deleteOne({ _id: key as any });
  }
}

export default class CollectorService {
  constructor() {
    this.setupCronJobs();
  }

  public async storeSensorLog(log: {
    type: string;
    value: number;
    controller: string;
  }) {
    await new (sensorLogModel as any)(log).save();
  }

  /** Forward squash from last completed bucket to now (exclusive). */
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

  /** Backfill historical buckets (MongoDB v4-safe). */
  public async backfill(
    interval: Interval,
    opts?: { from?: Date; to?: Date; chunkDays?: number }
  ) {
    const minDoc = await (sensorLogModel as any)
      .findOne()
      .sort({ timestamp: 1 })
      .select({ timestamp: 1 });
    const maxDoc = await (sensorLogModel as any)
      .findOne()
      .sort({ timestamp: -1 })
      .select({ timestamp: 1 });
    if (!minDoc || !maxDoc) {
      console.log(`[backfill:${interval}] no raw logs`);
      return;
    }

    const globalFrom = opts?.from
      ? new Date(opts.from)
      : floorUTC(minDoc.timestamp, interval);
    const liveEnd = floorUTC(new Date(), interval);
    const globalTo = opts?.to ? new Date(opts.to) : liveEnd;
    if (globalFrom >= globalTo) return;

    const chunkDays =
      opts?.chunkDays ??
      (interval === 'minute' ? 2 : interval === 'hour' ? 14 : 90);
    let sliceStart = new Date(globalFrom);

    while (sliceStart < globalTo) {
      const sliceEnd = new Date(sliceStart);
      sliceEnd.setUTCDate(sliceEnd.getUTCDate() + chunkDays);
      if (sliceEnd > globalTo) sliceEnd.setTime(globalTo.getTime());

      const pipeline: any[] = [
        { $match: { timestamp: { $gte: sliceStart, $lt: sliceEnd } } },
        { $addFields: { bucket: bucketExpr(interval) } },
        {
          $group: {
            _id: {
              type: '$type',
              controller: '$controller',
              bucket: '$bucket',
            },
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

      if (logs.length) {
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

          return {
            updateOne: {
              filter: {
                interval,
                type: log._id.type,
                controller: log._id.controller,
                timestamp: log._id.bucket,
              },
              update: {
                $set: {
                  averageValue: log.averageValue,
                  average: log.averageValue,
                  variance: log.variance,
                  median,
                  datapoints: log.datapoints,
                  min: { date: min.date, value: min.value },
                  max: { date: max.date, value: max.value },
                },
              },
              upsert: true,
            },
          };
        });

        try {
          const res = await (SquashedSensorLog as any).bulkWrite(ops, {
            ordered: false,
          });
          console.log(
            `[backfill:${interval}] upserted=${
              res.upsertedCount ?? 0
            } modified=${res.modifiedCount ?? 0}`
          );
        } catch (e: any) {
          if (e?.writeErrors?.some((we: any) => we.code === 11000)) {
            console.warn(
              `[backfill:${interval}] duplicate keys encountered (ok with unique index).`
            );
          } else {
            throw e;
          }
        }
      }

      sliceStart = sliceEnd;
    }
    console.log(`[backfill:${interval}] done.`);
  }

  public async backfillAll() {
    await withLock('backfill:all', async () => {
      await this.backfill('minute');
      await this.backfill('hour');
      await this.backfill('day');
      await this.backfill('week');
    });
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
