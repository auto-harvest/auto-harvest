#ifndef CONFIG_H
#define CONFIG_H

// Network Configuration
// Choose your MQTT broker connection method:
//
// Option 1 - Cloudflare Tunnel (recommended for internet access):
//   - Set up cloudflared with TCP service (see config.yml below)
//   - Provides stable hostname accessible from anywhere
//   - No TLS required (raw TCP tunnel)
//   - Example: #define MQTT_BROKER_HOST "mqtt.yourdomain.com"
//
// Option 2 - mDNS (recommended for local network):
//   - Requires Avahi (Linux) or Bonjour (Windows/Mac) on broker
//   - Example: #define MQTT_BROKER_HOST "mqtt-broker.local"
//
// Option 3 - Regular hostname (requires DNS/router config):
//   - Example: #define MQTT_BROKER_HOST "mqtt-broker"
//
// Option 4 - IP address (fallback, not recommended):
//   - Example: #define MQTT_BROKER_HOST "192.168.100.102"

// Cloudflare Tunnel example config.yml:
// ingress:
//   - hostname: mqtt.yourdomain.com
//     service: tcp://localhost:3011
//   - service: http_status:404

// TROUBLESHOOTING:
// If you get error -2 (MQTT_CONNECT_FAILED):
//   1. Test with IP first to isolate DNS: #define MQTT_BROKER_HOST "192.168.100.102"
//   2. Verify Cloudflare tunnel is running: systemctl status cloudflared
//   3. Test tunnel externally: nc -zv mqtt.autoharvest.solutions 3011
//   4. Check DNS from another device: nslookup mqtt.autoharvest.solutions
//   5. Ensure router/DNS can resolve the hostname
//   6. For local network, try mDNS: "mqtt-broker.local"

// Using IP address until hostname resolution is configured
#define MQTT_BROKER_HOST "192.168.100.102"  // Your MQTT broker's local IP
#define MQTT_BROKER_PORT 3011

// Once you configure mDNS or Cloudflare Tunnel properly, use:
// #define MQTT_BROKER_HOST "mqtt-broker.local"  // for mDNS (local network)
// #define MQTT_BROKER_HOST "mqtt.autoharvest.solutions"  // for Cloudflare Tunnel

// WiFi Configuration (currently stored in code, could be moved here)
// #define WIFI_SSID "your-ssid"
// #define WIFI_PASSWORD "your-password"

// Access Point Configuration
#define AP_SSID "ESP8266"
#define AP_PASSWORD "12345678"
#define AP_IP_ADDRESS IPAddress(5, 5, 5, 5)
#define AP_GATEWAY IPAddress(5, 5, 5, 5)
#define AP_SUBNET IPAddress(255, 255, 255, 0)

#endif // CONFIG_H
