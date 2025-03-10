import { connect, MqttClient } from 'mqtt';
import { fromEvent, Observable } from 'rxjs';
import { map, mergeMap, catchError } from 'rxjs/operators';
import SensorLogService from './sensorLog.service'; // Import the service for saving logs
import { ValueType } from '../models/sensorLog.model';
import { io } from './io.service';
import CollectorService from './collector.service';
// MQTT Configuration
const mqttConfig = {
  brokerURL: 'mqtt://34.105.172.73:3011', // Replace with your MQTT broker URL
  topic: 'sensor-data', // Topic to subscribe to
}; 
const collectorService = new CollectorService();
// Connect to the MQTT broker
export let client: MqttClient;
export const startMqttClient = () => {
  client = connect(mqttConfig.brokerURL);
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
        io.to(roomName).emit('sensor-info', JSON.stringify(sensorData));
        delete sensorData['client-id'];
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
