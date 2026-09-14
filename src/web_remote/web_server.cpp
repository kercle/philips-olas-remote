#include <config.h>

#include <web_remote/assets/index.html.h>
#include <web_remote/web_server.h>

ESP8266WebServer server(80);

FanControllerWebServer& FanControllerWebServer::get_instance()
{
    static FanControllerWebServer instance;
    return instance;
}

void FanControllerWebServer::initialize()
{
    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html", assets::INDEX_HTML);
    });

    server.on(F("/api/light/on"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning light on",
            [](olas::Controller& ctl) {
                ctl.light_on();
            });
    });

    server.on(F("/api/light/off"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning light off",
            [](olas::Controller& ctl) {
                ctl.light_off();
            });
    });

    server.on(F("/api/fan/off"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning off fan",
            [](olas::Controller& ctl) {
                ctl.light_on();
            });
    });

    server.on(F("/api/fan/1"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning on fan with speed 1",
            [](olas::Controller& ctl) {
                ctl.fan_on(1);
            });
    });

    server.on(F("/api/fan/2"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning on fan with speed 2",
            [](olas::Controller& ctl) {
                ctl.fan_on(2);
            });
    });

    server.on(F("/api/fan/3"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning on fan with speed 3",
            [](olas::Controller& ctl) {
                ctl.fan_on(3);
            });
    });

    server.on(F("/api/fan/4"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning on fan with speed 4",
            [](olas::Controller& ctl) {
                ctl.fan_on(4);
            });
    });

    server.on(F("/api/fan/5"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning on fan with speed 5",
            [](olas::Controller& ctl) {
                ctl.fan_on(5);
            });
    });

    server.on(F("/api/fan/6"), HTTP_POST, []() {
        FanControllerWebServer::get_instance().schedule(
            "Turning on fan with speed 6",
            [](olas::Controller& ctl) {
                ctl.fan_on(6);
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
    if (millis() - task_added < TASK_EXECUTION_WAIT_PERIOD) {
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
