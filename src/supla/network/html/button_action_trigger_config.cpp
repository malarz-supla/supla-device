/*
 * Copyright (C) AC SOFTWARE SP. Z O.O
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "button_action_trigger_config.h"

#include <string.h>
#include <supla/network/web_sender.h>
#include <supla/storage/config.h>
#include <supla/storage/storage.h>
#include <supla/tools.h>
#include <supla/storage/config_tags.h>

#include <stdio.h>

namespace Supla {

namespace Html {

ButtonActionTriggerConfig::ButtonActionTriggerConfig(int channelNumber,
    int buttonNumber, const char* labelPrefix) :
  HtmlElement(HTML_SECTION_FORM),
  channelNumber(channelNumber),
  buttonNumber(buttonNumber) {
  if (labelPrefix) {
    int size = strlen(labelPrefix);
    this->labelPrefix = new char[size + 1];
    if (this->labelPrefix) {
      snprintf(this->labelPrefix, size + 1, "%s", labelPrefix);
    }
  }
}

ButtonActionTriggerConfig::~ButtonActionTriggerConfig() {
  if (labelPrefix) {
    delete[] labelPrefix;
    labelPrefix = nullptr;
  }
}

void ButtonActionTriggerConfig::send(Supla::WebSender* sender) {
  auto cfg = Supla::Storage::ConfigInstance();
  if (cfg) {
    int32_t value = 0;
    char key[SUPLA_CONFIG_MAX_KEY_SIZE] = {};
    Supla::Config::generateKey(key, channelNumber,
                             Supla::ConfigTag::BtnActionTriggerCfgTagPrefix);
    cfg->getInt32(key, &value);

    char label[100] = {};
    if (labelPrefix) {
      snprintf(
          label, sizeof(label), "%s MQTT action trigger type", labelPrefix);
    } else {
      snprintf(label, sizeof(label), "IN%d MQTT action trigger type",
          buttonNumber);
    }

    // form-field BEGIN
    sender->send("<div class=\"form-field\">");
    sender->sendLabelFor(key, label);
    sender->send("<div>");
    sender->send("<select ");
    sender->sendNameAndId(key);
    sender->send(">"
        "<option value=\"0\"");
    sender->send(selected(value == 0));
    sender->send(
        ">Publish based on Supla Cloud config</option>"
        "<option value=\"1\"");
    sender->send(selected(value == 1));
    sender->send(
        ">Publish all triggers, don't disable local function</option>"
        "<option value=\"2\"");
    sender->send(selected(value == 2));
    sender->send(
        ">Publish all triggers, disable local function</option></select>");
    sender->send("</div>");
    sender->send("</div>");
    // form-field END
  }
}

bool ButtonActionTriggerConfig::handleResponse(const char* key,
                                               const char* value) {
  auto cfg = Supla::Storage::ConfigInstance();
  char keyRef[SUPLA_CONFIG_MAX_KEY_SIZE] = {};
  Supla::Config::generateKey(keyRef, channelNumber,
      Supla::ConfigTag::BtnActionTriggerCfgTagPrefix);
  if (strcmp(key, keyRef) == 0) {
    int atType = stringToUInt(value);
    switch (atType) {
      default: {
        cfg->setInt32(keyRef, 0);
        break;
      }
      case 0:
      case 1:
      case 2: {
        cfg->setInt32(keyRef, atType);
        break;
      }
    }
    return true;
  }
  return false;
}

};  // namespace Html
};  // namespace Supla


