import { connect, MqttClient } from 'mqtt';
import { fromEvent, Observable } from 'rxjs';
import { map, mergeMap, catchError } from 'rxjs/operators';
import SensorLogService from './sensorLog.service'; // Import the service for saving logs
import { ValueType } from '../models/sensorLog.model';
import { io } from './io.service';
import CollectorService from './collector.service';
// MQTT Configuration
const mqttConfig = {
  brokerURL: 'wss://mqtt.autoharvest.solutions', // Replace with your MQTT broker URL
  topic: 'sensor-data', // Topic to subscribe to
};
export let lastLogs: any = {};
const collectorService = new CollectorService();
// Connect to the MQTT broker
export let client: MqttClient;
export const sendMessage = (topic: string, message: string) => {
  if (client && client.connected) {
    client.publish(topic, message);
  } else {
    console.error('MQTT client is not connected.');
  }
};
export const startMqttClient = () => {
  client = connect(mqttConfig.brokerURL, {
    clientId: 'nodejs-test-' + Math.random().toString(16).slice(2, 10),
    username: 'yourUser',
    password: 'yourPass',
    protocolVersion: 4, // MQTT 3.1.1
    keepalive: 60,
    reconnectPeriod: 2000,

    wsOptions: {
      headers: { 'Sec-WebSocket-Protocol': 'mqtt' },
      // Some environments prefer the 'ws' subprotocol field instead:
      // protocol: "mqtt"
    },
  });
  client.on('connect', () => {
    console.log('Connected to MQTT broker.');
    client.subscribe(mqttConfig.topic, (err) => {
      if (err) {
        console.error('Failed to subscribe to topic:', err);
      } else {
        console.log(`Subscribed to topic: ${mqttConfig.topic}`);
      }
    });
  });

  // Create an observable from MQTT messages
  const messageStream$: Observable<{ topic: string; message: string }> =
    fromEvent(client, 'message').pipe(
      map(([topic, message]: any) => ({
        topic,
        message: message.toString(), // Convert Buffer to string
      }))
    );

  // Process incoming messages
  messageStream$
    .pipe(
      map(({ message }) => JSON.parse(message)), // Parse the message as JSON
      mergeMap(async (sensorData: any) => {
        // Validate and process the message
        const clientId = sensorData['client-id'];

        const roomName = `controller:${clientId}`;
        console.log(roomName);
        //
        //clients.map((v) => v.emit('sensor-info', JSON.stringify(obj)));

        //print all sockets
        console.log('Sockets:', await io.in(roomName).allSockets());
        delete sensorData['client-id'];
        delete sensorData['flow-rate-hz'];
        delete sensorData['flow-rate-liters'];
        sensorData['liters-per-minute'];
        delete sensorData['pulses'];
        sensorData['vpd'] = +vpdKPa({
          airTempC: sensorData['temperature'],
          rhPct: sensorData['humidity'],
        }).toFixed(2);
        console.log('Sensor Data:', sensorData);
        lastLogs = sensorData;
        io.to(roomName).emit('sensor-info', JSON.stringify(sensorData));
        io.of('/report-server')
          .to(roomName)
          .emit('sensor-info', JSON.stringify(sensorData));
        const promises = [];
        for (const key in sensorData) {
          promises.push(
            collectorService.storeSensorLog({
              type: key,
              value: sensorData[key],
              controller: clientId,
            })
          );
        }
        return await Promise.all(promises);

        // Save the sensor log using the service
      }),
      catchError((error) => {
        console.error('Error processing sensor data:', error);
        return []; // Skip failed message
      })
    )
    .subscribe({
      next: (result) =>
        console.log('Sensor log saved successfully:', result.count),
      error: (err) => console.error('Error in subscription:', err),
    });
};

// Saturation vapor pressure (kPa) @ Celsius
const es = (Tc: number) => 0.6108 * Math.exp((17.27 * Tc) / (Tc + 237.3));

/** Leaf VPD (preferred). If leafTempC is missing, estimate leaf ≈ air - 1°C. */
export function vpdKPa({
  airTempC,
  rhPct,
  leafTempC,
}: {
  airTempC: number;
  rhPct: number;
  leafTempC?: number;
}): number {
  const rh = Math.min(100, Math.max(0, rhPct)) / 100; // clamp
  const ea = rh * es(airTempC); // actual vapor pressure
  const eLeaf = es(leafTempC ?? airTempC - 1); // leaf saturation VP
  return Math.max(0, eLeaf - ea); // kPa
}
