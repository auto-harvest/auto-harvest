import { Server, Socket } from 'socket.io';
import { server } from '../main';
import jwt from 'jsonwebtoken';

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
    await socket.join(['broadcast', 'controller:A0:20:A6:1C:7A:7A']);
  });
};
