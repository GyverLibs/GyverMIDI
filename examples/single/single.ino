#define BUZ_1 10

#include "GyverMIDI.h"
#include "mario.h"

GyverMIDISoft midi0(BUZ_1, track_0, sizeof(track_0));

void setup() {
    midi0.start();
}

void loop() {
    midi0.tick();
}
