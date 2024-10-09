import React, { useEffect, useState } from 'react';
import JSONPretty from 'react-json-pretty';

const SensorDataComponent = () => {
  const [data, setData] = useState(null);

  useEffect(() => {
    const ws = new WebSocket('ws://192.168.100.3:3000'); 

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
      <div>
        <JSONPretty
          id="json-pretty"
          style={{ fontSize: '1.1em' }}
          data={data}
          mainStyle="padding:1em"
          valueStyle="font-size:1.5em"
        ></JSONPretty>
      </div>
    </div>
  );
};

export default SensorDataComponent;
