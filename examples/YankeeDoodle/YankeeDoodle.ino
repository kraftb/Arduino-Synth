
#include <avr/pgmspace.h>
#include <Synth.h>

// Yankee doodle dandy

const byte track1_notes[] PROGMEM = {
	67, 195, 67, 195, 69, 197, 71, 199, 67, 195, 71, 199, 69, 197, 62, 190, 
	67, 195, 67, 195, 69, 197, 71, 199, 67, 195, 66, 194, 67, 195, 67, 195, 
	69, 197, 71, 199, 72, 200, 71, 199, 69, 197, 67, 195, 66, 194, 62, 190, 
	64, 192, 66, 194, 67, 195, 67, 195
};
const word track1_timeOffsets[] PROGMEM = {
	0, 256, 0, 256, 0, 256, 0, 256, 0, 256, 0, 256, 0, 256, 0, 256, 
	0, 256, 0, 256, 0, 256, 0, 256, 0, 512, 0, 512, 0, 256, 0, 256, 
	0, 256, 0, 256, 0, 256, 0, 256, 0, 256, 0, 256, 0, 256, 0, 256, 
	0, 256, 0, 256, 0, 512, 0, 512
};

const SYNTH_TRACK tracks[] PROGMEM = {
	// TRACK 1
	{
		.notes = track1_notes,
		.timeOffsets = track1_timeOffsets,
		.noteCount = 56,
	},
};


void setup() {
	Synth.init(3);
}


void loop() {
	if (!Synth.isPlaying()) {
		delay(1000);
		Synth.play(tracks, 1, 29);
	} else {
		// Do something. ~3/5 ths of the CPU cycles are still available.
	}
}

