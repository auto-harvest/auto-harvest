import { Request, Response } from 'express';
import SensorLogService from '../services/sensorLog.service';

type Range = 'day' | 'week' | 'month' | 'year';
type Interval = 'hour' | 'day' | 'week';
export function createSensorLog() {}
function startEndUTC(
  startLike: string,
  range: Range
): { start: Date; endExclusive: Date; interval: Interval } {
  const d = new Date(startLike);
  if (Number.isNaN(d.getTime())) throw new Error('Invalid start date');

  // normalize in UTC
  const start = new Date(d);
  start.setUTCSeconds(0, 0);

  if (range === 'day') {
    // use hourly buckets over the selected day
    start.setUTCHours(0, 0, 0, 0);
    const endExclusive = new Date(start);
    endExclusive.setUTCDate(endExclusive.getUTCDate() + 1); // next day 00:00
    return { start, endExclusive, interval: 'hour' };
  }

  if (range === 'week') {
    // ISO week: Monday 00:00 UTC to next Monday 00:00 UTC
    const day = start.getUTCDay() || 7; // Sun=0 -> 7
    start.setUTCDate(start.getUTCDate() - (day - 1));
    start.setUTCHours(0, 0, 0, 0);
    const endExclusive = new Date(start);
    endExclusive.setUTCDate(endExclusive.getUTCDate() + 7);
    return { start, endExclusive, interval: 'day' };
  }

  if (range === 'month') {
    // first day 00:00 → first day of next month 00:00
    start.setUTCDate(1);
    start.setUTCHours(0, 0, 0, 0);
    const endExclusive = new Date(
      Date.UTC(start.getUTCFullYear(), start.getUTCMonth() + 1, 1, 0, 0, 0, 0)
    );
    return { start, endExclusive, interval: 'day' };
  }

  // year
  start.setUTCMonth(0, 1); // Jan 1
  start.setUTCHours(0, 0, 0, 0);
  const endExclusive = new Date(
    Date.UTC(start.getUTCFullYear() + 1, 0, 1, 0, 0, 0, 0)
  );
  return { start, endExclusive, interval: 'week' };
}

export const getSensorLogs = async (
  req: Request & { user?: any },
  res: Response
) => {
  try {
    const user = req.user ?? {};
    const type = (req.query.type as string)?.trim();
    const range = (req.query.range as Range) || 'day';
    const startParam = req.query.start as string;
    const controller =
      (req.query.controller as string) || user.controllerId || user.id; // adjust to your auth model

    if (!type || !startParam) {
      return res
        .status(400)
        .json({ message: 'Missing required query params: type and start' });
    }
    if (!controller) {
      return res
        .status(400)
        .json({
          message: 'Missing controller (query.controller or user context)',
        });
    }

    const { start, endExclusive, interval } = startEndUTC(startParam, range);
    console.log({
      type,
      controller,
      interval, // 'hour' | 'day' | 'week'
      start,
      endExclusive, // exclusive upper bound
   } )
    const logs = await SensorLogService.getSquashedByTypeAndRange({
      type,
      controller,
      interval, // 'hour' | 'day' | 'week'
      start,
      endExclusive, // exclusive upper bound
    });

    return res.status(200).json(logs);
  } catch (error: any) {
    console.error('Error in getSensorLogs controller:', error);
    return res.status(500).json({ message: 'Internal Server Error' });
  }
};
