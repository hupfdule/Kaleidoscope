/* -*- mode: c++ -*-
 * Kaleidoscope-LED-ActiveLayerKeys -- Light up the LEDs based on the keys specified in the active layer
 * Copyright (C) 2018  Keyboard.io, Inc
 *
 * This program is free software: you can redistribute it and/or modify it under it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "kaleidoscope/plugin/LED-ActiveLayerKeys.h"

#include <Arduino.h>  // for pgm_read_byte
#include <stdint.h>   // for uint8_t

#include "kaleidoscope/KeyAddr.h"               // for KeyAddr
#include "kaleidoscope/Runtime.h"               // for Runtime, Runtime_
#include "kaleidoscope/event_handler_result.h"  // for EventHandlerResult, EventHandlerResult::OK
#include "kaleidoscope/layers.h"                // for Layer, Layer_
#include "kaleidoscope/plugin/LEDControl.h"     // for LEDControl

namespace kaleidoscope {
namespace plugin {

const cRGB *LEDActiveLayerKeysEffect::colormap_;

LEDActiveLayerKeysEffect::TransientLEDMode::TransientLEDMode(
  const LEDActiveLayerKeysEffect *parent)
  : parent_(parent),
    active_color_{0, 0, 0} {}

void LEDActiveLayerKeysEffect::setColormap(const cRGB colormap[]) {
  colormap_ = colormap;
}

cRGB LEDActiveLayerKeysEffect::TransientLEDMode::getActiveColor() {
  cRGB color;

  uint8_t top_layer = ::Layer.mostRecent();

  color.r = pgm_read_byte(&(parent_->colormap_[top_layer].r));
  color.g = pgm_read_byte(&(parent_->colormap_[top_layer].g));
  color.b = pgm_read_byte(&(parent_->colormap_[top_layer].b));

  return color;
}

void LEDActiveLayerKeysEffect::TransientLEDMode::onActivate() {
  if (!Runtime.has_leds)
    return;

  uint8_t top_layer = ::Layer.mostRecent();
  active_color_ = getActiveColor();
  cRGB non_layer_color = CRGB(0xff, 0x80, 0x00);

  for (auto key_addr : KeyAddr::all()) {
    Key k         = Layer.lookupOnActiveLayer(key_addr);
    Key layer_key = Layer.getKey(top_layer, key_addr);

    if ((k != layer_key) || (k == Key_NoKey) || (k.getFlags() != KEY_FLAGS)) {
      // TODO: refreshAt somehow seems to apply `active_color_` rather than
      //       refreshing whatever the previous color was :/
      // ::LEDControl.refreshAt(KeyAddr(key_addr));
      ::LEDControl.setCrgbAt(KeyAddr(key_addr), non_layer_color);
    } else {
      ::LEDControl.setCrgbAt(KeyAddr(key_addr), active_color_);
    }
  }
}

void LEDActiveLayerKeysEffect::TransientLEDMode::refreshAt(KeyAddr key_addr) {
  ::LEDControl.setCrgbAt(key_addr, active_color_);
}

EventHandlerResult LEDActiveLayerKeysEffect::onLayerChange() {
  if (::LEDControl.get_mode_index() == led_mode_id_)
    ::LEDControl.get_mode<TransientLEDMode>()->onActivate();

  return EventHandlerResult::OK;
}

}  // namespace plugin
}  // namespace kaleidoscope

kaleidoscope::plugin::LEDActiveLayerKeysEffect LEDActiveLayerKeysEffect;
