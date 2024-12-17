import mongoose, { Schema } from 'mongoose';
import { ValueType } from './sensorLog.model';
import BaseEntitySchema, { IBaseEntity } from './baseEntity.model';
//Pure data model

export interface IHistoricSensorLog extends IBaseEntity {
  metadata: {
    granularity: 'm' | 'h' | 'd' | 'w';
    type: ValueType;
    key: string;
    statistics: {
      variance: number;
      median: number;
      average: number;
      datapoints: number;
      min: { date: Date; value: number };
      max: { date: Date; value: number };
    };
  };
  data: Record<string, string>; // e.g., timestamp -> value
}

const HistoricSensorLogSchema: Schema = new Schema(
  {
    metadata: {
      granularity: { type: String, enum: ['m', 'h', 'd', 'w'], required: true },
      type: { type: String, required: true },
      key: { type: String, required: true },
      statistics: {
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
    },
    data: { type: Object, of: String }, // Map of timestamps to values
  },
  { timestamps: true }
)

export default mongoose.model<IHistoricSensorLog>(
  'HistoricSensorLog',
  HistoricSensorLogSchema
);
