#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <math.h>

#include "cr_API.h"
#include "library.h"


crFILE p;
char disk_path[256];


int main(){
  memset(disk_path, '\0', sizeof(disk_path));
	strcpy(disk_path, "./simdiskfilled.bin");

  char * path = "thanos/thanosgif";
  if(strchr(path, '.')){
    printf("FILE!!!!!\n");

  }
  int counter = 0;
  int num_folders = 0;
  char * original_path = path;
  int * pos_slash = malloc(10*sizeof(int));

  FILE * f = fopen(disk_path, "r");
  int end = 1;
  while(end==1){
    if(path[0] == '/'){
      pos_slash[num_folders] = counter;
      printf(" HALLO: %d\n", pos_slash[num_folders]);
      num_folders++;
    }
    if(!*path++){
      if(num_folders>0 && !(pos_slash[num_folders-1] == counter-1)){
        pos_slash[num_folders] = counter;
        printf(" HALLO: %d\n", pos_slash[num_folders]);
        num_folders++;
      }
      end = 0;
    }
    counter ++;
  }

  unsigned char * buffer = malloc( sizeof(unsigned char) * 32 );
  int bloque_directorio = 0;
  for(int j = 0; j<num_folders;j++){

    int beginning = 0;
    if(j > 0){beginning = pos_slash[j-1]+1;}
    else if (j == 0 && original_path[0] == '/'){beginning = 1;}

    char * folder = malloc((pos_slash[j]-beginning)*sizeof(char));
    memcpy(folder, &original_path[beginning], pos_slash[j]-beginning);

    for(int i = 0; i < 64; i++ ) {
      p.offset = 32*i;
			fseek(f, 2048*bloque_directorio + 32 * i, SEEK_SET );
			fread(buffer, sizeof( unsigned char ), 32, f);
			char folder_name[27];
			memcpy(folder_name, &buffer[1], 26);
      printf("FOLDER IN FILE: %s\n", folder_name);
			if (buffer[0] == (unsigned char)1 ){
				printf("Path invalido\n");
				free(buffer);
				fclose(f);
				return 0;
			} else {
				if (strcmp(folder, folder_name) == 0){
        	bloque_directorio = (unsigned int)buffer[30] * 256 + (unsigned int)buffer[31];
          p.offset = p.offset + 28;
          //p.block = bloque_directorio;
          printf("BLOCK: %d\n", bloque_directorio);
					break;
				}
			}
			if (i == 63) {
				free(buffer);
				fclose(f);
				return 0;
			}
		}
  }

}

int move_index(char* path, crFILE* p){


	FILE * f = fopen(disk_path, "rb");

	p->offset = 0;
/*
const char folder[256];

const char line[256];
strcpy(line, path);
*/
	int level = 0;
	char * folder = "HOLA";
	//strtok(line, "/");


	unsigned char * buffer = malloc( sizeof(unsigned char) * 32 );
	while(*path++){
		printf("PATH: %s\n", path);
		for(int i = 0; i < 64; i++ ) {
			fseek(f, 32 * i, SEEK_SET );
			p->offset = 32*i;
			fread(buffer, sizeof( unsigned char ), 32, f);
			char folder_name[27];
			memcpy(folder_name, &buffer[1], 26);
			if (buffer[0] == (unsigned char)1 ){
				printf("Path invalido\n");
				free(buffer);
				fclose(f);
				return 0;
			} else {
				if (strcmp(folder, folder_name) == 0){
					p->offset = p->offset + 28;
					folder = strtok(NULL, "/");
					level++;
					break;
				}
			}
			if (i == 63) {
				free(buffer);
				fclose(f);
				return 0;
			}
		}
	}
	free(buffer);
	fclose(f);
	return 1;
}
