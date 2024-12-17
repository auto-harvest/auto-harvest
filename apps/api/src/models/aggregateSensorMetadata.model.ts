import mongoose, { Schema } from "mongoose";

export interface IAggregateSensorMetadata extends Document {
  granularity: 'm' | 'h' | 'd' | 'w';
  statistics: mongoose.Types.ObjectId;
  calculationMethod: 'average' | 'median' | 'sum';
  lastUpdated: Date;
}

const AggregateSensorMetadataSchema: Schema = new Schema(
  {
    granularity: { type: String, enum: ['m', 'h', 'd', 'w'], required: true },
    statistics: {
      type: mongoose.Schema.Types.ObjectId,
      ref: 'Statistics',
      required: true,
    },
    calculationMethod: {
      type: String,
      enum: ['average', 'median', 'sum'],
      required: true,
    },
    lastUpdated: { type: Date, default: Date.now },
  },
  { timestamps: true }
);

export default mongoose.model<IAggregateSensorMetadata>(
  'AggregateSensorMetadata',
  AggregateSensorMetadataSchema
);
