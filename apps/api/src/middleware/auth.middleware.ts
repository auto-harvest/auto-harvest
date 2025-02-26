import { Request, Response, NextFunction } from 'express';
import jwt from 'jsonwebtoken';

const secret = 'your_jwt_secret';

export const authenticate = (req: Request & { user?: any }, res: Response, next: NextFunction) => {
  const token = req.headers['authorization']?.split(' ')[1];
  if (!token) {
    return res.status(401).json({ message: 'No token provided' });
  }
  jwt.verify(token, secret, (err, decoded) => {
    if (err) {
      return res.status(401).json({ message: 'Failed to authenticate token' });
    }
    req.user = decoded;
    next();
  });
};