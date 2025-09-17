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


// Configuration Manager Static Functions
bool ConfigurationManager::start_config_mode() {
  if(ConfigurationManager::is_ble_active) return false;
  // Initialize BLE in the ESP32
  #ifdef SHOW_INFO
  Serial.println("[Configuration] Starting BLE Device");
  #endif

  NimBLEDevice::init(ENV_DEVICE_NAME);

  // Create BLE Server
  ble_server = NimBLEDevice::createServer();
  ble_server->setCallbacks(&serverCallbacks);

  // Create BLE Service
  NimBLEService *ble_wifi_service = ble_server->createService(wifi_service_uuid);

  // Create BLE Characteristics for containing SSID and password
  wifi_ssid_characteric = ble_wifi_service->createCharacteristic(ENV_WIFI_SSID_BLE_UUID, DEFAULT_BLE_PROPERTIES);
  wifi_pass_characteric = ble_wifi_service->createCharacteristic(ENV_WIFI_PASS_BLE_UUID, DEFAULT_BLE_PROPERTIES);
  wifi_log_characteric = ble_wifi_service->createCharacteristic(ENV_WIFI_LOG_BLE_UUID, DEFAULT_BLE_PROPERTIES);

  // Setting BLE Listener
  wifi_ssid_characteric->setCallbacks(new LambdaCharacteristicCallback<void (*)(NimBLECharacteristic*, NimBLEConnInfo&)>(
    [](NimBLECharacteristic *characteristics, NimBLEConnInfo& connection_info) {
      String value = String(characteristics->getValue());
      ConfigurationManager::set_wifi_ssid(value);
      wifi_log_characteric->setValue("WiFi SSID saved");
    }
  ));
  
  wifi_pass_characteric->setCallbacks(new LambdaCharacteristicCallback<void (*)(NimBLECharacteristic*, NimBLEConnInfo&)>(
    [](NimBLECharacteristic *characteristics, NimBLEConnInfo& connection_info) {
      String value = String(characteristics->getValue());
      ConfigurationManager::set_wifi_pass(value);
      wifi_log_characteric->setValue("WiFi password saved");
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
  bool result = NimBLEDevice::deinit(true);
  if(result) {
    ConfigurationManager::is_ble_active = false;
  }
  return result;
}


void ConfigurationManager::get_wifi_creds(String &ssid_container, String &pass_container) {
  preferences.begin("wms-dev", true);

  ssid_container = preferences.getString("wifi-ssid", "");
  pass_container = preferences.getString("wifi-pass", "");
  
  preferences.end();
}


void ConfigurationManager::set_wifi_ssid(String &new_ssid) {
  preferences.begin("wms-dev", false);

  #ifdef SHOW_INFO
  Serial.println("[Configuration] Saving new WiFi SSID...");
  #endif
  
  preferences.putString("wifi-ssid", new_ssid);

  #ifdef SHOW_INFO
  Serial.println("[Configuration] New WiFi SSID saved!");
  #endif

  preferences.end();
}

void ConfigurationManager::set_wifi_pass(String &new_pass) {
  preferences.begin("wms-dev", false);

  #ifdef SHOW_INFO
  Serial.println("[Configuration] Saving new WiFi password...");
  #endif

  preferences.putString("wifi-pass", new_pass);

  #ifdef SHOW_INFO
  Serial.println("[Configuration] New WiFi password saved!");
  #endif
  
  preferences.end();
}
