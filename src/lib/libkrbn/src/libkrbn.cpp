#include "libkrbn.h"
#include "application_launcher.hpp"
#include "constants.hpp"
#include "core_configuration/core_configuration.hpp"
#include "dispatcher_utility.hpp"
#include "json_utility.hpp"
#include "launchctl_utility.hpp"
#include "libkrbn_cpp.hpp"
#include "monitor/version_monitor.hpp"
#include "process_utility.hpp"
#include "types.hpp"
#include "update_utility.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <pqrs/osx/frontmost_application_monitor.hpp>
#include <string>

namespace {
class libkrbn_components_manager {
public:
  ~libkrbn_components_manager(void) {
    version_monitor_ = nullptr;
    frontmost_application_monitor_ = nullptr;
  }

  // version_monitor_

  void enable_version_monitor(libkrbn_version_monitor_callback callback,
                              void* refcon) {
    version_monitor_ = std::make_unique<krbn::version_monitor>(krbn::constants::get_version_file_path());

    version_monitor_->changed.connect([callback, refcon](auto&& version) {
      if (callback) {
        callback(refcon);
      }
    });

    version_monitor_->async_start();
  }

  void disable_version_monitor(void) {
    version_monitor_ = nullptr;
  }

  // frontmost_application_monitor_

  void enable_frontmost_application_monitor(libkrbn_frontmost_application_monitor_callback callback,
                                            void* refcon) {
    frontmost_application_monitor_ = std::make_unique<pqrs::osx::frontmost_application_monitor::monitor>(
        pqrs::dispatcher::extra::get_shared_dispatcher());

    frontmost_application_monitor_->frontmost_application_changed.connect([callback, refcon](auto&& application_ptr) {
      if (application_ptr && callback) {
        std::string bundle_identifier = application_ptr->get_bundle_identifier().value_or("");
        std::string file_path = application_ptr->get_file_path().value_or("");

        callback(bundle_identifier.c_str(),
                 file_path.c_str(),
                 refcon);
      }
    });

    frontmost_application_monitor_->async_start();
  }

  void disable_frontmost_application_monitor(void) {
    frontmost_application_monitor_ = nullptr;
  }

private:
  std::unique_ptr<krbn::version_monitor> version_monitor_;
  std::unique_ptr<pqrs::osx::frontmost_application_monitor::monitor> frontmost_application_monitor_;
};

std::unique_ptr<libkrbn_components_manager> libkrbn_components_manager_;
} // namespace

void libkrbn_initialize(void) {
  krbn::dispatcher_utility::initialize_dispatchers();

  if (!libkrbn_components_manager_) {
    libkrbn_components_manager_ = std::make_unique<libkrbn_components_manager>();
  }
}

void libkrbn_terminate(void) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_ = nullptr;
  }

  krbn::dispatcher_utility::terminate_dispatchers();
}

const char* libkrbn_get_distributed_notification_observed_object(void) {
  return krbn::constants::get_distributed_notification_observed_object();
}

const char* libkrbn_get_distributed_notification_console_user_server_is_disabled(void) {
  return krbn::constants::get_distributed_notification_console_user_server_is_disabled();
}

const char* libkrbn_get_distributed_notification_device_grabbing_state_is_changed(void) {
  return krbn::constants::get_distributed_notification_device_grabbing_state_is_changed();
}

const char* libkrbn_get_grabber_alerts_json_file_path(void) {
  return krbn::constants::get_grabber_alerts_json_file_path();
}

const char* libkrbn_get_devices_json_file_path(void) {
  return krbn::constants::get_devices_json_file_path();
}

const char* libkrbn_get_device_details_json_file_path(void) {
  return krbn::constants::get_device_details_json_file_path();
}

const char* libkrbn_get_manipulator_environment_json_file_path(void) {
  return krbn::constants::get_manipulator_environment_json_file_path();
}

const char* libkrbn_get_user_complex_modifications_assets_directory(void) {
  return krbn::constants::get_user_complex_modifications_assets_directory().c_str();
}

bool libkrbn_lock_single_application_with_user_pid_file(const char* _Nonnull pid_file_name) {
  return krbn::process_utility::lock_single_application_with_user_pid_file(pid_file_name);
}

void libkrbn_unlock_single_application(void) {
  krbn::process_utility::unlock_single_application();
}

void libkrbn_launchctl_manage_console_user_server(bool load) {
  krbn::launchctl_utility::manage_console_user_server(load);
}

void libkrbn_launchctl_restart_console_user_server(void) {
  krbn::launchctl_utility::restart_console_user_server();
}

void libkrbn_check_for_updates_in_background(void) {
  krbn::update_utility::check_for_updates_in_background();
}

void libkrbn_check_for_updates_stable_only(void) {
  krbn::update_utility::check_for_updates_stable_only();
}

void libkrbn_check_for_updates_with_beta_version(void) {
  krbn::update_utility::check_for_updates_with_beta_version();
}

void libkrbn_launch_event_viewer(void) {
  krbn::application_launcher::launch_event_viewer();
}

void libkrbn_launch_menu(void) {
  krbn::application_launcher::launch_menu();
}

void libkrbn_launch_preferences(void) {
  krbn::application_launcher::launch_preferences();
}

bool libkrbn_system_core_configuration_file_path_exists(void) {
  return pqrs::filesystem::exists(krbn::constants::get_system_core_configuration_file_path());
}

void libkrbn_get_key_code_name(char* buffer, size_t length, uint32_t key_code) {
  buffer[0] = '\0';
  if (auto name = krbn::types::make_key_code_name(krbn::key_code(key_code))) {
    strlcpy(buffer, name->c_str(), length);
  }
}

void libkrbn_get_consumer_key_code_name(char* buffer, size_t length, uint32_t consumer_key_code) {
  buffer[0] = '\0';
  if (auto name = krbn::types::make_consumer_key_code_name(krbn::consumer_key_code(consumer_key_code))) {
    strlcpy(buffer, name->c_str(), length);
  }
}

bool libkrbn_device_identifiers_is_apple(const libkrbn_device_identifiers* p) {
  if (p) {
    return libkrbn_cpp::make_device_identifiers(*p).is_apple();
  }
  return false;
}

void libkrbn_enable_version_monitor(libkrbn_version_monitor_callback callback,
                                    void* refcon) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->enable_version_monitor(callback,
                                                        refcon);
  }
}

void libkrbn_disable_version_monitor(void) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->disable_version_monitor();
  }
}

void libkrbn_enable_frontmost_application_monitor(libkrbn_frontmost_application_monitor_callback callback,
                                                  void* refcon) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->enable_frontmost_application_monitor(callback,
                                                                      refcon);
  }
}

void libkrbn_disable_frontmost_application_monitor(void) {
  if (libkrbn_components_manager_) {
    libkrbn_components_manager_->disable_frontmost_application_monitor();
  }
}
