#include <config.h>

#include <web_remote/assets/resources.h>

#include <web_remote/web_server.h>

ESP8266WebServer server(80);

FanControllerWebServer& FanControllerWebServer::get_instance()
{
    static FanControllerWebServer instance;
    return instance;
}

void FanControllerWebServer::initialize()
{
    // Static assets

    WEB_SERVER_REGISTER_STATIC_ASSETS(server);

    // API endpoints

    server.on(F("/api/light/on"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning light on.",
            [](olas::Controller& ctl) {
                ctl.light_on();
            });
    });

    server.on(F("/api/light/off"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning light off.",
            [](olas::Controller& ctl) {
                ctl.light_off();
            });
    });

    server.on(F("/api/light/dim"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Dimming light.",
            [](olas::Controller& ctl) {
                ctl.decrease_brightness(1);
            });
    });

    server.on(F("/api/light/brighten"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Brightening light.",
            [](olas::Controller& ctl) {
                ctl.increase_brightness(1);
            });
    });

    server.on(F("/api/fan/off"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning fan off.",
            [](olas::Controller& ctl) {
                ctl.fan_off();
            });
    });

    server.on(F("/api/fan/1"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning on fan with speed 1.",
            [](olas::Controller& ctl) {
                ctl.fan_on(1);
            });
    });

    server.on(F("/api/fan/2"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning on fan with speed 2.",
            [](olas::Controller& ctl) {
                ctl.fan_on(2);
            });
    });

    server.on(F("/api/fan/3"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning on fan with speed 3.",
            [](olas::Controller& ctl) {
                ctl.fan_on(3);
            });
    });

    server.on(F("/api/fan/4"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning on fan with speed 4.",
            [](olas::Controller& ctl) {
                ctl.fan_on(4);
            });
    });

    server.on(F("/api/fan/5"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning on fan with speed 5.",
            [](olas::Controller& ctl) {
                ctl.fan_on(5);
            });
    });

    server.on(F("/api/fan/6"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning on fan with speed 6.",
            [](olas::Controller& ctl) {
                ctl.fan_on(6);
            });
    });

    server.on(F("/api/sleep_wind"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Sleep wind activated.",
            [](olas::Controller& ctl) {
                ctl.sleep_wind();
            });
    });

    server.on(F("/api/reverse_direction"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Reversing direction.",
            [](olas::Controller& ctl) {
                ctl.reverse_direction();
            });
    });

    server.onNotFound([]() {
        server.send(404, "text/plain", "Not found.");
    });

    server.begin();
}

void FanControllerWebServer::schedule(const char* msg, CmdFn fn)
{
    if (task.has_value()) {
        server.send(
            409,
            "text/plain",
            "Another command is already queued");

        return;
    }

    task = fn;
    task_added = millis();

    server.send(202, "text/plain", msg);
}

std::optional<CmdFn> FanControllerWebServer::take_task()
{
    if (millis() - task_added < config::task_execution_time_gap) {
        return std::nullopt;
    }

    auto t = task;
    task = std::nullopt;
    return t;
}

void FanControllerWebServer::handle_client()
{
    server.handleClient();
}
