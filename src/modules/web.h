#ifndef WEB_H
#define WEB_H

#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <WiFi.h>
#include <ArduinoJson.h>

extern WebSocketsServer webSocket;
extern DynamicJsonDocument doc;


void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void processCommands(char *command, uint8_t num);
void initializeWifi();
void send2DataToClient(byte** data, size_t rows, size_t cols, uint8_t clientID);
void sendDataToClient(byte data[], size_t dataSize, uint8_t clientID);
int getFirstClientID();
void sendStringToClient(String message);
#endif