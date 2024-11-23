#include "webserver.service.h"
#include "utility/parseRequest.util.h"
#include "context/app.context.h"
WebServerService::WebServerService(DiskManagerService &diskManager, WiFiService &wifiService) : server(80), diskManager(diskManager), wifiService(wifiService) {}

void WebServerService::begin()
{

    // Start the server
    server.begin();
    Serial.println("Server started");
    server.status();
}

String WebServerService::getState()
{
    return server.status() == 1 ? "running" : "stopped";
}

void WebServerService::handleClient()
{
    WiFiClient client = server.available();
    if (!client)
    {
        return;
    }
    client.setTimeout(5000); // 5-second timeout for reading the request
    Serial.println("New client connected");

    String requestLine = "";
    int contentLength = 0;

    // Read headers with timeout
    unsigned long startTime = millis();
    while (client.connected() && millis() - startTime < 5000)
    { // 5-second timeout for headers
        if (client.available())
        {
            String line = client.readStringUntil('\r');
            client.read(); // Consume '\n'
            line.trim();   // Remove whitespace

            if (line.length() > 0)
            {
                Serial.println("Header: " + line);

                // Capture Content-Length
                if (line.startsWith("Content-Length:"))
                {
                    contentLength = line.substring(15).toInt();
                    Serial.println("Parsed Content-Length: " + String(contentLength));
                }

                // Save the request line (only if it is the first line)
                if (requestLine == "")
                {
                    requestLine = line;
                }
            }
            else
            {
                // Break on empty line indicating end of headers
                Serial.println("End of headers");
                break;
            }
        }
        else
        {
            Serial.println("No data available");
            // Delay to allow more data to arrive
            delay(10);
        }
    }

    // Check for timeout or incomplete headers
    if (requestLine == "")
    {
        Serial.println("Error: No request line received");
        client.stop();
        return;
    }

    Serial.println("Request Line: " + requestLine);

    // Process GET or POST requests
    if (requestLine.startsWith("GET / "))
    {
        handleRoot(client);
    }
    else if (requestLine.startsWith("POST /save"))
    {
        if (contentLength > 0)
        {
            String postBody = readRequestBody(client, contentLength);
            Serial.println("Post Body: " + postBody);
            handleSaveCredentials(client, postBody);
        }
        else
        {
            Serial.println("Error: Content-Length is 0 or missing");
            client.println("HTTP/1.1 400 Bad Request");
            client.println("Content-Type: text/html");
            client.println();
            client.println("<html><body><h1>400 Bad Request: No Content-Length</h1></body></html>");
            client.stop();
        }
    }
    else
    {
        // Send 404 response
        client.println("HTTP/1.1 404 Not Found");
        client.println("Content-Type: text/html");
        client.println();
        client.println("<html><body><h1>404 Not Found</h1></body></html>");
        client.stop();
    }
}

void WebServerService::handleRoot(WiFiClient &client)
{
    String html = "<html><body><form action='/save' method='POST'>"
                  "SSID: <input type='text' name='ssid'><br>"
                  "Password: <input type='text' name='password'><br>"
                  "<input type='submit' value='Save'>"
                  "</form></body></html>";

    // Send HTTP response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println(html);
    client.stop();
}

void WebServerService::handleSaveCredentials(WiFiClient &client, String &body)
{
    Serial.print("Received body: ");
    Serial.println(body);
    String request = client.readStringUntil('\r');
    auto queryParams = parsePostBody(body);
    Serial.println("Request params:");
    Serial.println(queryParams.size());
    Serial.println(queryParams["ssid"]);
    Serial.println(queryParams["password"]);

    // Extract ssid and password from the request body
    // int ssidIndex = queryParams["ssid"];
    // int passwordIndex = queryParams["password"];
    // if ()
    // {
    //     client.println("HTTP/1.1 400 Bad Request");
    //     client.println("Content-Type: text/html");
    //     client.println("Connection: close");
    //     client.println();
    //     client.println("<html><body><h1>Invalid request</h1></body></html>");
    //     client.stop();
    //     return;
    // }

    String ssid = queryParams["ssid"];
    String password = queryParams["password"];

    // Save credentials
    diskManager.save("ssid", ssid);
    diskManager.save("password", password);
    // Send response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println("<html><body><h1>Credentials saved. Please restart the device.</h1></body></html>");
    client.stop();
    wifiService.connectToWiFi(ssid.c_str(), password.c_str());
}

String WebServerService::readRequestBody(WiFiClient &client, int contentLength)
{
    String postBody = "";
    unsigned long startTime = millis();

    while (postBody.length() < contentLength)
    {
        // Timeout check
        if (millis() - startTime > 2000)
        { // 2-second timeout
            Serial.println("Error: Body read timeout");
            break;
        }

        // Read available data
        if (client.available())
        {
            char c = client.read();
            postBody += c;
        }
    }

    if (postBody.length() < contentLength)
    {
        Serial.println("Warning: Incomplete body received");
    }

    return postBody;
}