/*
 * Copyright (C) malarz
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

#include <supla/version.h>
#include <supla/protocol/aqi.eco.h>

Supla::Protocol::AQIECO::AQIECO(Supla::Network* _network, char token[], int refresh, char server[], int id) : Supla::Protocol::WebSender(_network) {
  // serverAddress
  strncpy(serverAddress, server, 32);
  serverAddress[32] = 0;

  // apiToken
  if (strlen(token) == 32) {
    strncpy(apiToken, token, 32);
    apiToken[32] = 0;
  } else {
    apiToken[0] = 0;
  }
  SUPLA_LOG_DEBUG("aqi.eco: token: %s", apiToken);

  // refreshTime
  if (refresh<120) {
    refreshTime = 120;  
  } else {
    refreshTime = refresh;
  }
  SUPLA_LOG_DEBUG("aqi.eco: refresh time: %d", refreshTime);

  // sensorId
  if (id == 0) {
    uint8_t mac[6] = {};
    _network->getMacAddr(mac);
    sensorId = ((mac[2]*256+mac[3])*256+mac[4])*256+mac[5];
  } else {
    sensorId = id;
  }
}

bool Supla::Protocol::AQIECO::sendData() {
  if (strlen(apiToken) != 32) {
    SUPLA_LOG_DEBUG("aqi.eco: token złej długości albo pusty: %s", apiToken);
    return false;
  }
  
  StaticJsonDocument<768> jsonBuffer;
  JsonObject json = jsonBuffer.to<JsonObject>();

  json["esp8266id"] = sensorId;
  json["software_version"] = "Supla_" SUPLA_SHORT_VERSION;
  JsonArray sensordatavalues = json.createNestedArray("sensordatavalues");

  for (int i=0; i<MAXSENSORS; i++) {
    if (sensors[i]) {
      double value = getSensorValue(i);
      String type = "unknown";
      switch(i) {
        case Supla::SenorType::PM1:
          type = "SPS30_P0";
          break;
        case Supla::SenorType::PM2_5:
          type = "SPS30_P2";
          break;
        case Supla::SenorType::PM4:
          type = "SPS30_P4";
          break;
        case Supla::SenorType::PM10:
          type = "SPS30_P1";
          break;
        case Supla::SenorType::TEMP:
          type = "BME280_temperature";
          break;
        case Supla::SenorType::HUMI:
          type = "BME280_humidity";
          break;
        case Supla::SenorType::PRESS:
          type = "BME280_pressure";
          break;
        case Supla::SenorType::LIGHT:
          type = "ambient_light";
          break;
        case Supla::SenorType::WIND:
          type = "wind_speed";
          break;
        case Supla::SenorType::RAIN:
          type = "rainfall";
          break;
        case Supla::SenorType::CO2:
          type = "conc_co2_ppm";
          break;
      }
      
      if (!isnan(value)) {
        JsonObject jo = sensordatavalues.createNestedObject();
        jo["value_type"] = type;
        jo["value"] = value;
      } else {
        return false;
      }
    }
  }
  char output[768];
  serializeJson(json, output, 768);
  Serial.print("aqi.eco: JSON: ");
  Serial.println(output);

  WiFiClientSecure client;
  client.setInsecure();
  if (client.connect(serverAddress, 443)) {
    client.print("POST /update/");
    client.print(apiToken);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(serverAddress);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(strlen(output));
    client.println();
    client.println(output);

    SUPLA_LOG_DEBUG("aqi.eco: sended %d bytes to %s/update/%s", strlen(output), serverAddress, apiToken);

    // waiting for response
    delay(100);
    if (!client.available()) {
      SUPLA_LOG_DEBUG("aqi.eco: no bytes to read from %s", serverAddress);
      return false;
    }
    SUPLA_LOG_DEBUG("aqi.eco: reading from %s: %d bytes", serverAddress, client.available());
    
    output[client.available()] = 0;
    for (int i=0; client.available(); i++) {
      output[i] = client.read();
      if (output[i] == '\n') {
        output[i] = 0;
      }
    }
    SUPLA_LOG_DEBUG("aqi.eco: response from %s: %s", serverAddress, output);
    return true;
  }
  return false;
}
