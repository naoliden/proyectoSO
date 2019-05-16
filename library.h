typedef struct blockIndex {
	int block_number;
	int byte_number;
	int bit_number;
	unsigned char new_byte[8];
} blockIndex;

char* dirfinder(char* path);

char* basefinder(char* path);

blockIndex* find_empty_block();

void change_bitmap(blockIndex* block);




