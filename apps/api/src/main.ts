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
const app = express();

app.use('/assets', express.static(path.join(__dirname, 'assets')));

app.get('/api', (req, res) => {
  res.send({ message: 'Welcome to report-server!' });
});

const port = process.env.PORT || 3333;
const server = app.listen(port, () => {
  console.log(`Listening at http://localhost:${port}/api`);
});

const mqttClient: MqttClient = mqtt.connect('mqtt://localhost:3011');
//create a queue
const queue = [];
//connect to the mqtt broker
mqttClient.on('connect', () => {
  mqttClient.subscribe('my/topic', function (err) {
    if (!err) {
      console.log('Subscribed to my/topic');
    } else {
      console.error('Failed to subscribe to my/topic');
    }
  });
});
 
mqttClient.on('message', function (topic, message) {
  // message is Buffer
  console.log(`Received message on ${topic}: ${message.toString()}`);

  // Send the message to all connected WebSocket clients
  clients.forEach((client) => {
    if (client.readyState === WebSocket.OPEN) {
      client.send(message.toString());
    }
  });
});

const wss = new WebSocket.Server({ port: 3000 });

let clients = [];

wss.on('connection', (ws) => {
  clients.push(ws);

  ws.on('message', (message) => {
    console.log('Received: %s', message);
  });

  ws.on('close', () => {
    clients = clients.filter((client) => client !== ws);
  });

  ws.send('Hello! Message from server!!');
});

const connectionString = `mongodb://myuser:mypassword@localhost:27017/api?authSource=admin`;

mongoose
  .connect(connectionString)
  .then(() => console.log('Database connected successfully'))
  .catch((err) => console.log(err));
