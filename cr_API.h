// Librerias Std:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


char disk_path[256];

typedef struct blockIndex {
	// Si algo falla tal vez haya que cambiar block_number a unsigned
	int block_number;
	int byte_number;
	int bit_number;
	unsigned char new_byte[8];
} blockIndex;


typedef struct crFILE{
	unsigned char * cursor;
	int offset;
	int exists;
	unsigned int block;
	unsigned char file_name[27];
	unsigned char data[2048];
}crFILE;


char* dirfinder(char* path);

char* basefinder(char* path);

blockIndex* find_empty_block();

void change_bitmap(blockIndex* block);

char* itoa (int value, char *result, int base);

void cr_mount(char* diskname);

void cr_bitmap();

int cr_exists(char* path);

void cr_ls(char* path);

int cr_mkdir(char *foldername) ;

crFILE* cr_open(char* path, char mode);

int cr_read(crFILE* file_desc, void* buffer, int nbytes);

int cr_write(crFILE* file_desc, void* buffer, int nbytes);

int cr_close(crFILE* file_desc);

int cr_rm(char* path);

int cr_hardlink(char* orig, char* dest);

int cr_unload(char* orig, char* dest);

int cr_load(char* orig);
