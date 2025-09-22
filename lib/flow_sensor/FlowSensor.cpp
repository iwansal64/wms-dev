#include <FlowSensor.h>
#include <Arduino.h>
#include <env.h>

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED; //? Used to get the KEY to LOCK the freaking VOLATILE CHANGES happening in all of the program


FlowSensor::FlowSensor(uint8_t sensor_pin, uint8_t buzzer_pin, float calibration_factor) {
  this->sensor_pin = sensor_pin;
  this->buzzer_pin = buzzer_pin;
  this->calibration_factor = calibration_factor;

  pinMode(buzzer_pin, OUTPUT);
  pinMode(sensor_pin, INPUT_PULLUP);

  attachInterruptArg(digitalPinToInterrupt(sensor_pin), this->isrRouter, this, FALLING);
}

//? Interruption handlers
void IRAM_ATTR FlowSensor::handlePulse() {
  this->pulse_count++;
}

void FlowSensor::isrRouter(void* arg) {
  FlowSensor* self = static_cast<FlowSensor*>(arg);
  self->handlePulse();
}

//? Flow Rate and Total litres Calculation
void FlowSensor::update() {
  uint64_t current_time = millis();
  uint64_t elapsed = current_time - this->last_time;

  if (elapsed >= 1000) {  // updates every seconds
    detachInterrupt(digitalPinToInterrupt(this->sensor_pin));
    uint32_t count = this->pulse_count;
    this->pulse_count = 0;

    float frequency = (1000.0 / elapsed) * count;
    this->flow_rate = frequency / this->calibration_factor;
    this->total_litres += (this->flow_rate / 60.0f) * (elapsed / 1000.0f);

    this->last_time = current_time;
    attachInterruptArg(digitalPinToInterrupt(this->sensor_pin), this->isrRouter, this, FALLING);
  }
}



//? Getter Setter
float FlowSensor::get_flow_rate() const {
  return this->flow_rate;
}

float FlowSensor::get_total_litres() const {
  return this->total_litres;
}



//? Notify
void FlowSensor::buzz(uint8_t value) {
  digitalWrite(this->buzzer_pin, value);
}
