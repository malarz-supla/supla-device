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

#include "virtual_binary.h"

#include <supla/storage/storage.h>
#include <supla/actions.h>

namespace Supla {
namespace Sensor {

VirtualBinary::VirtualBinary(bool keepStateInStorage)
    : keepStateInStorage(keepStateInStorage) {
}

void VirtualBinary::setKeepStateInStorage(bool keepStateInStorage) {
  this->keepStateInStorage = keepStateInStorage;
}

bool VirtualBinary::getValue() {
  return state;
}

void VirtualBinary::onInit() {
  channel.setNewValue(getValue());
}

void VirtualBinary::onSaveState() {
  if (keepStateInStorage) {
    Supla::Storage::WriteState((unsigned char *)&state, sizeof(state));
  }
}

void VirtualBinary::onLoadState() {
  if (keepStateInStorage) {
    bool value = false;
    if (Supla::Storage::ReadState((unsigned char *)&value, sizeof(value))) {
      state = value;
    }
  }
}

void VirtualBinary::handleAction(int event, int action) {
  (void)(event);
  switch (action) {
    case SET: {
      set();
      break;
    }
    case CLEAR: {
      clear();
      break;
    }
    case TOGGLE: {
      toggle();
      break;
    }
  }
}

void VirtualBinary::set() {
  state = true;
}

void VirtualBinary::clear() {
  state = false;
}

void VirtualBinary::toggle() {
  state = !state;
}

};  // namespace Sensor
};  // namespace Supla
