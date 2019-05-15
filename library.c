#include <string.h>
#include <libgen.h>

#include "cr_API.h"
#include "library.h"

typedef struct crFILE{
	//FIXME
	FILE* cursor;
	FILE* root;
	unsigned int* offset;

	int block;
	int size; 
	unsigned char name[12];
	unsigned char data[1021 * 2048];

} crFILE;

char* dirfinder(char* path) {
	char *dirc, *dname;
	dirc = strdup(path);
	dname = dirname(dirc);	
	return dname;
}

char* basefinder(char* path) {
	char *basec, *bname;
	basec = strdup(path);
	bname = basename(basec);	
	return bname;
}

