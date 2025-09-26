/**
 * This is not a production server yet!
 * This is only a minimal backend to get started.
 */

import express from 'express';
import * as path from 'path';

import WebSocket from 'ws';
//connect to mqtt
import mqtt from 'mqtt';
import { MqttClient } from 'mqtt';
import mongoose from 'mongoose';
import http from 'http';

import { Server, Socket } from 'socket.io';
import { io, startIo } from './services/io.service';
import { startMqttClient } from './services/mqtt.service';
import HistoricSensorLogService from './services/historicSensorLog.service';
import { routes } from './routes/routes';
import morgan from 'morgan';
import cors from 'cors';
const app = express();
app.use(morgan('dev'));
//setup json text body

app.use(express.json());
app.use(
  cors({
    origin: ['http://localhost:8081', 'https://autoharvest.ngrok.dev'],
  })
);
app.use('/api', routes);
app.use('/assets', express.static(path.join(__dirname, 'assets')));

const port = process.env.PORT || 3333;

export const server = app.listen(+port, '0.0.0.0', null, () => {
  startIo();
  startMqttClient();
  console.log(`Listening at http://localhost:${port}/api`);
});

const connectionString = `mongodb://myuser:mypassword@localhost:27017`;

mongoose
  .connect(connectionString, { dbName: 'auto-harvest' })
  .then(() => console.log('Database connected successfully'))
  .catch((err) => console.log(err));
