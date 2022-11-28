#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace hlw8012 {

enum HLW8012InitialMode { HLW8012_INITIAL_MODE_CURRENT = 0, HLW8012_INITIAL_MODE_VOLTAGE };

enum HLW8012SensorModels {
  HLW8012_SENSOR_MODEL_HLW8012 = 0,
  HLW8012_SENSOR_MODEL_CSE7759,
  HLW8012_SENSOR_MODEL_BL0937
};

// copied from pulse_width component
/// Store data in a class that doesn't use multiple-inheritance (vtables in flash)
class HLWSensorStore {
 public:
  void setup(InternalGPIOPin *pin) {
    pin->setup();
    this->pin_ = pin->to_isr();
    this->last_rise_ = micros();
    pin->attach_interrupt(&HLWSensorStore::gpio_intr, this, gpio::INTERRUPT_ANY_EDGE);
  }
  static void gpio_intr(HLWSensorStore *arg);
  uint32_t get_pulse_width_us() const { return this->last_width_; }
  float get_pulse_width_s() const { return this->last_width_ / 1e6f; }
  uint32_t get_last_rise() const { return last_rise_; }

  uint32_t get_last_period() const { return this->last_period_; }
  bool get_valid() const { return this->valid_; }
  void reset();

 protected:
  ISRInternalGPIOPin pin_;
  volatile uint32_t last_width_{0};
  volatile uint32_t last_period_{0};
  volatile uint32_t last_rise_{0};

  volatile bool skip_{true};
  volatile bool valid_{false};
};



class HLW8012Component : public PollingComponent {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;

  void set_initial_mode(HLW8012InitialMode initial_mode) {
    current_mode_ = initial_mode == HLW8012_INITIAL_MODE_CURRENT;
  }
  void set_sensor_model(HLW8012SensorModels sensor_model) { sensor_model_ = sensor_model; }
  void set_change_mode_every(uint32_t change_mode_every) { change_mode_every_ = change_mode_every; }
  void set_current_resistor(float current_resistor) { current_resistor_ = current_resistor; }
  void set_voltage_divider(float voltage_divider) { voltage_divider_ = voltage_divider; }
  void set_sel_pin(GPIOPin *sel_pin) { sel_pin_ = sel_pin; }
  void set_cf_pin(InternalGPIOPin *cf_pin) { cf_pin_ = cf_pin; }
  void set_cf1_pin(InternalGPIOPin *cf1_pin) { cf1_pin_ = cf1_pin; }
  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { voltage_sensor_ = voltage_sensor; }
  void set_current_sensor(sensor::Sensor *current_sensor) { current_sensor_ = current_sensor; }
  void set_power_sensor(sensor::Sensor *power_sensor) { power_sensor_ = power_sensor; }

 protected:
  float period_to_power(float period_in);
  float period_to_current(float period_in);
  float period_to_voltage(float period_in);
  float period_to_hz(float period_in);
  void change_mode();

  uint32_t nth_value_{0};
  bool current_mode_{false};
  uint32_t change_mode_at_{0};
  uint32_t change_mode_every_{8};
  float current_resistor_{0.001};
  float voltage_divider_{2351};
  HLW8012SensorModels sensor_model_{HLW8012_SENSOR_MODEL_HLW8012};
  uint64_t cf_total_pulses_{0};
  GPIOPin *sel_pin_;
  InternalGPIOPin *cf_pin_;
  HLWSensorStore cf_store_;
  InternalGPIOPin *cf1_pin_;
  HLWSensorStore cf1_store_;
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *power_sensor_{nullptr};

  float voltage_multiplier_{0.0f};
  float current_multiplier_{0.0f};
  float power_multiplier_{0.0f};
};

}  // namespace hlw8012
}  // namespace esphome
