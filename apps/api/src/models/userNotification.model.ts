import mongoose, { Schema } from 'mongoose';

export interface IUserNotification extends Document {
  type: 'alert' | 'status' | 'general';
  message: string;
  seen: boolean;
  priority: 'high' | 'medium' | 'low';
  expiryDate?: Date;
  user: mongoose.Types.ObjectId;
  controller?: mongoose.Types.ObjectId;
}

const UserNotificationSchema: Schema = new Schema(
  {
    type: {
      type: String,
      enum: ['alert', 'status', 'general'],
      required: true,
    },
    message: { type: String, required: true },
    seen: { type: Boolean, default: false },
    priority: { type: String, enum: ['high', 'medium', 'low'], default: 'low' },
    expiryDate: { type: Date },
    user: { type: mongoose.Schema.Types.ObjectId, ref: 'User', required: true },
    controller: { type: mongoose.Schema.Types.ObjectId, ref: 'Controller' },
  },
  { timestamps: true }
);

export default mongoose.model<IUserNotification>(
  'UserNotification',
  UserNotificationSchema
);
