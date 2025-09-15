#pragma once

#include <Arduino.h>
#include <vector>

class FlowSensor
{
public:
  uint8_t pin;
  uint8_t error;
  
  void IRAM_ATTR handlePulse();
  FlowSensor(uint8_t pin, float calibration_factor);
  float get_flow_rate() const;
  float get_total_litres() const;
  void update();
  
private:
  float calibration_factor;

  volatile uint32_t pulse_count;
  uint64_t last_time;
  float flow_rate;
  float total_litres;

  static void IRAM_ATTR isrRouter(void* arg);
};