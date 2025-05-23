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

#include <gtest/gtest.h>
#include <supla/channel.h>
#include <arduino_mock.h>
#include <simple_time.h>
#include <storage_mock.h>
#include <supla/device/register_device.h>

#include <supla/control/roller_shutter.h>
#include <supla/actions.h>
#include "gmock/gmock.h"

using ::testing::_;
using ::testing::AtLeast;

class RollerShutterFixture : public testing::Test {
 public:
  DigitalInterfaceMock ioMock;
  SimpleTime time;
  int gpioUp = 1;
  int gpioDown = 2;

  RollerShutterFixture() {
  }

  virtual ~RollerShutterFixture() {
  }

  void SetUp() {
    Supla::Channel::resetToDefaults();
  }

  void TearDown() {
    Supla::Channel::resetToDefaults();
  }
};

TEST_F(RollerShutterFixture, basicTests) {
  Supla::Control::RollerShutter rs(gpioUp, gpioDown);

  int number = rs.getChannelNumber();
  ASSERT_EQ(number, 0);
  TDSC_RollerShutterValue value = {};
  EXPECT_EQ(rs.getChannel()->getChannelType(), SUPLA_CHANNELTYPE_RELAY);
  EXPECT_EQ(rs.getChannel()->getFuncList(),
            SUPLA_BIT_FUNC_CONTROLLINGTHEROLLERSHUTTER |
                      SUPLA_BIT_FUNC_CONTROLLINGTHEROOFWINDOW |
                      SUPLA_BIT_FUNC_TERRACE_AWNING |
                      SUPLA_BIT_FUNC_ROLLER_GARAGE_DOOR |
                      SUPLA_BIT_FUNC_CURTAIN |
                      SUPLA_BIT_FUNC_PROJECTOR_SCREEN);

  EXPECT_EQ(rs.getChannel()->getDefaultFunction(),
            SUPLA_CHANNELFNC_CONTROLLINGTHEROLLERSHUTTER);
  EXPECT_EQ(rs.getChannel()->getFlags(),
            SUPLA_CHANNEL_FLAG_CHANNELSTATE |
                SUPLA_CHANNEL_FLAG_RS_SBS_AND_STOP_ACTIONS |
                SUPLA_CHANNEL_FLAG_RUNTIME_CHANNEL_CONFIG_UPDATE |
                SUPLA_CHANNEL_FLAG_CALCFG_RECALIBRATE);
  EXPECT_EQ(0,
            memcmp(Supla::RegisterDevice::getChannelValuePtr(number),
                   &value,
                   SUPLA_CHANNELVALUE_SIZE));
}

TEST_F(RollerShutterFixture, onInitHighIsOn) {
  Supla::Control::RollerShutter rs(gpioUp, gpioDown);

  EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
  EXPECT_CALL(ioMock, pinMode(gpioUp, OUTPUT));
  EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));
  EXPECT_CALL(ioMock, pinMode(gpioDown,  OUTPUT));

  rs.onInit();
}

TEST_F(RollerShutterFixture, onInitLowIsOn) {
  Supla::Control::RollerShutter rs(gpioUp, gpioDown, false);

  EXPECT_CALL(ioMock, digitalWrite(gpioUp, 1));
  EXPECT_CALL(ioMock, pinMode(gpioUp, OUTPUT));
  EXPECT_CALL(ioMock, digitalWrite(gpioDown, 1));
  EXPECT_CALL(ioMock, pinMode(gpioDown,  OUTPUT));

  rs.onInit();
}

#pragma pack(push, 1)
struct RollerShutterStateDataTests {
  uint32_t closingTimeMs;
  uint32_t openingTimeMs;
  int8_t currentPosition;  // 0 - closed; 100 - opened
};
#pragma pack(pop)

TEST_F(RollerShutterFixture, notCalibratedStartup) {
  Supla::Control::RollerShutter rs(gpioUp, gpioDown);

  ::testing::InSequence seq;

  // init
  EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
  EXPECT_CALL(ioMock, pinMode(gpioUp, OUTPUT));
  EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));
  EXPECT_CALL(ioMock, pinMode(gpioDown,  OUTPUT));

  // move down
  EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
  EXPECT_CALL(ioMock, digitalWrite(gpioDown, 1));

  // move up - it first call stop
  EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
  EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

  // then actual move up:
  EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));
  EXPECT_CALL(ioMock, digitalWrite(gpioUp, 1));

  // stop
  EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
  EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

  rs.onInit();

  for (int i = 0; i < 10; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  TDSC_RollerShutterValue value = {};
  value.position = -1;
  TDSC_RollerShutterValue *valuePtr =
      reinterpret_cast<TDSC_RollerShutterValue *>(
          Supla::RegisterDevice::getChannelValuePtr(0));
  EXPECT_EQ(0, memcmp(valuePtr, &value, SUPLA_CHANNELVALUE_SIZE));

  rs.handleAction(0, Supla::MOVE_DOWN);
  for (int i = 0; i < 10; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(0, memcmp(Supla::RegisterDevice::getChannelValuePtr(0),
                          &value,
                          SUPLA_CHANNELVALUE_SIZE));

  rs.handleAction(0, Supla::MOVE_UP);
  for (int i = 0; i < 100; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(0, memcmp(Supla::RegisterDevice::getChannelValuePtr(0),
                          &value,
                          SUPLA_CHANNELVALUE_SIZE));

  rs.handleAction(0, Supla::STOP);
  for (int i = 0; i < 10; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }
}

using ::testing::Return;

TEST_F(RollerShutterFixture, movementTests) {
  StorageMock storage;
  Supla::Control::RollerShutter rs(gpioUp, gpioDown);

  storage.defaultInitialization(9);
  EXPECT_CALL(storage, scheduleSave(_)).Times(AtLeast(0));

  // updates of section preamble
  EXPECT_CALL(storage, writeStorage(8, _, 7)).WillRepeatedly(Return(7));
  EXPECT_CALL(storage, commit()).WillRepeatedly(Return());

  {
    ::testing::InSequence seq;

    EXPECT_CALL(
        storage, readStorage(_, _, /* sizeof(RollerShutterStateData) */ 9, _))
        .WillOnce([](uint32_t, unsigned char *data, int, bool) {
          RollerShutterStateDataTests rsData = {.closingTimeMs = 10000,
                                                .openingTimeMs = 10000,
                                                .currentPosition = 0};
          EXPECT_EQ(9, sizeof(rsData));
          memcpy(data, &rsData, sizeof(RollerShutterStateDataTests));
          return 9;
        });

    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, pinMode(gpioUp, OUTPUT));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));
    EXPECT_CALL(ioMock, pinMode(gpioDown, OUTPUT));

    // move down
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 1));

    // move up - it first call stop
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

    // then actual move up:
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 1));

    // stop after reaching limit
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

    // sbs - move down
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 1));

    // sbs - stop
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

    // sbs - move up
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 1));

    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));
  }

  Supla::Storage::LoadStateStorage();
  rs.onInit();

  for (int i = 0; i < 10; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  TDSC_RollerShutterValue value = {};
  EXPECT_EQ(0, memcmp(Supla::RegisterDevice::getChannelValuePtr(0),
                          &value,
                          SUPLA_CHANNELVALUE_SIZE));

  rs.handleAction(0, Supla::MOVE_DOWN);
  for (int i = 0; i < 11; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 9);

  rs.handleAction(0, Supla::MOVE_UP);
  // relays are disabled after 60s timeout
  for (int i = 0; i < 700; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 0);

  rs.handleAction(0, Supla::STEP_BY_STEP);  // sbs - move down
  for (int i = 0; i < 11; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 9);

  rs.handleAction(0, Supla::STEP_BY_STEP);  // sbs - stop
  for (int i = 0; i < 11; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 10);

  rs.handleAction(0, Supla::STEP_BY_STEP);  // sbs - move up
  for (int i = 0; i < 700; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 0);
}

TEST_F(RollerShutterFixture, movementByServerTests) {
  StorageMock storage;
  Supla::Control::RollerShutter rs(gpioUp, gpioDown);

  storage.defaultInitialization(9);
  EXPECT_CALL(storage, scheduleSave(_)).Times(AtLeast(0));

  // updates of section preamble
  EXPECT_CALL(storage, writeStorage(8, _, 7)).WillRepeatedly(Return(7));
  EXPECT_CALL(storage, commit()).WillRepeatedly(Return());

  {
    ::testing::InSequence seq;

    EXPECT_CALL(
        storage, readStorage(_, _, /* sizeof(RollerShutterStateData) */ 9, _))
        .WillOnce([](uint32_t, unsigned char *data, int, bool) {
          RollerShutterStateDataTests rsData = {.closingTimeMs = 10000,
                                                .openingTimeMs = 10000,
                                                .currentPosition = 0};
          EXPECT_EQ(9, sizeof(rsData));
          memcpy(data, &rsData, sizeof(RollerShutterStateDataTests));
          return 9;
        });

    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, pinMode(gpioUp, OUTPUT));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));
    EXPECT_CALL(ioMock, pinMode(gpioDown, OUTPUT));

    // move down
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 1));

    // move up - it first call stop
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

    // then actual move up:
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 1));

    // stop after reaching limit
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

    // sbs - move down
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 1));

    // sbs - stop
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

    // sbs - move up
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 1));

    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

    // sbs - move down
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 1));

    // stop
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

    // move down
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 1));

    // stop
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

    // move down
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 1));

    // stop
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

    // move up
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 1));

    // stop
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

    // down
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 1));

    // stop
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));

    // move up
    EXPECT_CALL(ioMock, digitalWrite(gpioDown, 0));
    EXPECT_CALL(ioMock, digitalWrite(gpioUp, 1));
  }

  Supla::Storage::LoadStateStorage();
  rs.onInit();

  for (int i = 0; i < 10; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  TCSD_RollerShutterValue *value = nullptr;
  TSD_SuplaChannelNewValue newValueFromServer = {};

  value = reinterpret_cast<TCSD_RollerShutterValue*>(newValueFromServer.value);

  newValueFromServer.DurationMS = (100 << 16) | 100;
  newValueFromServer.ChannelNumber = 0;
  value->position = 1;  // move down

  rs.handleNewValueFromServer(&newValueFromServer);
  for (int i = 0; i < 11; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 9);

  value->position = 2;  // up
  rs.handleNewValueFromServer(&newValueFromServer);
  // relays are disabled after 60s timeout
  for (int i = 0; i < 700; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 0);

  value->position = 5;  // step by step -> move down
  rs.handleNewValueFromServer(&newValueFromServer);
  for (int i = 0; i < 11; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 9);

  rs.handleNewValueFromServer(&newValueFromServer);  // sbs - stop
  for (int i = 0; i < 11; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 10);

  rs.handleNewValueFromServer(&newValueFromServer);  // sbs - move up
  for (int i = 0; i < 700; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 0);

  rs.handleNewValueFromServer(&newValueFromServer);  // sbs - move down
  for (int i = 0; i < 11; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 10);

  value->position = 0;
  rs.handleNewValueFromServer(&newValueFromServer);  // stop
  for (int i = 0; i < 11; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 10);

  value->position = 3;  // down or stop
  rs.handleNewValueFromServer(&newValueFromServer);  // down
  for (int i = 0; i < 11; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 19);

  rs.handleNewValueFromServer(&newValueFromServer);  // stop (down or stop)
  for (int i = 0; i < 11; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 20);

  rs.handleNewValueFromServer(&newValueFromServer);  // down (down or stop)
  for (int i = 0; i < 11; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 28);

  value->position = 4;  // move up or stop
  rs.handleNewValueFromServer(&newValueFromServer);  // stop (up or stop)
  for (int i = 0; i < 11; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 30);

  rs.handleNewValueFromServer(&newValueFromServer);  // up (up or stop)
  for (int i = 0; i < 11; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 20);

  value->position = 1;  // down
  rs.handleNewValueFromServer(&newValueFromServer);  // down
  for (int i = 0; i < 16; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 29);

  value->position = 2;  // up
  rs.handleNewValueFromServer(&newValueFromServer);  // up
  for (int i = 0; i < 16; i++) {
    rs.onTimer();
    rs.iterateAlways();
    time.advance(100);
  }

  EXPECT_EQ(Supla::RegisterDevice::getChannelValuePtr(0)[0], 22);
}
