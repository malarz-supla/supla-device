/*
   Copyright (C) AC SOFTWARE SP. Z O.O

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

#include "binary_base.h"

#include <supla/time.h>
#include <supla/log_wrapper.h>
#include <supla/storage/config.h>
#include <supla/storage/config_tags.h>
#include <supla/protocol/protocol_layer.h>

using Supla::Sensor::BinaryBase;


BinaryBase::BinaryBase() {
  channel.setType(SUPLA_CHANNELTYPE_BINARYSENSOR);
  channel.setFlag(SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE);
}

BinaryBase::~BinaryBase() {
}

void BinaryBase::onLoadConfig(SuplaDeviceClass *sdc) {
  (void)(sdc);
  auto cfg = Supla::Storage::ConfigInstance();
  if (cfg) {
    loadFunctionFromConfig();

    char key[SUPLA_CONFIG_MAX_KEY_SIZE] = {};
    generateKey(key, Supla::ConfigTag::BinarySensorServerInvertedLogicTag);
    uint8_t storedServerInvertLogic = 0;
    cfg->getUInt8(key, &storedServerInvertLogic);
    setServerInvertLogic(storedServerInvertLogic > 0);
    SUPLA_LOG_INFO("Binary[%d] config serverInvertLogic %d",
                   getChannelNumber(),
                   storedServerInvertLogic);
  }
}

void BinaryBase::purgeConfig() {
  Supla::ElementWithChannelActions::purgeConfig();
  auto cfg = Supla::Storage::ConfigInstance();
  if (!cfg) {
    return;
  }
  char key[SUPLA_CONFIG_MAX_KEY_SIZE] = {};
  generateKey(key, Supla::ConfigTag::BinarySensorServerInvertedLogicTag);
  cfg->eraseKey(key);
}

void BinaryBase::onRegistered(Supla::Protocol::SuplaSrpc *suplaSrpc) {
  configFinishedReceived = false;
  defaultConfigReceived = false;
  Supla::Element::onRegistered(suplaSrpc);
}

uint8_t BinaryBase::applyChannelConfig(TSD_ChannelConfig *newConfig,
                                       bool local) {
  SUPLA_LOG_DEBUG("Binary[%d]: processing%s channel config",
                  getChannelNumber(),
                  local ? " local" : "");

  if (newConfig->ConfigType != SUPLA_CONFIG_TYPE_DEFAULT) {
    return SUPLA_CONFIG_RESULT_TYPE_NOT_SUPPORTED;
  }

  auto newFunction = newConfig->Func;
  if (newFunction != getChannel()->getDefaultFunction() && newFunction != 0) {
    SUPLA_LOG_INFO("Binary[%d]: function changed to %d",
                   getChannelNumber(),
                   newFunction);
    setAndSaveFunction(newFunction);
    for (auto proto = Supla::Protocol::ProtocolLayer::first();
        proto != nullptr; proto = proto->next()) {
      proto->notifyConfigChange(getChannelNumber());
    }
  }

  if (newConfig->ConfigSize < sizeof(TChannelConfig_BinarySensor)) {
    if (newConfig->ConfigSize == 0) {
      SUPLA_LOG_DEBUG("Binary[%d]: config missing on server",
                      getChannelNumber());
      return SUPLA_CONFIG_RESULT_TRUE;
    }
    return SUPLA_CONFIG_RESULT_DATA_ERROR;
  }

  defaultConfigReceived = true;

  TChannelConfig_BinarySensor *config =
      reinterpret_cast<TChannelConfig_BinarySensor *>(newConfig->Config);

  setServerInvertLogic(config->InvertedLogic > 0);

  auto cfg = Supla::Storage::ConfigInstance();
  if (cfg) {
    char key[SUPLA_CONFIG_MAX_KEY_SIZE] = {};
    generateKey(key, Supla::ConfigTag::BinarySensorServerInvertedLogicTag);
    uint8_t storedServerInvertLogic = (config->InvertedLogic > 0) ? 1 : 0;
    cfg->setUInt8(key, storedServerInvertLogic);
    cfg->saveWithDelay(2000);
  }

  return SUPLA_CONFIG_RESULT_TRUE;
}

void BinaryBase::iterateAlways() {
  if (millis() - lastReadTime > readIntervalMs) {
    lastReadTime = millis();
    channel.setNewValue(getValue());
  }
}

Supla::Channel *BinaryBase::getChannel() {
  return &channel;
}

const Supla::Channel *BinaryBase::getChannel() const {
  return &channel;
}

void BinaryBase::setServerInvertLogic(bool invertLogic) {
  channel.setServerInvertLogic(invertLogic);
}

void BinaryBase::setReadIntervalMs(uint32_t intervalMs) {
  if (intervalMs == 0) {
    intervalMs = 100;
  }
  readIntervalMs = intervalMs;
}

void BinaryBase::handleChannelConfigFinished() {
  ElementWithChannelActions::handleChannelConfigFinished();
  if (!defaultConfigReceived) {
    // set default config on device
    SUPLA_LOG_DEBUG("Binary[%d]: setting default channel config",
                    getChannelNumber());
    TSD_ChannelConfig defaultConfig = {};
    defaultConfig.ConfigSize = sizeof(TChannelConfig_BinarySensor);
    applyChannelConfig(&defaultConfig, true);
  }
}

void BinaryBase::fillChannelConfig(void *channelConfig, int *size) {
  if (size) {
    *size = 0;
  } else {
    return;
  }
  if (channelConfig == nullptr) {
    return;
  }

  auto config = reinterpret_cast<TChannelConfig_BinarySensor *>(
          channelConfig);
  *size = sizeof(TChannelConfig_BinarySensor);
  config->InvertedLogic = channel.isServerInvertLogic() ? 1 : 0;
  config->FilteringTimeMs = 0;
}
