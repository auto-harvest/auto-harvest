import { Request, Response } from 'express';
import bcrypt from 'bcrypt';
import jwt from 'jsonwebtoken';
import User from '../models/user.model';

const secret = 'your_jwt_secret';

export const signup = async (req: Request, res: Response) => {
  try {
    const { username, email, password } = req.body;
    const hashedPassword = await bcrypt.hash(password, 10);
    const newUser = new User({ username, email, password: hashedPassword });
    const user = await newUser.save();
    delete user.password;
    res.json({
      user,
    });
  } catch (error) {
    res.status(500).send({ message: 'Error creating user', error });
  }
};

export const login = async (req: Request, res: Response) => {
  try {
    const { email, password } = req.body;
    const user = await User.findOne({ email }).lean();
    if (!user) {
      return res.status(404).send({ message: 'User not found' });
    }
    const isPasswordValid = await bcrypt.compare(password, user.password);
    if (!isPasswordValid) {
      return res.status(401).send({ message: 'Invalid password' });
    }
    const token = jwt.sign({ id: user._id, role: user.role }, secret, {
      expiresIn: '1h',
    });
    delete user.password;
    res.status(200).send({ message: 'Login successful', token });
  } catch (error) {
    res.status(500).send({ message: 'Error logging in', error });
  }
};
