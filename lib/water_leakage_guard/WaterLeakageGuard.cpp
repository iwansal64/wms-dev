#include <WaterLeakageGuard.h>
#include <Arduino.h>
#include <env.h>

void WaterLeakageGuard::add_sensor(uint8_t sensor_pin) {
  this->flow_sensors.push_back(FlowSensor(sensor_pin, 7.5));
  #ifdef SHOW_INFO
  Serial.println("[WaterLeakageGuard] Successfully added new flow sensor");
  #endif
}

int8_t WaterLeakageGuard::monitor() {
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
  
  return current_sensor_index == this->flow_sensors.size() - 1 ? 0 : current_sensor_index + 1;
}

bool WaterLeakageGuard::is_leaked(FlowSensor sensor_1, FlowSensor sensor_2) {
  float first_flow_rate = sensor_1.get_flow_rate();
  float second_flow_rate = sensor_2.get_flow_rate();
  // If there's difference in 10 litres
  if(second_flow_rate - 10 > first_flow_rate || first_flow_rate > second_flow_rate + 10) {
    return true;
  }

  return false;
}