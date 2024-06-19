import styled from 'styled-components';

import NxWelcome from './nx-welcome';
import WebSocketComponent from '../components/websocket.component';
import SensorDataComponent from '../components/sensor.component';

const StyledApp = styled.div`
  // Your style here
`;

export function App() {
  return (
    <StyledApp>
      <SensorDataComponent></SensorDataComponent>
    </StyledApp>
  );
}

export default App;
 