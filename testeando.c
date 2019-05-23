#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <math.h>
#include "cr_API.h"

char disk_path[256];
blockIndex* find_block(){
	int encontrado = 0;
	unsigned int block_num = 0;
	unsigned char * buffer = malloc( sizeof( unsigned char ) * 2048*4);
	unsigned char mask = 1;
	blockIndex* block = malloc(sizeof(blockIndex));

	FILE* file = fopen("./simdiskfilled.bin", "r+b");
	fseek(file, 2048, SEEK_SET);
	fread(buffer, 1, 2048*4, file);
	unsigned char byte[8];
	for (int k = 0; k < 2048*4; k++) {
		printf("K=%d\n", k);
		byte[7] = (unsigned char) (buffer[k] & (mask <<0)) != 0;
		byte[6] = (unsigned char) (buffer[k] & (mask <<1)) != 0;
    byte[5] = (unsigned char) (buffer[k] & (mask <<2)) != 0;
    byte[4] = (unsigned char) (buffer[k] & (mask <<3)) != 0;
    byte[3] = (unsigned char) (buffer[k] & (mask <<4)) != 0;
    byte[2] = (unsigned char) (buffer[k] & (mask <<5)) != 0;
    byte[1] = (unsigned char) (buffer[k] & (mask <<6)) != 0;
    byte[0] = (unsigned char) (buffer[k] & (mask <<7)) != 0;
		printf("BYTE: %d%d%d%d%d%d%d%d\n", byte[0], byte[1], byte[2], byte[3], byte[4], byte[5], byte[6], byte[7]);
		for(int j = 0; j<8; j++){
			printf("bit %d = %d\n",j, byte[j]);


			//byte[j] = (buffer[k] & (mask << j)) != 0;
			if (byte[j] == 0) {
        printf("BLOCK: %d\n", block_num);
				encontrado = 1;

				block->block_number = block_num;
				block->byte_number = k;
				block->bit_number = j;

				//memcpy(block->new_byte, byte, 8);
				// strcpy(block->new_byte, byte);
				byte[j] = (unsigned char)1;
				block->new_byte = (unsigned int)byte[0] * 1 + (unsigned int)byte[1]*2 + (unsigned int)byte[2] * 4 + (unsigned int)byte[3]*8 +(unsigned int)byte[4] * 16 + (unsigned int)byte[5]*32+(unsigned int)byte[6] * 64 + (unsigned int)byte[7]*128;
        printf("NEW BYTE: %u\n", block->new_byte);
        unsigned char * data = malloc(2*sizeof(unsigned char));
      	data[0] = (unsigned int)byte[0] * 1 + (unsigned int)byte[1]*2 + (unsigned int)byte[2] * 4 + (unsigned int)byte[3]*8 +(unsigned int)byte[4] * 16 + (unsigned int)byte[5]*32+(unsigned int)byte[6] * 64 + (unsigned int)byte[7]*128;
        fseek(file, 2048 + k, SEEK_SET);
				fwrite(data, 1, 1, file);
        break;
			}
      block_num++;
		}
		if (encontrado){
			break;
		}
	}
	//free(byte);
	free(buffer);
	fclose(file);
	return block;
}

int main(){
  cr_mount("./simdiskfilled.bin");
  cr_bitmap();
  find_block();
  cr_bitmap();
}
