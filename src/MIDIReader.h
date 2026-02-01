#pragma once
#include <Arduino.h>

#include "MIDINote.h"
#include "MIDIPlayer.h"

// ================ MIDIReader ================
// играет массив внешним плеером
class MIDIReader {
   public:
    MIDIReader() {}
    MIDIReader(const MIDINote* notes, size_t size, bool pgm = true) {
        setNotes(notes, size, pgm);
    }
    MIDIReader(MIDIPlayer* player, const MIDINote* notes, size_t size, bool pgm = true) : MIDIReader(notes, size, pgm) {
        setPlayer(player);
    }

    // подключить плеер
    void setPlayer(MIDIPlayer* player) {
        _player = player;
    }

    // подключить ноты
    void setNotes(const MIDINote* notes, size_t size, bool pgm = true) {
        _notes = notes;
        _len = size / sizeof(MIDINote);
        _pgm = pgm;
        stop();
    }

    // установить сдвиг частоты (в тонах)
    void setShift(int8_t shift) {
        _shift = shift;
    }

    // запустить воспроизведение
    void start() {
        if (!_notes || !_player) return;
        _tmr = millis();
        if (!_tmr) --_tmr;
        _i = 0;
        _play();
    }

    // остановить воспроизведение
    void stop() {
        _tmr = 0;
        if (_player) _player->stop();
    }

    // воспроизводится
    bool isPlaying() {
        return _tmr;
    }

    // тикер, вызывать в loop. Вернёт true в конце воспроизведения
    bool tick() {
        if (_player) _player->tick();

        if (_tmr && (uint16_t)((uint16_t)millis() - _tmr) >= _note.delay) {
            _tmr += _note.delay;
            if (++_i >= _len) {
                stop();
                return true;
            } else {
                _play();
            }
        }
        return false;
    }

   private:
    const MIDINote* _notes = nullptr;
    MIDIPlayer* _player = nullptr;
    uint32_t _tmr = 0;
    uint16_t _len = 0;
    uint16_t _i = 0;
    MIDINote _note;
    int8_t _shift = 0;
    bool _pgm = true;

    void _play() {
        if (!_notes) return;

        if (_pgm) memcpy_P(&_note, _notes + _i, sizeof(MIDINote));
        else _note = _notes[_i];

        _note.note += _shift;

        uint16_t dur = _note.getDur();
        // if (dur == _note.delay) dur -= dur >> 2;
        if (dur) _player->play(_note.note, dur);
    }
};