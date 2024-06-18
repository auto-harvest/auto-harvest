import styled from 'styled-components';

import NxWelcome from './nx-welcome';
import WebSocketComponent from '../components/websocket.component';

const StyledApp = styled.div`
  // Your style here
`;

export function App() {
  return (
    <StyledApp>
      <NxWelcome title="auto-harvest" />
      <WebSocketComponent></WebSocketComponent>
    </StyledApp> 
  );
}

export default App;
 