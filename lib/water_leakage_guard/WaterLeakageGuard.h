/**
 * @author Ridwan Setiawan (iwansal64@gmail.com)
 * @date Wednesday, 17 September 2025
 * 
 */
#pragma once

#include <Arduino.h>
#include <vector>
#include <FlowSensor.h>

class WaterLeakageGuard
{
private:
  std::vector<FlowSensor> flow_sensors;

  /**
   * @brief Compare two Flow Sensors to know if there's leakage
   * @param sensor_1 is a sensor data used for anchor
   * @param sensor_2 is a sensor data to compare with
   * 
   * example usage:
   * @code
   * FlowSensor sensor_1 = FlowSensor(3); // Flow sensor in pin 3
   * FlowSensor sensor_2 = FlowSensor(4); // Flow sensor in pin 4
   * bool leaked = is_leaked(sensor_1, sensor_2);
   * if(leaked) {
   *    Serial.println("There's leakage between sensor 1 and sensor 2")
   * }
   * @endcode
   */
  bool is_leaked(FlowSensor sensor_1, FlowSensor sensor_2);
  
public:
  
  /**
   * @brief Add sensor pin to monitor
   * @param sensor_pin is the pin for water flow sensor to add
   * 
   * example usage:
   * @code
   * void setup() {
   *  water_leakage_guard.add_sensor(2); // add water flow sensor in pin 2
   *  water_leakage_guard.add_sensor(3); // add water flow sensor in pin 3
   * }
   * @endcode
   */
  void add_sensor(uint8_t sensor_pin);
  
  /**
   * @brief Used to update the data
   * @note Required to update the data of all sensors
   * 
   * example usage:
   * @code
   * 
   * void loop() {
   *  water_leakage_guard.run()
   * }
   * 
   * @endcode
   * 
   */
  void run();
  
  /**
   * @brief Monitor for water leakage
   * @attention Required minimal 2 sensors in storage and 2 sensors active
   * @note Use add_sensor function to add sensors
   * @return Return a sensor number that has leak for the pipe after it
   * 
   * example usage:
   * @code
   * 
   * void loop() {
   *  uint8_t water_leak = water_leakage_guard.get_water_leak_value();
   * 
   *  if(water_leak > 0) {
   *    Serial.printf("Pipe between sensor number #%d and number #%d has a leak!\n", water_leak, water_leak + 1)
   *  }
   *  else {
   *    Serial.print("There's no leak!\n");
   *  }
   * }
   * 
   * @endcode
   * 
   */
  int8_t get_water_leak_value();

  /**
   * @brief Used to get value
   * @attention Required minimal 1 sensor in storage and 1 sensor active
   * @note Use add_sensor function to add sensors
   * 
   * example usage:
   * @code
   * 
   * void loop() {
   *  float flow_value = water_leakage_guard.get_flow_value(0) // Get flow value for the first flow sensor value
   *  Serial.printf("Water flow: %f", flow_value);
   * }
   * 
   * @endcode
   * 
   */
  float get_flow_value(uint8_t sensor_index);
  
  /**
   * @brief Used to update the data
   * @note Required to update the data of all sensors
   * 
   * example usage:
   * @code
   * 
   * void loop() {
   *  water_leakage_guard.run()
   * }
   * 
   * @endcode
   * 
   */
  float get_average_flow_value();
};

