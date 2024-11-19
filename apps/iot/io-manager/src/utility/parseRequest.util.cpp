#include <Arduino.h>
#include <map>
// Helper function to decode URL-encoded characters
String urlDecode(const String &input)
{
    String decoded = "";
    for (unsigned int i = 0; i < input.length(); ++i)
    {
        if (input[i] == '+')
        {
            decoded += ' '; // Convert '+' to space
        }
        else if (input[i] == '%' && i + 2 < input.length())
        {
            // Decode %XX (hexadecimal encoding)
            char hex[3] = {input[i + 1], input[i + 2], '\0'};
            char decodedChar = strtol(hex, nullptr, 16); // Convert hex to char
            decoded += decodedChar;
            i += 2; // Skip the two hex digits
        }
        else
        {
            decoded += input[i]; // Append regular character
        }
    }
    return decoded;
}
// Function to parse query parameters into a map
std::map<String, String> extractQueryParams(const String &requestLine)
{
    std::map<String, String> queryMap;

    // Find the start of the query string
    int queryStart = requestLine.indexOf('?');
    int queryEnd = requestLine.indexOf(' ', queryStart); // Query ends at the first space
    if (queryStart == -1 || queryEnd == -1)
    {
        return queryMap; // No query string found
    }

    // Extract the query string
    String query = requestLine.substring(queryStart + 1, queryEnd);

    // Split the query string by '&'
    int start = 0;
    while (start < query.length())
    {
        int pairEnd = query.indexOf('&', start);
        if (pairEnd == -1)
            pairEnd = query.length(); // Last pair

        // Extract key-value pair
        String pair = query.substring(start, pairEnd);
        int equalSign = pair.indexOf('=');
        if (equalSign != -1)
        {
            String key = pair.substring(0, equalSign);
            String value = pair.substring(equalSign + 1);

            // Replace '+' with spaces and decode "%20"
            key.replace('+', ' ');
            value.replace('+', ' ');
            key.replace("%20", " ");
            value.replace("%20", " ");

            // Store in the map
            queryMap[key] = value;
        }

        start = pairEnd + 1; // Move to the next pair
    }

    return queryMap;
}
// Function to parse POST body into a map
std::map<String, String> parsePostBody(const String &body)
{
    std::map<String, String> bodyMap;

    // Split the body by '&' (key-value pairs are separated by '&')
    int start = 0;
    while (start < body.length())
    {
        int pairEnd = body.indexOf('&', start);
        if (pairEnd == -1)
            pairEnd = body.length(); // Last pair

        // Extract key-value pair
        String pair = body.substring(start, pairEnd);
        int equalSign = pair.indexOf('=');
        if (equalSign != -1)
        {
            // Decode key and value
            String key = urlDecode(pair.substring(0, equalSign));
            String value = urlDecode(pair.substring(equalSign + 1));

            // Store in the map
            bodyMap[key] = value;
        }

        start = pairEnd + 1; // Move to the next pair
    }

    return bodyMap;
}