import controllerModel from '../models/controller.model';
import SensorLog, { ISensorLog } from '../models/sensorLog.model';
import SquashedSensorLog from '../models/squashedSensorLog.model';
import userModel from '../models/user.model';
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
    endDate: Date,
    interval: 'hour' | 'day' | 'week',
    userId: string
  ) {
    try {
      const user = await userModel
        .findById(userId)
        .populate('controllers')
        .exec();
      const data = await SquashedSensorLog.find({
        type,
        timestamp: { $gte: startDate, $lte: endDate },
        interval,
        controller: { $in: user?.controllers.map((v: any) => v.code) || [] },
      }).sort({ timestamp: 1 }); // Sort by ascending timestamp
      console.log(
        user,
        {
          type,
          timestamp: { $gte: startDate, $lte: endDate },
          interval,
          controller: { $in: user?.controllers || [] },
        },
        data
      );
      return data;
    } catch (error) {
      console.error('Error while fetching sensor logs:', error);
      throw new Error('Failed to fetch sensor logs');
    }
  }
  static async getSquashedByTypeAndRange(params: {
    type: string;
    controller: string;
    interval: 'hour' | 'day' | 'week';
    start: Date; // inclusive
    endExclusive: Date; // exclusive
  }) {
    const { type, controller: id, interval, start, endExclusive } = params;
    const controller = await controllerModel.findById(id);
    // Query pre-squashed collection 
    const docs = await (SquashedSensorLog as any)
      .find(
        {
          interval,
          type,
          timestamp: { $gte: start, $lt: endExclusive },
        },
        {
          _id: 0,
          interval: 1,
          type: 1,
          controller: 1,
          timestamp: 1,
          averageValue: 1,
          average: 1,
          variance: 1,
          median: 1,
          datapoints: 1,
          min: 1,
          max: 1, 
        }
      )
      .sort({ timestamp: 1 })
      .lean();

    return docs;
  }
}
