#pragma once
#include <Arduino.h>
#include <GyverIO.h>

#include "MIDIConv.h"

// ================ MIDIPlayer ================
// базовая пищалка на одну ноту
class MIDIPlayer {
   public:
    // тикер
    virtual bool tick() { return false; }

    // остановить ноту (-1 - остановить в любом случае). true если была остановка
    virtual bool stop(int8_t note) { return true; }

    // играть ноту длительностью в мс
    virtual void play(int8_t note, uint16_t duration) = 0;

    // плеер играет ноту (-1 - играет любую)
    virtual bool busy(int8_t note) { return false; }

    // остановить плеер
    void stop() {
        stop(-1);
    }

    // включить ноту
    void play(int8_t note) {
        play(note, 0xffff);
    }

    // плеер играет
    bool busy() {
        return busy(-1);
    }
};

// битбэнг пищалка
class MIDIPlayerSoft : public MIDIPlayer {
   public:
    MIDIPlayerSoft(uint8_t pin) : _pin(pin) {
        gio::init(_pin, OUTPUT);
    }

    virtual bool tick() override {
        if (_ticks && (uint16_t)((uint16_t)micros() - _tmr) >= _us) {
            _tmr += _us;
            --_ticks;
            gio::write(_pin, _flag = !_flag);
            return true;
        }
        return false;
    }

    void play(int8_t note, uint16_t duration) override {
        _count = (_note == note) ? _count + 1 : 1;
        _note = note;

        _tmr = micros();
        _us = getMIDIus(note);
        _flag = false;
        _ticks = (1000ul * duration / _us) * 2;
        _us /= 2;
    }

    bool stop(int8_t note) override {
        if (_note == note && _count && --_count) return true;

        if (note < 0 || _note == note) {
            gio::write(_pin, 0);
            _ticks = 0;
            _count = 0;
            return true;
        }
        return false;
    }

    bool busy(int8_t note) override {
        return note < 0 ? _ticks : _note == note;
    }

    using MIDIPlayer::play;
    using MIDIPlayer::stop;

   private:
    uint16_t _tmr, _us;
    uint16_t _ticks = 0;
    uint8_t _pin;
    uint8_t _count = 0;
    int8_t _note = 0;
    bool _flag;
};

// tone пищалка
// class MIDIPlayerTone : public MIDIPlayer {
//    public:
//     MIDIPlayerTone(uint8_t pin) : _pin(pin) {}

//     void play(int8_t note, uint16_t duration) override {
//         if (!duration && _note != note) return;
//         _note = note;

//         if (duration) tone(_pin, getMIDIhz(note), duration);
//         else stop();
//     }

//     void stop(int8_t note) override {
//         if (note < 0 || _note == note) noTone(_pin);
//     }

//     using MIDIPlayer::play;
//     using MIDIPlayer::stop;

//    private:
//     uint8_t _pin;
//     int8_t _note = -1;
// };

class MIDIFloppy : public MIDIPlayerSoft {
   public:
    MIDIFloppy(uint8_t step, uint8_t dir, uint16_t steps) : MIDIPlayerSoft(step), _dir(dir), _steps(steps) {
        pinMode(_dir, OUTPUT);
    }

    bool tick() {
        if (MIDIPlayerSoft::tick()) {
            if (!_step--) {
                _step = _steps;
                gio::write(_dir, !gio::read(_dir));
            }
        }
        return true;
    }

   private:
    uint8_t _dir;
    uint16_t _steps;
    uint16_t _step = 0;
};

// мульти пищалка
class MIDIPlayerMulti : public MIDIPlayer {
   public:
    MIDIPlayerMulti(MIDIPlayer** players, uint8_t len) : _players(players), _len(len) {}

    void setPlayers(MIDIPlayer** players, uint8_t len) {
        _players = players;
        _len = len;
    }

    bool tick() override {
        for (uint8_t i = 0; i < _len; i++) _players[i]->tick();
        return true;
    }

    void play(int8_t note, uint16_t duration) override {
        for (uint8_t i = 0; i < _len; i++) {
            if (!_players[i]->busy() || _players[i]->busy(note)) {
                _players[i]->play(note, duration);
                break;
            }
        }
    }

    bool stop(int8_t note) override {
        if (note < 0) {
            for (uint8_t i = 0; i < _len; i++) {
                _players[i]->stop();
            }
            return true;
        } else {
            for (uint8_t i = 0; i < _len; i++) {
                if (_players[i]->stop(note)) return true;
            }
            return false;
        }
    }

    using MIDIPlayer::play;
    using MIDIPlayer::stop;

   private:
    MIDIPlayer** _players;
    uint8_t _len;
};