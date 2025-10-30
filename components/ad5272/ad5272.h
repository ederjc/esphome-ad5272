/*
 * AD5272 Digital Potentiometer Component for ESPHome
 * 
 * This component provides ESPHome integration for the AD5272/AD5274 
 * digital potentiometer using I2C communication.
 */

#pragma once

#include "esphome/core/component.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace ad5272 {

// AD5272/AD5274 I2C Addresses (based on ADDR pin connection)
static const uint8_t AD5274_BASE_ADDR_GND = 0x2F;   // ADDR connected to GND
static const uint8_t AD5274_BASE_ADDR_VDD = 0x2C;   // ADDR connected to VDD  
static const uint8_t AD5274_BASE_ADDR_FLOAT = 0x2E; // ADDR floating (unipolar power only)

// Command constants from the datasheet
static const uint8_t AD5274_COMMAND_NOP = 0x00;
static const uint8_t AD5274_RDAC_WRITE = 0x01;
static const uint8_t AD5274_RDAC_READ = 0x02;
static const uint8_t AD5274_50TP_WRITE = 0x03;
static const uint8_t AD5274_RDAC_REFRESH = 0x04;
static const uint8_t AD5274_50TP_WIPER_READ = 0x05;
static const uint8_t AD5274_50TP_LAST_USED = 0x06;
static const uint8_t AD5274_CONTROL_WRITE = 0x07;
static const uint8_t AD5274_CONTROL_READ = 0x08;
static const uint8_t AD5274_SHUTDOWN = 0x09;

// Control bit constants
static const uint8_t AD5274_50TP_WRITE_ENABLE = 0x01;
static const uint8_t AD5274_RDAC_WIPER_WRITE_ENABLE = 0x02;
static const uint8_t AD5274_RDAC_CALIB_DISABLE = 0x04;
static const uint8_t AD5274_50TP_WRITE_SUCCESS = 0x08;

class AD5272Component : public Component, public i2c::I2CDevice {
 public:
  // Component lifecycle methods
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

  // Configuration methods
  void set_max_resistance(float max_resistance) { max_resistance_ = max_resistance; }

  // Main functionality methods
  bool set_resistance(float target_ohms);
  float get_current_resistance();
  uint16_t get_current_wiper_position();
  bool is_device_available();
  std::string get_status_message();

  // Low-level I2C communication methods
  bool command_write(uint8_t command, uint16_t data);
  int16_t command_read(uint8_t command, uint8_t data = 0x00);
  bool unlock_rdac();
  bool verify_rdac_unlock();

 protected:
  float max_resistance_ = 20000.0;
  uint16_t current_wiper_ = 512;
  bool device_available_ = false;
  std::string status_message_ = "Initializing...";
  uint32_t last_read_time_ = 0;

  // Helper methods
  uint16_t resistance_to_wiper(float resistance_ohms);
  float wiper_to_resistance(uint16_t wiper_position);
  void update_status(const std::string& message);
  bool check_device_presence();
};

// Sensor class for resistance and wiper position
class AD5272Sensor : public sensor::Sensor, public Component {
 public:
  void set_parent(AD5272Component *parent) { parent_ = parent; }
  void set_sensor_type(const std::string &type) { sensor_type_ = type; }
  void update() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  AD5272Component *parent_;
  std::string sensor_type_;
};

// Binary sensor for device connectivity
class AD5272BinarySensor : public binary_sensor::BinarySensor, public Component {
 public:
  void set_parent(AD5272Component *parent) { parent_ = parent; }
  void update() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  AD5272Component *parent_;
};

// Text sensor for status messages
class AD5272TextSensor : public text_sensor::TextSensor, public Component {
 public:
  void set_parent(AD5272Component *parent) { parent_ = parent; }
  void update() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  AD5272Component *parent_;
};

}  // namespace ad5272
}  // namespace esphome