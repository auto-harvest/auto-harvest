import express from 'express';
import {
  createController,
  listUserControllers,
} from '../controllers/controller.controller';
import { authenticate } from '../middleware/auth.middleware';

const router = express.Router();

router.post('', authenticate, createController);
router.get('', authenticate, listUserControllers);
export const controllerRoutes = router;
