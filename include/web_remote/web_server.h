#pragma once

#include <functional>
#include <optional>

#include <ESP8266WebServer.h>

#include <olas/controller.h>

using CmdFn = std::function<void(olas::Controller&)>;

class FanControllerWebServer {
    std::optional<CmdFn> task;
    uint64_t task_added;

    FanControllerWebServer() = default;

public:
    FanControllerWebServer(const FanControllerWebServer&) = delete;
    FanControllerWebServer& operator=(const FanControllerWebServer&) = delete;

    static FanControllerWebServer& get_instance();

    void initialize();

    void schedule(const char* msg, CmdFn fn);
    std::optional<CmdFn> take_task();

    void handle_client();
};
