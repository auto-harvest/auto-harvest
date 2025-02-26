import mongoose, { Schema, Document } from 'mongoose';
export interface IController extends Document {
  code: string;
  waterLevel: 'high' | 'adequate' | 'low' | 'n/a';
  systemStatus: 'on' | 'off';
  cropType: 'Basil' | 'Chilly' | 'Strawberry' | 'Lettuce' | 'Greens';
  location: string;
  lastMaintenanceDate?: Date;
  capacity: number;
}

const ControllerSchema: Schema = new Schema(
  {
    code: { type: String, required: true },
    waterLevel: {
      type: String,
      enum: ['high', 'adequate', 'low', 'n/a'],
      default: 'n/a',
      required: true,
    },
    systemStatus: {
      type: String,
      enum: ['on', 'off'],
      default: 'on',
      required: true,
    },
    cropType: {
      type: String,
      enum: ['Basil', 'Chilly', 'Strawberry', 'Lettuce', 'Greens'],
      default: 'Greens',
      required: true,
    },
    location: { type: String, required: true }, 
    capacity: { type: Number, required: true },
    lastMaintenanceDate: { type: Date, default: Date.now },
  },
  { timestamps: true }
);
//add index to code field
ControllerSchema.index({ code: 1 }, { unique: true });
export default mongoose.model<IController>('Controller', ControllerSchema);
 