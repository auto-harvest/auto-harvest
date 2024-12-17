import mongoose, { Schema, Document } from 'mongoose';

export interface IBaseEntity extends Document {
  createdAt: Date;
  updatedAt: Date;
  deleted: boolean;
}

const BaseEntitySchema: Schema = new Schema(
  {
    createdAt: { type: Date, default: Date.now },
    updatedAt: { type: Date, default: Date.now },
    deleted: { type: Boolean, default: false },
  },
  { timestamps: true }
);

export default BaseEntitySchema;
