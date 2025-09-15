#include <FlowSensor.h>
#include <Arduino.h>
#include <env.h>

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; //? Used to get the KEY to LOCK the freaking VOLATILE CHANGES happening in all of the program


FlowSensor::FlowSensor(uint8_t pin, float calibration_factor) {
  this->pin = pin;
  this->calibration_factor = calibration_factor;

  pinMode(pin, INPUT_PULLUP);
  attachInterruptArg(digitalPinToInterrupt(pin), this->isrRouter, this, FALLING);
}

//? Flow Rate and Total litres Calculation
void IRAM_ATTR FlowSensor::handlePulse() {
  portENTER_CRITICAL_ISR(&mux); //? Safely use volatile variable in ISR
  this->pulse_count++;
  portEXIT_CRITICAL_ISR(&mux);
}

void FlowSensor::isrRouter(void* arg) {
  FlowSensor* self = static_cast<FlowSensor*>(arg);
  self->handlePulse();
}

void FlowSensor::update() {
  uint64_t current_time = millis();
  uint64_t elapsed = current_time - this->last_time;

  if (elapsed >= 1000) {  // updates every seconds
    portENTER_CRITICAL(&mux); //? Safely use volatile variable
    uint32_t count = this->pulse_count;
    this->pulse_count = 0;
    portEXIT_CRITICAL(&mux);

    float frequency = (1000.0 / elapsed) * count;
    this->flow_rate = frequency / this->calibration_factor;
    this->total_litres += (this->flow_rate / 60.0f) * (elapsed / 1000.0f);

    this->last_time = current_time;
  }
}



//? Getter Setter
float FlowSensor::get_flow_rate() const {
  return this->flow_rate;
}

float FlowSensor::get_total_litres() const {
  return this->total_litres;
}

