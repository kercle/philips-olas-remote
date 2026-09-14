#pragma once

#include <functional>
#include <optional>

#include <ESP8266WebServer.h>

#include <olas/controller.h>

typedef std::function<void(olas::Controller&)> CmdFn;

class FanControllerWebServer {
    std::optional<CmdFn> task;
    uint64_t task_added;

    FanControllerWebServer() = default;
public:
    FanControllerWebServer(const FanControllerWebServer&) = delete;
    FanControllerWebServer& operator=(const FanControllerWebServer&) = delete;

    static FanControllerWebServer& get_instance();

    void initialize();

    void schedule(CmdFn fn, const char *msg);
    std::optional<CmdFn> take_task();

    void handle_client();
};
