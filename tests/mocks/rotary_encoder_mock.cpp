#include "rotary_encoder_mock.hpp"

namespace rotary_encoder
{
    rotary_encoder rotary_encoders[NUM_ROTARY_ENCODERS];

    int32_t rotary_encoder_fetch_counter(rotary_encoder *re)
    {
        int32_t delta = re->counter;
        re->counter = 0; // Reset counter after reading
        return delta;
    }

    uint8_t rotary_encoder_fetch_sw_state(rotary_encoder *re)
    {
        return re->sw_state;
    }

    void rotary_encoders_init(void)
    {
        for (int i = 0; i < NUM_ROTARY_ENCODERS; i++)
        {
            rotary_encoders[i].counter = 0;
            rotary_encoders[i].sw_state = ROTARY_ENCODER_SW_RELEASED;
        }
    }

    // Test helper functions
    void mock_set_encoder_delta(int encoder_index, int32_t delta)
    {
        if (encoder_index >= 0 && encoder_index < NUM_ROTARY_ENCODERS)
        {
            rotary_encoders[encoder_index].counter = delta;
        }
    }

    void mock_set_switch_state(int encoder_index, uint8_t state)
    {
        if (encoder_index >= 0 && encoder_index < NUM_ROTARY_ENCODERS)
        {
            rotary_encoders[encoder_index].sw_state = state;
        }
    }
}