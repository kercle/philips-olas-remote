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

    server.on("/api/light/on", HTTP_POST, []() {
        Serial.println(F("Turning on lights"));
        FanControllerWebServer::get_instance().schedule([](olas::Controller& ctl) {
            ctl.light_on();
        },
            "Turning on lights");
    });

    server.onNotFound([]() {
        server.send(404, "text/plain", "Not found.");
    });

    server.begin();
}

void FanControllerWebServer::schedule(CmdFn fn, const char* msg)
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
