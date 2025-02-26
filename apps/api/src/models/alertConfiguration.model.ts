import mongoose, { Schema } from 'mongoose';

export interface IAlertConfiguration extends Document {
  name: string;
  controllerSensor: mongoose.Types.ObjectId;
  actionsKey: string;
  min: number;
  max: number;
  notificationType: 'email' | 'SMS' | 'push';
  isActive: boolean;
}

const AlertConfigurationSchema: Schema = new Schema(
  {
    name: { type: String, required: true },
    controllerSensor: {
      type: mongoose.Schema.Types.ObjectId,
      ref: 'ControllerSensor',
      required: true, 
    },
    actionsKey: { type: String, required: true },
    min: { type: Number, required: true },
    max: { type: Number, required: true },
    notificationType: {
      type: String,
      enum: ['email', 'SMS', 'push'],
      required: true,
    },
    isActive: { type: Boolean, default: true },
  },
  { timestamps: true }
);

export default mongoose.model<IAlertConfiguration>(
  'AlertConfiguration',
  AlertConfigurationSchema
);
