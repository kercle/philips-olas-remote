#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <RadioLib.h>

#include <secrets.h>
#include <web_remote/assets/index.html.h>

ESP8266WebServer server(80);

void setup_wifi()
{
    WiFi.mode(WIFI_STA);
    WiFi.hostname("philips-olas-remote");
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    Serial.print(F("Connecting to Wi-Fi"));
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print('.');
    }

    Serial.println(F(" OK"));
}

void setup_webserver()
{
    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html", assets::INDEX_HTML);
    });

    server.onNotFound([]() {
        server.send(404, "text/plain", "Not found.");
    });

    server.begin();

    Serial.print(F("Web server ready.\nGo to http://"));
    Serial.print(WiFi.localIP());
    Serial.print('/');
    Serial.println(" to access the dashboard.");
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println(F("\nPhilips Ceiling Fan Controller"));

    setup_wifi();
    setup_webserver();
}

void loop()
{
    server.handleClient();
    yield();
}
