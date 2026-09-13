#include <ESP8266WiFi.h>
#include <RadioLib.h>
#include <SPI.h>

#define PIN_CS 15
#define PIN_GDO0 4
#define PIN_GDO2 5

#define CC1101_REG_IOCFG0 0x02
#define CC1101_REG_PKTCTRL0 0x08

#define BUFFER_CAPACITY 2000
#define MAX_EDGE_COUNT_PER_ROW 10

volatile uint64_t edge_times[BUFFER_CAPACITY];
volatile bool edge_levels[BUFFER_CAPACITY];
volatile uint32_t edge_count = 0;
volatile uint64_t last_edge_time = 0;

// For moving edges and durations out of volatile states
uint64_t current_edge_times[BUFFER_CAPACITY];
bool current_edge_levels[BUFFER_CAPACITY];

uint16_t edges_in_row = 0;

CC1101 radio = new Module(PIN_CS, PIN_GDO0, RADIOLIB_NC, PIN_GDO2);

void write_cc1101_reg(uint8_t addr, uint8_t value)
{
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(addr);
    SPI.transfer(value);
    digitalWrite(PIN_CS, HIGH);
}

void IRAM_ATTR on_edge()
{
    uint64_t now = micros();

    if (edge_count < BUFFER_CAPACITY) {
        edge_levels[edge_count] = digitalRead(PIN_GDO0);
        edge_times[edge_count] = now - last_edge_time;
        ++edge_count;
    }

    last_edge_time = now;
}

void setup()
{
    Serial.begin(74880);
    delay(1000);

    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();

    int state = radio.begin(433.92, 4.8, 5.0, 325.0, 10, 16);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print(F("Cannot initialize CC1101 module."));
        Serial.println(state);
        delay(5000);
        ESP.reset();
    }

    radio.setOOK(true);
    radio.startReceive();

    write_cc1101_reg(CC1101_REG_IOCFG0, 0x0D);
    write_cc1101_reg(CC1101_REG_PKTCTRL0, 0x32);

    pinMode(PIN_GDO0, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_GDO0), on_edge, CHANGE);

    Serial.println(F("Listening for remote signal..."));
}

void loop()
{
    delay(1000);

    if (edge_count < 10) {
        return;
    }

    noInterrupts();
    uint32_t current_edge_count = edge_count;

    for (uint32_t i = 0; i < current_edge_count; ++i) {
        current_edge_times[i] = edge_times[i];
        current_edge_levels[i] = edge_levels[i];
    }

    edge_count = 0;
    interrupts();

    for (uint32_t i = 0; i < current_edge_count; ++i) {
        if (edges_in_row > MAX_EDGE_COUNT_PER_ROW) {
            Serial.println();
            edges_in_row = 0;
        }

        Serial.print(current_edge_levels[i] ? "H[" : "L[");
        Serial.print(current_edge_times[i]);
        Serial.print("] ");

        ++edges_in_row;
    }
}
