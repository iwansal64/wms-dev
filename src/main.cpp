#include <Arduino.h>
#include <WebSocketManager.h>

#include <env.h> // Create a new header file to make this works in lib/env/env.h

#define INTERVAL_PER_DATA 2000

uint64_t last_time_update_data = 0;

WebSocketManager ws_manager;

void on_websocket_data(WEBSOCKET_DATA);
void check_water_leakage();

void setup() {  
  Serial.begin(9600);

  #ifdef SHOW_DEBUG
  Serial.print("[WIFI] Connecting to ");
  Serial.println(ENV_WIFI_SSID);
  #endif

  WiFi.begin(ENV_WIFI_SSID, ENV_WIFI_PASS);
  while(WiFi.status() != WL_CONNECTED) delay(500);

  #ifdef SHOW_DEBUG
  Serial.println("[WIFI] Connected!");
  #endif
  
  #ifdef SHOW_DEBUG
  Serial.print("[WiFi] IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("[WiFi] Default Gateway: ");
  Serial.println(WiFi.gatewayIP());
  #endif
  
  ws_manager.init(ENV_WS_ADDR, (uint16_t) 8040);
}

void loop() {
  ws_manager.loop();
  uint64_t elapsed = millis() - last_time_update_data;
  if(elapsed > INTERVAL_PER_DATA) {
    #ifdef SHOW_INFO
    Serial.println("[MAIN] Checking Water Leakage");
    #endif
    check_water_leakage();
    Serial.println(elapsed);
    last_time_update_data = millis();
  }
}

void on_websocket_data(WEBSOCKET_DATA) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected from server");
      break;

    case WStype_CONNECTED:
      Serial.printf("Connected to: %s\n", payload);
      ws_manager.put("Hello Server");
      break;

    case WStype_TEXT:
      Serial.printf("Message from server: %s\n", payload);
      break;

    case WStype_BIN:
      Serial.printf("Binary message received (%d bytes)\n", length);
      break;
  }
}

void check_water_leakage() {
  ws_manager.wait_to_connect();
  
  uint8_t water_leak_value = random(0, 4);

  // Prepare the data
  ws_manager.put(String("leak="));
  ws_manager.put(water_leak_value);

  // Send the data
  bool result = ws_manager.launch();
}