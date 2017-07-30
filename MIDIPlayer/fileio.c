#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fileio.h"

char * read_file(const char * path, unsigned int * len){
	char * buffer = NULL;
	long length;
	FILE * f = fopen(path, "rb");
	
	if(f){
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		buffer = malloc(length);
		if(buffer){
			fread(buffer, 1, length, f);
		}
		fclose(f);
	}
	
	*len = length;
	return buffer;
}

unsigned char * read_data_rel_path(const char * fname, unsigned int * len){
	char cwd[1024];
	if(getcwd(cwd, sizeof(cwd)) == NULL){
		perror("Could not get working directory.\n");
		return 0;
	}
	
	if(strlen(fname) + strlen(cwd) >= 1024){
		perror("Directory name(s) is too long.\n");
	}
	strcat(cwd, fname);
	printf("Directory: %s\n", cwd);
	
	char * output = read_file(cwd, len);
	return (unsigned char *)output;
}
