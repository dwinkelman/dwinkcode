#ifndef _FILEIO_H_
#define _FILEIO_H_

char * read_file(const char * path, unsigned int * len);
unsigned char * read_data_rel_path(const char * fname, unsigned int * len);

#endif