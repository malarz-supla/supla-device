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

#include <supla-common/srpc.h>

#include "srpc_mock.h"

_supla_int_t srpc_ds_async_channel_extendedvalue_changed(
    void *, unsigned char, TSuplaChannelExtendedValue *) {
  return 0;
}

_supla_int_t srpc_ds_async_action_trigger(void *, TDS_ActionTrigger *at) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->actionTrigger(at->ChannelNumber,
                                                at->ActionTrigger);
}

_supla_int_t srpc_ds_async_get_channel_config_request(
    void *, TDS_GetChannelConfigRequest *request) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->getChannelConfig(request->ChannelNumber,
                                                   request->ConfigType);
}

_supla_int_t srpc_ds_async_set_device_config_result(
    void *, TSDS_SetDeviceConfigResult *result) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->setDeviceConfigResult(result);
}

_supla_int_t srpc_ds_async_set_device_config_request(
    void *, TSDS_SetDeviceConfig *request) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->setDeviceConfigRequest(request);
}

_supla_int_t srpc_ds_async_set_channel_config_result(
    void *, TSDS_SetChannelConfigResult *result) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->setChannelConfigResult(result);
}

_supla_int_t srpc_ds_async_set_channel_config_request(
    void *, TSDS_SetChannelConfig *request) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->setChannelConfigRequest(request);
}

_supla_int_t srpc_ds_async_channel_value_changed_c(void *_srpc,
                                                   unsigned char channel_number,
                                                   char *value,
                                                   unsigned char offline,
                                                   unsigned _supla_int_t
                                                       validity_time_sec) {
  assert(SrpcInterface::instance);
  std::vector<char> vec(value, value + 8);
  return SrpcInterface::instance->valueChanged(
      _srpc, channel_number, vec, offline, validity_time_sec);
}

_supla_int_t srpc_dcs_async_set_activity_timeout(
    void *_srpc, TDCS_SuplaSetActivityTimeout *dcs_set_activity_timeout) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->srpc_dcs_async_set_activity_timeout(
      _srpc, dcs_set_activity_timeout);
}

void srpc_params_init(TsrpcParams *params) {
  assert(SrpcInterface::instance);
  SrpcInterface::instance->srpc_params_init(params);
}

_supla_int_t srpc_ds_async_set_channel_result(void *_srpc,
                                              unsigned char ChannelNumber,
                                              _supla_int_t SenderID,
                                              char Success) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->srpc_ds_async_set_channel_result(
      _srpc, ChannelNumber, SenderID, Success);
}

_supla_int_t srpc_ds_async_device_calcfg_result(
    void *_srpc, TDS_DeviceCalCfgResult *result) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->srpc_ds_async_device_calcfg_result(_srpc,
                                                                     result);
}

void *srpc_init(TsrpcParams *params) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->srpc_init(params);
}

void srpc_free(void *srpc) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->srpc_free(srpc);
}

void srpc_rd_free(TsrpcReceivedData *rd) {
  assert(SrpcInterface::instance);
  SrpcInterface::instance->srpc_rd_free(rd);
}

char srpc_getdata(void *_srpc,
                  TsrpcReceivedData *rd,
                  unsigned _supla_int_t rr_id) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->srpc_getdata(_srpc, rd, rr_id);
}

char srpc_iterate_device(void *_srpc) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->srpc_iterate(_srpc);
}

void srpc_set_proto_version(void *_srpc, unsigned char version) {
  assert(SrpcInterface::instance);
  SrpcInterface::instance->srpc_set_proto_version(_srpc, version);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_registerdevice_in_chunks(
    void *_srpc, TDS_SuplaRegisterDeviceHeader *registerdevice,
    TDS_SuplaDeviceChannel_D *(*)(int)) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->srpc_ds_async_registerdevice_in_chunks(
      _srpc, registerdevice);
}

_supla_int_t SRPC_ICACHE_FLASH srpc_ds_async_registerdevice_in_chunks_g(
    void *_srpc, TDS_SuplaRegisterDeviceHeader *registerdevice,
    TDS_SuplaDeviceChannel_E *(*)(int)) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->srpc_ds_async_registerdevice_in_chunks_g(
      _srpc, registerdevice);
}

_supla_int_t srpc_dcs_async_ping_server(void *_srpc) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->srpc_dcs_async_ping_server(_srpc);
}

_supla_int_t srpc_csd_async_channel_state_result(void *_srpc,
                                                 TDSC_ChannelState *state) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->srpc_csd_async_channel_state_result(_srpc,
                                                                      state);
}

_supla_int_t srpc_dcs_async_get_user_localtime(void *_srpc) {
  assert(SrpcInterface::instance);
  return SrpcInterface::instance->srpc_dcs_async_get_user_localtime(_srpc);
}

_supla_int_t srpc_ds_async_register_push_notification(
    void *, TDS_RegisterPushNotification *reg) {
  assert(SrpcInterface::instance);
  assert(reg);
  return SrpcInterface::instance->registerPushNotification(
    reg->Context, reg->ServerManagedFields);
}

_supla_int_t srpc_ds_async_set_subdevice_details(
    void *, TDS_SubdeviceDetails *reg) {
  assert(SrpcInterface::instance);
  assert(reg);
  return SrpcInterface::instance->setSubdeviceDetails(
    reg->SubDeviceId, reg->Name, reg->ProductCode, reg->SerialNumber,
    reg->SoftVer);
}


_supla_int_t srpc_ds_async_send_push_notification(void *,
                                                  TDS_PushNotification *push) {
  assert(SrpcInterface::instance);
  assert(push);
  return SrpcInterface::instance->sendPushNotification(push->Context,
      push->TitleSize, push->BodySize, push->TitleAndBody);
}

SrpcInterface::SrpcInterface() {
  instance = this;
}

SrpcInterface::~SrpcInterface() {
  instance = nullptr;
}

SrpcInterface *SrpcInterface::instance = nullptr;

// method copied directly from srpc.c
_supla_int_t srpc_evtool_v3_emextended2extended(
    const TElectricityMeter_ExtendedValue_V3 *em_ev,
    TSuplaChannelExtendedValue *ev) {
  if (em_ev == NULL || ev == NULL || em_ev->m_count > EM_MEASUREMENT_COUNT ||
      em_ev->m_count < 0) {
    return 0;
  }

  memset(ev, 0, sizeof(TSuplaChannelExtendedValue));
  ev->type = EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V3;

  ev->size = sizeof(TElectricityMeter_ExtendedValue_V3) -
             sizeof(TElectricityMeter_Measurement) * EM_MEASUREMENT_COUNT +
             sizeof(TElectricityMeter_Measurement) * em_ev->m_count;

  if (ev->size > 0 && ev->size <= SUPLA_CHANNELEXTENDEDVALUE_SIZE) {
    memcpy(ev->value, em_ev, ev->size);
    return 1;
  }

  ev->size = 0;
  return 0;
}

_supla_int_t SRPC_ICACHE_FLASH
srpc_evtool_v3_extended2emextended(const TSuplaChannelExtendedValue *ev,
                                   TElectricityMeter_ExtendedValue_V3 *em_ev) {
  if (em_ev == NULL || ev == NULL ||
      ev->type != EV_TYPE_ELECTRICITY_METER_MEASUREMENT_V3 || ev->size == 0 ||
      ev->size > sizeof(TElectricityMeter_ExtendedValue_V3)) {
    return 0;
  }

  memset(em_ev, 0, sizeof(TElectricityMeter_ExtendedValue_V3));
  memcpy(em_ev, ev->value, ev->size);

  uint32_t expected_size = 0;

  if (em_ev->m_count <= EM_MEASUREMENT_COUNT) {
    expected_size =
        sizeof(TElectricityMeter_ExtendedValue_V3) -
        sizeof(TElectricityMeter_Measurement) * EM_MEASUREMENT_COUNT +
        sizeof(TElectricityMeter_Measurement) * em_ev->m_count;
  }

  if (ev->size != expected_size) {
    memset(em_ev, 0, sizeof(TElectricityMeter_ExtendedValue_V3));
    return 0;
  }

  return 1;
}

_supla_int_t srpc_dcs_async_set_channel_caption(void *,
                                                TDCS_SetCaption *caption) {
  assert(SrpcInterface::instance);
  assert(caption);
  return SrpcInterface::instance->setChannelCaption(caption->ChannelNumber,
      caption->Caption);
}

SrpcMock::SrpcMock() {}
SrpcMock::~SrpcMock() {}
