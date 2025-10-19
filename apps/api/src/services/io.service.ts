import { Server, Socket } from 'socket.io';
import jwt from 'jsonwebtoken';
import userModel from '../models/user.model';
import { IController } from '../models/controller.model';
import { sendMessage } from './mqtt.service';

export let io: Server;

function authMw(socket: Socket, next: (err?: Error) => void) {
  const q = socket.handshake.query as any;
  const a = socket.handshake.auth as any;
  const token = a?.token ?? q?.token;
  console.log('Auth middleware', { token });
  if (!token) return next(new Error('Authentication error'));
  jwt.verify(token, 'your_jwt_secret', (err, decoded) => {
    if (err) return next(new Error('Authentication error'));
    (socket as Socket & { user: any }).user = decoded;
    next();
  });
}

async function attachHandlers(nsp: ReturnType<Server['of']>) {
  nsp.use(authMw);

  nsp.on('connection', async (socket: Socket & { user: any }) => {
    const user = await userModel
      .findOne({ _id: socket.user.id })
      .populate('controllers')
      .exec();

    console.log(`[SIO] ${nsp.name} connected`, socket.id, socket.user?.id);

    socket.on('disconnect', () =>
      console.log(`[SIO] ${nsp.name} disconnected`, socket.id)
    );

    socket.on('ping', () => {
      setTimeout(() => socket.emit('pong', Date.now()), 15000);
    });

    socket.on('pump', (data) => {
      if (user.controllers.length === 0) {
        socket.emit('error', 'No controllers associated with user');
        return;
      }
      if (data === 'pump-on' || data === 'pump-off') sendMessage(data, '');
    });
    socket.on('air-pump', (data) => {
      if (user.controllers.length === 0) {
        socket.emit('error', 'No controllers associated with user');
        return;
      }
      if (data === 'air-pump-on' || data === 'air-pump-off')
        sendMessage(data, '');
    });
    socket.on('fan', (data) => {
      if (user.controllers.length === 0) {
        socket.emit('error', 'No controllers associated with user');
        return;
      }
      if (data === 'fan-on' || data === 'fan-off') sendMessage(data, '');
    });
    await socket.join([
      'broadcast',
      ...(user?.controllers ?? []).map(
        (c) => `controller:${(c as unknown as IController).code}`
      ),
    ]);

    socket.emit('rooms', Array.from(socket.rooms.entries()));
  });
}

export const startIo = (server: any) => {
  io = new Server(server, {
    path: '/socket.io', // keep engine path stable
    cors: { origin: '*' },
  });

  // Default namespace
  attachHandlers(io.of('/'));

  // ✅ Accept `/report-server` namespace too (same behavior)
  attachHandlers(io.of('/report-server'));
};
