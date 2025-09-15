#pragma once

#include <Arduino.h>


class ConfigurationManager
{
public:
  static bool is_storage_initialized;
  static bool is_ble_active;

  /**
   * @brief Used for initialize configuration storage system
   * @attention This should be called before using any functions
   * @note You could put this in the void setup()
   */
  static void init_storage();

  /**
   * @brief Used to start configuration process including BLE server setup
   * @note You could put this if you want to go configuration mode
   * @attention Use this after calling init_storage() function
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
   * @brief Used to get WiFi SSID from persistance storage
   * @attention Use this after calling init_storage() function
   * @return WiFi SSID as string if exists,
   *         empty string otherwise
   */
  static String get_wifi_ssid();

  /**
   * @brief Used to get WiFi password from persistance storage
   * @attention Use this after calling init_storage() function
   * @return WiFi password as string if exists,
   *         empty string otherwise
   */
  static String get_wifi_pass();
  
  

  /**
   * @brief Used to set WiFi SSID to the persistance storage
   * @attention Use this after calling init_storage() function
   */
  static void set_wifi_ssid(String new_ssid);

  /**
   * @brief Used to set WiFi password to the persistance storage
   * @attention Use this after calling init_storage() function
   */
  static void set_wifi_pass(String new_pass);
};
