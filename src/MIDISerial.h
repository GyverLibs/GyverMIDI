#pragma once
#include <Arduino.h>

// MIDICmd
enum class MIDICmd : uint8_t {
    None = 0,
    NoteOff = 0x80,
    NoteOn = 0x90,
    Aftertouch = 0xA0,
    Continuous = 0xB0,
    ProgChange = 0xC0,  // 1
    Pressure = 0xD0,    // 1
    PitchBend = 0xE0,
    Etc = 0xF0,
};

// MIDIMsg
struct MIDIMsg {
    MIDICmd cmd;
    uint8_t channel;
    uint8_t data[2];

    int8_t note() const {
        return data[0];
    }
    uint8_t velocity() const {
        return data[1];
    }
};

// MIDISerial
class MIDISerial {
    typedef void (*MsgCb)(const MIDIMsg&);

   public:
    MIDISerial(Stream& s) : _s(s) {}

    void onMIDI(MsgCb cb) {
        _cb = cb;
    }

    void noteOn(uint8_t channel, int8_t note, uint8_t velocity = 127) {
        _s.write(uint8_t(MIDICmd::NoteOn) | (channel - 1));
        _s.write(note);
        _s.write(velocity);
    }

    void noteOff(uint8_t channel, int8_t note, uint8_t velocity = 127) {
        _s.write(uint8_t(MIDICmd::NoteOff) | (channel - 1));
        _s.write(note);
        _s.write(velocity);
    }

    void pitchBend(uint8_t channel, int value) {
        value = constrain(value, 0, 16383);
        Serial.write(uint8_t(MIDICmd::PitchBend) | (channel - 1));
        Serial.write(value & 0x7F);
        Serial.write(value >> 7);
    }

    void tick() {
        while (_s.available()) {
            uint8_t b = _s.read();

            // realtime
            if (b >= 0xF8) {
                continue;
            }

            // system common / sysex
            if (b >= 0xF0) {
                _runn = 0;
                continue;
            }

            // status
            if (b & 0x80) {
                _runn = b;
                _msg.cmd = MIDICmd(b & 0xF0);
                _msg.channel = (b & 0x0F);
                _idx = 0;
                continue;
            }

            // data byte
            if (!_runn) {
                continue;
            }

            _msg.data[_idx] = b;

            uint8_t needed = (_msg.cmd == MIDICmd::ProgChange || _msg.cmd == MIDICmd::Pressure) ? 1 : 2;

            if (++_idx >= needed) {
                _idx = 0;
                if (_cb) _cb(_msg);
            }
        }
    }

   private:
    MsgCb _cb = nullptr;
    Stream& _s;

    MIDIMsg _msg;
    uint8_t _runn = 0;
    uint8_t _idx = 0;
};