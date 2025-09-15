#include <Arduino.h>
#include <WebSocketManager.h>

#define SSID "IWANS-LAPTOP 4425"
#define PASS "3142521359"


WebSocketManager ws_manager;


void on_websocket_data(WEBSOCKET_DATA) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected from server");
      break;

    case WStype_CONNECTED:
      Serial.printf("Connected to: %s\n", payload);
      ws_manager.print("Hello Server");
      break;

    case WStype_TEXT:
      Serial.printf("Message from server: %s\n", payload);
      break;

    case WStype_BIN:
      Serial.printf("Binary message received (%d bytes)\n", length);
      break;
  }
}

void setup() {  
  ws_manager.init(SSID, PASS, "192.168.137.1", (uint16_t) 8040);
}

void loop() {
  ws_manager.auto_reconnect();
}