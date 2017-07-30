#ifndef _MIDI_H_
#define _MIDI_H_

#include <stdint.h>

#define MIDI_EVENT_TYPE_EVENT 		101
#define MIDI_EVENT_TYPE_META		102
#define MIDI_EVENT_TYPE_SYSEX		103

#define MIDI_EVENT_NOTE_ON			104 // 0x8n
#define MIDI_EVENT_NOTE_OFF			105 // 0x9n
#define MIDI_EVENT_PROGRAM_CHANGE	106 // 0xCn

#define MIDI_META_INVALID			0xff
#define MIDI_META_SEQUENCE_NUMBER	0x00
#define MIDI_META_TEXT_EVENT		0x01
#define MIDI_META_COPYRIGHT_NOTICE	0x02
#define MIDI_META_SEQUENCE_TRACK_NAME 0x03
#define MIDI_META_INSTRUMENT_NAME	0x04
#define MIDI_META_LYRIC				0x05
#define MIDI_META_MARKER			0x06
#define MIDI_META_CUE_POINT			0x07
#define MIDI_META_CHANNEL_PREFIX	0x20
#define MIDI_META_END_OF_TRACK		0x2f
#define MIDI_META_TEMPO				0x51
#define MIDI_META_SMPTE_OFFSET		0x54
#define MIDI_META_TIME_SIGNATURE	0x58
#define MIDI_META_KEY_SIGNATURE		0x59
#define MIDI_META_SEQSPEC			0x7f

typedef struct tagMidiMetaData {
	uint8_t type;
	uint32_t length;
	uint16_t sequence_number; 		// 00
	char * text_event;				// 01
	char * copyright_notice;		// 02
	char * sequence_track_name;		// 03
	char * instrument_name;			// 04
	char * lyric;					// 05
	char * marker;					// 06
	char * cue_point;				// 07
	uint8_t midi_channel_prefix;	// 20
									// 2f (end of track)
	uint32_t tempo;					// 51
	uint8_t							// 54
		smpte_offset_hours,
		smpte_offset_minutes,
		smpte_offset_seconds,
		smpte_offset_frames,
		smpte_offset_frac_frames;
	uint8_t							// 58
		time_signature_numerator,
		time_signature_denominator,
		time_signature_metronome,
		time_signature_32notes;
	int8_t key_signature_sharps;	// 59
	uint8_t key_signature_is_minor;
	uint8_t seqspec_id;				// 7f
	uint8_t * seqspec_data;
} MidiMetaData;

typedef struct tagMidiEvent {
	uint8_t * data;
	uint32_t n_data;
	uint32_t delta_time;
	uint8_t event_type;
	uint8_t event_data;
} MidiEvent;

typedef struct tagMidiTrack {
	uint8_t * data;
	uint32_t length;
	
	// Events
	MidiEvent * events;
	uint32_t n_events;
} MidiTrack;

typedef struct tagMidiFile {
	uint8_t * data;
	
	// Header
	uint32_t header_len;
	uint16_t format;
	uint16_t n_tracks;
	uint16_t division;
	
	// Tracks
	MidiTrack * tracks;
} MidiFile;

uint16_t read_big_endian_short(const uint8_t * data);
uint32_t read_big_endian_int(const uint8_t * data);

uint32_t read_var_int(const uint8_t * data);
int var_int_bytes(const uint32_t parsed_int);

MidiFile * parse_midi(uint8_t * data, unsigned int n_data);

void parse_miditrack(MidiTrack * mt);

int midi_event_type(MidiEvent * ev);

void parse_midi_event_note_off(MidiEvent * ev, uint8_t * voice, uint8_t * note, uint8_t * velocity);
void parse_midi_event_note_on(MidiEvent * ev, uint8_t * voice, uint8_t * note, uint8_t * velocity);
void parse_midi_event_program_change(MidiEvent * ev, uint8_t * voice, uint8_t * instrument);

void parse_midi_meta(MidiEvent * ev, MidiMetaData * md);
MidiMetaData * midi_find_meta_data(MidiFile * file, uint8_t meta_type);

void print_midifile(const MidiFile * mf);
void print_miditrack(const MidiTrack * track);

#endif