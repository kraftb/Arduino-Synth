#!/usr/bin/php
<?php

/*
 * This script was used to generate the sine-table and the MIDI
 * note frequency/period tables.
 */

// This variable allows to switch between "C" style output for
// the PC (gcc) or Arduino.
$PC = FALSE;

$fSampleRate = 12500;

$tableSize = 256;
$resolutionInBits = 8;

$notesInOctave = array(
	-9 => 'C',
	-8 => 'C#',
	-7 => 'D',
	-6 => 'D#',
	-5 => 'E',
	-4 => 'F',
	-3 => 'F#',
	-2 => 'G',
	-1 => 'G#',
	+0 => 'A',
	+1 => 'A#',
	+2 => 'B'
);


$notes = array(
	'A3' => 440.0,
);


function sineTable($tableSize, $resolutionInBits) {
	$sineTable = array();
	$maxVal = intval(round(pow(2, $resolutionInBits - 1)) - 1);
	for ($index = 0; $index < $tableSize; $index++) {
		$rad = (($index*1.0)/$tableSize) * M_PI_2;
		$sin = sin($rad);
		$sineTable[$index] = round($sin * $maxVal);
	}
	return $sineTable;
}
$sineTable = sineTable($tableSize, $resolutionInBits);


function generateNotes($notes) {
	global $notesInOctave;
	for ($octave = -2; $octave <= 8; $octave++) {
		if ($octave === 3) {
			continue;
		}
		$notes['A' . $octave] = $notes['A3'] * pow(2.0, $octave - 3);
	}

	for ($octave = -2; $octave <= 8; $octave++) {
		foreach ($notesInOctave as $noteIndex => $noteInOctave) {
			if ($noteInOctave === 'A') {
				continue;
			}
			$notes[$noteInOctave . $octave] = $notes['A' . $octave] * pow(2, $noteIndex/12.0);
		}
	}
	return $notes;
}

function midiNotes($notes) {
	global $notesInOctave;

	$midiIndex = 0;

	$midiNotes = array();	
	$midiFrequencies = array();	
	$noteFrequencies = array();
	for ($octave = -2; $octave <= 8; $octave++) {
		foreach ($notesInOctave as $noteIndex => $noteInOctave) {
			$note = $noteInOctave . $octave;
			$midiNotes[$midiIndex] = $note;
			$midiFrequencies[$midiIndex] = $notes[$note];
			$noteFrequencies[$note] = $notes[$note];
			$midiIndex++;
			if ($midiIndex === 0x80) {
				break 2;
			}
		}
	}
	return array(
		'notes' => $midiNotes,
		'frequencies' => $midiFrequencies,
		'noteFrequencies' => $noteFrequencies,
	);
}

function periods($midiData, $sampleRate, $tableSize) {
	$samplesPerPeriod = $tableSize * 4;
	$baseFreq = $sampleRate / $samplesPerPeriod;
	$midiData['periods'] = array();
	foreach ($midiData['frequencies'] as $midiIndex => $frequency) {
		$period = $frequency / $baseFreq;
		$base = intval($period);
		$fraction = $period - $base;
		$fractionByte = intval(round($fraction * 256));
		if ($fractionByte == 256) {
			$fractionByte = 0;
			$base++;
		}
		$midiData['periods'][$midiIndex] = array($base, $fractionByte);
	}
	return $midiData;
}

// print_r($sineTable);

$notes = generateNotes($notes);

$midiData = midiNotes($notes);

$midiData = periods($midiData, $fSampleRate, $tableSize);

print_r($midiData);
exit();

if ($PC) {
	$sineTableCode = 'unsigned char sineTable[256] = { ';
} else {
	$sineTableCode = 'const byte sineTable[256] PROGMEM = { ';
}
foreach ($sineTable as $index => $value) {
	if (!($index % 16)) {
		$sineTableCode .= chr(10) . chr(9);
	}
	$sineTableCode .= $value . ', ';
}
$sineTableCode = substr($sineTableCode, 0, -2) . chr(10) . ' };' . chr(10);
echo $sineTableCode;

if ($PC) {
	$periodBaseCode = 'unsigned short periodBase[128] = { ';
	$periodFractionCode = 'unsigned char periodFraction[128] = { ';
} else {
	$periodBaseCode = 'const word periodBase[128] PROGMEM = { ';
	$periodFractionCode = 'const byte periodFraction[128] PROGMEM = { ';
}
foreach ($midiData['periods'] as $index => $value) {
	if (!($index % 16)) {
		$periodBaseCode .= chr(10) . chr(9);
		$periodFractionCode .= chr(10) . chr(9);
	}
	$periodBaseCode .= $value[0] . ', ';
	$periodFractionCode .= $value[1] . ', ';
}
$periodBaseCode = substr($periodBaseCode, 0, -2) . chr(10) . ' };' . chr(10);
$periodFractionCode = substr($periodFractionCode, 0, -2) . chr(10) . ' };' . chr(10);

echo $periodBaseCode;
echo $periodFractionCode;


