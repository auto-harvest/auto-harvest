import express from 'express';
import userRoutes from './user.routes';
import sensorLogRoutes from './sensorLog.routes';
import { controllerRoutes } from './controller.routes';
import { authenticate } from '../middleware/auth.middleware';
const router = express.Router();
export const routes = router
  .use('', userRoutes)
  .use('/logs', authenticate, sensorLogRoutes)
  .use('/controller', authenticate, controllerRoutes);
