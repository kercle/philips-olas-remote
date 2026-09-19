#pragma once

#define ENABLE_WEBSERVER
#define ENABLE_MQTT_CLIENT

namespace config {

//# Radio configuration

//## General settings

constexpr unsigned char pin_cs = 15;
constexpr unsigned char pin_gdo0 = 4;
constexpr unsigned char pin_gdo2 = 5;

//# Web server configuration
static const char hostname[] = "philips-olas-remote";
constexpr unsigned task_execution_time_gap = 50; // milliseconds

}
