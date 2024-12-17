import mongoose, { Schema, Document } from 'mongoose';
export interface IController extends Document {
  code: string;
  waterLevel: 'high' | 'adequate' | 'low';
  systemStatus: 'on' | 'off';
  cropType: 'Basil' | 'Chilly' | 'Strawberry' | 'Lettuce';
  location: string;
  lastMaintenanceDate?: Date;
  connectedSensors: mongoose.Types.ObjectId[];
}

const ControllerSchema: Schema = new Schema(
  {
    code: { type: String, required: true },
    waterLevel: {
      type: String,
      enum: ['high', 'adequate', 'low'],
      required: true,
    },
    systemStatus: { type: String, enum: ['on', 'off'], required: true },
    cropType: {
      type: String,
      enum: ['Basil', 'Chilly', 'Strawberry', 'Lettuce'],
      required: true,
    },
    location: { type: String, required: true },
    lastMaintenanceDate: { type: Date },
    connectedSensors: [{ type: mongoose.Schema.Types.ObjectId, ref: 'Sensor' }],
  },
  { timestamps: true }
);

export default mongoose.model<IController>('Controller', ControllerSchema);
