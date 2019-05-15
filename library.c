#include <string.h>
#include <libgen.h>

#include "library.h"


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
