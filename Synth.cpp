/*
	Synt.h - A audio synthesizer for Arduino
	Bernhard Kraft <kraftb@think-open.at>
  
	History:
	2015-08-03 - Version 0.1 released working on ATMEGA328P (Arduino UNO)
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <Synth.h>

/**
 * This synthesizer library for the Arduino uses the PWM feature of the
 * Arduino pins 3 or 11 to synthesize an audio-range sine wave. Altough
 * it would be possible to synthesize any arbitrary frequency in a limited
 * frequency window this library is intended to synthesize only frequencies
 * of notes within the MIDI note range.
 *
 * The audio sampling frequency is hardcoded to 12.5 kHz. Thus according
 * to the discrete-signal-theorem the maximum frequency to generate will
 * be 6.25 kHz. The last note in the MIDI note range which falls within
 * this range is the note with index 114 with a frequency of 5919.91076 Hz.
 * So every note above this value (MIDI notes 115 - 127) will most probably
 * not get played correctly.
 *
 * This implementation uses a precalculated sine lookup table and precalculated
 * sine table incrementation steps for the configured sampling frequency of
 * 12.5 kHz. If one knows about the Arduino internal buildup he/she will probably
 * know that it runs from a 16Mhz oscillator. As we use an 8-bit PWM signal (256 steps)
 * to generate the audio signal the PWM frequency is 16Mhz / 256 = 62.5 kHz.
 *
 * As a single PWM pulse would most probably not be sufficient to generate the
 * desired analog output value we assume it takes 5 PWM cycles for the output
 * to reach the desired analog value. So the sampling frequency is the PWM
 * frequency divided by 5 again. Using fewer PWM cycles could allow to achieve
 * a higher sampling frequency. But this would most probably only make sense
 * for more accurate external D/A converters.
 *
 * I started to write this synthesizer so I can play tunes like "Zum tanze da
 * geht ein Mädel" or "Der Mond ist aufgegangen" from the Arduino to my little
 * son Jakob Kraft.
 */


/*
 * If you do not know much about trigonometrics (sin/cos/tan), etc. lets give you
 * a short introduction into the anatomy of a sine wave.
 *

 y = sin(x)

  +y /|\
      |
      |                x=PI/2               x=PI              x=3*PI/2             x=2*PI
      |                                                                                  
      |<--- Quadrant I -->|<-- Quadrant II -->|<-- Quadrant III ->|<-- Quadrant IV -->|     
      |                                                                                  
      |                   |                   |                   |                   |  
   +1 | - - - - - - - ****=****                                                        
      |            ***    |A   ***            |                   |                   |
      |          **       |m      **                                                   
      |         *         |p        *         |                   |                   |
      |       **          |l         **                                                
      |     **            |i           **     |                   |                   |
      |    *              |t             *                                             
      |   *               |u              *   |                   |                   |
      | **                |d               **                                          
      |*                  |e                 *|                   |                   |
    0 *-------------------=-------------------*-------------------+-------------------+--> x
      |                                        *                  |                  *|
      |                                         **                                 **  
      |                                           *               |               *   |
      |                                            *                             *     
      |                                             **            |            **     |
      |                                               **                     **        
      |                                                 *         |         *         |
      |                                                  **               **           
      |                                                    ***    |    ***            | 
   -1 | - - - - - - - - - - - - - - - - - - - - - - - - - - - *********                
      |                                                                               |
      |                                                                                
      |<------------------------------- Period length ------------------------------->|
      |                                                                                
  -y \|/                                                                               
       
*/


// Precalculated lookup table for SIN(x).
// This table contains only the first PI/2 part of a whole sine. (Quadrant I)
// The rest of the sine is derivated by either mirroring or inversion.
// The whole sine should swing from 0 to 255 with 128 being its center.
// So as only the first PI/2 is stored here the values range from 0 to 127.
//
// As only the first quadrant of the sine is required and is defined here
// using 256 value our whole sine has a resolution of 1024 values.
//
// Please note that the sine table does not need to get adjusted for different
// sampling frequencies.
//
// Of course it would also be possible to use a triangular wave or a sawtooth
// instead of a sine if this would match the required sound better.
const byte SYNTH_sineTable[256] PROGMEM = { 
	0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 
	8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 
	16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 
	24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 
	32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39, 
	40, 40, 41, 41, 42, 42, 43, 43, 44, 44, 45, 45, 46, 46, 47, 47, 
	48, 48, 49, 49, 50, 50, 51, 51, 52, 52, 53, 53, 54, 54, 55, 55, 
	56, 56, 57, 57, 58, 58, 59, 59, 60, 60, 61, 61, 62, 62, 63, 63, 
	64, 64, 65, 65, 66, 66, 67, 67, 68, 68, 69, 69, 70, 70, 71, 71, 
	72, 72, 73, 73, 74, 74, 75, 75, 76, 76, 77, 77, 78, 78, 79, 79, 
	80, 80, 81, 81, 82, 82, 83, 83, 84, 84, 85, 85, 86, 86, 87, 87, 
	88, 88, 89, 89, 90, 90, 91, 91, 92, 92, 93, 93, 94, 94, 95, 95, 
	96, 96, 97, 97, 98, 98, 99, 99, 100, 100, 101, 101, 102, 102, 103, 103, 
	104, 104, 105, 105, 106, 106, 107, 107, 108, 108, 109, 109, 110, 110, 111, 111, 
	112, 112, 113, 113, 114, 114, 115, 115, 116, 116, 117, 117, 118, 118, 119, 119, 
	120, 120, 121, 121, 122, 122, 123, 123, 124, 124, 125, 125, 126, 126, 127, 127
};

// These two tables contain the precalculated increment values for each MIDI
// note. MIDI supports 128 different notes. The note "A3" which is defined
// to be 440 Hz is the value with index .69
// More or less this table represents the number of increment steps when getting
// the next value for a note from the above sineTable which can get seen as the
// period of the sine. For low frequency notes the resolution is too low. So the
// increment-value (period) has been split up into "periodBase" and "periodFraction".
//
// An example: Take the note with index "7" (counted from 0). It has a periodBase of 1
// and a period fraction of 1. Lets forget about the fraction so it has almost exactly
// a periodBase of 1. This means when playing the note the internal logic will advance
// by 1 step in above sineTable for every sample played. The sampling rate is hardcoded
// at 12.5 kHz which equals a samplit period t_s of 80µs. The sineTable consists of 1024
// values. So it will take 1024*80µs for one full swing of the sine which equals a sine/note
// period t_n of 81.92ms. The frequency of the sine is thus f= 1/t_n = ~ 12.21 Hz.
// If you look up the frequency of the note "7" (G-2) in some MIDI frequency table you
// will find for it a value of 12.25 Hz which is quite close to our 12.2 Hz. In fact
// it will even be closer as the "periodFraction" which we ignored in this calculation
// will increase the speed by which the sineTable is stepped over, so this will increase
// the calculated frequency by about 1/256th. So if you do the full calculation you will
// get a generated frequency of "12.2547149658" Hz compared to a "should be" frequency
// in the MIDI frequency table of "12.24985737443". I did never create a error calculation
// for each note to see how much the largest error margin is but I can assume it is quite
// small.
const word SYNTH_periodBase[128] PROGMEM = {
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 
	4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 8, 8, 9, 9, 10, 
	10, 11, 12, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24, 25, 
	27, 28, 30, 32, 34, 36, 38, 40, 42, 45, 48, 50, 54, 57, 60, 64, 
	68, 72, 76, 80, 85, 90, 96, 101, 108, 114, 121, 128, 136, 144, 152, 161, 
	171, 181, 192, 203, 216, 228, 242, 256, 272, 288, 305, 323, 342, 363, 384, 407, 
	432, 457, 484, 513, 544, 576, 611, 647, 685, 726, 769, 815, 864, 915, 969, 1027
 };
const byte SYNTH_periodFraction[128] PROGMEM = { 
	171, 182, 192, 204, 216, 229, 242, 1, 16, 32, 50, 68, 87, 107, 129, 152, 
	176, 202, 229, 2, 32, 65, 99, 135, 174, 215, 2, 48, 96, 147, 202, 4, 
	65, 129, 198, 15, 92, 173, 4, 95, 192, 39, 148, 7, 129, 3, 140, 29, 
	183, 90, 7, 190, 128, 78, 40, 14, 3, 6, 24, 59, 111, 181, 15, 125, 
	1, 156, 79, 29, 6, 11, 48, 117, 221, 106, 29, 250, 2, 56, 159, 57, 
	11, 23, 96, 235, 187, 212, 58, 243, 3, 111, 61, 115, 22, 46, 193, 214, 
	117, 168, 117, 230, 6, 223, 123, 230, 45, 92, 129, 172, 235, 79, 234, 205, 
	13, 190, 246, 204, 89, 184, 3, 88, 214, 158, 211, 154, 25, 123, 235, 152
 };


// Make an instance of "SynthClass" and name it "Synth".
// This variable will be accesible to an Arduino Sketch because
// it is declared "extern" in "Synth.h"

SynthClass Synth;

/**
 * Initializes the synthesizer to the specified pin. The only allowed pins are
 * pin 3 and 11 as all others by default can not output a 62.5 kHz PWM or are
 * already used by the default Arduino firmware.
 *
 * @param uint8_t pin: The pin which to use for generating the audio signal
 * @return void
 */
void SynthClass::init(uint8_t pin) {
	byte i;
	nextSampleValue = SYNTH__ANALOG_HALF;
	#ifdef __AVR_ATmega328P__
	if (pin == 3 || pin == 11) {
		// Works only on pin 3 or 11 as all other pins do not allow to
		// have a 62.5 kHz PWM. Pin 5 and 6 would also, but their ISR
		// is already used for the "delay" function. It would be possible
		// to use those pins as output but to use the ISR for TIMER2. But
		// for this to work the PWM frequency for pins 5/6 would still have
		// to get switched to 62.5 kHz which would result in the "delay()"
		// and "delayMicroseconds()" functions to only wait for 1/64 of
		// the requested delay.
		// So for now we stick to pins 3 and 11 and using TIMER2
		pinMode(pin, OUTPUT);
		TCCR2B = TCCR2B & 0b11111000 | 0x01;
		switch (pin) {
			case 3:
				TCCR2A = TCCR2A & 0b11001100 | 0b00100011;
				sampleRegister = (intptr_t)&OCR2B;
			break;
			case 11:
				TCCR2A = TCCR2A & 0b00111100 | 0b10000011;
				sampleRegister = (intptr_t)&OCR2A;
			break;
		}
		*((byte*)sampleRegister) = nextSampleValue;
	}
	#else
		#error Not implemented for your processor type!
	#endif
}

/**
 * Starts playing the passed tracks.
 *
 * @param SYNTH_TRACK *_tracks: A pointer to an array of track structures
 * @param byte _trackCount: The number of track structures being pointed to
 * @return void
 */
void SynthClass::play(const SYNTH_TRACK *_tracks, byte _trackCount, word _samplesPerTick) {
	byte i;

	if (trackCount > SYNTH__MAX_OSCILLATORS) {
		// A maxium of 4 simultaneous tracks (oscillators) is supported currently.
		// We are having only a 8-bit PWM. When mixing 4 channels/oscillator every 
		// channel must only swing at an amplitude of 256/4 = 64 = 6 bit. So in fact
		// for 4 channels we have just four 6-bit oscillators.
		// Lets be true: A 5-bit sine wouldn't sound very nice.
		return;
	}

	if (_trackCount == 0) {
		return;
	}
	trackCount = _trackCount;
	tracks = (SYNTH_TRACK*)_tracks;
	samplesPerTick = _samplesPerTick;

	// Reset all oscillators and tracks
	for (i = 0; i < SYNTH__MAX_OSCILLATORS; i++) {
		noteIndex[i] = 0;
		currentNote[i] = SYNTH__NOTE_PAUSE;		// Note 0x80 means pause;
		currentDuration[i] = 0;
		currentPeriodIndex[i] = 0;
		currentPeriodFraction[i] = 0;
	}
	handleTick();

	sampleCounter = samplesPerTick;
	pwmCounter = SYNTH__PWM_CYCLES_PER_SAMPLE;

	// enable timer 0 overflow interrupt
#ifdef __AVR_ATmega328P__
	sbi(TIMSK2, TOIE2);
#else
	#error Timer 0 overflow interrupt not set correctly
#endif
}

/**
 * Stops playing any currently playing tracks by disabling
 * the TIMER0 overflow interrupts
 *
 * @return void
 */
void SynthClass::stop() {
	// disable timer 0 overflow interrupt
#ifdef __AVR_ATmega328P__
	cbi(TIMSK2, TOIE2);
#endif
	// We use "trackCount" to determine whether currently a song is playing
	trackCount = 0;
}

/**
 * Can get used to determine whether currently a song is playing
 *
 * @return bool Returns TRUE if a song is playing currently.
 */
bool SynthClass::isPlaying() {
	return trackCount ? true : false;
}


/*************************************************************************
 **
 ** BELOW ARE PRIVATE METHODS
 **
 ** Thease are not part of the public interface
 **
 ************************************************************************/

/**
 * This method retrieves the specified sine wave value from the precalculated table. 
 * The range for "index" is 0 to 0x3FF which gives a range of 1024 values for a whole
 * swing.
 *
 * It is currently defined inline as it will get used only once. If this method will
 * get used multiple times in the future it can get declared as real method also.
 *
 * @param word index: The index for which to get the value
 * @return byte: The value at the specified index
 */
inline byte SynthClass::getSineValue(word index) {
   byte sineIndex = index & 0xFF;
   byte quadrant = (index >> 8) & 0x3;
	byte result;
   if (quadrant & 0x1) {
		// Quadrant II + IV. Values are approaching zero.
		// Read sineTable from behind (Mirror)
      result = pgm_read_byte(&SYNTH_sineTable[0xFF - sineIndex]);
   } else {
		// Quadrant I + III. Values are getting away from zero.
      result = pgm_read_byte(&SYNTH_sineTable[sineIndex]);
   }
   if (quadrant & 0x2) {
		// Quadrant III + IV. Negative half-wave
      result = SYNTH__ANALOG_HALF - result;
   } else {
		// Quadrant I + II. Positive half-wave
      result += SYNTH__ANALOG_HALF;
   }
   return result;
}

/**
 * This method handles a MIDI "tick". It advances (decrements) the duration counter
 * of each osciallator. If the duration counter of an oscillator reaches zero it
 * advances the oscillator to the next note.
 *
 * If there are no more notes for a track it silences the track by letting it play
 * a pause.
 *
 * If all tracks have ended this method also deactivates the interrupts and playing
 * by calling SynthClass::stop();
 *
 * @return void;
 */
void SynthClass::handleTick() {
	byte i;
	word tmp;
	byte finishedTracks = 0;
	bool trackStart;

	// These program-memory variables get loaded into data memory
	word noteCount;
	PGM_P notes;
	PGM_P timeOffsets;
	
	for (i = 0; i < trackCount; i++) {
		trackStart = false;
		// We can just read the noteCount from program memory as it's address
		// can get evaluated by the compiler
		noteCount = pgm_read_word( &(tracks[i].noteCount) );
		// For notes and timeOffsets read the address pointer first
		notes = (PGM_P) pgm_read_word( &(tracks[i].notes) );
		timeOffsets = (PGM_P) pgm_read_word( &((*(tracks+i)).timeOffsets) );
		
		if (noteIndex[i] >= noteCount) {
			// No more notes in this track.
			currentNote[i] = SYNTH__NOTE_PAUSE;
			currentDuration[i] = 0;
			finishedTracks++;
		} else {
			if (currentDuration[i] == 0) {
				// When duration is already 0 at this place it is the special case that
				// the synth has just been started.
				trackStart = true;
			}
			if (currentDuration[i]) {
				// A note is playing. Just decrement its duration.
				currentDuration[i]--;
			}
			do {
				if (!currentDuration[i]) {
					// A note has ended. First play the note which is actually pointed to as
					// its event-time has now been reached. But take care if this is gets
					// called for the first time in a track. Then we should play pause (0x80)
					// as no note event may have been reached yet.
					if (trackStart) {
						currentNote[i] = SYNTH__NOTE_PAUSE;
						currentDuration[i] = 0;
						trackStart = false;
						// Intentionally do NOT advance the note index here so the note index
						// still points to the first note. Usually the note index should get
						// advanced after having loaded the currently playing note. But using
						// this mechanism we simulate that the noteIndex pointed to "-1" when
						// the track/song got started (without using a signed number).
					} else {
						// This works for reading the note. But why? In fact the macro would have
						// To retrieve the ".notes" pointer first, then add the noteIndex offset
						// and then retrieve the note.
						// currentNote[i] = pgm_read_byte(tracks[i].notes + noteIndex[i]);
						currentNote[i] = pgm_read_byte( notes + noteIndex[i] );

						// Advance the note index of this track.
						noteIndex[i]++;
					}

					// The noteIndex now points to the next note and its event-time. Load the
					// event time for the next note into the currentDuration variable so the
					// duration until that event can get counted down.
					tmp = noteIndex[i] << 1;
					currentDuration[i] = pgm_read_word( timeOffsets + tmp );
				}
			// Load next event as long as the current event has a zero event-time offset.
			} while (currentDuration[i] == 0);
		}
	}
	if (finishedTracks >= trackCount) {
		// Song has ended.
		stop();
	}
}


/**
 * This method determines the next analog sample value which has to get written
 * to the PWM (or external D/A). It does so by getting the current sine/pause
 * value for each configured track, adding them together and then dividing the
 * final result through the total number of tracks. This achieves a digital
 * signal mixing.
 *
 * @return byte The next analog sample value
 */
byte SynthClass::calculateNextSampleValue() {
	byte i;
	word base;
	byte fraction;
	byte note;

	word value = 0;

	for (i = 0; i < trackCount; i++) {
		note = currentNote[i];
		if (note & SYNTH__NOTE_PAUSE) {
			// When no note is not being played (silence) add the 0-line value 0x80
			value += SYNTH__ANALOG_HALF;
		} else {
			// First add the current sine amplitude to the total output value
			// "Mix-in"
      	value += getSineValue(currentPeriodIndex[i]);


			// Now increment the period index according to the current note.
			base = pgm_read_word(&SYNTH_periodBase[note]);
			fraction = pgm_read_byte(&SYNTH_periodFraction[note]);
	
			currentPeriodIndex[i] += base;
			currentPeriodFraction[i] += fraction;

			// When fraction overflows decrement it and increment index
			if (currentPeriodFraction[i] > 0xFF) {
				currentPeriodFraction[i] -= 0x100;
				currentPeriodIndex[i]++;
			}
			// We do not need to check for "index" overflowing 0x3FF. If this
			// is the case we simply ignore it as it gets truncated in "getSineValue"
			// anyways.
		}
	}


#if SYNTH__MAX_OSCILLATORS > 4
	#error Mixer divide for more than four oscillators not implemented
#endif

#if SYNTH__MAX_OSCILLATORS > 2
	if (trackCount == 3) {
		// Is this too slow?
		value = value / 3;
	} else if (trackCount == 4) {
		// Divide by four
		value = value >> 2;
	} else if (trackCount == 2) {
		// Divide by 2
		value = value >> 1;
	}
#else
	#if SYNTH__MAX_OSCILLATORS > 1
		if (trackCount > 1) {
			// Divide by 2
			value = value >> 1;
		}
	#endif
# endif
	return (byte) value;
}

/**
 * This method handles SynthClass for the ISR.
 *
 * It write the next analog sample value to the PWM (or external DAC)
 * every SYNTH__PWM_CYCLES_PER_SAMPLE times. This will usually happen
 * 12500 times per second.
 *
 * If a sample has been written the next value for the sample will
 * get calculated.
 *
 * This method also counts the number of samples. If the number of samples
 * having been played equals "samplesPerTick" this means that one MIDI
 * tick has elapsed. It will then advance the MIDI event queue by one tick
 * and eventually load any new note to be played.
 *
 * TODO: Switch handleTick() and calculateNextSampleValue(). So first a new
 * note is required and then the new sample values get calculated. It should
 * usually not make any difference as all notes/pauses are just updated one
 * sample later. But as they shift together it won't make a difference.
 *
 * @return void;
 */
void SynthClass::handleIsr() {
	if (--pwmCounter == 0) {
		*((byte*)sampleRegister) = nextSampleValue;
		pwmCounter = SYNTH__PWM_CYCLES_PER_SAMPLE;

		// This hopefully always takes less than 256 CPU cycles
		// Else calculating the next sample value would have to
		// get moved into the main routine.
		//
		// If the Arduino is not required to do anything other next
		// to playing music this would be fine as it could be done
		// in a main-routine loop waiting for the track finishing play.
		nextSampleValue = calculateNextSampleValue();

		// Decrement sample counter. Whenever sampleCounter reaches zero
		// this means a MIDI "tick" has occured.
		// The overflow will get handled in next call to this interrupt
		// routine where pwmCounter is surely not "0" after decrementing
		// because it has been set to "SYNTH__PWM_CYCLES_PER_SAMPLE" in
		// this block.
		--sampleCounter;
	} else if (sampleCounter == 0) {
		// A tick has occured.
		sampleCounter = samplesPerTick;

		// If it can get assured that this method for itself will not take
		// more than 256 CPU cycles then this is fine here.
		// Else move to "play" routine.
		handleTick();
	}
}

/**
 * This is the interrupt service routine for TIMER0 overflows.
 * It simply calls the "handleIsr" method of SynthClass.
 *
 *
 * @return void
 */
#ifdef __AVR_ATmega328P__
	ISR(TIMER2_OVF_vect)
#endif
{
	Synth.handleIsr();
}

