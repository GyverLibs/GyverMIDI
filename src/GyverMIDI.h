#pragma once
#include <Arduino.h>
#include <GyverIO.h>

// ================ GyverMIDI ================
class GyverMIDI {
   public:
    struct Note {
        uint16_t us, duration, delay;
    };

    GyverMIDI() {}
    GyverMIDI(const Note* notes, size_t len) {
        setNotes(notes, len);
    }

    // подключить ноты
    void setNotes(const Note* notes, size_t len) {
        _notes = notes;
        _len = len / sizeof(Note);
        stop();
    }

    // запустить воспроизведение
    void start() {
        _tmr = millis();
        if (!_tmr) --_tmr;
        _i = 0;
        load(0);
        if (_note.duration) beep(_note.us, _note.duration);
    }

    // остановить воспроизведение
    void stop() {
        _tmr = 0;
    }

    // воспроизводится
    bool isPlaying() {
        return _tmr;
    }

    // тикер, вызывать в loop. Вернёт true в конце воспроизведения
    virtual bool tick() {
        if (_tmr && (uint16_t)((uint16_t)millis() - _tmr) >= _note.delay) {
            _tmr += _note.delay;
            if (++_i >= _len) {
                stop();
                return true;
            } else {
                load(_i);
                if (_note.duration) beep(_note.us, _note.duration);
            }
        }
        return false;
    }

   protected:
    virtual void beep(uint16_t us, uint16_t duration) {}

    virtual void load(uint16_t i) {
        memcpy_P(&_note, _notes + i, sizeof(Note));
    }

    const Note* _notes = nullptr;
    Note _note;

   private:
    uint16_t _tmr;
    uint16_t _len = 0;
    uint16_t _i = 0;
};

// ================ GyverMIDISoft ================
class GyverMIDISoft : public GyverMIDI {
   public:
    GyverMIDISoft(uint8_t pin, const Note* notes, size_t len) : GyverMIDI(notes, len), _pin(pin) {
        gio::init(pin, OUTPUT);
    }

    // тикер, вызывать в loop. Вернёт true в конце воспроизведения
    bool tick() override {
        if (_ticks && (uint16_t)((uint16_t)micros() - _tmr) >= _us) {
            _tmr += _us;
            gio::write(_pin, _flag = !_flag);
            --_ticks;
        }
        return GyverMIDI::tick();
    }

   protected:
    void beep(uint16_t us, uint16_t duration) {
        _tmr = micros();
        _us = us / 2;
        _flag = false;
        _ticks = (1000ul * duration / us) * 2;
    }

   private:
    uint16_t _tmr, _us;
    uint16_t _ticks;
    uint8_t _pin;
    bool _flag;
};

// ================ GyverMIDIMulti ================
class GyverMIDIMulti {
    typedef void (*EndCallback)();

   public:
    GyverMIDIMulti(uint8_t channels) : _len(channels) {
        _channels = new GyverMIDI*[_len]();
    }
    ~GyverMIDIMulti() {
        delete[] _channels;
    }

    // подключить канал
    void setChannel(uint8_t n, GyverMIDI& midi) {
        _channels[n] = &midi;
    }

    // обработчик конца воспроизведения
    void onEnd(EndCallback cb) {
        _cb = cb;
    }

    // запустить воспроизведение
    void start() {
        _start = true;
        for (uint8_t i = 0; i < _len; i++) {
            _channels[i]->start();
        }
    }

    // остановить воспроизведение
    void stop() {
        _start = false;
        for (uint8_t i = 0; i < _len; i++) {
            _channels[i]->stop();
        }
    }

    // воспроизводится
    bool isPlaying() {
        for (uint8_t i = 0; i < _len; i++) {
            if (_channels[i]->isPlaying()) return true;
        }
        return false;
    }

    // тикер, вызывать в loop
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