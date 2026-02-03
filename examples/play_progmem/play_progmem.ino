#include <MIDIPlayer.h>
#include <MIDIReader.h>

#include "gravity.h"

// проект в симуляторе
// https://wokwi.com/projects/421869878672920577

#define BUZ_0 3
#define BUZ_1 4
#define BUZ_2 5
#define BUZ_3 6
#define BUZ_4 7
#define BUZ_5 8

MIDIPlayer* players_arr[] = {
  new MIDIPlayerSoft(BUZ_0),
  new MIDIPlayerSoft(BUZ_1),
  new MIDIPlayerSoft(BUZ_2),
  new MIDIPlayerSoft(BUZ_3),
  new MIDIPlayerSoft(BUZ_4),
  new MIDIPlayerSoft(BUZ_5),
};
MIDIPlayerMulti players(players_arr, sizeof(players_arr) / sizeof(players_arr[0]));

MIDIReader midi(&players, gravity_falls, gravity_falls_size);

void setup() {
  Serial.begin(115200);
  midi.start();
}

void loop() {
  midi.tick();
}