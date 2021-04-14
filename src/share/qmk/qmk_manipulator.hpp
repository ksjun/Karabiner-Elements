#pragma once

#include "../manipulator/manipulators/base.hpp"
#include "../types.hpp"
#include <pqrs/dispatcher.hpp>

namespace krbn {
namespace manipulator {
namespace manipulators {
namespace qmk_manipulator {
class qmk final : public base, public pqrs::dispatcher::extra::dispatcher_client {
public:
  qmk() : base(), dispatcher_client() {
  }

  virtual ~qmk(void) {
    detach_from_dispatcher();
  }

  virtual bool already_manipulated(const event_queue::entry& front_input_event) {
    return false;
  }

  virtual manipulate_result manipulate(event_queue::entry& front_input_event,
                                       const event_queue::queue& input_event_queue,
                                       std::shared_ptr<event_queue::queue> output_event_queue,
                                       absolute_time_point now) {
    return manipulate_result::passed;
    if (front_input_event.get_validity() == validity::invalid) {
      return manipulate_result::passed;
    }

    auto et = front_input_event.get_event_type();
    if ((et == event_type::key_down || et == event_type::key_up) && validity_ != validity::invalid) {
      if (auto ev = front_input_event.get_event().get_if<momentary_switch_event>()) {
        auto usage_pair = ev->get_usage_pair();
        if (usage_pair.get_usage_page() == pqrs::hid::usage_page::keyboard_or_keypad && usage_pair.get_usage() == pqrs::hid::usage::keyboard_or_keypad::keyboard_1) {
          front_input_event.set_validity(validity::invalid);

          event_queue::event e(momentary_switch_event(krbn::momentary_switch_event(pqrs::hid::usage_page::keyboard_or_keypad,
                                                                                   pqrs::hid::usage::keyboard_or_keypad::keyboard_2)));
          output_event_queue->emplace_back_entry(front_input_event.get_device_id(),
                                                 front_input_event.get_event_time_stamp(),
                                                 e,
                                                 et,
                                                 front_input_event.get_original_event(),
                                                 front_input_event.get_state());
          return manipulate_result::manipulated;
        }
      }
    }
    return manipulate_result::passed;
  }

  virtual bool active(void) const {
    return true;
  }

  virtual bool needs_virtual_hid_pointing(void) const {
    return false;
  }

  virtual void handle_device_keys_and_pointing_buttons_are_released_event(const event_queue::entry& front_input_event,
                                                                          event_queue::queue& output_event_queue) {
  }

  virtual void handle_device_ungrabbed_event(device_id device_id,
                                             const event_queue::queue& output_event_queue,
                                             absolute_time_point time_stamp) {
  }

  virtual void handle_pointing_device_event_from_event_tap(const event_queue::entry& front_input_event,
                                                           event_queue::queue& output_event_queue) {
    unset_alone_if_needed(front_input_event.get_original_event(),
                          front_input_event.get_event_type());
  }

private:
  void unset_alone_if_needed(const event_queue::event& event,
                             event_type event_type) {
    if (auto e = event.get_if<momentary_switch_event>()) {
      if (event_type == event_type::key_down) {
        goto run;
      }
    }

    if (auto pointing_motion = event.get_pointing_motion()) {
      if (pointing_motion->get_vertical_wheel() != 0 ||
          pointing_motion->get_horizontal_wheel() != 0) {
        goto run;
      }
    }

    return;
  run:
    return;
  }
};
} // namespace qmk_manipulator
} // namespace manipulators
} // namespace manipulator
} // namespace krbn
