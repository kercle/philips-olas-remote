# PHILIPS Olas ceiling fan

This little project aims at implementing the protocol of the PHILIPS Olas ceiling fan for the purpose of home automation. The software was implemented for an ESP8266 micro-controller (NodeMCU) connected to a CC1101 module, but it can easily be adopted to other platforms.

## Hardware

The remote can be prototyped on a simple breadboard. The circuit diagram using a NodeMCU (ESP8266) together with a CC1101 transceiver module (AYWHP) is given as follows:

![Circuit diagram for NodeMCU](./assets/circuit-diag-simple.png)

## Protocol

The protocol was recorded using the firmware compiled from the `scanner` build-target. The firmware records permanently records high/low edges together with the time passed since the last detected edge. The protocol extraction was partly supported by Claude.

### Physical layer

| Parameter        | Value            |
| :--------------- | :--------------- |
| Frequency        | 433.92 MHz       |
| Modulation       | OOK (ASK)        |
| Carrier ON       | logical 1 in raw RF bitstream |
| Carrier OFF      | logical 0 in raw RF bitstream |

### Packet timing

#### Sync marker (one per frame, before data bits)

| Field      | RF State     | Duration   |
| :--------- | :----------- | :--------- |
| Sync ON    | Carrier ON   | ~7 400 µs  |
| Sync OFF   | Carrier OFF  | ~1 090 µs  |

#### Data bits

Each logical bit has a total period of ~1 070 µs.

| Bit value | RF sequence              | ON duration | OFF duration |
| :-------- | :----------------------- | :---------- | :----------- |
| **0**     | short ON, long OFF       | ~340 µs     | ~730 µs      |
| **1**     | long ON, short OFF       | ~735 µs     | ~340 µs      |

### Frame repetition

A single button press transmits **4–6 identical frames** back-to-back
(no gap between frames; the next frame's sync marker follows immediately
after the last data bit of the previous frame).

## Packet Structure

Each frame carries **41 bits**, transmitted MSB first.

```
Bits:
40 [ - - - - - - - - - - - - - - - - - - - - - - - - ] 17
     Fan ID: 24 bits
16 [ - - - - - - - - ]  9
     Command: 8 bits
8  [ - - - - - - - - ]  1
     Check: 8 bits
0  [ 0 ]  0
```

| Field     | Bits  | Width | Description                              |
| :-------- | :---- | :---- | :--------------------------------------- |
| Fan ID | 40–17 | 24    | Fixed hardware ID of the remote          |
| Command   | 16–9  | 8     | Upper 6 bits = function; lower 2 = sequence counter |
| Check     | 8–1   | 8     | Integrity byte: `command XOR 0x5B`       |
| Trailer   | 0     | 1     | Always `0`                               |

The two sequence counter bits continuous decrement, i.e. assuming the current counter position is `2`, the next four packets will contain the counter position `1`, `0`, `3` and `2` in that order.

Frames captured in the same physical button press all carry the **same**
sequence value.

### Complete Command Table

All function codes below are the base value with sequence bits zeroed
(`functionCode & 0xFC`).

| Button               | Function Code | Check (seq = 0) |
| :------------------- | :-----------: | :-------------: |
| Fan Off              | `0x10`        | `0x4B`          |
| Fan Speed 1          | `0x40`        | `0x1B`          |
| Fan Speed 2          | `0xAC`        | `0xF7`          |
| Fan Speed 3          | `0x9C`        | `0xC7`          |
| Fan Speed 4          | `0x20`        | `0x7B`          |
| Fan Speed 5          | `0x80`        | `0xDB`          |
| Fan Speed 6          | `0x8C`        | `0xD7`          |
| Fan Forward/Reverse  | `0x50`        | `0x0B`          |
| Fan Sleep            | `0x30`        | `0x6B`          |
| Fan Timer 1 h        | `0x1C`        | `0x47`          |
| Fan Timer 3 h        | `0x18`        | `0x43`          |
| Fan Timer 6 h        | `0x14`        | `0x4F`          |
| Brightness Up        | `0x70`        | `0x2B`          |
| Brightness Down      | `0x28`        | `0x73`          |
| Warm White           | `0x6C`        | `0x37`          |
| Day White            | `0x84`        | `0xDF`          |
| Light On             | `0x7C`        | `0x27`          |
| Light Off            | `0xBC`        | `0xE7`          |
