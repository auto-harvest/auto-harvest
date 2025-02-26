
import { Expo, ExpoPushMessage } from 'expo-server-sdk';

const expo = new Expo();

export const sendPushNotification = async (
  pushToken: string,
  message: string
) => {
  if (!Expo.isExpoPushToken(pushToken)) {
    console.error(`Push token ${pushToken} is not a valid Expo push token`);
    return;
  }

  const messages: ExpoPushMessage[] = [
    {
      to: pushToken,
      sound: 'default',
      body: message,
      data: { withSome: 'data' },
    },
  ];

  try {
    const ticketChunk = await expo.sendPushNotificationsAsync(messages);
    console.log(ticketChunk);
  } catch (error) {
    console.error(error);
  }
};
