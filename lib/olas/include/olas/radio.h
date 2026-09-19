#pragma once

#include <optional>

#include <olas/protocol.h>

namespace olas {

class RadioTransmitter {
public:
    virtual bool transmit(Frame frm) = 0;
    virtual bool transmit_bursts(Frame frm, uint8_t repeats) = 0;

    virtual std::optional<Frame> receive_frame() = 0;
};

}
