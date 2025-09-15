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
NimBLECharacteristic *wifi_ssid_characteric;
NimBLECharacteristic *wifi_pass_characteric;

// UUID for characteristics and services of BLE
static BLEUUID wifi_pass_uuid(ENV_WIFI_PASS_BLE_UUID);
static BLEUUID wifi_ssid_uuid(ENV_WIFI_SSID_BLE_UUID);
static BLEUUID wifi_service_uuid(ENV_WIFI_SERVICE_BLE_UUID);


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


// Configuration Manager Static Variables
bool ConfigurationManager::is_storage_initialized = false;
bool ConfigurationManager::is_ble_active = false;


// Configuration Manager Static Functions
void ConfigurationManager::init_storage() {
  if(ConfigurationManager::is_storage_initialized) return;
  preferences.begin("wms-dev", false);
  ConfigurationManager::is_storage_initialized = true;
}

bool ConfigurationManager::start_config_mode() {
  if(ConfigurationManager::is_ble_active) return false;
  // Setting up device name
  std::string device_name = "ESP32-";
  device_name += ENV_DEVICE_ID;
  
  // Initialize BLE in the ESP32
  #ifdef SHOW_INFO
  Serial.println("[Configuration] Starting BLE Device");
  #endif

  NimBLEDevice::init(device_name);

  // Create BLE Server
  BLEServer *ble_server = NimBLEDevice::createServer();

  // Create BLE Service
  BLEService *ble_wifi_service = ble_server->createService(wifi_service_uuid);

  // Create BLE Characteristics for containing SSID and password
  wifi_ssid_characteric = ble_wifi_service->createCharacteristic(wifi_ssid_uuid, DEFAULT_BLE_PROPERTIES);
  wifi_pass_characteric = ble_wifi_service->createCharacteristic(wifi_pass_uuid, DEFAULT_BLE_PROPERTIES);

  // Setting BLE Listener
  wifi_ssid_characteric->setCallbacks(new LambdaCharacteristicCallback<void (*)(NimBLECharacteristic*, NimBLEConnInfo&)>(
    [](NimBLECharacteristic *characteristics, NimBLEConnInfo& connection_info) {
      String value = characteristics->getValue();
      ConfigurationManager::set_wifi_ssid(value);
    }
  ));
  
  wifi_pass_characteric->setCallbacks(new LambdaCharacteristicCallback<void (*)(NimBLECharacteristic*, NimBLEConnInfo&)>(
    [](NimBLECharacteristic *characteristics, NimBLEConnInfo& connection_info) {
      String value = characteristics->getValue();
      ConfigurationManager::set_wifi_pass(value);
    }
  ));

  // Start the BLE server
  ble_wifi_service->start();
  NimBLEAdvertising* ble_advertising = NimBLEDevice::getAdvertising();
  ble_advertising->addServiceUUID(ble_wifi_service->getUUID());
  NimBLEDevice::startAdvertising();

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


String ConfigurationManager::get_wifi_ssid() {
  return preferences.getString("wifi-ssid", "");
}

String ConfigurationManager::get_wifi_pass() {
  return preferences.getString("wifi-pass", "");
}


void ConfigurationManager::set_wifi_ssid(String new_ssid) {
  #ifdef SHOW_INFO
  Serial.println("[Configuration] Saving new WiFi SSID...");
  #endif
  
  preferences.putString("wifi-ssid", new_ssid);

  #ifdef SHOW_INFO
  Serial.println("[Configuration] New WiFi SSID saved!");
  #endif
}

void ConfigurationManager::set_wifi_pass(String new_pass) {
  #ifdef SHOW_INFO
  Serial.println("[Configuration] Saving new WiFi password...");
  #endif

  preferences.putString("wifi-pass", new_pass);

  #ifdef SHOW_INFO
  Serial.println("[Configuration] New WiFi password saved!");
  #endif
}
