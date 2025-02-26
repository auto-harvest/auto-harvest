import { Request, Response } from 'express';
import Controller from '../models/controller.model';
import User from '../models/user.model';

export const createController = async (
  req: Request & { user: any },
  res: Response
) => {
  try {
    const { name, code, location, capacity } = req.body;
    const userId = req.user.id; // Assuming user ID is available in the request object
    console.log(name, code, location, capacity, userId);
    const newController = new Controller({
      name,
      code,
      user: userId,
      location,
      capacity,
    });

    const controller = await newController.save();

    await User.findByIdAndUpdate(userId, {
      $push: { controllers: controller._id },
    });

    res
      .status(200)
      .send({ message: 'Controller created successfully', error: 0 });
  } catch (error) {
    if (error.code === 11000) {
      res.status(200).send({ message: 'Controller already exists', error: 1 });
    } else {
      res.status(200).send({ message: 'Error creating controller', error: 1 });
    }
  }
};

export const listUserControllers = async (
  req: Request & { user: any },
  res: Response
) => {
  try {
    const userId = req.user.id; // Assuming user ID is available in the request object

    const user = await User.findById(userId).populate('controllers');

    res.status(200).json(user?.controllers);
  } catch (error) {
    res.status(500).send({ message: 'Error fetching controllers', error });
  }
};
