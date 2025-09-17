//? ------> Libraries
#include <Arduino.h>
#include <WebSocketManager.h>
#include <ConfigurationManager.h>

//? ------> Enviroment Variables
#include <env.h> // Create a new header file to make this works in lib/env/env.h

//? ------> Pins
#define CONFIG_SWITCH_PIN 10

//? ------> Other 
#define INTERVAL_PER_DATA 2000

#define NORMAL_MODE 0
#define CONFIGURATION_MODE 1


//? ------> Variables
uint64_t last_time_update_data = 0;
WebSocketManager ws_manager;
uint8_t mode = CONFIGURATION_MODE;
bool wifi_configurated = false;


//? ------> Function Definitions
void on_websocket_data(WEBSOCKET_DATA);
void check_water_leakage();


//? ------> Main Program
void setup() {  
  // Setup Serial
  Serial.begin(115200);

  // Setup Pins
  pinMode(CONFIG_SWITCH_PIN, INPUT_PULLDOWN);

  // Get the WiFi SSID and PASS from persistance storage
  String ssid = "";
  String pass = "";
  ConfigurationManager::get_wifi_creds(ssid, pass);

  // If the WiFi credentials is not empty
  if(!ssid.isEmpty() && !pass.isEmpty())
  {
    if(mode == NORMAL_MODE) {
      wifi_configurated = true;
  
      #ifdef SHOW_INFO
      Serial.print("[WIFI] Connecting to ");
      Serial.println(ssid);
      #endif
    
      // Begin connection to WiFi with SSID and PASS from configuration
      WiFi.begin(ssid, pass);
      while(WiFi.status() != WL_CONNECTED) delay(500);
    
      #ifdef SHOW_INFO
      Serial.println("[WIFI] Connected!");
      #endif
    
      #ifdef SHOW_DEBUG
      Serial.print("[WiFi] IP Address: ");
      Serial.println(WiFi.localIP());
      Serial.print("[WiFi] Default Gateway: ");
      Serial.println(WiFi.gatewayIP());
      #endif
  
      // Begin connection to WebSocket
      ws_manager.init(ENV_WS_ADDR, (uint16_t) 8040);
    }
    else {
      wifi_configurated = false;
      Serial.println("[WIFI] Configuration Mode!");
    }
  }
  else {
    wifi_configurated = false;
    Serial.println("[WIFI] There's no WiFi Configuration!");
  }
}

void loop() {
  // If it's on configuration mode :)
  if(mode == CONFIGURATION_MODE) {
    // Start configuration mode :D
    ConfigurationManager::start_config_mode();
  }
  // If it's not it's on normal mode.. perhaps :]
  else if(mode == NORMAL_MODE) {
    // Check if WiFi is configurated
    if(wifi_configurated) {
      // Looping web socket connection to make it works.. welll :D
      ws_manager.loop();
    
      // Check water leakage per some time :>
      uint64_t elapsed = millis() - last_time_update_data;
      if(elapsed > INTERVAL_PER_DATA) {
        #ifdef SHOW_INFO
        Serial.println("[MAIN] Checking Water Leakage");
        #endif
        check_water_leakage();
        last_time_update_data = millis();
      }
    }
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
  // Wait to connect to Web Socket manager first
  ws_manager.wait_to_connect();
  
  // Get random data
  uint8_t water_leak_value = random(0, 4);

  // Prepare the data
  ws_manager.put(String("leak="));
  ws_manager.put(water_leak_value);

  // Send the data
  bool result = ws_manager.launch();
}