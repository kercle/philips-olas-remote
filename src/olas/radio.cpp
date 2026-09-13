#include <olas/radio.h>

namespace olas {

class WaveFormBuilder {
};

RadioTransmitter::RadioTransmitter(FrameBuilder& frame_builder, CC1101 radio)
    : frame_builder(frame_builder)
    , radio(radio)
{
}

}
