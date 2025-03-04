/*
   Copyright (C) malarz

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License898
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
   */

// Dependencies:
// https://github.com/kevinlutzer/Arduino-PM1006K

#ifndef SRC_SUPLA_SENSOR_PM1006K_H_
#define SRC_SUPLA_SENSOR_PM1006K_H_

#include <supla/sensor/general_purpose_measurement.h>

#include <PM1006K.h>

namespace Supla {
namespace Sensor {
class pm1006k : public GeneralPurposeMeasurement {
 public:
  // rx_pin, tx_pin: pins to which the sensor is connected
  // refresh: time between readings (in minutes: 1-1440)
  explicit pm1006k(int rx_pin, int tx_pin, int fan_pin = 0, int refresh = 10)
      : GeneralPurposeMeasurement(nullptr, false) {
    if (refresh < 1) {
      refresh = 10;
    } else if (refresh > 1440) {
      refresh = 10;
    }

    // FAN setup
    fanPin = fan_pin;
    if (fanPin) {
      pinMode(fanPin, OUTPUT);
      fanOff = false;
      digitalWrite(fanPin, HIGH);
      SUPLA_LOG_DEBUG("PM1006K FAN: started & on");
    }

    // Setup and create instance of the PM1006K driver
    // The baud rate for the serial connection must be PM1006K::BAUD_RATE.
    Serial1.begin(PM1006K::BAUD_RATE, SERIAL_8N1, rx_pin, tx_pin);
    sensor = new PM1006K(&Serial1);

    refreshIntervalMs = refresh * 60 * 1000;
    setDefaultUnitAfterValue("μg/m³");
    setInitialCaption("PM 2.5");
    getChannel()->setDefaultIcon(8);
  }

  void onInit() override {
    iterateAlways();
  }

  void iterateAlways() override {
    // 15 sec befor reading sensor
    if (millis() - lastReadTime > refreshIntervalMs-15000) {
      if (fanPin && fanOff) {
        fanOff = false;
        digitalWrite(fanPin, HIGH);
        SUPLA_LOG_DEBUG("PM1006K FAN: on");
      }
    }

    if (millis() - lastReadTime > refreshIntervalMs) {
      int32_t value = NAN;
      if (!sensor->takeMeasurement()) {
        SUPLA_LOG_DEBUG("PM1006K: failed to take measurement");
      } else {
        value = sensor->getPM2_5();
        SUPLA_LOG_DEBUG("PM1006K: read: %d", value);
      }

      if (fanPin) {
        fanOff = true;
        digitalWrite(fanPin, LOW);
        SUPLA_LOG_DEBUG("PM1006K FAN: off");
      }

      if (isnan(value) || value <= 0) {
        if (invalidCounter < 3) {
          invalidCounter++;
        } else {
          lastValue = NAN;
        }
      } else {
        invalidCounter = 0;
        lastValue = value;
      }
      channel.setNewValue(getValue());
      lastReadTime = millis();
    }
  }

  double getValue() override {
    return lastValue;
  }

 protected:
  ::PM1006K* sensor;
  uint32_t refreshIntervalMs = 600000;
  uint32_t lastReadTime = 0;
  uint32_t lastValue = NAN;
  int fanPin = 0;
  bool fanOff = true;
  int invalidCounter = 0;
};

}  // namespace Sensor
}  // namespace Supla

#endif  // SRC_SUPLA_SENSOR_PM1006K_H_
