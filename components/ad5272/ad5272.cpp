/*
 * AD5272 Digital Potentiometer Component Implementation for ESPHome
 */

#include "ad5272.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ad5272 {

static const char *const TAG = "ad5272";

void AD5272Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AD5272 Digital Potentiometer...");
  
  // Check if device is present
  if (!this->check_device_presence()) {
    ESP_LOGE(TAG, "AD5272 device not found at address 0x%02X", this->address_);
    this->update_status("Device Not Found");
    return;
  }

  this->device_available_ = true;
  this->update_status("Device Connected");
  
  // Read current wiper position
  int16_t current_wiper = this->command_read(AD5274_RDAC_READ);
  if (current_wiper >= 0) {
    this->current_wiper_ = (uint16_t)current_wiper;
    ESP_LOGI(TAG, "Current wiper position: %d (%.1fΩ)", 
            this->current_wiper_, this->wiper_to_resistance(this->current_wiper_));
  } else {
    ESP_LOGW(TAG, "Failed to read initial wiper position");
    this->current_wiper_ = 512; // Default to mid-scale
  }

  this->update_status("Initialized Successfully");
  ESP_LOGCONFIG(TAG, "AD5272 setup complete");
}

void AD5272Component::loop() {
  // Periodically check device availability (every 30 seconds)
  static uint32_t last_check = 0;
  uint32_t now = millis();
  
  if (now - last_check > 30000) {
    this->device_available_ = this->check_device_presence();
    last_check = now;
    
    if (!this->device_available_) {
      this->update_status("Device Disconnected");
    }
  }
}

void AD5272Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AD5272 Digital Potentiometer:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "  Max Resistance: %.0fΩ", this->max_resistance_);
  ESP_LOGCONFIG(TAG, "  Current Wiper: %d", this->current_wiper_);
  ESP_LOGCONFIG(TAG, "  Current Resistance: %.1fΩ", this->wiper_to_resistance(this->current_wiper_));
  ESP_LOGCONFIG(TAG, "  Device Available: %s", this->device_available_ ? "YES" : "NO");
}

bool AD5272Component::set_resistance(float target_ohms) {
  if (!this->device_available_) {
    ESP_LOGE(TAG, "Device not available for resistance setting");
    this->update_status("Device Not Available");
    return false;
  }

  // Validate target resistance
  if (target_ohms < 0 || target_ohms > this->max_resistance_) {
    ESP_LOGE(TAG, "Target resistance %.1fΩ is out of range (0 to %.0fΩ)", 
            target_ohms, this->max_resistance_);
    this->update_status("Invalid Resistance Value");
    return false;
  }

  // Calculate wiper position
  uint16_t wiper_position = this->resistance_to_wiper(target_ohms);
  
  ESP_LOGI(TAG, "Setting resistance to %.1fΩ (wiper position: %d/1023)", 
          target_ohms, wiper_position);

  // Unlock RDAC for writing
  if (!this->unlock_rdac()) {
    ESP_LOGE(TAG, "Failed to unlock RDAC");
    this->update_status("RDAC Unlock Failed");
    return false;
  }

  // Write the wiper position
  if (!this->command_write(AD5274_RDAC_WRITE, wiper_position)) {
    ESP_LOGE(TAG, "Failed to write wiper position");
    this->update_status("Write Failed");
    return false;
  }

  // Verify the write
  delay(10); // Small delay for write to complete
  int16_t readback = this->command_read(AD5274_RDAC_READ);
  if (readback < 0 || (uint16_t)readback != wiper_position) {
    ESP_LOGE(TAG, "Write verification failed. Expected: %d, Got: %d", 
            wiper_position, readback);
    this->update_status("Write Verification Failed");
    return false;
  }

  // Update current state
  this->current_wiper_ = wiper_position;
  float actual_resistance = this->wiper_to_resistance(this->current_wiper_);
  
  ESP_LOGI(TAG, "Successfully set resistance to %.1fΩ", actual_resistance);
  this->update_status("Resistance Set Successfully");
  
  return true;
}

float AD5272Component::get_current_resistance() {
  if (!this->device_available_) {
    return NAN;
  }

  // Read current wiper position
  int16_t wiper = this->command_read(AD5274_RDAC_READ);
  if (wiper >= 0) {
    this->current_wiper_ = (uint16_t)wiper;
    return this->wiper_to_resistance(this->current_wiper_);
  }
  
  // Return cached value if read failed
  return this->wiper_to_resistance(this->current_wiper_);
}

uint16_t AD5272Component::get_current_wiper_position() {
  if (!this->device_available_) {
    return 0;
  }

  int16_t wiper = this->command_read(AD5274_RDAC_READ);
  if (wiper >= 0) {
    this->current_wiper_ = (uint16_t)wiper;
  }
  
  return this->current_wiper_;
}

bool AD5272Component::is_device_available() {
  return this->device_available_;
}

std::string AD5272Component::get_status_message() {
  return this->status_message_;
}

bool AD5272Component::command_write(uint8_t command, uint16_t data) {
  // Create 16-bit command: bits 15:14 = 00, bits 13:10 = command, bits 9:0 = data
  uint16_t cmd_word = ((uint16_t)command << 10) | (data & 0x3FF);
  
  // Send as two bytes (MSB first)
  uint8_t buffer[2] = {
    (uint8_t)((cmd_word >> 8) & 0xFF),
    (uint8_t)(cmd_word & 0xFF)
  };

  ErrorCode err = this->write(buffer, 2);
  if (err != ERROR_OK) {
    ESP_LOGE(TAG, "I2C write failed: %d", (int)err);
    return false;
  }

  return true;
}

int16_t AD5272Component::command_read(uint8_t command, uint8_t data) {
  // First write the command
  if (!this->command_write(command, data)) {
    return -1;
  }

  // Small delay between write and read
  delay(1);

  // Read 2 bytes response
  uint8_t buffer[2];
  ErrorCode err = this->read(buffer, 2);
  if (err != ERROR_OK) {
    ESP_LOGE(TAG, "I2C read failed: %d", (int)err);
    return -1;
  }

  // Combine bytes to form 16-bit result
  uint16_t result = ((uint16_t)buffer[0] << 8) | buffer[1];
  
  // For data reads, the actual data is in the lower 10 bits
  return (int16_t)(result & 0x3FF);
}

bool AD5272Component::unlock_rdac() {
  // Write to control register to enable RDAC writing
  if (!this->command_write(AD5274_CONTROL_WRITE, AD5274_RDAC_WIPER_WRITE_ENABLE)) {
    return false;
  }

  // Verify unlock was successful
  return this->verify_rdac_unlock();
}

bool AD5272Component::verify_rdac_unlock() {
  int16_t control_reg = this->command_read(AD5274_CONTROL_READ);
  if (control_reg < 0) {
    return false;
  }

  return (control_reg & AD5274_RDAC_WIPER_WRITE_ENABLE) != 0;
}

uint16_t AD5272Component::resistance_to_wiper(float resistance_ohms) {
  // AD5272 has 1024 positions (0-1023)
  // Linear relationship: wiper_pos = (resistance / max_resistance) * 1023
  float ratio = resistance_ohms / this->max_resistance_;
  uint16_t wiper_pos = (uint16_t)(ratio * 1023.0 + 0.5); // +0.5 for rounding
  
  // Ensure we don't exceed bounds
  if (wiper_pos > 1023) {
    wiper_pos = 1023;
  }
  
  return wiper_pos;
}

float AD5272Component::wiper_to_resistance(uint16_t wiper_position) {
  // Linear relationship: resistance = (wiper_pos / 1023) * max_resistance
  float ratio = (float)wiper_position / 1023.0;
  return ratio * this->max_resistance_;
}

void AD5272Component::update_status(const std::string& message) {
  this->status_message_ = message;
  ESP_LOGD(TAG, "Status: %s", message.c_str());
}

bool AD5272Component::check_device_presence() {
  // Try to read the control register as a simple presence check
  int16_t result = this->command_read(AD5274_CONTROL_READ);
  return result >= 0;
}

// AD5272Sensor implementation
void AD5272Sensor::update() {
  if (this->parent_ == nullptr) return;
  
  if (this->sensor_type_ == "resistance") {
    float resistance = this->parent_->get_current_resistance();
    if (!isnan(resistance)) {
      this->publish_state(resistance);
    }
  } else if (this->sensor_type_ == "wiper_position") {
    uint16_t wiper = this->parent_->get_current_wiper_position();
    this->publish_state((float)wiper);
  }
}

// AD5272BinarySensor implementation
void AD5272BinarySensor::update() {
  if (this->parent_ == nullptr) return;
  
  bool available = this->parent_->is_device_available();
  this->publish_state(available);
}

// AD5272TextSensor implementation
void AD5272TextSensor::update() {
  if (this->parent_ == nullptr) return;
  
  std::string status = this->parent_->get_status_message();
  this->publish_state(status);
}

}  // namespace ad5272
}  // namespace esphome