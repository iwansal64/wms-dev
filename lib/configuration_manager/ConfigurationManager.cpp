// Required Libraries
#include <ConfigurationManager.h>
#include <Preferences.h>
#include <NimBLEDevice.h>

// Environment Variables
#include <env.h>


// Helper Definitions
#define DEFAULT_BLE_PROPERTIES NIMBLE_PROPERTY::READ |\
                               NIMBLE_PROPERTY::WRITE |\
                               NIMBLE_PROPERTY::NOTIFY


// Used for accessing persistence storage
Preferences preferences;


// BLE Characteristics
static NimBLECharacteristic *wifi_ssid_characteric;
static NimBLECharacteristic *wifi_pass_characteric;
static NimBLECharacteristic *wifi_log_characteric;

// UUID for characteristics and services of BLE
static BLEUUID wifi_ssid_uuid(ENV_WIFI_SSID_BLE_UUID);
static BLEUUID wifi_pass_uuid(ENV_WIFI_PASS_BLE_UUID);
static BLEUUID wifi_log_uuid(ENV_WIFI_LOG_BLE_UUID);
static BLEUUID wifi_service_uuid(ENV_WIFI_SERVICE_BLE_UUID);

// Static server
static NimBLEServer *ble_server;

// BLE Callbacks
template <typename F>
class LambdaCharacteristicCallback : public NimBLECharacteristicCallbacks {
  F func;
public:
  LambdaCharacteristicCallback(F f) : func(f) {}
  void onWrite(NimBLECharacteristic* pCharacteristics, NimBLEConnInfo& connInfo) override {
    func(pCharacteristics, connInfo);
  }
};

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *ble_server, NimBLEConnInfo &ble_conn_info) override {
    Serial.printf("[Bluetooth] Connected client with address: %s\n", ble_conn_info.getAddress().toString().c_str());
  }

  void onDisconnect(NimBLEServer *ble_server, NimBLEConnInfo &ble_conn_info, int reason) override {
    Serial.printf("[Bluetooth] Disconnected client with address: %s\n", ble_conn_info.getAddress().toString().c_str());
    NimBLEDevice::startAdvertising();
  }

  uint32_t onPassKeyDisplay() override {
    Serial.printf("[Bluetooth] Server Passkey Display.\n");
    return BLE_PASSKEY;
  }

  void onConfirmPassKey(NimBLEConnInfo& connInfo, uint32_t pass_key) override {
    Serial.printf("[Bluetooth] The passkey for number: %" PRIu32 "\n", pass_key);
    
    /** Inject false if passkeys don't match. */
    NimBLEDevice::injectConfirmPasskey(connInfo, pass_key == BLE_PASSKEY);
  }

  void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
    /** Check that encryption was successful, if not we disconnect the client */
    if (!connInfo.isEncrypted()) {
        NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
        Serial.print("[Bluetooth] Encrypt connection failed - disconnecting client\n");
        return;
    }

    Serial.printf("[Bluetooth] Secured connection to: %s\n", connInfo.getAddress().toString().c_str());
  }
} serverCallbacks;




// Configuration Manager Static Variables
bool ConfigurationManager::is_storage_open = false;
bool ConfigurationManager::is_ble_active = false;

String ConfigurationManager::data_chunked = "";


// Configuration Manager Static Functions
bool ConfigurationManager::start_config_mode() {
  if(ConfigurationManager::is_ble_active) return false;
  // Initialize BLE in the ESP32
  #ifdef SHOW_INFO
  Serial.println("[Configuration] Starting BLE Device");
  #endif

  NimBLEDevice::init(ENV_DEVICE_NAME);
  NimBLEDevice::setMTU(100);

  // Create BLE Server
  ble_server = NimBLEDevice::createServer();
  ble_server->setCallbacks(&serverCallbacks);

  // Create BLE Service
  NimBLEService *ble_wifi_service = ble_server->createService(wifi_service_uuid);

  // Create BLE Characteristics for containing SSID and password
  wifi_ssid_characteric = ble_wifi_service->createCharacteristic(ENV_WIFI_SSID_BLE_UUID, DEFAULT_BLE_PROPERTIES);
  wifi_pass_characteric = ble_wifi_service->createCharacteristic(ENV_WIFI_PASS_BLE_UUID, DEFAULT_BLE_PROPERTIES);
  wifi_log_characteric = ble_wifi_service->createCharacteristic(ENV_WIFI_LOG_BLE_UUID, DEFAULT_BLE_PROPERTIES);

  wifi_ssid_characteric->setValue((uint8_t *)"123456789012345678901234567890", 30); // Set the maximum BLE value to 30
  wifi_pass_characteric->setValue((uint8_t *)"123456789012345678901234567890", 30); // Set the maximum BLE value to 30

  // Setting BLE Listener
  wifi_ssid_characteric->setCallbacks(new LambdaCharacteristicCallback<void (*)(NimBLECharacteristic*, NimBLEConnInfo&)>(
    [](NimBLECharacteristic *characteristics, NimBLEConnInfo& connection_info) {
      String value = String(characteristics->getValue());
      Serial.printf("[Bluetooth] SSID Value: <%s>\n", value);

      if(value == "[") {
        ConfigurationManager::data_chunked = "";
      }
      else if(value == "]") {
        Serial.printf("[Bluetooth] SSID Final Value: <%s>\n", ConfigurationManager::data_chunked.c_str());
        ConfigurationManager::set_wifi_ssid(ConfigurationManager::data_chunked);
        wifi_log_characteric->setValue("WiFi SSID saved");
        ConfigurationManager::data_chunked = "";
      }
      else {
        ConfigurationManager::data_chunked += value;
      }
    }
  ));
  
  wifi_pass_characteric->setCallbacks(new LambdaCharacteristicCallback<void (*)(NimBLECharacteristic*, NimBLEConnInfo&)>(
    [](NimBLECharacteristic *characteristics, NimBLEConnInfo& connection_info) {
      String value = String(characteristics->getValue());
      
      Serial.printf("[Bluetooth] Pass Value: <%s>\n", value);

      if(value == "[") {
        ConfigurationManager::data_chunked = "";
      }
      else if(value == "]") {
        Serial.printf("[Bluetooth] Pass Final Value: <%s>\n", ConfigurationManager::data_chunked.c_str());
        ConfigurationManager::set_wifi_pass(ConfigurationManager::data_chunked);
        wifi_log_characteric->setValue("WiFi password saved");
        ConfigurationManager::data_chunked = "";
      }
      else {
        ConfigurationManager::data_chunked += value;
      }
    }
  ));

  // Start the BLE server
  ble_wifi_service->start();
  NimBLEAdvertising* ble_advertising = NimBLEDevice::getAdvertising();
  ble_advertising->setName(ENV_DEVICE_NAME);
  ble_advertising->addServiceUUID(ble_wifi_service->getUUID());
  ble_advertising->enableScanResponse(true);
  ble_advertising->start();


  #ifdef SHOW_INFO
  Serial.println("[Configuration] BLE Advertising Started!");
  #endif

  // Set BLE state to active if successfully initialized
  bool result = NimBLEDevice::isInitialized();
  if(result) {
    ConfigurationManager::is_ble_active = true;
  }
  return result;
}

bool ConfigurationManager::stop_config_mode() {
  // Check if the device is currently not active
  if(!ConfigurationManager::is_ble_active) return false;

  // De-initialized BLE Device and return the result
  bool result = NimBLEDevice::deinit(false);
  if(result) {
    ConfigurationManager::is_ble_active = false;
  }
  return result;
}


void ConfigurationManager::get_wifi_creds(String &ssid_container, String &pass_container) {
  preferences.begin("wms-dev", true);

  ssid_container = ConfigurationManager::get_string("wifi-ssid");
  pass_container = ConfigurationManager::get_string("wifi-pass");

  preferences.end();
}



void ConfigurationManager::set_wifi_ssid(String &new_ssid) {
  #ifdef SHOW_INFO
  Serial.println("[Configuration] Saving new WiFi SSID...");
  #endif
  
  ConfigurationManager::set_string("wifi-ssid", new_ssid);

  #ifdef SHOW_INFO
  Serial.println("[Configuration] New WiFi SSID saved!");
  #endif
}

void ConfigurationManager::set_wifi_pass(String &new_pass) {
  #ifdef SHOW_INFO
  Serial.println("[Configuration] Saving new WiFi password...");
  #endif

  ConfigurationManager::set_string("wifi-pass", new_pass);

  #ifdef SHOW_INFO
  Serial.println("[Configuration] New WiFi password saved!");
  #endif
}

void ConfigurationManager::set_string(const char* key, String &value) {
  preferences.begin("wms-dev", false);

  // Check if the value length is less than 16
  if(value.length() < 10) {
    preferences.putString(key, value);

    String key_ps = "ps-" + String(key);
    preferences.putShort(key_ps.c_str(), 0);
  }

  // If the value length is more than 16
  else {
    String current_value = "";
    uint8_t character_index = 0;
    uint8_t current_partition = 0;

    // Loop through characters of the value
    for(char c : value) {
      character_index++;
      current_value += c;
      
      // If the number of characters 10 or above 
      if(character_index >= 10) {

        // Save the part
        String key_partition = String(key) + "-" + String(current_partition);
        preferences.putString(key_partition.c_str(), current_value);
        Serial.printf("Partition Part-%d Info: %s\n", current_partition, current_value);
        
        current_value = "";
        character_index = 0;
        current_partition += 1;
      }
    }

    if(current_value != "") {
      String key_partition = String(key) + "-" + String(current_partition);
      preferences.putString(key_partition.c_str(), current_value);
      Serial.printf("Partition Part-%d Info: %s\n", current_partition, current_value);
      current_partition += 1;
    }

    String key_ps = "ps-" + String(key);
    preferences.putShort(key_ps.c_str(), current_partition);
  }
  
  preferences.end();
}

String ConfigurationManager::get_string(const char* key) {
  // Check if there's partition for it
  String key_ps = "ps-" + String(key);
  uint16_t partition_size_info = preferences.getShort(key_ps.c_str(), 0);
  Serial.printf("Partition Size Info: %d\n", partition_size_info);

  // If there's no partition
  if(partition_size_info == 0) {
    return preferences.getString(key, "");
  }

  // If there's partition
  else {
    String result = "";
    for(uint16_t partition_index = 0; partition_index < partition_size_info; partition_index++) {
      String key_partition = String(key) + "-" + String(partition_index);
      result += preferences.getString(key_partition.c_str(), "");
    }
    return result;
  }
}
