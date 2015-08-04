#include <stdio.h>
#include <stdlib.h>

/**
 * This file was used to test the concept of the software synthesizer
 * You can easily compile it under Linux using:
 *
 * gcc -o synt synt.c
 *
 * You can then call it and pass it's output to "aplay" or some similar
 * tool which feeds the output into the soundcard:
 *
 * ./synt | aplay -r 12500 -c 1 -f U8
 *
 */


unsigned char sineTable[256] = { 
	0, 1, 2, 2, 3, 4, 5, 5, 6, 7, 8, 9, 9, 10, 11, 12, 
	12, 13, 14, 15, 16, 16, 17, 18, 19, 19, 20, 21, 22, 22, 23, 24, 
	25, 26, 26, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36, 
	37, 38, 38, 39, 40, 41, 41, 42, 43, 44, 44, 45, 46, 46, 47, 48, 
	49, 49, 50, 51, 51, 52, 53, 54, 54, 55, 56, 56, 57, 58, 58, 59, 
	60, 61, 61, 62, 63, 63, 64, 65, 65, 66, 67, 67, 68, 69, 69, 70, 
	71, 71, 72, 72, 73, 74, 74, 75, 76, 76, 77, 78, 78, 79, 79, 80, 
	81, 81, 82, 82, 83, 84, 84, 85, 85, 86, 86, 87, 88, 88, 89, 89, 
	90, 90, 91, 91, 92, 93, 93, 94, 94, 95, 95, 96, 96, 97, 97, 98, 
	98, 99, 99, 100, 100, 101, 101, 102, 102, 102, 103, 103, 104, 104, 105, 105, 
	106, 106, 106, 107, 107, 108, 108, 109, 109, 109, 110, 110, 111, 111, 111, 112, 
	112, 112, 113, 113, 113, 114, 114, 114, 115, 115, 115, 116, 116, 116, 117, 117, 
	117, 118, 118, 118, 118, 119, 119, 119, 120, 120, 120, 120, 121, 121, 121, 121, 
	122, 122, 122, 122, 122, 123, 123, 123, 123, 123, 124, 124, 124, 124, 124, 124, 
	125, 125, 125, 125, 125, 125, 125, 126, 126, 126, 126, 126, 126, 126, 126, 126, 
	126, 126, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127
};

unsigned short periodBase[128] = { 
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 
	4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 8, 8, 9, 9, 10, 
	10, 11, 12, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24, 25, 
	27, 28, 30, 32, 34, 36, 38, 40, 42, 45, 48, 50, 54, 57, 60, 64, 
	68, 72, 76, 80, 85, 90, 96, 101, 108, 114, 121, 128, 136, 144, 152, 161, 
	171, 181, 192, 203, 216, 228, 242, 256, 272, 288, 305, 323, 342, 363, 384, 407, 
	432, 457, 484, 513, 544, 576, 611, 647, 685, 726, 769, 815, 864, 915, 969, 1027
 };
unsigned char periodFraction[128] = { 
	171, 182, 192, 204, 216, 229, 242, 1, 16, 32, 50, 68, 87, 107, 129, 152, 
	176, 202, 229, 2, 32, 65, 99, 135, 174, 215, 2, 48, 96, 147, 202, 4, 
	65, 129, 198, 15, 92, 173, 4, 95, 192, 39, 148, 7, 129, 3, 140, 29, 
	183, 90, 7, 190, 128, 78, 40, 14, 3, 6, 24, 59, 111, 181, 15, 125, 
	1, 156, 79, 29, 6, 11, 48, 117, 221, 106, 29, 250, 2, 56, 159, 57, 
	11, 23, 96, 235, 187, 212, 58, 243, 3, 111, 61, 115, 22, 46, 193, 214, 
	117, 168, 117, 230, 6, 223, 123, 230, 45, 92, 129, 172, 235, 79, 234, 205, 
	13, 190, 246, 204, 89, 184, 3, 88, 214, 158, 211, 154, 25, 123, 235, 152
 };


// #include "songs/der-mond.h"
// #include "songs/zum-tanze.h"
#include "songs/super-mario.h"

char getSine(unsigned short index) {
	unsigned char quadrant = (index & 0x300) >> 8;
	char result = 0;
	index &= 0xFF;
	if (quadrant & 0x1) {
		result = sineTable[0xFF-index];
	} else {
		result = sineTable[index];
	}
	if (quadrant & 0x2) {
		result = 0x80 - result;
	} else {
		result += 0x80;
	}
	return result;
}

void note(unsigned char index, unsigned short duration) {
	unsigned short base = periodBase[index];
	unsigned char fraction = periodFraction[index];
	static unsigned short currentBase = 0;
	static unsigned short currentFraction = 0;
	unsigned int i = 0;
	char amplitude = 0;

	for (i = 0; i < duration; i++) {

		amplitude = getSine(currentBase);
		fprintf(stdout, "%c", amplitude);

		currentFraction += fraction;		
		currentBase += base;
		if (currentFraction >= 256) {
			currentFraction -= 256;
			currentBase += 1;
		}
		while (currentBase >= 1024) {
			currentBase -= 1024;
		}

	}
}

void pause(unsigned int duration) {
	unsigned int i = 0;
	for (i = 0; i < duration; i++) {
		fprintf(stdout, "%c", 0x80);
	}
}


int main(int argc, char **argv) {
	unsigned short noteIndex = 0;
	unsigned char currentNote = 0;
	unsigned char lastNote = 0;
	unsigned short timeDiff = 0;

	unsigned short samplesPerClick = 90;

	while (noteIndex < noteCount) {
		currentNote = songNotes[noteIndex];
		timeDiff = songTimeDiff[noteIndex];
		while (timeDiff) {
			if (lastNote & 0x80) {
				pause(samplesPerClick);
			} else {
				note(lastNote, samplesPerClick);
			}
			timeDiff--;
		}
		lastNote = currentNote;
		noteIndex++;
      fprintf(stderr, "%d: %d\n", noteIndex, lastNote);
	}
}


