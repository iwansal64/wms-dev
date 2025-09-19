//? ------> [DEPS] Libraries
#include <Arduino.h>                // Basic built-in Arduino library
#include <WebSocketManager.h>       // Custom web socket handler library
#include <ConfigurationManager.h>   // Custom configuration through bluetooth library
#include <WaterLeakageGuard.h>      // Custom water leakage monitoring library
#include <HTTPUpdateServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>


//? ------> [ENV] Enviroment Variables

#include <env.h> // Create a new header file to make this works in lib/env/env.h

//? ------> [GPIO] GPIO Pins

#define CONFIG_SWITCH_PIN 22
#define WATER_FLOW_SENSOR_1_PIN 4
#define WATER_FLOW_SENSOR_2_PIN 2
#define CONFIG_INDICATOR_PIN 21
#define WIFI_INDICATOR_PIN 18
#define ERROR_INDICATOR_PIN 19

//? ------> [HELPER] Convinient Definitions

#define ON HIGH
#define OFF LOW

#define INTERVAL_PER_DATA 2000
#define INTERVAL_FOR_WIFI_INDICATOR 1000
#define INTERVAL_OTA_PROGRESS_UPDATE 1000

#define NORMAL_MODE 0
#define CONFIGURATION_MODE 1

#define CURRENT_MODE digitalRead(CONFIG_SWITCH_PIN)

//? ------> [VARIABLES] Data

// Web Socket data communication
uint64_t last_time_update_data = 0UL;
WebSocketManager ws_manager;
WaterLeakageGuard water_leakage_guard;

// WiFi states
bool wifi_configurated = false;
bool wifi_connected = false;
bool wifi_led_state = false;
uint64_t last_wifi_led_changed = 0UL;

// Mode handler
uint8_t previous_mode = NORMAL_MODE;

// Water Flow data
uint8_t previous_water_flow_value = 0;
uint8_t previous_water_leak_value = 0;

// Arduino OTA
WebServer sync_server(8080);
HTTPUpdateServer http_ota_updater;
uint64_t last_ota_progress_update = 0UL;

//? ------> [FUNCTIONS] Function Definitions

// Starting and ending functions
void start_normal_mode();
void start_configuration_mode();
void stop_configuration_mode();

// Looped Functions
void loop_normal_mode();

// Water Leakage Handler functions
void monitor_water_leakage();

// WiFi functions
void check_wifi_connection();
void connect_wifi(String ssid, String pass);

// Elegant OTA Listener
void on_ota_start();
void on_ota_progress(size_t current, size_t final);
void on_ota_end(bool success);

// Web Socket Listener 
void on_websocket_data(WEBSOCKET_DATA);

//? ------> [SETUP] Executed Once Program

void setup() {  
  /**
   * @brief Setup Serial
   * @note If you want more stable data, choose 9600 baud rate
   * @note If you want more data throughput, choose 115200 baud rate
   */
  Serial.begin(9600);


  // Setup Pins
  pinMode(CONFIG_SWITCH_PIN, INPUT_PULLDOWN);
  pinMode(CONFIG_INDICATOR_PIN, OUTPUT);
  pinMode(WIFI_INDICATOR_PIN, OUTPUT);
  pinMode(WIFI_INDICATOR_PIN, OUTPUT);
  

  // Setup Water Flow Sensors
  water_leakage_guard.add_sensor(WATER_FLOW_SENSOR_1_PIN);
  

  /**
   * @brief Check current configuration pin voltage
   * @brief If the voltage is high, go into a configuration mode
   * @brief else, go into a normal mode
   * 
   */
  if(CURRENT_MODE == CONFIGURATION_MODE) {
    // Start Configuration Mode
    start_configuration_mode();
  }
  else if(CURRENT_MODE == NORMAL_MODE) {
    // Start Normal Mode
    start_normal_mode();
  }

  // Setup WiFi
  WiFi.setHostname(ENV_DEVICE_NAME);
}

//? ------> [LOOP] Executed Continously Program



void loop() {
  // If it's on configuration mode :)
  if(CURRENT_MODE == CONFIGURATION_MODE && previous_mode == NORMAL_MODE) {
    // Start configuration mode :D
    start_configuration_mode();
    previous_mode = CONFIGURATION_MODE;
  }

  // If it's not it's on normal mode :]
  if(CURRENT_MODE == NORMAL_MODE) {
    // If previously configuration mode :)
    if(previous_mode == CONFIGURATION_MODE) {
      start_normal_mode();
      stop_configuration_mode();
    }

    // Loop normal mode :>
    loop_normal_mode();

    previous_mode = NORMAL_MODE;
  }

  // If WiFi is currently connected :]
  if(wifi_connected) {
    // Run the OTA updater :|
    sync_server.handleClient();
  }
}



//? ------> [FEATURES] Functions Contents

/**
 * @brief Check WiFi connection
 * 
 */
void check_wifi_connection() {  
  // WiFi is connected
  if(WiFi.status() == WL_CONNECTED) {
    // First time connected after not connected
    // On Connected to the WiFi
    if(!wifi_connected) {
      // Change wifi connection state to true
      wifi_connected = true;
      
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


      // Begin OTA Setup
      if(MDNS.begin("esp32")) {
        Serial.print("[OTA] mDNS started!\n");
      }

      http_ota_updater.setup(&sync_server);
      Serial.print("[OTA] Async OTA started!\n");

      sync_server.begin();
      Serial.print("[OTA] Web servers started!\n");
    }

    digitalWrite(WIFI_INDICATOR_PIN, 1);
    return;
  }

  wifi_connected = false;
  if(millis() - last_wifi_led_changed > INTERVAL_FOR_WIFI_INDICATOR) {
    // Flip WiFi indicator LED state
    digitalWrite(WIFI_INDICATOR_PIN, wifi_led_state);
    wifi_led_state = !wifi_led_state;
    last_wifi_led_changed = millis();
  }
}

/**
 * @brief Connect to WiFi
 * @param SSID WiFi SSID that you want to connect
 * @param PASS WiFi password
 * 
 */
void connect_wifi(String SSID, String PASS) {
  WiFi.begin(SSID.c_str(), PASS.c_str());
}

/**
 * @brief Web Socket on event callback function
 * @details When the data is sent through web socket connection from the web socket server, this function gets called
 *          and it gets the data from the server.
 * @attention This function should be inside the web socket listen function
 * 
 * @example
 * WebSocketManager ws_manager = WebSocketManager();
 * 
 * void setup() {
 *  ws_manager.listen(on_websocket_data) // <- Use like this 
 * }
 * 
 */
void on_websocket_data(WEBSOCKET_DATA) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("[WebSocket] Disconnected from server");
      break;

    case WStype_CONNECTED:
      Serial.printf("[WebSocket] Connected to: %s\n", payload);
      break;

    case WStype_TEXT:
      Serial.printf("[WebSocket] Message from server: %s\n", payload);
      break;

    case WStype_BIN:
      Serial.printf("[WebSocket] Binary message received (%d bytes)\n", length);
      break;
  }
}

/**
 * @brief Starting normal mode
 * @attention This function should be called when starting normal mode
 * 
 */
void start_normal_mode() {
  // Get the WiFi SSID and PASS from persistance storage
  String ssid = "";
  String pass = "";
  ConfigurationManager::get_wifi_creds(ssid, pass);
  Serial.printf("SSID[%s] | PASS[%s]\n", ssid, pass);

  
  // If the WiFi credentials is empty
  if(ssid.isEmpty() || pass.isEmpty())
  {
    wifi_configurated = false;
    Serial.println("[WIFI] There's no WiFi Configuration!");
    return;
  }
  
  
  #ifdef SHOW_INFO
  Serial.printf("[WIFI] Connecting to: %s\n", ssid.c_str());
  Serial.printf("[WIFI] %s\n", pass.c_str());
  #endif

  // Begin connection to WiFi with SSID and PASS from configuration
  connect_wifi(ssid, pass);

  wifi_configurated = true;
}

/**
 * @brief Start configuration mode
 * @details
 * 1. Start BLE device
 * 2. Listen to BLE changes
 * 3. Update configuration from data through BLE
 * 
 */
void start_configuration_mode() {
  digitalWrite(CONFIG_INDICATOR_PIN, ON);
  digitalWrite(WIFI_INDICATOR_PIN, OFF);
  ConfigurationManager::start_config_mode();
}

void stop_configuration_mode() {
  digitalWrite(CONFIG_INDICATOR_PIN, OFF);
  ConfigurationManager::stop_config_mode();
}



/**
 * @brief Loop in normal mode
 * @attention This function should be called inside loop function
 *
 */
void loop_normal_mode() {
  // Check if WiFi is connected
  if(wifi_connected) {
    // Looping web socket connection to make it works smoothly :D
    ws_manager.loop();
  }

  // Check water leakage per some time :>
  uint64_t elapsed = millis() - last_time_update_data;
  if(elapsed > INTERVAL_PER_DATA) {
    #ifdef SHOW_INFO
    Serial.println("[MAIN] Checking Water Leakage");
    #endif

    monitor_water_leakage();
    last_time_update_data = millis();
  }
    
  // Update the sensors data
  water_leakage_guard.run();

  // Check WiFi connection
  check_wifi_connection();
}


/**
 * @brief Check for water leakage
 * @attention This function should be called after initializing Water Leakage Guard instance!
 * @note Update the data if there's changes
 * 
 */
void monitor_water_leakage() {
  //? UPDATING AVERAGE FLOW VALUE
  // Get average flow value
  float current_water_flow_value = water_leakage_guard.get_average_flow_value();
  Serial.printf("Water Flow: %f litre / minute\n", current_water_flow_value);
  
  
  //? UPDATING WATER LEAK VALUE
  // Get water leak value
  int8_t current_water_leak_value = water_leakage_guard.get_water_leak_value();
  // int8_t current_water_leak_value = random(0, 4);
  Serial.printf("Water Leak: %d\n", current_water_leak_value);
  

  if(!wifi_connected) return;
  //? UPDATE TO WEB SOCKET
  // If the data changed, update to the websocket
  if(current_water_flow_value != previous_water_flow_value) {

    // Prepare the data for flow value
    ws_manager.put(String("aflow="));
    ws_manager.put(current_water_flow_value);
    
    // Send the data
    bool result = ws_manager.launch();

    previous_water_flow_value = current_water_flow_value;
  }

  // If the data changed, update to the websocket
  if(current_water_leak_value != previous_water_leak_value && current_water_leak_value != -1) {
  
    // Prepare the data for flow value
    ws_manager.put(String("leak="));
    ws_manager.put(current_water_leak_value);
  
    // Send the data
    bool result = ws_manager.launch();

    previous_water_leak_value = current_water_leak_value;
  }
}


/**
 * @brief On OTA firmware update start
 * 
 */
void on_ota_start() {
  Serial.print("[OTA] Begin to upgrade firmware\n");
}

/**
 * @brief On OTA firmware update progress
 * 
 */
void on_ota_progress(size_t current, size_t final) {
  if(millis() - last_ota_progress_update > INTERVAL_OTA_PROGRESS_UPDATE) {
    Serial.printf("[OTA] Progress : %u bytes | Final : %u bytes\r", current, final);
    last_ota_progress_update = millis();
  }
}

/**
 * @brief On OTA firmware update finished
 * 
 */
void on_ota_end(bool success) {
  if(success) {
    Serial.print("[OTA] Successfully upgrade firmware\n");
  }
  else {
    Serial.print("[OTA] Upgrade firmware failed... IDK WHY :(\n");
  }
}