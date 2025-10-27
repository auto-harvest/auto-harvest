import { connect, MqttClient } from 'mqtt';
import { fromEvent, Observable } from 'rxjs';
import { map, mergeMap, catchError } from 'rxjs/operators';
import { io } from './io.service';
import CollectorService from './collector.service';

// Sensor validation ranges
const SENSOR_VALIDATION_RANGES: Record<
  string,
  { min: number; max: number; allowZero?: boolean }
> = {
  // pH sensors (both uppercase and lowercase variants)
  pH: { min: 0, max: 14 },
  ph: { min: 0, max: 14 },

  // EC/TDS sensors
  ec: { min: 0.1, max: 10000 }, // EC in µS/cm, should be positive
  tds: { min: 0.1, max: 5000 }, // TDS (Total Dissolved Solids) in ppm

  // Temperature sensors
  temp: { min: -10, max: 60 }, // Air temperature in Celsius
  'water-temperature': { min: 0, max: 50 }, // Water temperature in Celsius

  // Humidity
  hum: { min: 1, max: 100 }, // Humidity percentage (0 indicates sensor not connected)

  // Flow sensors
  flow: { min: 0, max: 1000, allowZero: true }, // Flow rate, can be 0 when no flow
  pulses: { min: 0, max: 1000000, allowZero: true }, // Flow pulses counter
  'liters-per-minute': { min: 0, max: 1000, allowZero: true }, // Flow rate in L/min
  // Level and pump states
  'water-level': { min: -1, max: 1, allowZero: true }, // Water level percentage (0 = empty is valid)
  'water-pump': { min: 0, max: 1, allowZero: true }, // Pump state (0 = off, 1 = on)
  'air-pump': { min: 0, max: 1, allowZero: true }, // Air pump state (0 = off, 1 = on)
};

/**
 * Validates sensor data to ensure values are within acceptable ranges
 * @param type - Sensor type
 * @param value - Sensor value
 * @returns true if valid, false otherwise
 */
function isValidSensorValue(type: string, value: number): boolean {
  const validation = SENSOR_VALIDATION_RANGES[type];

  if (!validation) {
    console.warn(`No validation range defined for sensor type: ${type}`);
    return true; // Allow unknown types to pass through
  }

  // Check if value is a valid number
  if (typeof value !== 'number' || isNaN(value)) {
    return false;
  }

  // Special case: if zero is not allowed and value is 0
  if (!validation.allowZero && value === 0) {
    return false;
  }

  // Check if value is within range
  if (value < validation.min || value > validation.max) {
    return false;
  }

  return true;
}
// MQTT Configuration from environment variables
const mqttConfig = {
  brokerURL: 'mqtt://192.168.100.102:3011', // Classic MQTT protocol
  topic: process.env.MQTT_TOPIC || 'sensor-data',
  username: process.env.MQTT_USERNAME || 'yourUser',
  password: process.env.MQTT_PASSWORD || 'yourPass',
};
export let lastLogs: any = {};
const collectorService = new CollectorService();
// Connect to the MQTT broker
export let client: MqttClient;
let timeSyncInterval: NodeJS.Timeout | null = null;

export const sendMessage = (topic: string, message: string) => {
  if (client && client.connected) {
    client.publish(topic, message);
  } else {
    console.error('MQTT client is not connected.');
  }
};

/**
 * Start periodic time sync to Arduino devices
 * Sends Athens timezone time every 60 seconds via MQTT
 */
const startTimeSync = () => {
  // Clear any existing interval
  if (timeSyncInterval) {
    clearInterval(timeSyncInterval);
  }

  // Send initial time immediately
  const sendAthensTime = () => {
    try {
      // Get current time in Athens timezone
      const now = new Date();

      // Format time as HH:MM:SS
      const timeStr = now.toLocaleString('en-US', {
        timeZone: 'Europe/Athens',
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit',
        hour12: false,
      });

      // Format date as DD/MM/YYYY
      const dateStr = now.toLocaleString('en-GB', {
        timeZone: 'Europe/Athens',
        day: '2-digit',
        month: '2-digit',
        year: 'numeric',
      });

      // Send as JSON: {"time": "14:30:45", "date": "20/10/2025"}
      const payload = JSON.stringify({ time: timeStr, date: dateStr });
      sendMessage('time-sync', payload);

      console.log(`⏰ Sent Athens time: ${timeStr} ${dateStr}`);
    } catch (error) {
      console.error('Error sending time sync:', error);
    }
  };

  // Send immediately on start
  sendAthensTime();

  // Then send every 60 seconds
  timeSyncInterval = setInterval(sendAthensTime, 60000);
  console.log('✅ Time sync started - sending Athens time every 60 seconds');
};

/**
 * Stop time sync interval
 */
const stopTimeSync = () => {
  if (timeSyncInterval) {
    clearInterval(timeSyncInterval);
    timeSyncInterval = null;
    console.log('⏸️  Time sync stopped');
  }
};

export const startMqttClient = () => {
  // Generate unique client ID for each instance
  const uniqueClientId = `autoharvest-api-${
    process.env.INSTANCE_ID || Math.random().toString(16).slice(2, 10)
  }`;

  console.log('Connecting to MQTT broker:', mqttConfig.brokerURL);
  console.log('Client ID:', uniqueClientId);

  client = connect(mqttConfig.brokerURL, {
    clientId: uniqueClientId,
    username: mqttConfig.username,
    password: mqttConfig.password,
    protocolVersion: 4, // MQTT 3.1.1
    keepalive: 60,
    reconnectPeriod: 5000, // Retry every 5 seconds
    connectTimeout: 30000, // 30 second timeout
    clean: true, // Clean session
  });
  client.on('connect', () => {
    console.log(
      `✅ Connected to MQTT broker with client ID: ${uniqueClientId}`
    );

    // Subscribe with QoS 1 for reliable delivery
    client.subscribe(mqttConfig.topic, { qos: 1 }, (err) => {
      if (err) {
        console.error('Failed to subscribe to topic:', err);
      } else {
        console.log(`✅ Subscribed to topic: ${mqttConfig.topic} with QoS 1`);
      }
    });

    // Start time sync to Arduino devices
    startTimeSync();
  });

  client.on('reconnect', () => {
    console.log('🔄 Reconnecting to MQTT broker...');
  });

  client.on('error', (error: any) => {
    console.error('❌ MQTT error:', error.message || error);
  });

  client.on('offline', () => {
    console.warn('⚠️  MQTT client is offline');
    stopTimeSync(); // Stop time sync when offline
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

        if (!clientId) {
          console.error('Received sensor data without client-id');
          return [];
        }

        // Remove unnecessary fields before validation
        delete sensorData['client-id'];
        delete sensorData['flow-rate-hz'];
        delete sensorData['flow-rate-liters'];
        sensorData['liters-per-minute'] = sensorData['lpm'];
        // delete sensorData['liters-per-minute'];
        sensorData['air-pump'] = sensorData['ap'];
        sensorData['water-pump'] = sensorData['wp'];
        sensorData['water-level'] = sensorData['wl'];
        sensorData['water-temperature'] = sensorData['wt'];
        sensorData['humidity'] = sensorData['h'];
        sensorData['temperature'] = sensorData['t'];
        delete sensorData['wl'];
        delete sensorData['wt'];
        delete sensorData['ap'];
        delete sensorData['wp'];
        delete sensorData['lpm'];
        // Validate and filter sensor data
        const validatedData: Record<string, number> = {};
        const invalidData: Record<string, number> = {};

        for (const key in sensorData) {
          const value = sensorData[key];
          if (typeof value === 'number' && !isNaN(value)) {
            if (isValidSensorValue(key, value)) {
              validatedData[key] = value;
            } else {
              invalidData[key] = value;
              console.warn(
                `Invalid sensor value detected for controller ${clientId}: ${key}=${value} (out of acceptable range)`
              );
            }
          }
        }
        const vpd = vpdKPa({
          airTempC: validatedData['temperature'],
          rhPct: validatedData['humidity'],
        });
        if (!isNaN(vpd)) {
          validatedData['vpd'] = parseFloat(vpd.toFixed(2));
        } else {
          console.warn(
            `Could not calculate VPD for controller ${clientId} due to missing or invalid temperature/humidity data.`
          );
        }
        // If no valid data, skip broadcasting and storage
        if (Object.keys(validatedData).length === 0) {
          console.warn(
            `No valid sensor data received from controller ${clientId}. All values invalid:`,
            invalidData
          );
          return [];
        }

        // Store the validated data
        lastLogs = validatedData;

        // Broadcast only validated sensor data to connected users
        if (!io) {
          console.error(
            'Socket.io not initialized, cannot broadcast sensor data'
          );
        } else {
          const roomName = `controller:${clientId}`;
          console.log(
            `Broadcasting validated sensor data to room: ${roomName}`,
            validatedData
          );
          if (Object.keys(invalidData).length > 0) {
            console.log(`Filtered out invalid values:`, invalidData);
          }
          io.to(roomName).emit('sensor-info', validatedData);
        }

        // Store only valid sensor values in database
        const promises = [];
        for (const key in validatedData) {
          promises.push(
            collectorService.storeSensorLog({
              type: key,
              value: validatedData[key],
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
