#!/usr/bin/php
<?php

/**
 * This script can get used to convert a MIDI file into a C
 * array which has to get included in an Arduino sketch.
 *
 * The generated C array will reside in Arduino program memory.
 */


// This variable allows to switch between "C" style output for
// the PC (gcc) or Arduino.
$PC = FALSE;

class HighNibbleException extends \Exception {};
class MultiNoteException extends \Exception {};

$data = file_get_contents($argv[1]);

function dumpBytes($data) {
	for ($x = 0; $x < strlen($data); $x++) {
		$byte = ord(substr($data, $x, 1));
		echo ' 0x' . dechex($byte);
	}
	echo "\n";
}
	
function getVariableLengthValue(&$chunk) {
	$value = 0;
	for ($x = 0; $x < 9; $x++) {
		$value = $value << 7;
		$byte = ord(substr($chunk, 0, 1));
		$chunk = substr($chunk, 1);
		$value |= $byte & 0x7F;
		if (!($byte & 0x80)) {
			break;
		}
	}
	return $value;
}

/* ------------ Retrieve data items from stream --- begin ------- */
function getByte(&$chunk) {
	if (strlen($chunk) === 0) {
		throw new \Exception('No byte left! (#3)');
	}
	$byte = ord(substr($chunk, 0, 1));
	$chunk = substr($chunk, 1);
	return $byte;
}
function getTriplet(&$chunk) {
	$byte0 = getByte($chunk);
	$byte1 = getByte($chunk);
	$byte2 = getByte($chunk);
	return ($byte0 << 16) | ($byte1 << 8) | $byte2;
}

function getBytes(&$chunk, $length) {
	if ($length > strlen($chunk)) {
		throw new \Exception('Not enough bytes left! (#2)');
	}
	$data = substr($chunk, 0, $length);
	$chunk = substr($chunk, $length);
	return $data;
}
function getVariableBytes(&$chunk) {
	$length = getVariableLengthValue($chunk);
	if ($length > strlen($chunk)) {
		throw new \Exception('Not enough bytes left! (#1)');
	}
	$data = getBytes($chunk, $length);
	return $data;
}
/* ------------ Retrieve data items from stream --- end --------- */


function getMetaEvent(&$chunk) {
	$nextByte = getByte($chunk);
	switch ($nextByte) {
		case 0x00:	// Sequence number
dumpBytes($getByte);
exit();
			return array(
				'type' => 'meta',
				'meta' => 'sequence-number',
				'number' => getByte($chunk),
			);
		break;

		case 0x01:	// Text event
			return array(
				'type' => 'meta',
				'meta' => 'text',
				'data' => getVariableBytes($chunk),
			);
		break;

		case 0x02:	// Copyright notice
			return array(
				'type' => 'meta',
				'meta' => 'copyright',
				'data' => getVariableBytes($chunk),
			);
		break;

		case 0x03:	// Sequence/Track Name
			return array(
				'type' => 'meta',
				'meta' => 'sequence-name',
				'data' => getVariableBytes($chunk),
			);
		break;

		case 0x04:	// Instrument Name
			return array(
				'type' => 'meta',
				'meta' => 'instrument-name',
				'data' => getVariableBytes($chunk),
			);
		break;

		case 0x05:	// Lyric
			return array(
				'type' => 'meta',
				'meta' => 'lyric',
				'data' => getVariableBytes($chunk),
			);
		break;

		case 0x06:	// Marker
			return array(
				'type' => 'meta',
				'meta' => 'marker',
				'data' => getVariableBytes($chunk),
			);
		break;

		case 0x07:	// Cue point
			return array(
				'type' => 'meta',
				'meta' => 'cue-point',
				'data' => getVariableBytes($chunk),
			);
		break;

		case 0x20:	// Channel prefix
dumpBytes($chunk);
exit();
			return array(
				'type' => 'meta',
				'meta' => 'channel-prefix',
				'number' => getByte($chunk),
			);
		break;

		case 0x2F:	// End of track
			if (getByte($chunk) !== 0x00) {
				throw new \Exception('Illegal end of track meta event!');
			}
			return array(
				'type' => 'meta',
				'meta' => 'end-of-track',
			);
		break;

		case 0x51:	// Set tempo
			if (getByte($chunk) !== 0x03) {
				throw new \Exception('Illegal set tempo meta event!');
			}
			return array(
				'type' => 'meta',
				'meta' => 'set-tempo',
				'tempo' => getTriplet($chunk),
			);
		break;

		case 0x54:	// SMPTE Offset
			if (getByte($chunk) !== 0x05) {
				throw new \Exception('Illegal SMPTE offset meta event!');
			}
			return array(
				'type' => 'meta',
				'meta' => 'smpte-offset',
				'hour' => getByte($chunk),
				'minute' => getByte($chunk),
				'second' => getByte($chunk),
				'frame' => getByte($chunk),
				'fractional' => getByte($chunk),
			);
		break;

		case 0x58:	// Time signature
			if (getByte($chunk) !== 0x04) {
				throw new \Exception('Illegal time signature meta event!');
			}
			return array(
				'type' => 'meta',
				'meta' => 'time-signature',
				'numerator' => getByte($chunk),
				'denominator' => getByte($chunk),
				'clocks' => getByte($chunk),
				'notes32_in_quarter' => getByte($chunk),
			);
		break;

		case 0x59:	// Key signature
			if (getByte($chunk) !== 0x02) {
				throw new \Exception('Illegal key signature meta event!');
			}
			return array(
				'type' => 'meta',
				'meta' => 'key-signature',
				'sf' => getByte($chunk),
				'minor' => getByte($chunk),
			);
		break;

		case 0x7F:	// Sequencer specific meta event
			return array(
				'type' => 'meta',
				'meta' => 'sequencer-specific',
				'data' => getVariableBytes($chunk),
			);
		break;

		case 0x21:	// Unknown
			return array(
				'type' => 'meta',
				'meta' => 'unknown-0x21',
				'data' => getVariableBytes($chunk),
			);
		break;

		default:
			dumpBytes($chunk);
			throw new \Exception('Unrecognized meta event! ('.$nextByte.')');
		break;
	}
}

function getMidiEvent($statusByte, &$chunk, $runningStatusCall = FALSE) {
	static $runningStatus = 0;
	static $localStatus = 0;
	$runningStatus = $localStatus;
	$localStatus = $statusByte;

	$highNibble = ($statusByte & 0xF0) >> 4;
	$lowNibble = ($statusByte & 0x0F);

	if ($highNibble === 0xF) {
		switch ($lowNibble) {
			case 0x2:
				$l = getByte($chunk);
				$m = getByte($chunk);
				return array(
					'type' => 'midi',
					'midi' => 'position-pointer',
					'number' => ($m << 7) | $l,
				);
			break;
			case 0x3:
				return array(
					'type' => 'midi',
					'midi' => 'song-select',
					'number' => getByte($chunk),
				);
			break;
			case 0x6:
				return array(
					'type' => 'midi',
					'midi' => 'tune-request',
				);
			break;
			case 0x7:
				throw new \Exception('End of exclusive should not occur here!');
			break;
			default:
				throw new \Exception('Unexpected low nibble!');
			break;
		}
	} else {
		switch ($highNibble) {
			case 0x8:
				return array(
					'type' => 'midi',
					'midi' => 'note-off',
					'channel' => $lowNibble,
					'note' => getByte($chunk),
					'velocity' => getByte($chunk),
				);
			break;
			case 0x9:
				return array(
					'type' => 'midi',
					'midi' => 'note-on',
					'channel' => $lowNibble,
					'note' => getByte($chunk),
					'velocity' => getByte($chunk),
				);
			break;
			case 0xA:
				return array(
					'type' => 'midi',
					'midi' => 'key-pressure',
					'channel' => $lowNibble,
					'note' => getByte($chunk),
					'value' => getByte($chunk),
				);
			break;
			case 0xB:
				return array(
					'type' => 'midi',
					'midi' => 'control-change',
					'channel' => $lowNibble,
					'controller' => getByte($chunk),
					'value' => getByte($chunk),
				);
			break;
			case 0xC:
				return array(
					'type' => 'midi',
					'midi' => 'program-change',
					'channel' => $lowNibble,
					'program' => getByte($chunk),
				);
			break;
			case 0xD:
				return array(
					'type' => 'midi',
					'midi' => 'channel-pressure',
					'channel' => $lowNibble,
					'value' => getByte($chunk),
				);
			break;
			case 0xE:
				$l = getByte($chunk);
				$m = getByte($chunk);
				return array(
					'type' => 'midi',
					'midi' => 'pitch-wheel-change',
					'channel' => $lowNibble,
					'pitch' => ($m << 7) | $l,
				);
			break;
			case 0x0:
/*
				// This is just a try:
				if ($runningStatusCall) {
					throw new \Exception('Avoid recursive calling!');
				}
				$localStatus = $runningStatus;
				$combinedStatus = $runningStatus & 0xF0 | $statusByte & 0x0F;
				$event = getMidiEvent($combinedStatus, $chunk, TRUE);
				return $event;
*/
				if ($lowNibble === 0xA) {
					return array(
						'type' => 'midi',
						'midi' => 'unknown-0x0A',
						'data' => getByte($chunk).getByte($chunk).getByte($chunk).getByte($chunk),
					);
				} else {
					throw new \Exception('Unexpected status byte!');
				}

			break;
			default:
				if ($runningStatusCall) {
					throw new \Exception('Avoid recursive calling!');
				}
				$localStatus = $runningStatus;
				$chunk = chr($statusByte) . $chunk;
				$event = getMidiEvent($runningStatus, $chunk, TRUE);
				return $event;
			break;
		}
	}
}

function parseTrack($chunk, $trackIndex) {
	$backupChunk = $chunk;
	$events = array();
	$sysExEvent = FALSE;

	$nextByte = 0;

	while (strlen($chunk)) {
		$event = NULL;
		$vTime = getVariableLengthValue($chunk);
		$lastByte = $nextByte;
		$nextByte = getByte($chunk);
		if ($nextByte === 0xFF) {
			$event = getMetaEvent($chunk);
		} elseif ($nextByte === 0xF0 && !$sysExEvent) {
			$sysExEvent = TRUE;
			$eventData = getVariableBytes($chunk);
			$event = array(
				'type' => 'system',
				'data' => $eventData,
			);
		} elseif ($nextByte === 0xF0 && $sysExEvent) {
			throw new \Exception('Exclusive system message!');
		} elseif ($nextByte === 0xF7 && $sysExEvent) {
			$eventData = getVariableBytes($chunk);
			if (ord(substr($eventData, -1)) === 0xF7) {
				$sysExEvent = FALSE;
			}
			$event = array(
				'type' => 'system',
				'data' => $eventData,
			);
		} elseif ($nextByte === 0xF7 && !$sysExEvent) {
			$eventData = getVariableBytes($chunk);
			$event = array(
				'type' => 'escape',
				'data' => $eventData,
			);
		} else {
			try {
				$event = getMidiEvent($nextByte, $chunk);
			} catch (\Exception $e) {
/*
				dumpBytes($backupChunk);
				print_r($events);
*/
				throw $e;
			}
		}
		$event['delta-time'] = $vTime;
		$events[] = $event;
	}
	return $events;
}

function midiGetChunk(&$data, $trackIndex = 0) {
	$type = substr($data, 0, 4);
	$length_string = substr($data, 4, 4);
	$length = array_pop(unpack('Nlength', $length_string));
	if ($type === 'MThd' && $length !== 6) {
		throw new \Exception('MIDI header chunks must be 6 bytes long!');
	}
	$chunk = substr($data, 8, $length);
	$data = substr($data, 8 + $length);
	if ($type === 'MThd') {
		return array(
			'type' => 'header',
			'data' => unpack('nformat/ntracks/ndivision', $chunk),
		);
	} elseif ($type === 'MTrk') {
		return array(
			'type' => 'track',
			'data' => parseTrack($chunk, $trackIndex),
		);
	} else {
		throw new \Exception('Invalid chunk!');
	}
}

function getTempo($events) {
	$value = NULL;
	foreach ($events as $event) {
		if (isset($event['type']) && $event['type'] === 'meta' && isset($event['meta']) && $event['meta'] === 'set-tempo') {
			if ($value !== NULL) {
				throw new Exception('Tempo switching not implemented! ('.$value.'/'.$event['tempo'].')');
			}
			$value = intval($event['tempo']);
		}
	}
	return $value;
}

function splitNotes($events) {
	$notes = array();
	$currentNote = array();
	$time = 0;
	$debugEvents = array();
	$useChannel = NULL;
	foreach ($events as $event) {
		$debugEvents[] = $event;
		$time += $event['delta-time'];

		if ($event['type'] === 'midi') {
			$useIndex = NULL;
			if ($event['midi'] === 'note-on') {
				if ($event['velocity'] == 0) {
					$event['midi'] = 'note-off';
				} else {
					foreach ($notes as $splitIndex => $splitNotes) {
						if (!isset($currentNote[$splitIndex])) {
							$currentNote[$splitIndex] = $event['note'];
							$useIndex = $splitIndex;
							break;
						}
					}
					if ($useIndex === NULL) {
						$currentNote[] = $event['note'];
						$useIndex = array_pop(array_keys($currentNote));
					}
				}
			}
			if ($event['midi'] === 'note-off') {
				foreach ($notes as $splitIndex => $splitNotes) {
					if (isset($currentNote[$splitIndex]) && $currentNote[$splitIndex] === $event['note']) {
						$useIndex = $splitIndex;
						unset($currentNote[$splitIndex]);
					}
				}
				if ($useIndex === NULL) {
					echo "No multi-note which can get switched off!\n";
					continue;
					throw new \Exception('No multi-note which can get switched off!');
				}
			}
			if ($event['midi'] === 'note-on' || $event['midi'] === 'note-off') {
				if ($useChannel === NULL) {
					$useChannel = $event['channel'];
				} else {
					if ($useChannel !== $event['channel']) {
						throw new \Exception('Splitting a tracks notes which uses multiple channels is not supported!');
					}
				}
				if ($useIndex === NULL) {
					throw new \Exception('This shouldn\'t happen!');
				}
				$notes[$useIndex][] = array(
					'type' => $event['midi'],
					'abs-time' => $time,
					'note' => $event['note'],
				);
			}
		}
	}
	foreach ($notes as $sheetIndex => $sheet) {
		$lastTime = 0;
		foreach ($sheet as $noteIndex => $note) {
			$tmpTime = $note['abs-time'];
			$notes[$sheetIndex][$noteIndex]['delta-time'] = $tmpTime - $lastTime;
			$lastTime = $tmpTime;
		}
	}
	return $notes;
}
	
	
function filterNotes($events) {
	$notes = array();
	$currentNote = NULL;
	$time = 0;
	foreach ($events as $event) {
		$time += $event['delta-time'];

		if ($event['type'] === 'midi') {
			if ($event['midi'] === 'note-on') {
				if ($event['note'] === $currentNote && $event['velocity'] == 0) {
					$event['midi'] = 'note-off';
				} else {
					if ($currentNote !== NULL) {
						throw new \MultiNoteException('Multiple notes not supported!');
					}
					$currentNote = $event['note'];
				}
			}
			if ($event['midi'] === 'note-off') {
				if ($currentNote === NULL) {
					throw new \Exception('No note which can get switched off!');
				}
				if ($currentNote !== $event['note']) {
					throw new \Exception('Switching different note off than which is on!');
				}
				$currentNote = NULL;
			}
			if ($event['midi'] === 'note-on' || $event['midi'] === 'note-off') {
				$notes[] = array(
					'type' => $event['midi'],
					'delta-time' => $time,
					'note' => $event['note'],
					'channel' => $event['channel'],
				);
				$time = 0;
			}
		}
	}
	return $notes;
}


/* --------- Main program -------------- */

$header = midiGetChunk($data);
$ticksPerQuarterNote = (float)$header['data']['division'];

if ($header['data']['format'] !== 1) {
	throw new \Exception('Format not supported!');
}
if ($header['data']['division'] & 0x8000) {
	throw new \Exception('SMPTE not supported!');
}


$noteCounts = array();

$useTempo = NULL;


echo "
#include <avr/pgmspace.h>
#include <Synth.h>

";


$tracksWithNotes = array();

for ($track = 0; $track < $header['data']['tracks']; $track++) {
	$trackNumber = $track+1;

	echo "// TRACK $track\n";

	try {
		$trackChunk = midiGetChunk($data, $track);
	} catch (\Exception $e) {
//		throw $e;
		continue;
	}

	try {
		$tempo = getTempo($trackChunk['data']);
		if ($tempo !== NULL) {
			if ($useTempo !== NULL && $tempo !== $useTempo) {
				throw new \Exception('Different tempo for different tracks is not implemented!');
			}
			$useTempo = $tempo;
		}
	} catch (\Exception $e) {
		echo "// ERROR: " . $e->getMessage()."\n";
	}

	if ($track > 0) {

		try {
			$notes = filterNotes($trackChunk['data']);
			if (count($notes)) {
				$tracksWithNotes[] = $notes;
			}
		} catch (\Exception $e) {
			if ($e instanceof \MultiNoteException) {
				$noteTracks = splitNotes($trackChunk['data']);
				foreach ($noteTracks as $noteTrack) {
					$tracksWithNotes[] = $noteTrack;
				}
			}
			echo "// Error filtering notes for track #".$trackNumber." (".$e->getMessage().")\n";
			continue;
		}

	}
}


foreach ($tracksWithNotes as $track => $notes) {
	$trackNumber = $track+1;
	echo "\n// TRACK $trackNumber\n";

	if ($PC) {
		$noteCode = 'unsigned char track'.$trackNumber.'_notes[] = { ';
		$deltaTimeCode = 'unsigned short track'.$trackNumber.'_timeOffsets[] = { ';
	} else {
		$noteCode = 'const byte track'.$trackNumber.'_notes[] PROGMEM = { ';
		$deltaTimeCode = 'const word track'.$trackNumber.'_timeOffsets[] PROGMEM = { ';
	}

	$noteCount = 0;
	foreach ($notes as $index => $note) {
		if (!($index % 16)) {
			$noteCode .= chr(10) . chr(9);
			$deltaTimeCode .= chr(10) . chr(9);
		}
		if ($note['type'] === 'note-on') {
			$noteCode .= $note['note'] . ', ';
		} else {
			$noteCode .= 0x80 . ', ';
		}
		if ($note['delta-time'] > 0xFFFF) {
			throw new \Exception('So long durations are not supported!');
		}
		$deltaTimeCode .= $note['delta-time'] . ', ';
		$noteCount++;
	}
	$noteCode = substr($noteCode, 0, -2) . chr(10) . '};' . chr(10);
	$deltaTimeCode = substr($deltaTimeCode, 0, -2) . chr(10) . '};' . chr(10);

	echo $noteCode;
	echo $deltaTimeCode;

	$noteCounts[$track] = $noteCount;
}

echo "\nconst SYNTH_TRACK tracks[] PROGMEM = {".chr(10);
$cnt = 0;
foreach ($noteCounts as $track => $noteCount) {
	$cnt++;
	$trackNumber = $track+1;
	echo "\t// TRACK $trackNumber\n";
	echo "\t{\n";
	echo "\t\t.notes = track".$trackNumber."_notes,\n";
	echo "\t\t.timeOffsets = track".$trackNumber."_timeOffsets,\n";
	echo "\t\t.noteCount = ".$noteCount.",\n";
	echo "\t},\n";
}
echo "};".chr(10);

$microsecondsPerQuarterNote = $useTempo = 600000;

// echo "// Microseconds per quarter note: ".$microsecondsPerQuarterNote."\n";

$microsecondsPerTick = $microsecondsPerQuarterNote / $ticksPerQuarterNote;
$ticksPerSecond = 1000000/$microsecondsPerTick;
$samplesPerSecond = 12500;

$samplesPerTick = (int)round($samplesPerSecond / $ticksPerSecond);

echo "

void setup() {
	Synth.init(3);
}

void loop() {
	if (!Synth.isPlaying()) {
		delay(1000);
		Synth.play(tracks, ".$cnt.", ".$samplesPerTick.");
	} else {
		// Do something. ~3/5 ths of the CPU cycles are still available.
	}
}

";


