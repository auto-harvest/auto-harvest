import mongoose, { Schema, Document } from 'mongoose';
export enum ValueType {
  PH = 'pH',
  EC = 'ec',
  TEMP = 'temp',
  HUM = 'hum',
  FLOW = 'flow',
}
export interface ISensorLog extends Document {
  type: ValueType; // The type of data being logged (e.g., pH, temperature, etc.)
  timestamp: Date; // The time of the reading
  value: number; // The actual sensor reading
  hour: Date;
  controller: string;
}

const SensorLogSchema: Schema = new Schema(
  {
    type: { type: String, required: true }, // e.g., 'pH', 'temp', 'hum'
    timestamp: { type: Date, default: Date.now }, // Automatically sets the current time if not provided
    value: { type: Number, required: true }, // The reading value
    controller: { type: String, required: true },
    hour: {
      type: Date,
      default: function (this: ISensorLog) {
        return new Date(this.timestamp).setMinutes(0, 0, 0);
      },
    },
  },
  { timestamps: true }
);

export default mongoose.model<ISensorLog>('SensorLog', SensorLogSchema);
