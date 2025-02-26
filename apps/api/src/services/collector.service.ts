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
    const lastLog = await sensorLogModel
      .findOne({ type: log.type, controller: log.controller })
      .sort({ timestamp: -1 });

    const newLog = new sensorLogModel(log);
    await newLog.save();
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
