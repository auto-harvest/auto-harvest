import express from 'express';
import userRoutes from './user.routes';
import sensorLogRoutes from './sensorLog.routes';
import { controllerRoutes } from './controller.routes';
const router = express.Router();
export const routes = router
  .use('/', userRoutes)
  .use('/logs', sensorLogRoutes)
  .use('/controller', controllerRoutes);
