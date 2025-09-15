#include <Arduino.h>
#include <WebSocketManager.h>

#include <env.h>

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
  Serial.begin(9600);
  Serial.println("Connecting..");
  ws_manager.init(ENV_WIFI_SSID, ENV_WIFI_PASS, ENV_WS_ADDR, (uint16_t) 8040);
  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Default Gateway: ");
  Serial.println(WiFi.gatewayIP());
}

void loop() {
  ws_manager.loop();
}