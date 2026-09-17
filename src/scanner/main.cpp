#include <ESP8266WiFi.h>
#include <RadioLib.h>
#include <SPI.h>

#define PIN_CS 15
#define PIN_GDO0 4
#define PIN_GDO2 5

#define CC1101_REG_IOCFG0 0x02
#define CC1101_REG_PKTCTRL0 0x08

#define BUFFER_CAPACITY 2000
#define MAX_EDGE_COUNT_PER_ROW 100

volatile uint64_t edge_times[BUFFER_CAPACITY];
volatile bool edge_levels[BUFFER_CAPACITY];
volatile uint32_t edge_count = 0;
volatile uint64_t last_edge_time = 0;

// For moving edges and durations out of volatile states
uint64_t current_edge_times[BUFFER_CAPACITY];
bool current_edge_levels[BUFFER_CAPACITY];

uint16_t edges_in_row = 0;

Module radio_module(PIN_CS, PIN_GDO0, RADIOLIB_NC, PIN_GDO2);
CC1101 radio(&radio_module);

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
    Serial.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();

    int state = radio.begin(434, 4.8, 5.0, 300.0, 10, 16);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print(F("Cannot initialize CC1101 module."));
        Serial.println(state);
        delay(5000);
        ESP.reset();
    }

    radio.setOOK(true);
    radio.startReceive();

    radio_module.SPIwriteRegister(CC1101_REG_IOCFG0, 0x0D);
    radio_module.SPIwriteRegister(CC1101_REG_PKTCTRL0, 0x32);

    pinMode(PIN_GDO0, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_GDO0), on_edge, CHANGE);

    Serial.println(F("Listening for remote signal..."));
}

enum class EdgeClass : uint8_t {
    ShortOn,
    ShortOff,
    LongOn,
    LongOff,
    SyncOn,
    SyncOff,
    Unknown,
};

EdgeClass filter(uint64_t duration, bool carrier_on)
{
    constexpr float p = 0.15;
    constexpr uint32_t window_width = 140;

    constexpr uint64_t short_duration = 340;
    // constexpr uint64_t short_lower_bound = short_duration * (1.0 - p);
    // constexpr uint64_t short_upper_bound = short_duration * (1.0 + p);
    constexpr uint64_t short_lower_bound = short_duration - window_width / 2;
    constexpr uint64_t short_upper_bound = short_duration + window_width / 2;

    constexpr uint64_t long_duration = 730;
    // constexpr uint64_t long_lower_bound = long_duration * (1.0 - p);
    // constexpr uint64_t long_upper_bound = long_duration * (1.0 + p);
    constexpr uint64_t long_lower_bound = long_duration - window_width / 2;
    constexpr uint64_t long_upper_bound = long_duration + window_width / 2;

    constexpr uint64_t sync_on_duration = 7400;
    // constexpr uint64_t sync_on_lower_bound = sync_on_duration * (1.0 - p);
    // constexpr uint64_t sync_on_upper_bound = sync_on_duration * (1.0 + p);
    constexpr uint64_t sync_on_lower_bound = sync_on_duration - window_width / 2;
    constexpr uint64_t sync_on_upper_bound = sync_on_duration + window_width / 2;

    constexpr uint64_t sync_off_duration = 1090;
    // constexpr uint64_t sync_off_lower_bound = sync_off_duration * (1.0 - p);
    // constexpr uint64_t sync_off_upper_bound = sync_off_duration * (1.0 + p);
    constexpr uint64_t sync_off_lower_bound = sync_off_duration - window_width / 2;
    constexpr uint64_t sync_off_upper_bound = sync_off_duration + window_width / 2;

    if (short_lower_bound <= duration && duration <= short_upper_bound) {
        return carrier_on
            ? EdgeClass::ShortOn
            : EdgeClass::ShortOff;
    }

    if (long_lower_bound <= duration && duration <= long_upper_bound) {
        return carrier_on
            ? EdgeClass::LongOn
            : EdgeClass::LongOff;
    }

    if (sync_on_lower_bound <= duration && duration <= sync_on_upper_bound) {
        return carrier_on
            ? EdgeClass::SyncOn
            : EdgeClass::Unknown;
    }

    if (sync_off_lower_bound <= duration && duration <= sync_off_upper_bound) {
        return carrier_on
            ? EdgeClass::Unknown
            : EdgeClass::SyncOff;
    }

    return EdgeClass::Unknown;
}

const char* edge_class_symbol(EdgeClass c)
{
    switch (c) {
    case EdgeClass::ShortOn:
        return "▔";

    case EdgeClass::ShortOff:
        return "▁";

    case EdgeClass::LongOn:
        return "▀";

    case EdgeClass::LongOff:
        return "▄";

    case EdgeClass::SyncOn:
        return "█";

    case EdgeClass::SyncOff:
        return "▂";

    default:
        return "▒";
    }
}

void loop()
{
    delay(250);

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
        auto cls = filter(current_edge_times[i], !current_edge_levels[i]);

        if (edges_in_row > MAX_EDGE_COUNT_PER_ROW) {
            Serial.println();
            edges_in_row = 0;
        }

        Serial.print(edge_class_symbol(cls));

        ++edges_in_row;
    }
}
