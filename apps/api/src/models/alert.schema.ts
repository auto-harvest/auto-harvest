import mongoose, { Schema, Document } from 'mongoose';
import { ValueType } from './sensorLog.model';

export interface IAlert extends Document {
  type: ValueType; // The type of data triggering the alert (e.g., pH, temperature)
  timestamp: Date; // When the alert was triggered
  message: string; // A human-readable description of the alert
  severity: 'low' | 'medium' | 'high' | 'critical'; // Severity level
  controller: mongoose.Types.ObjectId; // Reference to the controller associated with the alert
  sensor: mongoose.Types.ObjectId; // Reference to the sensor associated with the alert
  acknowledged: boolean; // Whether the alert has been acknowledged
  acknowledgedBy?: mongoose.Types.ObjectId; // Reference to the user who acknowledged the alert
  acknowledgedAt?: Date; // When the alert was acknowledged
}

const AlertSchema: Schema = new Schema(
  {
    type: { type: String, enum: Object.values(ValueType), required: true }, // e.g., 'pH', 'temp', 'hum'
    timestamp: { type: Date, default: Date.now }, // Default to the current timestamp
    message: { type: String, required: true }, // A message describing the alert
    severity: {
      type: String,
      enum: ['low', 'medium', 'high', 'critical'],
      default: 'low',
    }, // Severity of the alert
    controller: {
      type: mongoose.Schema.Types.ObjectId,
      ref: 'Controller',
      required: true,
    }, // Controller associated with the alert
    sensor: {
      type: mongoose.Schema.Types.ObjectId,
      ref: 'Sensor',
      required: true,
    }, // Sensor triggering the alert
    acknowledged: { type: Boolean, default: false }, // Whether the alert is acknowledged
    acknowledgedBy: { type: mongoose.Schema.Types.ObjectId, ref: 'User' }, // User who acknowledged the alert
    acknowledgedAt: { type: Date }, // Timestamp of when the alert was acknowledged
  },
  { timestamps: true }
);

export default mongoose.model<IAlert>('Alert', AlertSchema);
