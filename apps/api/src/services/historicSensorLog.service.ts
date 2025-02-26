// apps/api/src/services/historicSensorLog.service.ts

import HistoricSensorLog, {
  IHistoricSensorLog,
} from '../models/historicSensorLog.model';
import SensorLog, { ISensorLog, ValueType } from '../models/sensorLog.model';

type Granularity = 'm' | 'h' | 'd' | 'w';

type AggregatableLog = {
  timestamp: Date;
  value: number;
  type: ValueType;
};
const collectedTypes = [
  'temperature',
  'humidity',
  'water-temperature',
  'ph',
  'tds',
];
export default class HistoricSensorLogService {
  private static lastScanTime: Date = new Date(0); // Initialize to epoch time

  static async collectAndAggregateLogs() {
    await HistoricSensorLog.deleteMany({}); // Clear existing logs

    // Step 1: Fetch sensor logs and group by type
    const rawLogs = await SensorLog.find({
      type: { $in: collectedTypes },
    }).sort({ timestamp: 1 });
    if (rawLogs.length === 0) {
      console.log('No new logs to process.');
      return;
    }

    // Step 2: Group logs by type 
    const logsByType = rawLogs.reduce((acc, log) => {
      if (!acc[log.type]) acc[log.type] = [];
      acc[log.type].push(log);
      return acc;
    }, {} as Record<ValueType, ISensorLog[]>);

    // Step 3: Aggregate logs for each type
    for (const type in logsByType) {
      const logs = logsByType[type as ValueType];
      const minuteLogs = await this.aggregateAndSave(logs, 'm');
      const hourLogs = await this.aggregateAndSave(minuteLogs, 'h');
      const dayLogs = await this.aggregateAndSave(hourLogs, 'd');
      await this.aggregateAndSave(dayLogs, 'w');
    }
  }

  /**
   * Aggregate and save logs at a given granularity.
   *
   * @param logs - Logs to aggregate (either raw or historic).
   * @param granularity - Target granularity ('m', 'h', 'd', 'w').
   * @returns Aggregated logs.
   */
  private static async aggregateAndSave(
    logs: ISensorLog[] | IHistoricSensorLog[],
    granularity: Granularity
  ): Promise<IHistoricSensorLog[]> {
    const preparedLogs = logs.map(this.normalizeLog);
    const groupedLogs = this.groupLogsByAggregationKey(
      preparedLogs,
      granularity
    );
    const aggregatedLogs = this.calculateAggregatedStatistics(
      groupedLogs,
      granularity
    );

    // Save aggregated logs
    await HistoricSensorLog.insertMany(aggregatedLogs);

    return aggregatedLogs;
  }

  /**
   * Normalize logs to a consistent structure for aggregation.
   *
   * @param log - A sensor log or historic sensor log.
   * @returns A normalized log.
   */
  private static normalizeLog(
    log: ISensorLog | IHistoricSensorLog
  ): AggregatableLog {
    if (log instanceof SensorLog) {
      return { timestamp: log.timestamp, value: log.value, type: log.type };
    }
    const timestamp = new Date(
      log instanceof SensorLog
        ? log.timestamp
        : Object.keys((log as IHistoricSensorLog).data)[0]
    );
    return {
      timestamp,
      value:
        (log instanceof SensorLog && log.value) ||
        (log as IHistoricSensorLog).metadata.statistics.average,
      type:
        (log instanceof SensorLog && log.type) ||
        (log as IHistoricSensorLog).metadata.type,
    };
  }

  /**
   * Group logs by aggregation key based on granularity.
   *
   * @param logs - Logs to group.
   * @param granularity - Granularity for grouping ('m', 'h', 'd', 'w').
   * @returns Grouped logs.
   */
  private static groupLogsByAggregationKey(
    logs: AggregatableLog[],
    granularity: Granularity
  ): Record<string, AggregatableLog[]> {
    return logs.reduce((acc, log) => {
      const key = this.getAggregationKey(log.timestamp, granularity);

      if (!acc[key]) acc[key] = [];
      acc[key].push(log);

      return acc;
    }, {} as Record<string, AggregatableLog[]>);
  }

  /**
   * Calculate aggregated statistics for each group of logs.
   *
   * @param groupedLogs - Logs grouped by aggregation key.
   * @param granularity - Granularity of the aggregation ('m', 'h', 'd', 'w').
   * @returns Aggregated historic sensor logs.
   */
  private static calculateAggregatedStatistics(
    groupedLogs: Record<string, AggregatableLog[]>,
    granularity: Granularity
  ): IHistoricSensorLog[] {
    return Object.entries(groupedLogs).map(([key, logs]) => {
      const data: Record<string, number> = {};
      let sum = 0,
        min = Infinity,
        max = -Infinity;

      logs.forEach((log) => {
        const timestamp = log.timestamp.toISOString();
        const value = log.value;

        data[timestamp] = value;
        sum += value;
        if (value < min) min = value;
        if (value > max) max = value;
      });

      const values = Object.values(data);
      const average = sum / values.length;
      const variance =
        values.reduce((acc, value) => acc + Math.pow(value - average, 2), 0) /
        values.length;
      const median = this.calculateMedian(values);
      console.log(key);
      return {
        metadata: {
          granularity,
          type: logs[0].type, // All logs in the group share the same type
          key,
          statistics: {
            variance,
            median,
            average,
            datapoints: values.length,
            min: { date: new Date(key), value: min },
            max: { date: new Date(key), value: max },
          },
        },
        data,
      } as unknown as IHistoricSensorLog;
    });
  }

  /**
   * Calculate the median of an array of numbers.
   *
   * @param values - Array of numbers.
   * @returns The median value.
   */
  private static calculateMedian(values: number[]): number {
    values.sort((a, b) => a - b);
    const middle = Math.floor(values.length / 2);
    return values.length % 2 === 0
      ? (values[middle - 1] + values[middle]) / 2
      : values[middle];
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
    granularity: Granularity
  ): string {
    const date = new Date(timestamp);
    switch (granularity) {
      case 'm':
        return `${date.getUTCFullYear()}-${date.getUTCMonth()}-${date.getUTCDate()}/${date.getUTCHours()}:${date.getUTCMinutes()}`;
      case 'h':
        return `${date.getUTCFullYear()}-${date.getUTCMonth()}-${date.getUTCDate()}/${date.getUTCHours()}:00`;
      case 'd':
        return `${date.getUTCFullYear()}-${date.getUTCMonth()}-${date.getUTCDate()}`;
      case 'w': {
        const week = Math.floor(date.getUTCDate() / 7);
        return `${date.getUTCFullYear()}-${date.getUTCMonth()}-${date.getUTCDate()}`;
      }
      default:
        throw new Error('Invalid granularity');
    }
  }
}
