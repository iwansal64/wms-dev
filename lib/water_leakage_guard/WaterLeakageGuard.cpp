#include <WaterLeakageGuard.h>
#include <Arduino.h>
#include <env.h>

#define TIME_BEFORE_LEAKAGE_DEDUCTION_FINALIZED 1000  // It needs to wait 1 second to deduct if the water leaked or not 
#define WATER_LEAKAGE_TRIGGER_THRESHOLD 2             // It needs to be 2 liter / minutes different for leakage detection

void WaterLeakageGuard::add_sensor(uint8_t sensor_pin, uint8_t buzzer_pin) {
  this->flow_sensors.push_back(FlowSensor(sensor_pin, buzzer_pin, 7.5));
  #ifdef SHOW_INFO
  Serial.println("[WaterLeakageGuard] Successfully added new flow sensor");
  #endif
}


void WaterLeakageGuard::add_sensor(uint8_t sensor_pin, uint8_t buzzer_pin, float calibration_factor) {
  this->flow_sensors.push_back(FlowSensor(sensor_pin, buzzer_pin, calibration_factor));
  #ifdef SHOW_INFO
  Serial.println("[WaterLeakageGuard] Successfully added new flow sensor");
  #endif
}


int8_t WaterLeakageGuard::get_water_leak_value() {
  if(this->flow_sensors.size() < 2) {

    #ifdef SHOW_WARN
    Serial.println("[WaterLeakageGuard] Number of flow sensors aren't sufficient to perform monitoring!");
    #endif

    return -1;
  }

  uint8_t current_sensor_index = this->flow_sensors.size() - 1;

  while(this->is_leaked(this->flow_sensors[0], this->flow_sensors[current_sensor_index])) {
    current_sensor_index--;
    if(current_sensor_index <= 0) {
      break;
    }
  }

  uint8_t water_leak_value = (current_sensor_index == this->flow_sensors.size() - 1) ? 0 : current_sensor_index + 1;
  
  if(this->previous_water_leak_value != water_leak_value) {
    uint64_t current_time = millis();

    if(this->deducted_water_leak_value != water_leak_value) {
      this->deducted_water_leak_value = water_leak_value;
      this->last_deducted_leak_change_time = current_time;
    }

    if(current_time - this->last_deducted_leak_change_time > TIME_BEFORE_LEAKAGE_DEDUCTION_FINALIZED) {
      this->previous_water_leak_value = water_leak_value;
      return water_leak_value;
    }
  }

  return this->previous_water_leak_value;
}

bool WaterLeakageGuard::is_leaked(FlowSensor sensor_1, FlowSensor sensor_2) {
  float first_flow_rate = sensor_1.get_flow_rate();
  float second_flow_rate = sensor_2.get_flow_rate();
  float max_flow_rate = max(first_flow_rate, second_flow_rate);
  float min_flow_rate = min(first_flow_rate, second_flow_rate);

  // If the one flowrate is less than the half of another
  // if(min_flow_rate < max_flow_rate / 2) {
  //   return true;
  // }

  // If there's flow in one sensor and no flow in another
  if(abs(first_flow_rate - second_flow_rate) > WATER_LEAKAGE_TRIGGER_THRESHOLD) {
    return true;
  }

  return false;
}  

float WaterLeakageGuard::get_flow_value(uint8_t sensor_index) {
  if(sensor_index > this->flow_sensors.size()) return -1;
  
  return this->flow_sensors[sensor_index].get_flow_rate();
}

void WaterLeakageGuard::run() {
  if(this->flow_sensors.size() == 0) return;
  
  for(FlowSensor &flow_sensor : flow_sensors) {
    flow_sensor.update(); // Update all flow sensors
  }
}

float WaterLeakageGuard::get_average_flow_value() {
  if(this->flow_sensors.size() == 0) return 0;
  
  float average_flow_sensor = 0;
  for(FlowSensor &flow_sensor : flow_sensors) {
    average_flow_sensor += flow_sensor.get_flow_rate();
  }

  return average_flow_sensor / this->flow_sensors.size();
}

void WaterLeakageGuard::set_warning(uint8_t sensor_index, uint8_t value) {
  if(sensor_index > this->flow_sensors.size()) return;

  this->flow_sensors[sensor_index].buzz(value);
}

void WaterLeakageGuard::clear_warning() {
  for(FlowSensor flow_sensor : this->flow_sensors) {
    flow_sensor.buzz(0);
  }
}