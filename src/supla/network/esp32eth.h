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

/*
    - for LAN8720 + ESP32 -
  using as less gpio as possible
  - 50MHz oscillator disable on LAN8720 by bridging the oscillator pins 1 and 2
  -ESP32 Gpio-        -LAN8720 PIN -
  GPIO22 - EMAC_TXD1   : TX1
  GPIO19 - EMAC_TXD0   : TX0
  GPIO21 - EMAC_TX_EN  : TX_EN
  GPIO26 - EMAC_RXD1   : RX1
  GPIO25 - EMAC_RXD0   : RX0
  GPIO27 - EMAC_RX_DV  : CRS
  GPIO17 - EMAC_TX_CLK : nINT/REFCLK (50MHz)
  GPIO23 - SMI_MDC     : MDC
  GPIO18 - SMI_MDIO    : MDIO
  GND                  : GND
  3V3                  : VCC
*/

#ifndef SRC_SUPLA_NETWORK_ESP32ETH_H_
#define SRC_SUPLA_NETWORK_ESP32ETH_H_

#include <Arduino.h>
#include <ETH.h>
#include <supla/network/netif_lan.h>
#include <supla/supla_lib_config.h>
#include <supla/log_wrapper.h>

#ifdef ETH_CLK_MODE
#undef ETH_CLK_MODE
#endif
#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
// #define ETH_CLK_MODE    ETH_CLOCK_GPIO0_OUT  // unstable!!

// Pin# of the enable signal for the external crystal oscillator (-1 to disable
// for internal APLL source)
#define ETH_POWER_PIN -1

// Type of the Ethernet PHY (LAN8720 or TLK110)
#define ETH_TYPE ETH_PHY_LAN8720

// I²C-address of Ethernet PHY (0 or 1 for LAN8720, 31 for TLK110)
#define ETH_ADDR 1

// Pin# of the I²C clock signal for the Ethernet PHY
#define ETH_MDC_PIN 23

// Pin# of the I²C IO signal for the Ethernet PHY
#define ETH_MDIO_PIN 18

namespace Supla {
class ESPETH;
}  // namespace Supla

static Supla::ESPETH *thisEth = nullptr;

namespace Supla {
class ESPETH : public Supla::LAN {
 public:
  explicit ESPETH(uint8_t ethmode) {
    thisEth = this;
    if (ethmode == 0) {
      ETH_ADDRESS = 0;
    } else {
      ETH_ADDRESS = 1;
    }
  }

  ~ESPETH() {
    if (thisEth == this) {
      thisEth = nullptr;
    }
  }

  static void networkEventHandler(arduino_event_id_t event) {
    switch (event) {
      case ARDUINO_EVENT_ETH_GOT_IP: {
        Serial.print(F("[Ethernet] local IP: "));
        Serial.println(ETH.localIP());
        Serial.print(F("subnetMask: "));
        Serial.println(ETH.subnetMask());
        Serial.print(F("gatewayIP: "));
        Serial.println(ETH.gatewayIP());
        Serial.print(F("ETH MAC: "));
        Serial.println(ETH.macAddress());
        if (ETH.fullDuplex()) {
          Serial.print(F("FULL_DUPLEX , "));
        }
        Serial.print(ETH.linkSpeed());
        Serial.println(F("Mbps"));
        if (thisEth) {
          thisEth->setIpv4Addr(ETH.localIP());
        }
        break;
      }
      case ARDUINO_EVENT_ETH_DISCONNECTED: {
        Serial.println(F("[Ethernet] Disconnected"));
        if (thisEth) {
          thisEth->setIpv4Addr(0);
        }
        break;
      }
    }
  }


  void setup() override {
    allowDisable = true;
    if (initDone) {
      return;
    }

    ::Network.onEvent(Supla::ESPETH::networkEventHandler);

    Serial.println(F("[Ethernet] establishing LAN connection"));
    ETH.begin(ETH_TYPE,
              ETH_ADDRESS,
              ETH_MDC_PIN,
              ETH_MDIO_PIN,
              ETH_POWER_PIN,
              ETH_CLK_MODE);
    initDone = true;

    char newHostname[32] = {};
    generateHostname(hostname, macSizeForHostname, newHostname);
    strncpy(hostname, newHostname, sizeof(hostname) - 1);
    SUPLA_LOG_DEBUG("[%s] Network AP/hostname: %s", getIntfName(), hostname);
    ETH.setHostname(hostname);
  }

  void disable() override {
    if (!allowDisable) {
      return;
    }

    allowDisable = false;
    SUPLA_LOG_DEBUG("[%s] disabling ETH connection", getIntfName());
    DisconnectProtocols();
//    ETH.end();
  }

  bool getMacAddr(uint8_t *mac) override {
    if (initDone) {
      ETH.macAddress(mac);
    }
    return true;
  }

  void setHostname(const char *prefix, int macSize) override {
    macSizeForHostname = macSize;
    strncpy(hostname, prefix, sizeof(hostname) - 1);
    SUPLA_LOG_DEBUG("[%s] Network AP/hostname: %s", getIntfName(), hostname);
  }

  uint32_t getIP() override {
    return ETH.localIP();
  }

  const char *getIntfName() const override {
    return "ETH";
  }

 protected:
  uint8_t ETH_ADDRESS = {};
  bool allowDisable = false;
  int macSizeForHostname = 0;
  bool initDone = false;
};
};  // namespace Supla


#endif  // SRC_SUPLA_NETWORK_ESP32ETH_H_
