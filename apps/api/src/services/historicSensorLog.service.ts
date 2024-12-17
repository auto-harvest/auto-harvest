// apps/api/src/services/historicSensorLog.service.ts

import HistoricSensorLog, {
  IHistoricSensorLog,
} from '../models/historicSensorLog.model';
import SensorLog, { ISensorLog, ValueType } from '../models/sensorLog.model';

export default class HistoricSensorLogService {
  private static lastScanTime: Date = new Date(0); // Initialize to epoch time

  /**
   * Collect sensor logs inserted after the last scan and create historic sensor log entries.
   */
  static async collectAndAggregateLogs() {
    await HistoricSensorLog.deleteMany({}).then(() => {});
    const newLogs = await SensorLog.find({ type: 'ph' }).sort({ timestamp: 1 });

    if (newLogs.length === 0) {
      console.log('No new logs to process.');
      return;
    }

    // Update the last scan time to the timestamp of the latest log
    this.lastScanTime = newLogs[newLogs.length - 1].timestamp;
    //group by type
    const logsByType = newLogs.reduce((acc, log) => {
      if (!acc[log.type]) {
        acc[log.type] = [];
      }
      acc[log.type].push(log);
      return acc;
    }, {} as { [key in ValueType]: ISensorLog[] });
    // Aggregate logs by minute, hour, day, and week
    for (const type in logsByType) {
      await this.aggregateLogs(logsByType[type], 'm');
    }
    const minuteLogs = await this.fetchAggregatedLogs('m');

    for (const type in minuteLogs) {
      await this.aggregateLogs(minuteLogs[type], 'h');
    }
    const hourLogs = await this.fetchAggregatedLogs('h');
    for (const type in hourLogs) {
      await this.aggregateLogs(hourLogs[type], 'd');
    }
    const dayLogs = await this.fetchAggregatedLogs('d');

    for (const type in dayLogs) {
      await this.aggregateLogs(dayLogs[type], 'w');
    }
  }

  /**
   * Fetch aggregated logs based on the specified granularity.
   *
   * @param granularity - The granularity of the aggregation ('m', 'h', 'd', 'w').
   * @returns The aggregated logs.
   */
  private static async fetchAggregatedLogs(
    granularity: 'm' | 'h' | 'd' | 'w'
  ): Promise<Record<string, IHistoricSensorLog[]>> {
    const newLogs = await HistoricSensorLog.find({
      'metadata.granularity': granularity,
      'metadata.type': 'ph',
    });
    console.log(newLogs);
    if (newLogs.length === 0) {
      console.log('No new logs to process.');
      return;
    }

    // Update the last scan time to the timestamp of the latest log
    this.lastScanTime = newLogs[newLogs.length - 1].createdAt;
    //group by type
    const logsByType = newLogs.reduce((acc, log) => {
      if (!acc[log.metadata.type]) {
        acc[log.metadata.type] = [];
      }
      acc[log.metadata.type].push(log);
      return acc;
    }, {} as { [key in ValueType]: IHistoricSensorLog[] });
    return logsByType;
  }

  /**
   * Aggregate logs based on the specified granularity.
   *
   * @param logs - The sensor logs to aggregate.
   * @param granularity - The granularity of the aggregation ('m', 'h', 'd', 'w').
   */
  private static async aggregateLogs(
    logs: ISensorLog[] | IHistoricSensorLog[],
    granularity: 'm' | 'h' | 'd' | 'w'
  ) {
    const aggregatedLogs: { [key: string]: IHistoricSensorLog } = {};

    logs.forEach((log: ISensorLog | IHistoricSensorLog) => {
      let timestamp, value, type;
      if (log instanceof SensorLog) {
        timestamp = log.timestamp;
        value = { [timestamp]: log.value };
        type = log.type;
      }
      if (log instanceof HistoricSensorLog) {
        timestamp = Object.keys(log.data)[0];
        value = { [timestamp]: log.metadata.statistics.average };
        type = log.metadata.type;
      }
      const key = this.getAggregationKey(timestamp, granularity);
      if (!aggregatedLogs[key]) {
        aggregatedLogs[key] = {
          metadata: {
            granularity,
            type: type,
            key,
            statistics: {
              variance: 0,
              median: 0,
              average: 0,
              datapoints: 0,
              min: { date: timestamp, value: Infinity },
              max: { date: timestamp, value: -Infinity },
            },
          },
          data: {},
        } as IHistoricSensorLog;
      }

      const aggregatedLog = aggregatedLogs[key];
      for (const timestamp in value) {
        aggregatedLog.data[timestamp] = value[timestamp];
        aggregatedLog.metadata.statistics.datapoints += 1;
        aggregatedLog.metadata.statistics.average += value[timestamp];
        if (value[timestamp] < aggregatedLog.metadata.statistics.min.value) {
          aggregatedLog.metadata.statistics.min = {
            date: new Date(timestamp),
            value: value[timestamp],
          };
        }
        if (value[timestamp] > aggregatedLog.metadata.statistics.max.value) {
          aggregatedLog.metadata.statistics.max = {
            date: new Date(timestamp),
            value: value[timestamp],
          };
        }
      }
    });

    // Calculate final statistics
    for (const key in aggregatedLogs) {
      const aggregatedLog = aggregatedLogs[key];
      const values = Object.values(aggregatedLog.data).map(Number);

      // Calculate average
      aggregatedLog.metadata.statistics.average /=
        aggregatedLog.metadata.statistics.datapoints;

      // Calculate variance
      const mean = aggregatedLog.metadata.statistics.average;
      const variance =
        values.reduce((acc, value) => acc + Math.pow(value - mean, 2), 0) /
        values.length;
      aggregatedLog.metadata.statistics.variance = variance;

      // Calculate median
      values.sort((a, b) => a - b);
      const middle = Math.floor(values.length / 2);
      aggregatedLog.metadata.statistics.median =
        values.length % 2 === 0
          ? (values[middle - 1] + values[middle]) / 2
          : values[middle];

      // Save the aggregated log
      await HistoricSensorLog.create(aggregatedLog);
    }
  }

  /**
   * Get the aggregation key based on the timestamp and granularity.
   *
   * @param timestamp - The timestamp of the log.
   * @param granularity - The granularity of the aggregation ('m', 'h', 'd', 'w').
   * @returns The aggregation key.
   */
  private static getAggregationKey(
    timestamp: Date,
    granularity: 'm' | 'h' | 'd' | 'w'
  ): string {
    const date = new Date(timestamp);
    switch (granularity) {
      case 'm':
        return `${date.getUTCFullYear()}-${date.getUTCMonth()}-${date.getUTCDate()}-${date.getUTCHours()}-${date.getUTCMinutes()}`;
      case 'h':
        return `${date.getUTCFullYear()}-${date.getUTCMonth()}-${date.getUTCDate()}-${date.getUTCHours()}`;
      case 'd':
        return `${date.getUTCFullYear()}-${date.getUTCMonth()}-${date.getUTCDate()}`;
      case 'w': {
        const week = Math.floor(date.getUTCDate() / 7);
        return `${date.getUTCFullYear()}-${date.getUTCMonth()}-W${week}`;
      }
      default:
        throw new Error('Invalid granularity');
    }
  }
}
