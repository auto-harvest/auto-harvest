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
const app = express();
//setup json text body
app.use(express.json());
export const server = http.createServer(app);

app.use('/assets', express.static(path.join(__dirname, 'assets')));

app.get('/api', (req, res) => {
  res.send({ message: 'Welcome to report-server!' });
});

const port = process.env.PORT || 3333;
server.listen(port, () => {
  startIo();
  startMqttClient();
  console.log(`Listening at http://localhost:${port}/api`);
  HistoricSensorLogService.collectAndAggregateLogs();
});

const connectionString = `mongodb://myuser:mypassword@localhost:27017`;

mongoose
  .connect(connectionString, { dbName: 'auto-harvest' })
  .then(() => console.log('Database connected successfully'))
  .catch((err) => console.log(err));
