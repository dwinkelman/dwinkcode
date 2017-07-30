#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "textplayer.h"

void scrollplay_midi(MidiFile * mf){
	if(!mf){
		perror("Invalid NULL file\n");
	}
	
	// Division defined as ticks per quarter note
	if(0x8000 & mf->division){
		perror("Files without specified delta-time units per quarter note are not supported\n");
		return;
	}
	
	// Get tempo as number of ticks per quarter note
	int tempo = mf->division & 0x7fff;
	printf("Division: %i", tempo);
	
	// Get the range of the song
	int min = 128, max = 0;
	for(int i = 0; i < mf->n_tracks; i++){
		for(int j = 0; j < mf->tracks[i].n_events; j++){
			if(midi_event_type(&mf->tracks[i].events[j]) == MIDI_EVENT_TYPE_EVENT){
				if((mf->tracks[i].events[j].event_data >> 4) == 8){
					// note on
					
				}
			}
		}
	}
}