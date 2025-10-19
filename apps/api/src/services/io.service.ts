
  import { Server, Socket } from 'socket.io';
  import jwt from 'jsonwebtoken';
  import userModel from '../models/user.model';
  import { IController } from '../models/controller.model';
  import { sendMessage } from './mqtt.service';

  export let io: Server;

  export const startIo = (server) => {
    io = new Server(server, {
      cors: {
        origin: '*',
      },
    });

    // Authentication middleware
    io.use((socket, next) => {
      const token = socket.handshake.auth.token ||
  socket.handshake.query['token'];

      if (!token) {
        return next(new Error('Authentication error'));
      }

      jwt.verify(token, 'your_jwt_secret', (err, decoded) => {
        if (err) {
          return next(new Error('Authentication error'));
        }
        (socket as Socket & { user: any }).user = decoded;
        next();
      });
    }); 

    io.on('connection', async (socket: Socket & { user: any }) => {
      console.log(`User ${socket.user.id} connected`);

      // Fetch user and join rooms
      const user = await userModel
        .findOne({ _id: socket.user.id })
        .populate('controllers')
        .exec();

      if (!user) {
        console.error(`User ${socket.user.id} not found in database`);
        socket.disconnect();
        return;
      }

      // Join broadcast and controller-specific rooms
      const controllerRooms = user.controllers.map(
        (c) => `controller:${(c as unknown as IController).code}`
      );
      await socket.join(['broadcast', ...controllerRooms]);

      console.log(`User ${socket.user.id} joined rooms:`, ['broadcast', ...controllerRooms]);

      // Send initial room list
      socket.emit('rooms', Array.from(socket.rooms.entries()));

      // Optionally send a hello message
      socket.emit('hello-message', 'Connected to Auto Harvest Server');

      // Handle ping/pong - respond immediately (frontend expects ~5s cycle)
      socket.on('ping', (timestamp) => {
        socket.emit('pong', timestamp);
      });

      // Handle pump commands
      socket.on('pump', (data) => {
        console.log('Pump command received:', data);
        if (data === 'pump-on' || data === 'pump-off') {
          sendMessage(data, '');
        }
      });

      // Handle air pump commands
      socket.on('air-pump', (data) => {
        console.log('Air Pump command received:', data);
        if (data === 'air-pump-on' || data === 'air-pump-off') {
          sendMessage(data, '');
        }
      });

      // Handle disconnect
      socket.on('disconnect', (reason) => {
        console.log(`User ${socket.user.id} disconnected: ${reason}`);
      });
    });
  };

  // Helper to broadcast sensor info to relevant rooms
  export const broadcastSensorInfo = (controllerCode: string, sensorData: any) =>
  {
    if (!io) return;

    io.to(`controller:${controllerCode}`).emit('sensor-info', sensorData);
  };

  // Helper to broadcast to all users
  export const broadcastToAll = (event: string, data: any) => {
    if (!io) return;

    io.to('broadcast').emit(event, data);
  };
