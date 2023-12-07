#ifdef USE_ARDUINO

#include "ddp.h"
#include "ddp_basic_light_effect.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ddp {

static const char *const TAG = "ddp_basic_light_effect";

DDPBasicLightEffect::DDPBasicLightEffect(const std::string &name) : LightEffect(name) {}

const std::string &DDPBasicLightEffect::get_name() { return LightEffect::get_name(); }

void DDPBasicLightEffect::start() {

  LightEffect::start();
  DDPLightEffectBase::start();
  
  //Start in an off state when basic ddp enabled
  auto call = this->state_->turn_on();
  call.set_brightness(0.0f);
  call.set_state(true);
  call.set_publish(false);
  call.set_save(false);
  call.perform();
}

void DDPBasicLightEffect::stop() {

  DDPLightEffectBase::stop();
  LightEffect::stop();
}

void DDPBasicLightEffect::apply() {

  // if receiving DDP packets times out, reset to home assistant color.
  // apply function is not needed normally to display changes to the light
  // from Home Assistant, but it is needed to restore value on timeout.
  if ( this->timeout_check() ) {
    ESP_LOGD(TAG,"DDP stream for '%s->%s' timed out.", this->state_->get_name().c_str(), this->get_name().c_str());
    this->next_packet_will_be_first_ = true;

    // auto call = this->state_->turn_on();

    // call.set_publish(false);
    // call.set_save(false);

    // // restore backed up gamma value
    // // this->state_->set_gamma_correct(this->gamma_backup_);
    // call.perform();
   }

}

uint16_t DDPBasicLightEffect::process_(const uint8_t *payload, uint16_t size, uint16_t used) {

  // at least for now, we require 3 bytes of data (r, g, b).
  // If there aren't 3 unused bytes, return 0 to indicate error.
  if ( size < (used + 3) ) { return 0; }

  this->next_packet_will_be_first_ = false;
  this->last_ddp_time_ms_ = millis();

  ESP_LOGV(TAG, "Applying DDP data for '%s->%s': (%02x,%02x,%02x) size = %d, used = %d", this->state_->get_name().c_str(), this->get_name().c_str(), payload[used], payload[used+1], payload[used+2], size, used);

  float red   = (float)payload[used]/255.0f;
  float green = (float)payload[used+1]/255.0f;
  float blue  = (float)payload[used+2]/255.0f;

  ESP_LOGV(TAG, "Final RGB data for '%s->%s': (%02x,%02x,%02x)", this->state_->get_name().c_str(), this->get_name().c_str(), red, green, blue);

  // We can only turn on/off, so if we have white (255,255,255) we go on, anything less is off
  auto call = this->state_->turn_on();

  if ( (red + green + blue) < 3 ) {
    call.set_brightness(0.0f);
    // call.set_brightness(255.0f);
  } 
  // else 
  // {
  //   call.set_brightness(0.0f);
  // }

  call.set_state(true);
  call.set_color_mode_if_supported(light::ColorMode::ON_OFF);
  call.set_transition_length_if_supported(0);
  call.set_publish(false);
  call.set_save(false);
  call.perform();

  // manually calling loop otherwise we just go straight into processing the next DDP
  // packet without executing the light loop to display the just-processed packet.
  // Not totally sure why or if there is a better way to fix, but this works.
  this->state_->loop();

  return 3;
}

}  // namespace ddp
}  // namespace esphome

#endif  // USE_ARDUINO
