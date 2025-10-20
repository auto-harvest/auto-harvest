import mongoose from 'mongoose';
import cron from 'node-cron';
import sensorLogModel, { ISensorLog } from '../models/sensorLog.model';
import SquashedSensorLog from '../models/squashedSensorLog.model';

class CollectorService {
  constructor() {
    this.setupCronJobs();
  }

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
      this.updateSquashedLog(log.type, log.value, log.controller, timestamp, 'minute'),
      this.updateSquashedLog(log.type, log.value, log.controller, timestamp, 'hour'),
      this.updateSquashedLog(log.type, log.value, log.controller, timestamp, 'day'),
      this.updateSquashedLog(log.type, log.value, log.controller, timestamp, 'week'),
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

        // Approximate median update (weighted average approach)
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
   * Get the timestamp for the start of an interval (in UTC)
   */
  private getIntervalTimestamp(
    timestamp: Date,
    interval: 'minute' | 'hour' | 'day' | 'week'
  ): Date {
    const date = new Date(timestamp);

    switch (interval) {
      case 'minute':
        date.setUTCSeconds(0, 0);
        break;
      case 'hour':
        date.setUTCMinutes(0, 0, 0);
        break;
      case 'day':
        date.setUTCHours(0, 0, 0, 0);
        break;
      case 'week':
        // ISO week: floor to Monday 00:00 UTC
        // eslint-disable-next-line no-case-declarations
        const day = date.getUTCDay() || 7; // Sunday=0 -> 7
        date.setUTCDate(date.getUTCDate() - (day - 1)); // Go back to Monday
        date.setUTCHours(0, 0, 0, 0);
        break;
    }

    return date;
  }

  private async groupLogsByInterval(interval: string, dateFormat: string) {
    const lastSquashedLog = await SquashedSensorLog.findOne({ interval }).sort({
      timestamp: -1,
    });
    const lastTime = lastSquashedLog ? lastSquashedLog.timestamp : new Date(0);
    const now = new Date();

    const logs = await sensorLogModel.aggregate([
      {
        $match: {
          timestamp: { $gte: lastTime, $lt: now },
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
          averageValue: { $avg: '$value' },
          values: { $push: { value: '$value', date: '$timestamp' } },
          variance: { $stdDevPop: '$value' },
          datapoints: { $sum: 1 },
        },
      },
    ]);

    for (const log of logs) {
      const values = log.values
        .map((v: any) => v.value)
        .sort((a: number, b: number) => a - b);
      const median =
        values.length % 2 === 0
          ? (values[values.length / 2 - 1] + values[values.length / 2]) / 2
          : values[Math.floor(values.length / 2)];
      const min = log.values.reduce(
        (min: any, v: any) => (v.value < min.value ? v : min),
        log.values[0]
      );
      const max = log.values.reduce(
        (max: any, v: any) => (v.value > max.value ? v : max),
        log.values[0]
      );

      const squashedLog = new SquashedSensorLog({
        type: log._id.type,
        interval: interval,
        timestamp: new Date(log._id.interval),
        averageValue: log.averageValue,
        controller: log._id.controller,
        variance: log.variance,
        median: median,
        average: log.averageValue,
        datapoints: log.datapoints,
        min: { date: min.date, value: min.value },
        max: { date: max.date, value: max.value },
      });

      await squashedLog.save();
    }
  }

  private setupCronJobs() {
    this.setupCronJob('minute', '* * * * *', '%Y-%m-%dT%H:%M:00Z');
    this.setupCronJob('hour', '0 * * * *', '%Y-%m-%dT%H:00:00Z');
    this.setupCronJob('day', '0 0 * * *', '%Y-%m-%dT00:00:00Z');
    this.setupCronJob('week', '0 0 * * 0', '%Y-%m-%dT00:00:00Z'); // Adjust for week start if necessary
  }

  private setupCronJob(
    interval: string,
    cronExpression: string,
    dateFormat: string
  ) {
    cron.schedule(cronExpression, async () => {
      console.log(`Running ${interval} log grouping`);
      await this.groupLogsByInterval(interval, dateFormat);
      if (interval === 'week') {
        //delete all logs older than 1 week
        const weekAgo = new Date();
        weekAgo.setDate(weekAgo.getDate() - 7);
        await sensorLogModel.deleteMany({ timestamp: { $lt: weekAgo } });
      }
    });
  }
}

export default CollectorService;
