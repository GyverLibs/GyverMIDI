#pragma once
#include <Arduino.h>
#include <GyverIO.h>

class GyverMIDI {
   public:
    struct Note {
        uint16_t us, duration, delay;
    };

    GyverMIDI() {}
    GyverMIDI(const Note* notes, size_t len) {
        setNotes(notes, len);
    }

    void setNotes(const Note* notes, size_t len) {
        _notes = notes;
        _len = len / sizeof(Note);
    }

    void start() {
        _tmr = millis();
        if (!_tmr) --_tmr;
        _i = 0;
        load(0);
        if (_note.duration) beep(_note.us, _note.duration);
    }

    void stop() {
        _tmr = 0;
    }

    bool isPlaying() {
        return _tmr;
    }

    virtual void tick() {
        if (_tmr && (uint16_t)((uint16_t)millis() - _tmr) >= _note.delay) {
            _tmr += _note.delay;
            if (++_i >= _len) {
                stop();
            } else {
                load(_i);
                if (_note.duration) beep(_note.us, _note.duration);
            }
        }
    }

    virtual void beep(uint16_t us, uint16_t duration) {}

   protected:
    virtual void load(uint16_t i) {
        memcpy_P(&_note, _notes + i, sizeof(Note));
    }

   private:
    const Note* _notes = nullptr;
    Note _note;
    uint16_t _tmr;
    uint16_t _len = 0;
    uint16_t _i = 0;
};

class GyverMIDISoft : public GyverMIDI {
   public:
    GyverMIDISoft(uint8_t pin, const Note* notes, size_t len) : GyverMIDI(notes, len), _pin(pin) {
        gio::init(pin, OUTPUT);
    }

    void beep(uint16_t us, uint16_t duration) {
        _tmr = micros();
        _us = us / 2;
        _flag = false;
        _ticks = (1000ul * duration / us) * 2;
    }

    void tick() {
        GyverMIDI::tick();
        if (_ticks && (uint16_t)((uint16_t)micros() - _tmr) >= _us) {
            _tmr += _us;
            gio::write(_pin, _flag = !_flag);
            --_ticks;
        }
    }

   private:
    uint16_t _tmr, _us;
    uint16_t _ticks;
    uint8_t _pin;
    bool _flag;
};

class GyverMIDIMulti {
    typedef void (*EndCallback)();

   public:
    GyverMIDIMulti(uint8_t channels) : _len(channels) {
        _channels = new GyverMIDI*[_len]();
    }
    ~GyverMIDIMulti() {
        delete[] _channels;
    }

    void setChannel(uint8_t n, GyverMIDI& midi) {
        _channels[n] = &midi;
    }

    void onEnd(EndCallback cb) {
        _cb = cb;
    }

    void start() {
        _start = true;
        for (uint8_t i = 0; i < _len; i++) {
            _channels[i]->start();
        }
    }

    void stop() {
        _start = false;
        for (uint8_t i = 0; i < _len; i++) {
            _channels[i]->stop();
        }
    }

    bool isPlaying() {
        for (uint8_t i = 0; i < _len; i++) {
            if (_channels[i]->isPlaying()) return true;
        }
        return false;
    }

    void tick() {
        for (uint8_t i = 0; i < _len; i++) {
            _channels[i]->tick();
        }
        if (_cb && _start) {
            if (!isPlaying()) {
                _start = false;
                _cb();
            }
        }
    }

   private:
    GyverMIDI** _channels = nullptr;
    EndCallback _cb = nullptr;
    uint8_t _len;
    bool _start = false;
};