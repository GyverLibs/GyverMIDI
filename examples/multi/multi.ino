#define BUZ_1 10
#define BUZ_2 11
#define BUZ_3 12

#include "GyverMIDI.h"
#include "mario.h"

GyverMIDISoft midi0(BUZ_1, track_0, sizeof(track_0));
GyverMIDISoft midi1(BUZ_2, track_1, sizeof(track_1));
GyverMIDISoft midi2(BUZ_3, track_2, sizeof(track_2));
GyverMIDIMulti midi(3);

void setup() {
    Serial.begin(115200);

    midi.setChannel(0, midi0);
    midi.setChannel(1, midi1);
    midi.setChannel(2, midi2);
    midi.start();

    midi.onEnd([]() {
        Serial.println("end");
    });
}

void loop() {
    midi.tick();
}
