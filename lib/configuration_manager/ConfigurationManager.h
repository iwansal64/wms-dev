#pragma once

#include <Arduino.h>


class ConfigurationManager
{
public:
  static bool is_storage_open;
  static bool is_ble_active;

  /**
   * @brief Used to start configuration process including BLE server setup
   * @note You could put this if you want to go configuration mode
   * @return true if success, otherwise.. you know it :)
   */
  static bool start_config_mode();
  
  /**
   * @brief Used to stop BLE server
   * @note You could put this if you want to go configuration mode
   * @return true if success, otherwise.. you know it :)
   */
  static bool stop_config_mode();


  
  /**
   * @brief Used to get WiFi SSID and PASSWORD from persistance storage
   * @param ssid_container Used to contain WiFi SSID
   * @param pass_container Used to contain WiFi password
   */
  static void get_wifi_creds(String &ssid_container, String &pass_container);

  

  /**
   * @brief Used to set WiFi SSID to the persistance storage
   * @attention Use this after calling init_storage() function
   */
  static void set_wifi_ssid(String &new_ssid);

  /**
   * @brief Used to set WiFi password to the persistance storage
   * @attention Use this after calling init_storage() function
   */
  static void set_wifi_pass(String &new_pass);
};
