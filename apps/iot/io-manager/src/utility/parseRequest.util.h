#ifndef QUERY_PARSER_H
#define QUERY_PARSER_H

#include <Arduino.h>
#include <map>

// Function to parse query parameters from an HTTP request line into a map
std::map<String, String> extractQueryParams(const String &requestLine);
String urlDecode(const String &input);
std::map<String, String> parsePostBody(const String &body);

#endif // QUERY_PARSER_H
