/**
 * This is not a production server yet!
 * This is only a minimal backend to get started.
 */

import express from 'express';
import * as path from 'path';
import http from 'http';

import mqtt from 'mqtt';
import mongoose from 'mongoose';

import morgan from 'morgan';
import cors from 'cors';

import { startIo } from './services/io.service';
import { lastLogs, startMqttClient } from './services/mqtt.service';
import { routes } from './routes/routes';
import { environment } from './environment/environment';

// --- Setup express app ---
const app = express();
 
app.use(morgan('dev'));
app.use((req, res, next) => {
  console.log('Request:', req.method, req.url);
  next();
});
app.use(express.json());
app.use(
  cors({
    origin: [
      'http://localhost:8081',
      'http://localhost:8082',
      'http://localhost:8083',
      'https://stage.autoharvest.solutions',
    ],
  })
);
app.use('/api', routes);
app.use('/last-logs', (req, res) => {
  res.json(lastLogs);
});
app.use('/assets', express.static(path.join(__dirname, 'assets')));

// --- Create HTTP server and attach express ---

// --- Start server ---
const port = process.env.PORT || 3333;

export const server = app.listen(+port, '0.0.0.0', null, () => {
  startIo(server);
  startMqttClient();
  console.log(`Listening at http://localhost:${port}/api`);
});
// --- Connect to MongoDB ---
const connectionString = `mongodb://myuser:mypassword@192.168.100.102:27017`;

mongoose
  .connect(connectionString, {
    dbName: environment.production ? 'auto-harvest' : 'dev-auto-harvest',
  })
  .then(() => { 
    console.log('Database connected successfully');
    // CollectorService is instantiated when needed via MQTT service
  })
  .catch((err) => console.error('MongoDB connection error:', err));
 