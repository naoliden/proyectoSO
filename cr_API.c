// Librerias Std:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cr_API.h"

crFILE puntero;


int move(char* path ){

	char* folder = strtok(path, "/");
	unsigned char *buffer = malloc( sizeof( unsigned char ) * 32 );
	puntero.cursor = puntero.root;

	while(folder){

		for( int i = 0; i < 64; i++ ) {
			fseek( puntero.cursor, 32 * i, SEEK_SET );
			fread( buffer, sizeof( unsigned char ), 32, puntero.cursor );
			char folder_name[27];

			memcpy(folder_name, &buffer[1], 26);

			if (buffer[0] == (unsigned char)1 ){
				printf("Path invalido");
				free(buffer);
				return 0;
			} else {
				if (strcmp(folder, folder_name) == 0){
					memcpy(puntero.cursor, &buffer[28], 4);
					folder = strtok(NULL, "/");
					break;
				}
			}
			if (i == 63) {
				free(buffer);
				return 0;
			}
		}
	}

	free(buffer);
	return 1;
}


/*
Funcio ́n para montar el disco.
Establece como variable global la ruta local donde se encuentra el archivo .bin correspondiente al disco. */
void cr_mount(char* diskname){
	FILE* f = fopen(diskname, "rb");
	puntero.root = f;
}

/*
Funcio ́n para imprimir el bitmap. Cada vez que se llama esta funcio ́n, imprime en stderr el estado actual
del disco de acuerdo al contenido del bitmap. Debe imprimir una l ́ınea con el contenido del bitmap
(un 1 por cada bloque ocupado y un 0 por cada bloque libre), en la siguiente l ́ınea la cantidad de bloques ocupados,
y en una tercera l ́ınea la cantidad de bloques libres.*/

void cr_bitmap(){
	unsigned char mask = 1;
	int uno = 0;
	int cero = 0;
	unsigned char * buffer = malloc( sizeof( unsigned char ) * 2048*4);
	puntero.cursor = puntero.root;
	fseek(puntero.cursor, 2048, SEEK_SET);
	fread(buffer, 1, 2048*4, puntero.cursor);
	for (int k = 0; k < 2048*4; k++) {
		unsigned char bits[8];
		for(int j = 0;j<8;j++){
			bits[j] = (buffer[k] & (mask << j)) != 0;
			printf("%d", bits[j]);
			if (bits[j] == 1) {
				uno++;
			}
			else{
				cero++;
			}
		}
	}
	printf("\nOCUPADOS: %d", uno);
	printf("\nLIBRES: %d\n", cero);
	free(buffer);
}

/*
Funcion para ver si un archivo o carpeta existe en la ruta especificada por path.
 Retorna 1 si el archivo o carpeta existe y 0 en caso contrario.
*/

// TODO
int cr_exists(char* path){
	return move(path);
}

/*
Funcion para listar los elementos de un directorio del disco.Imprime en pantalla los nombres
de todos los archivos y directorios contenidos en el directorio indicado por path.*/

// TODO
void cr_ls(char* path){
	char* folder = strtok(path, "/");

	unsigned char *buffer = malloc( sizeof( unsigned char ) * 32 );
	move(path);


	for( int i = 0; i < 64; i++ ) {
		fseek( puntero.cursor, 32 * i, SEEK_SET );
		fread( buffer, sizeof( unsigned char ), 32, puntero.cursor );

		if ( buffer[0] == (unsigned char)1 ) {
			printf( "Entrada invalida\n");

		} else if (buffer[0] == (unsigned char)2 ) {

			printf( "DIR %s index: %u\n", buffer + 1, (unsigned int)buffer[30] * 256 + (unsigned int)buffer[31] );

		} else if (buffer[0] == (unsigned char)4){
			printf( "FILE %s index: %u\n", buffer + 1, (unsigned int)buffer[30] * 256 + (unsigned int)buffer[31] );

		} else {
			printf( "Entrada invalida\n");
		}
	}
	free( buffer );
}

/*
Función para crear directorios. Crea el directorio vacío referido por foldername.
Funciones de manejo de archivos
*/

int cr_mkdir(char *foldername){

	// CHECK IF DIRECTORY ALREADY EXISTS
	unsigned char * directory = malloc( sizeof( unsigned char ) * 32 );
	directory[0] = (unsigned char)2;

	int i = 0;
	unsigned int * foldername_bin = malloc(sizeof(unsigned int) * 27);
	while (foldername[i] != '\0'){
		for (int j = 7; j >= 0; --j){
			foldername_bin[i] = ( (foldername[i] & (1 << j)) ? '1' : '0' );
			i++;
		}
	}

	directory[1]= (unsigned char)foldername;
	printf("DIRECTORY IS %s\n", directory+1);

	unsigned char * buffer = malloc( sizeof( unsigned char ) * 2048*4);
	unsigned char mask = 1;
	puntero.cursor = puntero.root;
	fseek(puntero.cursor, 2048, SEEK_SET);
	fread(buffer, 1, 2048*4, puntero.cursor);
	int index = -1;
	for (int k = 0; k < 2048*4; k++) {
		unsigned char bits[8];
		for(int j = 0;j<8;j++){
			index++;
			bits[j] = (buffer[k] & (mask << j)) != 0;
			if (bits[j] == 0){
				directory[31] = (unsigned int)index;
				printf( "MAKING DIR %s index: %u\n", directory + 1, (unsigned int)directory[30] * 256 + (unsigned int)directory[31] );
				index = -1;
				break;
			}
		}
		if (index == -1) {
			break;
		}
	}
	puntero.cursor = puntero.root;
	for( int i = 0; i < 64; i++ ) {
		fseek(puntero.cursor, 32 * i, SEEK_SET );
		fread(buffer, sizeof( unsigned char ), 32, puntero.cursor );

		if (buffer[0] == (unsigned char)0 ) {
			puntero.cursor = (unsigned char)directory;
			free(directory);
			break;
		}
		if(i == 63){

		}
	}
	free(buffer);
}

/*
Funcion para abrir un archivo. Si mode es‘r’,busca el archivo en la ruta path y retorna un crFILE* que lo representa.*/

crFILE * cr_open(char * path, char mode){
	if (move(path) != 0 && mode == 'r'){
		return puntero.cursor;
	}
	else{
		printf("ERROR\n");
	}
}

/*
Funcio ́n para leer archivos. Lee los siguientes nbytes desde el archivo descrito por file desc y los guarda en
la direccio ́n apuntada por buffer. Debe retornar la cantidad de Byte efectivamente le ́ıdos desde el archivo.
Esto es importante si nbytes es mayor a la cantidad de Byte restantes en el archivo. La lectura de read se efectua
desde la posicio ́n del archivo inmediatamente posterior a la u ́ltima posicio ́n le ́ıda por un llamado a read.*/

int cr_read(crFILE* file_desc, void* buffer, int nbytes){
	int num_blocks = 1 + nbytes/2048;
	unsigned char * punteros = malloc( sizeof( unsigned char ) * 4*num_blocks);
	fseek(file_desc, 8, SEEK_SET);
	fread(punteros, 4*num_blocks,1, file_desc);


	int j = 0;
	int bytes_read = 0;
	unsigned int * p = malloc(sizeof(unsigned char) * 4);
	for (int i = 0; i < 4*num_blocks; i++) {
		p[j] = punteros[i];
		j++;
		if (j == 3) {
			/* OPEN AND READ FILE */
			puntero.cursor = p;
			fseek(puntero.cursor, 0, SEEK_SET);
			int size_f = ftell(puntero.cursor);
			bytes_read = bytes_read + size_f;
			fseek(puntero.cursor, 0, SEEK_SET);
			unsigned char buffer[size_f];
			fread(buffer, 1, size_f, puntero.cursor);
			printf("%s\n", buffer);
			j = 0;
		}
	}
	return bytes_read;
}

/*
Funcio ́n para escribir ar- chivos. Escribe en el archivo descrito por file desc los nbytes que se encuentren
en la direccio ́n indicada por buffer. Retorna la cantidad de Byte escritos en el archivo. Si se produjo un error
porque no pudo seguir escribiendo, ya sea porque el disco se lleno ́ o porque el archivo no puede crecer mas,
este nu ́mero puede ser menor a nbytes (incluso 0).*/

int cr_write(crFILE* file_desc, void* buffer, int nbytes){

}

/*
Funcio ́n para cerrar archivos. Cierra el archivo indicado por file desc. Debe garantizar que cuando esta funcion
retorna, el archivo se encuentra actualizado en disco.*/

int cr_close(crFILE* file_desc){

}

/*
Funcion para borrar archivos. Elimina el archivo referenciado por la ruta path del directorio correspondiente.
Los bloques que estaban siendo usados por el archivo deben quedar libres si, y solo si, la cantidad de hardlinks
restante es igual a cero.*/

int cr_rm(char* path){

}

/*
Funcio ́n que se encarga de crear un hardlink del archivo referenciado por orig en una nueva ruta dest,
aumentando la cantidad de referencias al archivo original.*/

int cr_hardlink(char* orig, char* dest){

}

/*
Funcio ́nqueseencargadecopiarunarchivoouna ́rbol de directorios (es decir, un directorio y todos sus contenidos)
del disco, referenciado por orig, hacia un nuevo archivo o directorio de ruta dest en su computador.*/

int cr_unload(char* orig, char* dest){

}

/*
Funcion que se encarga de copiar un archivo o arbol de directorios, referenciado por orig al disco. En caso de que
un archivo sea demasiado pesado para el disco, se debe escribir todo lo posible hasta acabar el espacio
disponible.*/

int cr_load(char* orig){

}
