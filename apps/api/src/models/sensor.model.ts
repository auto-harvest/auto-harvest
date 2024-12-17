import mongoose, { Schema } from 'mongoose';

export interface ISensor extends Document {
  name: string;
  type: string;
  description: string;
  code: string;
  unit: string;
  lastCalibrated?: Date;
  isCritical: boolean;
}

const SensorSchema: Schema = new Schema(
  {
    name: { type: String, required: true },
    type: { type: String, required: true },
    description: { type: String },
    code: { type: String, required: true },
    unit: { type: String, required: true },
    lastCalibrated: { type: Date },
    isCritical: { type: Boolean, default: false },
  },
  { timestamps: true }
);

export default mongoose.model<ISensor>('Sensor', SensorSchema);
