#include <stdio.h>
#include <stdlib.h>

int read_entry ( unsigned char *buffer ) {
	if ( buffer[0] == (unsigned char)1 ) {
		return 0;
	} else {
		if ( buffer[0] == (unsigned char)2 ) {
			printf( "DIR %s index: %u\n", buffer + 1, (unsigned int)buffer[30] * 256 + (unsigned int)buffer[31] );
		} else {
			printf( "FILE %s index: %u\n", buffer + 1, (unsigned int)buffer[30] * 256 + (unsigned int)buffer[31] );
		};
		return 0;
	};
};

int main( void ) {
	FILE* f = fopen( "simdiskfilled.bin", "rb" );
	unsigned char *buffer = malloc( sizeof( unsigned char ) * 32 );

	for( int i = 0; i < 64; i++ ) {
		fseek( f, 32 * i, SEEK_SET );
		fread( buffer, sizeof( unsigned char ), 32, f );
		read_entry( buffer );
	};

	free( buffer );
	return 0;
};