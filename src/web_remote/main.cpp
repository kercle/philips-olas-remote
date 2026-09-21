#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include <config.h>
#include <secrets.h>

#include <olas/controller.h>
#include <olas_cc1101/radio.h>

#include <web_remote/assets/web/index.html.h>
#include <web_remote/web_server.h>

olas_cc1101::RadioTransmitterCC1101 radio(config::pin_cs, config::pin_gdo0, config::pin_gdo2);
olas::Controller controller(radio, secrets::fan_id);

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

    auto res = radio.initialize();
    if (res.is_err()) {
        Serial.print(F("Radio transmitter initialization failed: "));
        Serial.println(res.code());
        delay(5000);
        ESP.reset();
    }
}

void loop()
{
    static auto fan_state = controller.get_state();

    FanControllerWebServer& web_server = FanControllerWebServer::get_instance();

    auto task = web_server.take_task();
    if (task.has_value()) {
        (*task)(controller);
    }

    controller.handle_received_data();

    if (fan_state != controller.get_state()) {
        fan_state = controller.get_state();
        Serial.print("Light: ");
        Serial.println(fan_state.light);
        Serial.print("Fan: ");
        Serial.println(fan_state.fan);
        Serial.print("Reversed: ");
        Serial.println(fan_state.reversed);
    }

    web_server.handle_client();
    MDNS.update();
    yield();
}
