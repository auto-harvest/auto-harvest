import { Request, Response } from 'express';
import SensorLogService from '../services/sensorLog.service';

/**
 * Controller for handling sensor log endpoints.
 */
export const createSensorLog = async (req: Request, res: Response) => {
  // try {
  //   const { type, value, timestamp } = req.body;

  //   if (!type || !value) {
  //     return res.status(400).json({ message: 'Type and value are required' });
  //   }

  //   const sensorLog = await SensorLogService.createSensorLog(
  //     type,
  //     value,
  //     timestamp
  //   );
  //   return res.status(201).json(sensorLog);
  // } catch (error) {
  //   console.error('Error in createSensorLog controller:', error);
  //   return res.status(500).json({ message: 'Internal Server Error' });
  // }
};

export const getSensorLogs = async (req: Request, res: Response) => {
  try {
    const { type, startDate, endDate } = req.query;

    if (!type || !startDate || !endDate) {
      return res
        .status(400)
        .json({ message: 'Type, startDate, and endDate are required' });
    }

    const logs = await SensorLogService.getSensorLogsByTypeAndDateRange(
      type as any,
      new Date(startDate as string),
      new Date(endDate as string)
    );

    return res.status(200).json(logs);
  } catch (error) {
    console.error('Error in getSensorLogs controller:', error);
    return res.status(500).json({ message: 'Internal Server Error' });
  }
};
