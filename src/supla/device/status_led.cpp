/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "status_led.h"

#include <SuplaDevice.h>
#include <supla/io.h>
#include <supla/protocol/protocol_layer.h>
#include <supla/storage/config.h>
#include <supla/storage/storage.h>
#include <supla/time.h>
#include <supla/log_wrapper.h>
#include <supla/protocol/supla_srpc.h>
#include <supla/device/remote_device_config.h>
#include <supla/mutex.h>
#include <supla/auto_lock.h>
#include <supla/storage/config_tags.h>

Supla::Device::StatusLed::StatusLed(Supla::Io *io, uint8_t outPin, bool invert)
    : Supla::Control::BlinkingLed(io, outPin, invert) {
}

Supla::Device::StatusLed::StatusLed(uint8_t outPin, bool invert)
    : Supla::Control::BlinkingLed(outPin, invert) {
}

void Supla::Device::StatusLed::onLoadConfig(SuplaDeviceClass *sdc) {
  if (sdc) {
    sdc->setStatusLed(this);
  }

  if (getMode() == LED_IN_CONFIG_MODE_ONLY || !useDeviceConfig) {
    return;
  }

  auto cfg = Supla::Storage::ConfigInstance();
  if (cfg) {
    int8_t value = defaultMode;
    if (!cfg->getInt8(Supla::ConfigTag::StatusLedCfgTag, &value)) {
      // update default value in config if it is missing
      cfg->setInt8(Supla::ConfigTag::StatusLedCfgTag, defaultMode);
    }
    switch (value) {
      default:
      case 0: {
        setMode(LED_ON_WHEN_CONNECTED);
        break;
      }
      case 1: {
        setMode(LED_OFF_WHEN_CONNECTED);
        break;
      }
      case 2: {
        setMode(LED_ALWAYS_OFF);
        break;
      }
    }

    // register DeviceConfig field bit:
    Supla::Device::RemoteDeviceConfig::RegisterConfigField(
        SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED);
  }
}

void Supla::Device::StatusLed::storeModeToConfig() {
  Supla::AutoLock autoLock(mutex);
  auto cfg = Supla::Storage::ConfigInstance();
  if (cfg) {
    int8_t currentCfgValue = 0;
    cfg->getInt8(Supla::ConfigTag::StatusLedCfgTag, &currentCfgValue);
    if (currentCfgValue != ledMode) {
      switch (ledMode) {
        default:
        case 0: {
          cfg->setInt8(Supla::ConfigTag::StatusLedCfgTag, 0);
          break;
        }
        case 1: {
          cfg->setInt8(Supla::ConfigTag::StatusLedCfgTag,
                       static_cast<int8_t>(ledMode));
          break;
        }
        case 2: {
          cfg->setInt8(Supla::ConfigTag::StatusLedCfgTag,
                       static_cast<int8_t>(ledMode));
          break;
        }
      }
      cfg->setDeviceConfigChangeFlag();
      cfg->saveWithDelay(2000);
    }
  }
}

void Supla::Device::StatusLed::iterateAlways() {
  Supla::AutoLock autoLock(mutex);
  int currentStatus = SuplaDevice.getCurrentStatus();
  if (ledMode == LED_ALWAYS_OFF && currentStatus != STATUS_SW_DOWNLOAD &&
      currentStatus != STATUS_CONFIG_MODE &&
      currentSequence != CUSTOM_SEQUENCE) {
    offDuration = 1000;
    onDuration = 0;
    return;
  }

  if (currentSequence == CUSTOM_SEQUENCE) {
    if (repeatLimit == 1 && onDuration == 0 && offDuration == 1000) {
      autoLock.unlock();
      setAutoSequence();
      autoLock.lock();
    } else {
      return;
    }
  }

  bool checkProtocolsStatus = false;
  switch (currentStatus) {
    case STATUS_INITIALIZED:
    case STATUS_NETWORK_DISCONNECTED:
      currentSequence = NETWORK_CONNECTING;
      break;

    case STATUS_REGISTER_IN_PROGRESS:
      currentSequence = SERVER_CONNECTING;
      checkProtocolsStatus = true;
      break;

    case STATUS_SOFTWARE_RESET:
      offDuration = 1000;
      onDuration = 0;
      currentSequence = CUSTOM_SEQUENCE;
      break;

    case STATUS_OFFLINE_MODE:
    case STATUS_REGISTERED_AND_READY:
      currentSequence = REGISTERED_AND_READY;
      checkProtocolsStatus = true;
      break;

    case STATUS_CONFIG_MODE:
      currentSequence = CONFIG_MODE;
      break;

    case STATUS_SW_DOWNLOAD: {
      currentSequence = SW_DOWNLOAD;
      break;
    }

    case STATUS_SUPLA_PROTOCOL_DISABLED:
      currentSequence = REGISTERED_AND_READY;
      checkProtocolsStatus = true;
      break;

    case STATUS_TEST_WAIT_FOR_CFG_BUTTON: {
      if (SuplaDevice.isSleepingDeviceEnabled()) {
        currentSequence = REGISTERED_AND_READY;
      } else {
        currentSequence = TESTING_PROCEDURE;
      }
      break;
    }

    case STATUS_UNKNOWN:
    case STATUS_ALREADY_INITIALIZED:
    case STATUS_MISSING_NETWORK_INTERFACE:
    case STATUS_UNKNOWN_SERVER_ADDRESS:
    case STATUS_UNKNOWN_LOCATION_ID:
    case STATUS_ALL_PROTOCOLS_DISABLED:
    case STATUS_SERVER_DISCONNECTED:
    case STATUS_ITERATE_FAIL:
    case STATUS_TEMPORARILY_UNAVAILABLE:
    case STATUS_INVALID_GUID:
    case STATUS_CHANNEL_LIMIT_EXCEEDED:
    case STATUS_PROTOCOL_VERSION_ERROR:
    case STATUS_BAD_CREDENTIALS:
    case STATUS_LOCATION_CONFLICT:
    case STATUS_CHANNEL_CONFLICT:
    case STATUS_DEVICE_IS_DISABLED:
    case STATUS_LOCATION_IS_DISABLED:
    case STATUS_DEVICE_LIMIT_EXCEEDED:
    case STATUS_REGISTRATION_DISABLED:
    case STATUS_MISSING_CREDENTIALS:
    case STATUS_INVALID_AUTHKEY:
    case STATUS_NO_LOCATION_AVAILABLE:
    case STATUS_UNKNOWN_ERROR:
    default:
      currentSequence = PACZKOW_WE_HAVE_A_PROBLEM;
      break;
  }

  if (checkProtocolsStatus) {
    for (auto proto = Supla::Protocol::ProtocolLayer::first();
        proto != nullptr;
        proto = proto->next()) {
      if (proto->isConnectionError()) {
        currentSequence = PACZKOW_WE_HAVE_A_PROBLEM;
        break;
      }
      if (proto->isConnecting() && !SuplaDevice.isSleepingDeviceEnabled()) {
        currentSequence = SERVER_CONNECTING;
      }
    }
  }

  if (getMode() == LED_IN_CONFIG_MODE_ONLY && currentSequence != CONFIG_MODE) {
    onDuration = 0;
    offDuration = 1000;
    return;
  }

  switch (currentSequence) {
    case NETWORK_CONNECTING:
      onDuration = 2000;
      offDuration = 2000;
      break;

    case SERVER_CONNECTING:
      onDuration = 500;
      offDuration = 500;
      break;

    case REGISTERED_AND_READY:
      if (ledMode == LED_ON_WHEN_CONNECTED) {
        onDuration = 1000;
        offDuration = 0;
      } else {
        onDuration = 0;
        offDuration = 1000;
      }
      break;

    case CONFIG_MODE:
      onDuration = 100;
      offDuration = 100;
      break;

    case SW_DOWNLOAD:
      onDuration = 20;
      offDuration = 20;
      break;

    case PACZKOW_WE_HAVE_A_PROBLEM:
      onDuration = 300;
      offDuration = 100;
      break;

    case TESTING_PROCEDURE:
      onDuration = 50;
      offDuration = 50;
      break;

    case CUSTOM_SEQUENCE:
    default:
      break;
  }
}

void Supla::Device::StatusLed::setCustomSequence(uint32_t onDurationMs,
                                                 uint32_t offDurationMs,
                                                 uint32_t pauseDurrationMs,
                                                 uint8_t onLimit,
                                                 uint8_t repeatLimit,
                                                 bool startWithOff) {
  currentSequence = CUSTOM_SEQUENCE;
  Supla::Control::BlinkingLed::setCustomSequence(
      onDurationMs, offDurationMs, pauseDurrationMs, onLimit, repeatLimit,
      startWithOff);
}

enum Supla::LedSequence Supla::Device::StatusLed::getCurrentSequence() const {
  return currentSequence;
}

void Supla::Device::StatusLed::setAutoSequence() {
  Supla::AutoLock autoLock(mutex);
  // resetting to defaults will trigger automatic sequence update on
  // iterateAlways call
  currentSequence = NETWORK_CONNECTING;
  repeatLimit = 0;
}

void Supla::Device::StatusLed::setMode(LedMode newMode) {
  Supla::AutoLock autoLock(mutex);
  ledMode = newMode;
}

Supla::LedMode Supla::Device::StatusLed::getMode() const {
  return ledMode;
}

void Supla::Device::StatusLed::onDeviceConfigChange(uint64_t fieldBit) {
  if (fieldBit == SUPLA_DEVICE_CONFIG_FIELD_STATUS_LED) {
    // reload config
    SUPLA_LOG_DEBUG("StatusLed: reload config");
    onLoadConfig(nullptr);
  }
}

void Supla::Device::StatusLed::setDefaultMode(enum LedMode newMode) {
  defaultMode = static_cast<int8_t>(newMode);
}

void Supla::Device::StatusLed::setUseDeviceConfig(bool value) {
  useDeviceConfig = value;
}

void Supla::Device::StatusLed::identify() {
  setCustomSequence(150, 150, 400, 3, 5);
}
