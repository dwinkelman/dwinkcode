#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "midi.h"

uint16_t read_big_endian_short(const uint8_t * data){
	return (uint16_t)data[0] << 8 | (uint16_t)data[1];
}

uint32_t read_big_endian_int(const uint8_t * data){
	return 		(uint32_t)data[0] << 24
			| 	(uint32_t)data[1] << 16
			| 	(uint32_t)data[2] << 8
			|	(uint32_t)data[3];
}

uint32_t read_var_int(const uint8_t * data){
	uint32_t output = 0;
	for(int i = 0; i < 4; i++){
		output |= data[i] & (uint8_t)0x7f;
		if(~data[i] & (uint8_t)0x80) break;
		else output <<= 7;
	}
	return output;
}

int var_int_bytes(const uint32_t parsed_int){
	if(parsed_int < (1 << 7)) return 1;
	else if(parsed_int < (1 << 14)) return 2;
	else if(parsed_int < (1 << 21)) return 3;
	else if(parsed_int < (1 << 28)) return 4;
	else return 5;
}

MidiFile * parse_midi(uint8_t * data, unsigned int n_data){
	printf("File at %p\n", data);
	
	// Check for correct file type
	if(data[0] != 'M' || data[1] != 'T' || data[2] != 'h' || data[3] != 'd'){
		perror("Error: file is not MIDI format\n");
		return NULL;
	}
	
	// Create MIDI file
	MidiFile * mf = malloc(sizeof(MidiFile));
	mf->data = data + 8;
	mf->header_len = read_big_endian_int(data + 4);
	mf->format = read_big_endian_short(data + 8);
	mf->n_tracks = read_big_endian_short(data + 10);
	mf->division = read_big_endian_short(data + 12);
	
	// Count number of tracks
	MidiTrack * tracks = malloc(mf->n_tracks * sizeof(MidiTrack));
	for(int i = 0, n = 0; i < n_data && n < mf->n_tracks; i++){
		if(data[i] == 'M'){
			if(data[i + 1] == 'T' && data[i + 2] == 'r' && data[i + 3] == 'k'){
				tracks[n].data = data + i + 8;
				tracks[n].length = read_big_endian_int(data + i + 4);
				parse_miditrack(tracks + n);
				n++;
			}
		}
	}
	
	mf->tracks = tracks;
	return mf;
}

void parse_miditrack(MidiTrack * mt){
	MidiEvent * events = malloc(mt->length / 3 * sizeof(MidiEvent));
	uint32_t n_events = 0;
	
	printf("*******Parsing Track********\n");
	
	uint8_t * i = mt->data, * end = mt->data + mt->length;
	for(; i < end; /* increment in loop */){
		
		// get time difference of event
		uint32_t delta_time = read_var_int(i);
		int delta_time_len = var_int_bytes(delta_time);
		
		uint8_t status = *(i + delta_time_len);
		// meta event found
		if(status == 0xff){
			uint8_t meta_type = *(i + delta_time_len + 1);
			uint32_t meta_len = read_var_int(i + delta_time_len + 2);
			int meta_len_len = var_int_bytes(meta_len);
			
			events[n_events].event_type = 0xff;
			events[n_events].event_data = meta_type;
			events[n_events].delta_time = delta_time;
			events[n_events].data = i + delta_time_len + 2 + meta_len_len;
			events[n_events].n_data = meta_len;
			
			//printf("Found meta of type %x and size %i (len %i) at time %i (len %i)\n", meta_type, meta_len, meta_len_len, delta_time, delta_time_len);
			
			i += delta_time_len /* Time parameter */ + 1 /* 0xFF flag */
				+ 1 /* Event Type */ + meta_len_len /* Length of the meta length */
				+ meta_len /* Length of the whole meta */;
			n_events++;
		}
		// sysex events
		else if(status == 0xf0 || status == 0xf7){
			uint32_t sysex_len = read_var_int(i + 1);
			int sysex_len_len = var_int_bytes(sysex_len);
			
			events[n_events].event_type = status;
			events[n_events].event_data = 0;
			events[n_events].delta_time = delta_time;
			events[n_events].data = i + delta_time_len + 1 + sysex_len_len;
			events[n_events].n_data = sysex_len;
			
			//printf("Found sysex event of size %i\n", sysex_len);
			
			i += delta_time_len /* Time parameter */ + 1 /* 0xF0/0xF7 flag */
				+ sysex_len_len /* Length of the sysex length */ + sysex_len;
			n_events++;
		}
		// MIDI events
		else{
			uint8_t type = (status >> 4) & 15;
			uint8_t channel = status & 15;
			
			events[n_events].event_type = type;
			events[n_events].event_data = channel;
			events[n_events].delta_time = delta_time;
			
			//printf("Found event type %x directed to channel %i at time %i \n", type, channel, delta_time);
			
			if(type == 0xC || type == 0xD){
				events[n_events].data = i + delta_time_len + 2;
				events[n_events].n_data = 1;
				i += delta_time_len + 2;
			}
			else{
				events[n_events].data = i + delta_time_len + 3;
				events[n_events].n_data = 2;
				i += delta_time_len + 3;
			}
			n_events++;
		}
	}
	
	printf("FOUND %i EVENTS\n", n_events);
	
	mt->events = events;
	mt->n_events = n_events;
}

int midi_event_type(MidiEvent * ev){
	switch(ev->event_type){
		case 0xff: return MIDI_EVENT_TYPE_META;
		case 0xf0: return MIDI_EVENT_TYPE_SYSEX;
		case 0xf7: return MIDI_EVENT_TYPE_SYSEX;
		default:   return MIDI_EVENT_TYPE_EVENT;
	}
}

static char * parse_midi_meta_string(uint8_t * data, unsigned int length){
	uint8_t * output = (uint8_t *)malloc(length + 1);
	memcpy(output, data, length);
	output[length] = 0; // null-terminate
	return (char *)output;
}

void parse_midi_meta(MidiEvent * ev, MidiMetaData * md){
	
	// Reset metadata
	free(md->text_event);
	free(md->copyright_notice);
	free(md->sequence_track_name);
	free(md->instrument_name);
	free(md->lyric);
	free(md->marker);
	free(md->cue_point);
	memset(md, 0, sizeof(*md));
	
	// Check that the event is a meta event
	if(ev->event_type != 0xff){
		md->type = MIDI_META_INVALID;
		return;
	}
	
	md->type = ev->event_data;
	md->length = ev->n_data;
	
	switch(ev->event_data){
		case MIDI_META_SEQUENCE_NUMBER:
			md->sequence_number = read_big_endian_short(ev->data);
			break;
		case MIDI_META_TEXT_EVENT:
			md->text_event = parse_midi_meta_string(ev->data, ev->n_data);
			break;
		case MIDI_META_COPYRIGHT_NOTICE:
			md->copyright_notice = parse_midi_meta_string(ev->data, ev->n_data);
			break;
		case MIDI_META_SEQUENCE_TRACK_NAME:
			md->sequence_track_name = parse_midi_meta_string(ev->data, ev->n_data);
			break;
		case MIDI_META_INSTRUMENT_NAME:
			md->instrument_name = parse_midi_meta_string(ev->data, ev->n_data);
			break;
		case MIDI_META_LYRIC:
			md->lyric = parse_midi_meta_string(ev->data, ev->n_data);
			break;
		case MIDI_META_MARKER:
			md->marker = parse_midi_meta_string(ev->data, ev->n_data);
			break;
		case MIDI_META_CUE_POINT:
			md->marker = parse_midi_meta_string(ev->data, ev->n_data);
			break;
		case MIDI_META_CHANNEL_PREFIX:
			md->midi_channel_prefix = ev->data[0];
			break;
		case MIDI_META_END_OF_TRACK:
			break;
		case MIDI_META_TEMPO:
			md->tempo = read_big_endian_int(ev->data);
			break;
		case MIDI_META_SMPTE_OFFSET:
			md->smpte_offset_hours = ev->data[0];
			md->smpte_offset_minutes = ev->data[1];
			md->smpte_offset_seconds = ev->data[2];
			md->smpte_offset_frames = ev->data[3];
			md->smpte_offset_frac_frames = ev->data[4];
			break;
		case MIDI_META_TIME_SIGNATURE:
			md->time_signature_numerator = ev->data[0];
			md->time_signature_denominator = ev->data[1];
			md->time_signature_metronome = ev->data[2];
			md->time_signature_32notes = ev->data[3];
			break;
		case MIDI_META_KEY_SIGNATURE:
			md->key_signature_sharps = ev->data[0];
			md->key_signature_is_minor = ev->data[1];
			break;
		case MIDI_META_SEQSPEC:
			break;
		default:
			md->type = MIDI_META_INVALID;
			break;
	}
}

MidiMetaData * midi_find_meta_data(MidiFile * mf, uint8_t meta_type){
	for(int i = 0; i < mf->n_tracks; i++){
		for(int j = 0; j < mf->tracks[i].n_events; j++){
			if(midi_event_type(&mf->tracks[i].events[j]) == MIDI_EVENT_TYPE_META){
				if(mf->tracks[i].events[j].event_data == meta_type){
					MidiMetaData * meta_data = (MidiMetaData *)malloc(sizeof(MidiMetaData));
					parse_midi_meta(&mf->tracks[i].events[j], meta_data);
					return meta_data;
				}
			}
		}
	}
	return NULL;
}

void print_midifile(const MidiFile * mf){
	printf("Midi File: [Format %.1i | %i Tracks | Division %i] [Memory %p]\n", mf->format, mf->n_tracks, mf->division, (void *)mf->data);
}

void print_miditrack(const MidiTrack * track){
	printf("Midi Track: [Length %i] [Memory %p]\n", track->length, (void *)track->data);
}