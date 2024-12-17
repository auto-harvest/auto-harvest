import SensorLog, { ISensorLog } from '../models/sensorLog.model';

/**
 * Service for managing sensor logs.
 */
export default class SensorLogService {
  /**
   * Create a new sensor log entry.
   *
   * @param type - The type of the sensor data (e.g., pH, temp).
   * @param value - The value of the sensor reading.
   * @param timestamp - The timestamp of the reading (optional, defaults to current time).
   * @returns The created SensorLog document.
   */
  static async createSensorLog(
    type: ISensorLog['type'],
    value: number,
    timestamp: Date,
    controller: string
  ): Promise<ISensorLog> {
    try {
      const sensorLog = new SensorLog({
        type,
        value,
        timestamp: timestamp || new Date(),
        controller,
      });

      return await sensorLog.save();
    } catch (error) {
      console.error('Error while saving sensor log:', error);
      throw new Error('Failed to save sensor log');
    }
  }

  /**
   * Fetch sensor logs for a specific type and time range.
   *
   * @param type - The type of sensor data (e.g., pH, temp).
   * @param startDate - The start of the time range (inclusive).
   * @param endDate - The end of the time range (inclusive).
   * @returns An array of matching sensor logs.
   */
  static async getSensorLogsByTypeAndDateRange(
    type: ISensorLog['type'],
    startDate: Date,
    endDate: Date
  ): Promise<ISensorLog[]> {
    try {
      return await SensorLog.find({
        type,
        timestamp: { $gte: startDate, $lte: endDate },
      }).sort({ timestamp: 1 }); // Sort by ascending timestamp
    } catch (error) {
      console.error('Error while fetching sensor logs:', error);
      throw new Error('Failed to fetch sensor logs');
    }
  }
}
