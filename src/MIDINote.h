#pragma once
#include <inttypes.h>

// ================ MIDINote ================
struct MIDINote {
    int8_t note;
    uint8_t duration;  // exp int 3/5
    uint16_t delay;

    static constexpr uint8_t exp_bits = 3;
    static constexpr uint8_t man_bits = 8 - exp_bits;
    static constexpr uint8_t man_base = 1ul << man_bits;
    static constexpr uint8_t man_mask = man_base - 1;
    static constexpr uint8_t exp_base = 1ul << exp_bits;
    static constexpr uint8_t exp_mask = exp_base - 1;

    constexpr uint16_t getDur() {
        return duration ? ((duration & man_mask) | man_base) << (duration >> man_bits) : 0;
    }

    static constexpr uint8_t encodeDur(uint16_t dur) {
#define _N_EXP_E ((int32_t)(31 - __builtin_clzl(dur)) - man_bits)
#define _N_EXP_E2 ((_N_EXP_E < 0) ? 0 : (_N_EXP_E > (int32_t)exp_mask ? exp_mask : _N_EXP_E))
        return (dur < man_base) ? 0 : ((_N_EXP_E2 << man_bits) | ((dur >> _N_EXP_E2) & man_mask));
    }
};