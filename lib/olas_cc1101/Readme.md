# Library `olas_cc1101`

This library implements the low-level radio transmitter based around the CC1101 chip and the ESP8266/ESP32 compatible library `RadioLib`. The library was tested on an ESP8266 chip. Since the implementation uses relatively modern C++ language features and STL it probably compile on an Arduino out-of-the-box.

The library also provides a reference for porting the provided functionality to other wireless modules.

To support another wireless module, the following implementation bridges the communication between the `Controller` class and the hardware

```cpp
class RadioTransmitterSomeModule : public olas::RadioTransmitter {
    ...
public:
    ...

    virtual bool transmit(olas::Frame frm) {
        ...
    }

    virtual bool transmit_bursts(olas::Frame frm, uint8_t repeats) {
        ...
    }

    virtual std::optional<olas::Frame> receive_frame() {
        ...
    }
};
```
