const developmentEnvironment = {
  production: false,
  apiUrl: 'http://localhost:3333/report-server',
  // Add other environment-specific variables here
};

const productionEnvironment = {
  production: true,
  apiUrl: 'https://stage.autoharvest.solutions/report-server',
  // Add other environment-specific variables here
};
export const getEnvironment = () => {
  return process.env.NODE_ENV === 'production'
    ? productionEnvironment
    : developmentEnvironment;
};
export const environment = getEnvironment();
