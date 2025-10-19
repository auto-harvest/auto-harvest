/**
 * Database Index Builder
 *
 * Run this script after deploying to ensure all indexes are created.
 * Usage: npx ts-node src/scripts/buildIndexes.ts
 */

import mongoose from 'mongoose';
import sensorLogModel from '../models/sensorLog.model';
import SquashedSensorLog from '../models/squashedSensorLog.model';

const connectionString = process.env.MONGO_URI || 'mongodb://myuser:mypassword@192.168.100.102:27017';
const dbName = process.env.MONGO_DB || 'auto-harvest';

async function buildIndexes() {
  try {
    console.log('Connecting to MongoDB...');
    await mongoose.connect(connectionString, { dbName });
    console.log('Connected successfully!');

    console.log('\n📊 Building indexes...\n');

    // Build SensorLog indexes
    console.log('Building SensorLog indexes...');
    await sensorLogModel.createIndexes();
    const sensorLogIndexes = await sensorLogModel.collection.getIndexes();
    console.log('✅ SensorLog indexes:', Object.keys(sensorLogIndexes));

    // Build SquashedSensorLog indexes
    console.log('\nBuilding SquashedSensorLog indexes...');
    await SquashedSensorLog.createIndexes();
    const squashedLogIndexes = await SquashedSensorLog.collection.getIndexes();
    console.log('✅ SquashedSensorLog indexes:', Object.keys(squashedLogIndexes));

    // Get collection stats
    console.log('\n📈 Collection Statistics:\n');



    console.log('\n✅ All indexes built successfully!');
  } catch (error) {
    console.error('❌ Error building indexes:', error);
    process.exit(1);
  } finally {
    await mongoose.disconnect();
    console.log('\nDisconnected from MongoDB');
    process.exit(0);
  }
}

buildIndexes();
