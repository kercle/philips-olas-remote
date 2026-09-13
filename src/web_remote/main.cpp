#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <RadioLib.h>

#include <config.h>
#include <secrets.h>

#include <olas/protocol.h>
#include <olas/radio.h>

#include <web_remote/assets/index.html.h>

ESP8266WebServer server(80);
CC1101 radio = new Module(PIN_CS, PIN_GDO0, RADIOLIB_NC, PIN_GDO2);

olas::FrameBuilder frame_builder(FAN_ID, 0);
olas::RadioTransmitter transmitter(frame_builder, radio);

void assert_not_err(int16_t state, const __FlashStringHelper* msg)
{
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print(F("CC1101 begin failed: "));
        Serial.println(state);
        delay(1000);
        ESP.reset();
    }
}

void setup_radio()
{
    assert_not_err(
        radio.begin(RF_FREQUENCY_MHZ, RAW_BITRATE_KBPS, 5.0, 325.0, 10, 16),
        F("CC1101 begin failed: "));

    assert_not_err(
        radio.setOOK(true),
        F("setOOK failed: "));

    /*
       We want the FIFO bytes to be transmitted literally.

       Promiscuous mode is useful here even though we're
       transmitting rather than receiving: in the current
       RadioLib CC1101 implementation it disables automatic
       sync-word insertion and disables CRC.
    */
    assert_not_err(
        radio.setPromiscuousMode(true),
        F("setPromiscuousMode failed: "));

    // No address byte.
    assert_not_err(
        radio.disableAddressFiltering(),
        F("disableAddressFiltering failed: "));

    /*
       Fixed packet length means there is no automatically
       transmitted length byte.

       The entire over-the-air payload is our 62-byte waveform.
    */
    assert_not_err(
        radio.fixedPacketLengthMode(WAVEFORM_BYTES),
        F("fixedPacketLengthMode failed: "));
}

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

    setup_radio();
    setup_wifi();
    setup_webserver();
}

void loop()
{
    server.handleClient();
    yield();
}
