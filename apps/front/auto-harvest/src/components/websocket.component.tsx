import React, { useEffect } from 'react';

const WebSocketComponent = () => {
  useEffect(() => {
    const ws = new WebSocket('ws://localhost:3000');

    ws.onopen = () => { 
      console.log('Connected to WebSocket server');
    };

    ws.onmessage = (event) => {
      console.log('Received: ' + event.data);
    };

    ws.onclose = () => {
      console.log('Disconnected from WebSocket server');
    };

    // Clean up the WebSocket connection when the component unmounts
    return () => {
      ws.close();
    };
  }, []);

  return <div>WebSocket Component</div>;
};

export default WebSocketComponent;
