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
  io.use((socket, next) => {
    console.log('AAAAAAAAAAAAAAa');
    const token = socket.handshake.query['token'];
    console.log(token);
    console.log(jwt.decode(token));
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
  io.on('error', async (socket) => {
    console.log(socket);
  });

  io.on('connection', async (socket: Socket & { user: any }) => {
    const user = await userModel
      .findOne({ _id: socket.user.id })
      .populate('controllers')
      .exec();
    console.log(socket.user, user);
    socket.on('disconnect', () => {
      console.log('user disconnected');
    });
    console.log(user);
    socket.on('ping', () => {
      console.log('ping received');
      setTimeout(() => {
        socket.emit('pong', Date.now());
      }, 15000);
    });
    socket.on('pump', (data) => {
      console.log('Pump command received:', data);
      if (data == 'pump-on' || data == 'pump-off') sendMessage(data, '');
    });
    socket.on('air-pump', (data) => {
      console.log('Air Pump command received:', data);
      if (data == 'air-pump-on' || data == 'air-pump-off')
        sendMessage(data, '');
    });
    console.log(
      `User listens to broadcast and controller rooms (${user?.controllers.length})`
    );
    await socket.join([
      'broadcast',
      ...user.controllers.map(
        (c) => `controller:${(c as unknown as IController).code}`
      ),
    ]);
    socket.emit('rooms', Array.from(socket.rooms.entries()));
  });
};
