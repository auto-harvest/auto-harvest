import express from 'express';
import {
  createSensorLog,
  getSensorLogs,
} from '../controllers/sensorLog.controller';

const router = express.Router();

router.post('/logs', createSensorLog); // Create a new sensor log
router.get('/logs', getSensorLogs); // Get sensor logs by type and date range

export default router;
