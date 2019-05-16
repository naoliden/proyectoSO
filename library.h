
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

char* dirfinder(char* path);

char* basefinder(char* path);



