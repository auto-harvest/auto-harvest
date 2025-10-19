import mongoose, { Schema, Document } from 'mongoose';
import { ValueType } from './sensorLog.model';

export interface ISquashedSensorLog extends Document {
  type: ValueType;
  interval: string;
  timestamp: Date;
  averageValue: number;
  controller: string;
  variance: number;
  median: number;
  average: number;
  datapoints: number;
  min: { date: Date; value: number };
  max: { date: Date; value: number };
}

export const SquashedSensorLogSchema: Schema = new Schema(
  {
    type: { type: String, required: true },
    interval: { type: String, required: true },
    timestamp: { type: Date, required: true },
    averageValue: { type: Number, required: true },
    controller: { type: String, required: true },
    variance: { type: Number, required: true },
    median: { type: Number, required: true },
    average: { type: Number, required: true },
    datapoints: { type: Number, required: true },
    min: {
      date: { type: Date, required: true },
      value: { type: Number, required: true },
    },
    max: {
      date: { type: Date, required: true },
      value: { type: Number, required: true },
    },
  },
  { timestamps: true }
);

// Index for finding last squashed log by interval
SquashedSensorLogSchema.index({ interval: 1, timestamp: -1 });
// Compound index for querying by type, controller, and interval
SquashedSensorLogSchema.index({ type: 1, controller: 1, interval: 1, timestamp: -1 });
// Unique index to prevent duplicate squashed logs for the same time period
SquashedSensorLogSchema.index(
  { type: 1, controller: 1, interval: 1, timestamp: 1 },
  { unique: true }
);

const SquashedSensorLog = mongoose.model<ISquashedSensorLog>(
  'SquashedSensorLog',
  SquashedSensorLogSchema
);
SquashedSensorLogSchema.index(
  { interval: 1, controller: 1, type: 1, timestamp: 1 },
  { unique: true, name: 'uniq_interval_controller_type_bucket' }
);
export default SquashedSensorLog;
