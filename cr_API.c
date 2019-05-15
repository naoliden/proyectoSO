// Librerias Std:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "cr_API.h"

crFILE puntero;

unsigned int offset;

int move_index(char* path, crFILE * p){
	/*
	if(strcmp(path[0],".") == 0){
		memcpy(path, path[1], sizeof(path));
	}
	*/
	p->offset = 0;
	FILE * f = fopen(disk_path, "rb");
	char * folder = malloc(256*sizeof(char));
	folder = strtok(path, "/");
	unsigned char * buffer = malloc( sizeof(unsigned char) * 32 );

	while(folder){
		printf("FOLDER: %s\n", folder);
		for(int i = 0; i < 64; i++ ) {
			fseek(f, 32 * i, SEEK_SET );
			p->offset = 32*i;
			fread(buffer, sizeof( unsigned char ), 32, f);
			char folder_name[27];
			memcpy(folder_name, &buffer[1], 26);
			if (buffer[0] == (unsigned char)1 ){
				printf("Path invalido");
				free(folder);
				free(buffer);
				fclose(f);
				return 0;
			} else {
				if (strcmp(folder, folder_name) == 0){
					p->offset = p->offset + 28;
					folder = strtok(NULL, "/");
					break;
				}
			}
			if (i == 63) {
				free(folder);
				free(buffer);
				fclose(f);
				return 0;
			}
		}
	}
	free(folder);
	free(buffer);
	fclose(f);
	return 1;
}


/*
Funcio ́n para montar el disco.
Establece como variable global la ruta local donde se encuentra el archivo .bin correspondiente al disco. */
void cr_mount(char* diskname){

	memset(disk_path, '\0', sizeof(disk_path));
	strcpy(disk_path, diskname);

	// FILE* f = fopen(diskname, "rb");
	// puntero.root = f;
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

int cr_exists(char* path){
	return move_index(path, &puntero);
}

/*
Funcion para listar los elementos de un directorio del disco.Imprime en pantalla los nombres
de todos los archivos y directorios contenidos en el directorio indicado por path.*/

void cr_ls(char* path){
	FILE * f = fopen(disk_path, "rw");
	char * folder = strtok(path, "/");
	unsigned char * buffer = malloc( sizeof( unsigned char ) * 32 );
	move_index(path, &puntero);
	printf("OFFSET: %d\n", puntero.offset);
	unsigned char index_block[4];
	int index_block_num = 0;
	if(puntero.offset > 0){
		fseek(f, puntero.offset, SEEK_SET);
		fread(index_block, 1, 4, f);
		index_block_num = (unsigned int)index_block[2] * 256 + (unsigned int)index_block[3];
	}


	for( int i = 0; i < 64; i++ ) {
		fseek(f, 2048*index_block_num + 32 * i, SEEK_SET);
		fread(buffer, sizeof( unsigned char ), 32, f);
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
	free(buffer);
	fclose(f);
}

/*
Función para crear directorios. Crea el directorio vacío referido por foldername.
Funciones de manejo de archivos
*/

int cr_mkdir(char *foldername){
	/*
	1. Verify that directory does not exist
	2. Find unused block in bitmap
	3. Set block in bitmap to 1
	4. Save a 32 bit string with indicator, name and location of puntero in the directory where new directory should be saved
	*/

	unsigned char* directory = malloc( sizeof( unsigned char ) * 32 );
	directory[0] = (unsigned char)2;

	int i = 0;
	unsigned int* foldername_bin = malloc(sizeof(unsigned int) * 27);
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
			puntero.cursor = (FILE*)directory;
			free(directory);
			break;
		}
		if(i == 63){printf("QUE PASA\n");}
	}
	free(buffer);
	return 0;
}

/*
Funcion para abrir un archivo.
Si mode es ‘r’,
busca el archivo en la ruta path y retorna un crFILE* que lo representa.
*/

crFILE * cr_open(char * path, char mode){
	crFILE * open_file = malloc(sizeof(crFILE));
	int existe = move_index(path, open_file);
	printf("HOLA\n");
	if (existe != 0 && mode == 'r'){
		return open_file;
	}
	else if (existe == 0 && mode == 'w'){
		crFILE * nuevo_archivo = malloc(sizeof(crFILE));
		/*
		Crear nuevo archivo
		Cambiar bit del bloque en bitmap
		*/
		return nuevo_archivo;
	}
	else{
		printf("ERROR\n");
		return NULL;
	}
}

/*
Funcio ́n para leer archivos. Lee los siguientes nbytes desde el archivo descrito por file desc y los guarda en
la direccio ́n apuntada por buffer. Debe retornar la cantidad de Byte efectivamente le ́ıdos desde el archivo.
Esto es importante si nbytes es mayor a la cantidad de Byte restantes en el archivo. La lectura de read se efectua
desde la posicio ́n del archivo inmediatamente posterior a la u ́ltima posicio ́n le ́ıda por un llamado a read.*/

int cr_read(crFILE * file_desc, void* buffer, int nbytes){
	FILE * f = fopen(disk_path, "r");

	// FINDING INDEX block
	unsigned char index_block[4];
	fseek(f, file_desc->offset, SEEK_SET);
	fread(index_block, 1, 4, f);
	int index_block_num = (unsigned int)index_block[2] * 256 + (unsigned int)index_block[3];
	printf("INDEX BLOCK: %d\n", index_block_num);

	// FINDING FILE SIZE
	unsigned char size[4];
	fseek(f, 2048*index_block_num, SEEK_SET);
	fread(size, 1, 4, f);
	int file_size = (unsigned int)size[0]*256^3 + (unsigned int)size[1] * 256^2 +(unsigned int)size[2] * 256 + (unsigned int)size[3];
	printf("FILE SIZE: %u\n", file_size);

	// FINDING NUMBER OF HARDLINKS
	unsigned char hardlinks[4];
	//fseek(f, 2048*index_block_num + 4, SEEK_SET);
	fread(hardlinks, 1, 4, f);
	unsigned int num_hardlinks = (unsigned int)hardlinks[0]*256^3 + (unsigned int)hardlinks[1] * 256^2 +(unsigned int)hardlinks[2] * 256 + (unsigned int)hardlinks[3];
	printf("HARDLINKS: %u \n", num_hardlinks);

	// HOW MANY BYTES TO READ
	if (nbytes > file_size) {
		nbytes = file_size;
	}
	int num_blocks =  ceil(nbytes/2048.0);
	unsigned char * punteros = malloc(4*num_blocks*sizeof(unsigned char));
	unsigned char * buffer1 = malloc(2048*sizeof(unsigned char));
	int to_read = 2048;
	// NOT SO SURE ABOUT THIS PART. LOTS OF SEGMENTATION FAULTS.
	// FIRST, WE SHOULD READ 4 BYTES TO GET DATA BLCK, THEN READ THE WHOLE DATA BLOCK.
	for(int i = 0;i<num_blocks;i++){
		if (i == num_blocks - 1){
			to_read = nbytes - 2048*i;
		}
		printf("TO READ: %d\n", to_read );
		fseek(f, file_desc->offset + 8 + i*4, SEEK_SET);
		fread(punteros, 1, 4, f); // READING PUNTERO
		unsigned int offset = (unsigned int)punteros[0]*256^3 + (unsigned int)punteros[1] * 256^2 +(unsigned int)punteros[2] * 256 + (unsigned int)punteros[3];
		printf("INDEX: %u\n", offset);
		//memcpy(f, &punteros[0], 4);
		fseek(f, 2048*offset, SEEK_SET);
		fread(buffer1, 1, to_read, f); // READING DATABLOCK
		printf("LEST DATA: %s\n", buffer1);
	}
	free(punteros);
	free(buffer1);
	fclose(f);
	return nbytes;
}

/*
Funcio ́n para escribir archivos. Escribe en el archivo descrito por file_desc los nbytes que se encuentren
en la direccio ́n indicada por buffer. Retorna la cantidad de Byte escritos en el archivo. Si se produjo un error
porque no pudo seguir escribiendo, ya sea porque el disco se lleno ́ o porque el archivo no puede crecer mas,
este nu ́mero puede ser menor a nbytes (incluso 0).*/

int cr_write(crFILE* file_desc, void* buffer, int nbytes){
	return 0;
}

/*
Funcio ́n para cerrar archivos. Cierra el archivo indicado por file desc. Debe garantizar que cuando esta funcion
retorna, el archivo se encuentra actualizado en disco.*/

int cr_close(crFILE* file_desc){
	return 0;
}

/*
Funcion para borrar archivos. Elimina el archivo referenciado por la ruta path del directorio correspondiente.
Los bloques que estaban siendo usados por el archivo deben quedar libres si, y solo si, la cantidad de hardlinks
restante es igual a cero.*/

int cr_rm(char* path){
	return 0;
}

/*
Funcio ́n que se encarga de crear un hardlink del archivo referenciado por orig en una nueva ruta dest,
aumentando la cantidad de referencias al archivo original.*/

int cr_hardlink(char* orig, char* dest){
	return 0;
}

/*
Funcio ́nqueseencargadecopiarunarchivoouna ́rbol de directorios (es decir, un directorio y todos sus contenidos)
del disco, referenciado por orig, hacia un nuevo archivo o directorio de ruta dest en su computador.*/

int cr_unload(char* orig, char* dest){
	return 0;
}

/*
Funcion que se encarga de copiar un archivo o arbol de directorios, referenciado por orig al disco. En caso de que
un archivo sea demasiado pesado para el disco, se debe escribir todo lo posible hasta acabar el espacio
disponible.*/

int cr_load(char* orig){
	return 0;
}
