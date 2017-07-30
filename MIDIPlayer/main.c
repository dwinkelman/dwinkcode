#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fileio.h"
#include "midi.h"
#include "textplayer.h"

int main(){
	const char * fname = "/data/bwv809a.mid";
	
	unsigned int len;
	unsigned char * data = read_data_rel_path(fname, &len);
	MidiFile * mf = parse_midi(data, len);
	
	scrollplay_midi(mf);
	
	return 0;
}