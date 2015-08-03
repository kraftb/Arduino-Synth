#ifndef SYNT_H
#define SYNT_H

#include <Arduino.h>

#define SYNTH__PWM_FREQUENCY 					62500		// 1/s
#define SYNTH__PWM_CYCLES_PER_SAMPLE		5			// 1/S
#define SYNTH__SAMPLES_PER_SECOND			( SYNTH__PWM_FREQUENCY / SYNTH__PWM_CYCLES_PER_SAMPLE )		// S/s

#define SYNTH__MAX_OSCILLATORS				4

	// #define SYNTH__TICKS_PER_BEAT					96.0		// T/B
	// #define SYNTH__BPM								120.0		// B/60s
	// #define SYNTH__TICKS_PER_SECOND			( ( SYNTH__TICKS_PER_BEAT * SYNTH__BPM ) / 60.0 )		// T/s
	// #define SYNTH__TICKS_PER_SECOND			(160)		// T/s
	// #define SYNTH__SAMPLES_PER_TICK			( (word) ( SYNTH__SAMPLES_PER_SECOND / SYNTH__TICKS_PER_SECOND ) )		// S/T

// Take care not to mix up NOTE_INDEX_PAUSE which is the number of the note which shall get recognized as
// pause. In fact every note above 0x7F will get played as pause as MIDI designates those notes as "note-off"
// events.
// The NOTE_INDEX_PAUSE value could get easily mixed up with ANALOG_HALF which is also 0x80 but means the
// analog value which will get outputed for silence. As the PWM gets used as DAC writing a value of "0" would
// leave the pin off, a value of 0xFF would force the pin to HIGH (+5V) and a value of 0x80 (ANALOG_HALF)
// should yield a 50% duty cycle which should put the pin at about +2.5V.
#define SYNTH__NOTE_PAUSE			0x80
#define SYNTH__ANALOG_FULL			0xFF
#define SYNTH__ANALOG_HALF			0x80

#if F_CPU != 16000000L
	#error Timing tables have been precalculated for 16MHz. Compiling for different CPU frequency.
#endif

// Bit clear/set macros. Taken from Wire.h
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


typedef struct _SYNTH_TRACK
{
	const byte *notes PROGMEM;
	const word *timeOffsets PROGMEM;
	const word noteCount PROGMEM;
} SYNTH_TRACK;

class SynthClass {

	private:
	// Private methods. See implementation for inline documentation
	byte getSineValue(word index);
	byte calculateNextSampleValue();
	void handleTick();

	/*****************************
	 ** TRACK VARIABLES
	 ****************************/

	// Contains number of tracks being currently played.
	byte trackCount;

	// This determines the speed of a song. It defines the length of
	// a midi tick (clock) in samples. So making this value larger
	// will result in slower play.
	word samplesPerTick;

	// Points to the tracks which should get played
	SYNTH_TRACK *tracks;


	/*****************************
	 ** OSCILLATORS VARIABLES
	 ****************************/

	// This variable array contains the index of the
	// note in its track. If a track reaches its last
	// note (noteIndex >= noteCount) the track will
	// continue to play a pause.
	word noteIndex[SYNTH__MAX_OSCILLATORS];

	// Those variables contain the current note and
	// duration which is being played by each oscillator.
	byte currentNote[SYNTH__MAX_OSCILLATORS];
	word currentDuration[SYNTH__MAX_OSCILLATORS];

	// Those variables contain the current period
	// position (offset in the sine wave) for each
	// oscillator.
	word currentPeriodIndex[SYNTH__MAX_OSCILLATORS];
	word currentPeriodFraction[SYNTH__MAX_OSCILLATORS];


	/*****************************
	 ** COUNTER VARIABLES
	 ****************************/

	// This contains the value to which the PWM pin will
	// get set for the next sample period.
	// This is the first thing done in the TIMER0 ISR when
	// pwmCounter reaches zero. This ensures that the
	// output value is updated at a precisely regular
	// interval.
	// After the value has been written to PWM pin the next
	// value for it will get calculated.
	byte nextSampleValue;

	// Counts number of interrupts until PWM gets updated
	// with a new value (sample period). So this variable
	// will become zero 12500 times a second.
	byte pwmCounter;

	// Counts number of samples having been played. After
	// "SYNTH_SAMPLES_PER_TICK" samples being played this
	// variable will reach zero and a tick will get issued.
	word sampleCounter;

	// Defines the register which to use to output a
	// sample value. Will usually be the PWM register
	// for the selected pin.
	intptr_t sampleRegister;

	// Public methods which can get called from an Arduino sketch
	public:
	void init(uint8_t pin);
	void play(const SYNTH_TRACK *_tracks, byte _trackCount, word _samplesPerTick);
	void stop();
	bool isPlaying();
	void handleIsr();

};

extern SynthClass Synth;

#endif

