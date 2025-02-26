import { Server, Socket } from 'socket.io';
import { server } from '../main';
import jwt from 'jsonwebtoken';
import userModel from '../models/user.model';

export let io: Server;

export const startIo = () => {
  io = new Server(server, {
    cors: {
      origin: '*',
    },
  });
  io.use((socket, next) => {
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
  io.on('connection', async (socket: Socket & { user: any }) => {
    const user = await userModel.findOne({ _id: socket.user.id });
    console.log(socket.user);
    socket.on('disconnect', () => {
      console.log('user disconnected');
    });

    socket.on('ping', () => {
      console.log('ping received');
      setTimeout(() => {
        socket.emit('pong', Date.now());
      }, 15000);
    });
    console.log(
      `User listens to broadcast and controller rooms (${user?.controllers.length})`
    );
    await socket.join([
      'broadcast',
      ...user.controllers.map((c) => `controller:${c}`),
    ]);
  });
};
