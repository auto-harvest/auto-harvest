import React, { useEffect, useState } from 'react';

const SensorDataComponent = () => {
  const [data, setData] = useState(null);

  useEffect(() => {
    const ws = new WebSocket('ws://localhost:3000');

    ws.onmessage = (message) => {
      setData(message.data);
    };

    return () => {
      ws.close();
    }; 
  }, []);

  return (
    <div>
      <h2>Sensor Data:</h2>
      {data ? <p>{data}</p> : <p>Loading...</p>}
    </div>
  );
};

export default SensorDataComponent;
 