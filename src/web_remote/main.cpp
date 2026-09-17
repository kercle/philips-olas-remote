#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include <config.h>
#include <secrets.h>

#include <olas/controller.h>

#include <web_remote/assets/index.html.h>
#include <web_remote/web_server.h>

olas::Controller controller(secrets::fan_id);

void setup_wifi()
{
    WiFi.mode(WIFI_STA);
    WiFi.hostname(config::hostname);
    WiFi.begin(secrets::wifi_ssid, secrets::wifi_pass);
    MDNS.begin(config::hostname);

    Serial.print(F("Connecting to Wi-Fi"));
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print('.');
    }

    Serial.println(F(" OK"));
}

void setup_webserver()
{
    FanControllerWebServer::get_instance().initialize();

    Serial.print(F("Web server ready.\nGo to http://"));
    Serial.print(WiFi.localIP());
    Serial.print(F("/ or http://"));
    Serial.print(config::hostname);
    Serial.println(".local/ to access the dashboard.");
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println(F("\nPhilips Ceiling Fan Controller"));

    setup_wifi();
    setup_webserver();

    auto res = controller.initialize();
    if (res.is_err()) {
        Serial.print(F("Radio transmitter initialization failed: "));
        Serial.println(res.code());
        delay(5000);
        ESP.reset();
    }
}

void loop()
{
    FanControllerWebServer& web_server = FanControllerWebServer::get_instance();

    auto task = web_server.take_task();
    if (task.has_value()) {
        (*task)(controller);
    }

    web_server.handle_client();
    MDNS.update();
    yield();
}
