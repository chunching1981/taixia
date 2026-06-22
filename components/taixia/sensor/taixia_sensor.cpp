#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "taixia_sensor.h"

namespace esphome {
namespace taixia {

static const char *const TAG = "taixia.sensor";

static inline uint16_t get_u16(std::vector<uint8_t> &response, int start) {
  return (response[start] << 8) + response[start + 1];
}

static inline int16_t get_i16(std::vector<uint8_t> &response, int start) {
  return (int16_t) ((response[start] << 8) + response[start + 1]);
}

static inline void publish_i16(std::vector<uint8_t> &response, int start, sensor::Sensor *sensor) {
  if ((response[start + 1] == 0xFF) && (response[start + 2] == 0xFF)) {
    sensor->publish_state(0.0f);
  } else {
    int16_t new_state = get_i16(response, start + 1);
    if (sensor->get_state() != new_state)
      sensor->publish_state(new_state);
  }
}

static inline void publish_u16(std::vector<uint8_t> &response, int start, sensor::Sensor *sensor) {
  if ((response[start + 1] == 0xFF) && (response[start + 2] == 0xFF)) {
    sensor->publish_state(0.0f);
  } else {
    uint16_t new_state = get_u16(response, start + 1);
    if (sensor->get_state() != new_state)
      sensor->publish_state(new_state);
  }
}

static inline void publish_u16_div_10(std::vector<uint8_t> &response, int start, sensor::Sensor *sensor) {
  if ((response[start + 1] == 0xFF) && (response[start + 2] == 0xFF)) {
    sensor->publish_state(0.0f);
  } else {
    uint16_t new_state = get_u16(response, start + 1) / 10.0f;
    if (sensor->get_state() != new_state)
      sensor->publish_state(new_state);
  }
}

static inline void publish_float_div_10(std::vector<uint8_t> &response, int start, sensor::Sensor *sensor) {
  if ((response[start + 1] == 0xFF) && (response[start + 2] == 0xFF)) {
    sensor->publish_state(0.0f);
  } else {
    float new_state = get_u16(response, start + 1) / 10.0f;
    if (sensor->get_state() != new_state)
      sensor->publish_state(new_state);
  }
}

void AirConditionerSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Air Conditioner:");
  if (this->temperature_indoor_sensor_ != nullptr) LOG_SENSOR("  ", "Temperature Indoor", this->temperature_indoor_sensor_);
  if (this->humidity_indoor_sensor_ != nullptr) LOG_SENSOR("  ", "Humidity Indoor", this->humidity_indoor_sensor_);
  if (this->temperature_outdoor_sensor_ != nullptr) LOG_SENSOR("  ", "Temperature Outdoor", this->temperature_outdoor_sensor_);
  if (this->humidity_outdoor_sensor_ != nullptr) LOG_SENSOR("  ", "Humidity Outdoor", this->humidity_outdoor_sensor_);
  if (this->operating_current_sensor_ != nullptr) LOG_SENSOR("  ", "Operating Current", this->operating_current_sensor_);
  if (this->operating_voltage_sensor_ != nullptr) LOG_SENSOR("  ", "Operating Voltage", this->operating_voltage_sensor_);
  if (this->operating_power_sensor_ != nullptr) LOG_SENSOR("  ", "Operating Power", this->operating_power_sensor_);
  if (this->energy_consumption_sensor_ != nullptr) LOG_SENSOR("  ", "Energy Consumption", this->energy_consumption_sensor_);
  if (this->operating_hours_sensor_ != nullptr) LOG_SENSOR("  ", "Operating Hours", this->operating_hours_sensor_);
  if (this->error_code_sensor_ != nullptr) LOG_SENSOR("  ", "Error Code", this->error_code_sensor_);
  if (this->filter_clean_hours_sensor_ != nullptr) LOG_SENSOR("  ", "Filiter Clean Hours", this->filter_clean_hours_sensor_);
  if (this->pm_2_5_sensor_ != nullptr) LOG_SENSOR("  ", "PM2.5", this->pm_2_5_sensor_);
  this->parent_->set_have_sensors(true);

  if (this->parent_->get_version() < 3.0)
    this->parent_->read_sa_status();
  else
    this->parent_->send(6, 0, 0, SERVICE_ID_READ_STATUS, 0xffff);
}

void AirConditionerSensor::handle_response(std::vector<uint8_t> &response) {
  uint8_t i;
  for (i = 9; i < response[0] - 3; i+=3) {
    if ((response[i + 1] == 0xFF) && (response[i + 2] == 0xFF)) continue;
    switch (response[i]) {
      case SERVICE_ID_CLIMATE_TEMPERATURE_INDOOR:
        if (this->temperature_indoor_sensor_ != nullptr) publish_i16(response, i, this->temperature_indoor_sensor_);
      break;
      case SERVICE_ID_CLIMATE_HUMIDITY_INDOOR:
        if (this->humidity_indoor_sensor_ != nullptr) publish_u16(response, i, this->humidity_indoor_sensor_);
      break;
      case SERVICE_ID_CLIMATE_TEMPERATURE_OUTDOOR:
        if (this->temperature_outdoor_sensor_ != nullptr) publish_i16(response, i, this->temperature_outdoor_sensor_);
      break;
      case SERVICE_ID_CLIMATE_HUMIDITY_OUTDOOR:
        if (this->humidity_outdoor_sensor_ != nullptr) publish_u16(response, i, this->humidity_outdoor_sensor_);
      break;
      case SERVICE_ID_CLIMATE_OPERATING_CURRENT:
        if (this->operating_current_sensor_ != nullptr) publish_float_div_10(response, i, this->operating_current_sensor_);
      break;
      case SERVICE_ID_CLIMATE_OPERATING_VOLTAGE:
        if (this->operating_voltage_sensor_ != nullptr) publish_u16(response, i, this->operating_voltage_sensor_);
      break;
      case SERVICE_ID_CLIMATE_OPERATING_WATT:
        if (this->operating_power_sensor_ != nullptr) publish_u16(response, i, this->operating_power_sensor_);
      break;
      case SERVICE_ID_CLIMATE_ENERGY_CONSUMPTION:
        if (this->energy_consumption_sensor_ != nullptr) publish_float_div_10(response, i, this->energy_consumption_sensor_);
      break;
      case SERVICE_ID_CLIMATE_OPERATING_HOURS:
        if (this->operating_hours_sensor_ != nullptr) publish_u16(response, i, this->operating_hours_sensor_);
      break;
      case SERVICE_ID_CLIMATE_ERROR_CODE:
        if (this->error_code_sensor_ != nullptr) publish_u16(response, i, this->error_code_sensor_);
      break;
      case SERVICE_ID_CLIMATE_FILTER_CLEAN_HOURS:
        if (this->filter_clean_hours_sensor_ != nullptr) publish_u16(response, i, this->filter_clean_hours_sensor_);
      break;
      case SERVICE_ID_CLIMATE_PM2_5:
        if (this->pm_2_5_sensor_ != nullptr) publish_u16(response, i, this->pm_2_5_sensor_);
      break;
    }
  }
}

void AirConditionerSensor::update() {
  if (this->parent_->get_version() < 3.0)
    this->parent_->read_sa_status();
  else
    this->parent_->send(6, 0, 0, SERVICE_ID_READ_STATUS, 0xffff);
}

void DehumidifierSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "Dehumidifier:");
  if (this->temperature_indoor_sensor_ != nullptr) LOG_SENSOR("  ", "Temperature Indoor", this->temperature_indoor_sensor_);
  if (this->humidity_indoor_sensor_ != nullptr) LOG_SENSOR("  ", "Humidity Indoor", this->humidity_indoor_sensor_);
  if (this->water_full_sensor_ != nullptr) LOG_SENSOR("  ", "Water Full", this->water_full_sensor_);
  if (this->filter_clean_sensor_ != nullptr) LOG_SENSOR("  ", "Filiter Clean", this->filter_clean_sensor_);
  if (this->side_air_vent_sensor_ != nullptr) LOG_SENSOR("  ", "Side Air Vent", this->side_air_vent_sensor_);
  if (this->error_code_sensor_ != nullptr) LOG_SENSOR("  ", "Error Code", this->error_code_sensor_);
  if (this->operating_current_sensor_ != nullptr) LOG_SENSOR("  ", "Operating Current", this->operating_current_sensor_);
  if (this->operating_voltage_sensor_ != nullptr) LOG_SENSOR("  ", "Operating Voltage", this->operating_voltage_sensor_);
  if (this->operating_power_sensor_ != nullptr) LOG_SENSOR("  ", "Operating power", this->operating_power_sensor_);
  if (this->energy_consumption_sensor_ != nullptr) LOG_SENSOR("  ", "Energy Consumption", this->energy_consumption_sensor_);
  if (this->operating_hours_sensor_ != nullptr) LOG_SENSOR("  ", "Operating Hours", this->operating_hours_sensor_);
  if (this->pm_2_5_sensor_ != nullptr) LOG_SENSOR("  ", "PM2.5", this->pm_2_5_sensor_);
  this->parent_->set_have_sensors(true);

  if (this->parent_->get_version() < 3.0)
    this->parent_->read_sa_status();
  else
    this->parent_->send(6, 0, 0, SERVICE_ID_READ_STATUS, 0xffff);
}

void DehumidifierSensor::handle_response(std::vector<uint8_t> &response) {
  uint8_t i;
  for (i = 3; i < response[0] - 3; i+=3) {
    if ((response[i + 1] == 0xFF) && (response[i + 2] == 0xFF)) continue;
    switch (response[i]) {
      case SERVICE_ID_DEHUMIDIFIER_TEMPERATURE_INDOOR:
        if (this->temperature_indoor_sensor_ != nullptr) publish_i16(response, i, this->temperature_indoor_sensor_);
      break;
      case SERVICE_ID_DEHUMIDIFIER_HUMIDITY_INDOOR:
        if (this->humidity_indoor_sensor_ != nullptr) publish_u16(response, i, this->humidity_indoor_sensor_);
      break;
      case SERVICE_ID_DEHUMIDIFIER_WATER_TANK_FULL:
        if (this->water_full_sensor_ != nullptr) publish_u16(response, i, this->water_full_sensor_);
      break;
      case SERVICE_ID_DEHUMIDIFIER_FILTER_NOTIFY:
        if (this->filter_clean_sensor_ != nullptr) publish_u16(response, i, this->filter_clean_sensor_);
      break;
      case SERVICE_ID_DEHUMIDIFIER_SIDE_AIR_VENT:
        if (this->side_air_vent_sensor_ != nullptr) publish_u16(response, i, this->side_air_vent_sensor_);
      break;
      case SERVICE_ID_DEHUMIDIFIER_DEFROST:
        if (this->defrost_sensor_ != nullptr) publish_u16(response, i, this->defrost_sensor_);
      break;
      case SERVICE_ID_DEHUMIDIFIER_ERROR_CODE:
        if (this->error_code_sensor_ != nullptr) publish_u16(response, i, this->error_code_sensor_);
      break;
      case SERVICE_ID_DEHUMIDIFIER_OPERATING_CURRENT:
        if (this->operating_current_sensor_ != nullptr) publish_float_div_10(response, i, this->operating_current_sensor_);
      break;
      case SERVICE_ID_DEHUMIDIFIER_OPERATING_VOLTAGE:
        if (this->operating_voltage_sensor_ != nullptr) publish_u16(response, i, this->operating_voltage_sensor_);
      break;
      case SERVICE_ID_DEHUMIDIFIER_OPERATING_WATT:
        if (this->operating_power_sensor_ != nullptr) publish_u16(response, i, this->operating_power_sensor_);
      break;
      case SERVICE_ID_DEHUMIDIFIER_ENERGY_CONSUMPTION:
        if (this->energy_consumption_sensor_ != nullptr) publish_float_div_10(response, i, this->energy_consumption_sensor_);
      break;
      case SERVICE_ID_DEHUMIDIFIER_PM_2_5:
        if (this->pm_2_5_sensor_ != nullptr) publish_u16(response, i, this->pm_2_5_sensor_);
      break;
      case SERVICE_ID_DEHUMIDIFIER_ODOURS:
        if (this->odours_sensor_ != nullptr) publish_u16(response, i, this->odours_sensor_);
      break;
    }
  }
}

void DehumidifierSensor::update() {
  if (this->parent_->get_version() < 3.0)
    this->parent_->read_sa_status();
  else
    this->parent_->send(6, 0, 0, SERVICE_ID_READ_STATUS, 0xffff);
}

// ... (後續 WashingMachine, AirPurifier, Erv, ElectricFan 類別保持不變) ...

}  // namespace taixia
}  // namespace esphome
