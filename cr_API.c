// Librerias Std:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <math.h>

// Librerias propias
#include "cr_API.h"

unsigned int offset;
crFILE puntero;

int move_index(char* path, crFILE* p){

	// crFILE fileStruct;

	FILE * f = fopen(disk_path, "rb");

	p->offset = 0;
	p->block = 0;

	char * folder = malloc(256*sizeof(char));
	int level = 0;

	folder = strtok(path, "/");

	unsigned char * buffer = malloc( sizeof(unsigned char) * 32 );
	while(folder){

		//review 
		
		unsigned int salto_a_bloque = p->block * 2048;

		for(int i = 0; i < 64; i++ ) {
			fseek(f, salto_a_bloque + 32 * i, SEEK_SET);
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
					strcpy(p->file_name, folder_name);
					p->offset = p->offset + 28;
					
					//todo revisar esto.
					if (buffer[0] == (unsigned char)2){
						p->dir = 1;
					} else {
						p->dir = 0;
					}
				
					char block_number_str[4];
					memcpy(block_number_str, &buffer[28], 4);
					p->block = (unsigned int)atoi(block_number_str);
					p->entry = i;
					folder = strtok(NULL, "/");
					level++;
					break;
				}
			}
			if (i == 63) {
				free(buffer);
				fclose(f);
				p->exists = 0;
				return 0;
			}
		}
	}
	free(buffer);
	fclose(f);
	p->exists = 1;


	printf("\nApuntando al bloque: %d\n", p->block);

	return 1;
}


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


blockIndex* find_empty_block(){
	int encontrado = 0;
	unsigned int block_num = 0;
	unsigned char * buffer = malloc( sizeof( unsigned char ) * 2048*4);
	unsigned char mask = 1;
	blockIndex* block = malloc(sizeof(blockIndex));

	FILE* file = fopen(disk_path, "rb");
	fseek(file, 2048, SEEK_SET);
	fread(buffer, 1, 2048*4, file);

	unsigned char * byte = malloc(sizeof(char) * 8);
	for (int k = 0; k < 2048*4; k++) {
		for(int j = 0; j<8; j++){

			block_num++;
			byte[j] = (buffer[k] & (mask << j)) != 0;

			if (byte[j] == '0') {

				encontrado = 1;
				block->block_number = block_num;
				block->byte_number = k;
				block->bit_number = j;

				memcpy(block->new_byte, byte, 8);
				// strcpy(block->new_byte, byte);
				block->new_byte[j] = '1';
				break;
			}
		}
		if (encontrado){
			break;
		}
	}
	free(byte);
	free(buffer);
	fclose(file);
	return block;
}


void change_bitmap(blockIndex* block){

	unsigned char old_byte[8];
	unsigned char new_byte[8];

	unsigned int offset;

	FILE* file = fopen(disk_path, "w+b");
	offset = 2048 * (block->block_number + 1) + block->byte_number;

	fseek(file, offset , SEEK_SET);
	fwrite(block->new_byte, 1, 1, file);

	fclose(file);

}


unsigned char * itoa(unsigned int value, unsigned char *result, int base){
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	unsigned char* ptr = result, *ptr1 = result, tmp_char;
	unsigned int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while (ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}


/*
Funcio ́n para montar el disco.
Establece como variable global la ruta local donde se encuentra el archivo .bin correspondiente al disco. */
void cr_mount(char* diskname){
	memset(disk_path, '\0', sizeof(disk_path));
	strcpy(disk_path, diskname);

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

	FILE * f = fopen(disk_path, "rb");
	fseek(f, 2048, SEEK_SET);
	fread(buffer, 1, 2048*4, f);
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

// FIXME, creo que no funciona bien, testearla.
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

	char* path_to_dir = dirfinder(foldername);
	char* new_dir = basefinder(foldername);

	blockIndex* new_block = find_empty_block();
	// move_index(path_to_dir, &puntero);

	FILE * f = fopen(disk_path, "rb");
	unsigned char * buffer = malloc( sizeof( unsigned char ) * 32 );

	// Review, cr_exists también llama a move_index, por lo que no es necesario llamar a move_index antes.
	int existe = cr_exists(foldername);
	
	if (existe == 1){
		printf("El directorio %s ya existe en %s", new_dir, path_to_dir);
		free(buffer);
		fclose(f);
		return 0;
	}

	for( int j = 0; j < 64; j++){

		fseek(f, 32 * j, SEEK_SET);
		fread(buffer, sizeof( unsigned char ), 32, f);

		if ( buffer[0] != (unsigned char)2 && buffer[0] != (unsigned char)4) {
			printf( "\nCreando directorio %s en %s\n", new_dir, path_to_dir);
			fclose(f);

			FILE * file = fopen(disk_path, "ab");

			// puntero es solo el numero de bloque
			unsigned int int_pointer = new_block->block_number;
			unsigned char aux_pointer[4];
			unsigned char* pointer = itoa(int_pointer, aux_pointer, 10);
			// char * ceros = '00';
			
			buffer[0] = '2';


			// review guardar el puntero como unsigned int, germy no sabe si fuciona como char
			memcpy(&buffer[1], new_dir, 27);
			
			// Se supone que los strings terminan en cero, asi el compilador los indentifica.
			buffer[27] = 0;
			
			memcpy(&buffer[28], pointer, 4);
			// memcpy(&buffer[28], ceros, 2);
			// memcpy(&buffer[30], pointer, 2);
			
			fseek(file, 32 * j, SEEK_SET);
			fwrite(buffer, 1, 32, file);

			change_bitmap(new_block);
			return 1;
		}
	}
	printf( "\nNo hay espacio suficiente en el bloque\n");
	free(buffer);
	fclose(f);
	return 0;
}

/*
Funcion para abrir un archivo.
Si mode es ‘r’, busca el archivo en la ruta path y retorna un crFILE* que lo representa.
*/

crFILE * cr_open(char * path, char mode){
	crFILE * open_file = malloc(sizeof(crFILE));
	int existe = move_index(path, open_file);

	if (existe != 0 && mode == 'r'){
		return open_file;
	}
	else if (existe == 0 && mode == 'w'){
		crFILE * nuevo_archivo = malloc(sizeof(crFILE));
		//int num = find_empty_block();
		//change_bitmap_value(num);
		int num = 1200;

		unsigned char * archivo_indice = malloc(32*sizeof(unsigned char));

		unsigned char nombre[28] = "New Germy.txt";
		unsigned char numero[] = "4";
		unsigned char new_block[4] = {(unsigned char) 0, (unsigned char) 0, (unsigned char) ((num)>>8) & 0xFF, (unsigned char) (num) & 0xFF};

		memcpy(&archivo_indice[0], numero, 1);
		memcpy(&archivo_indice[1], nombre, 27);
		memcpy(&archivo_indice[27], new_block, 4);

		for (size_t i = 0; i < 32; i++) {
			printf("archivo_indice[%zu]=%c\n",i, archivo_indice[i]);
		}

		printf("NUEVO NOMBRE: %s\n", archivo_indice);

		//Crear nuevo archivo

		//Cambiar bit del bloque en bitmap

		return open_file;
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
	// review, "rb"? 
	FILE * f = fopen(disk_path, "r");

	// FINDING INDEX block
	unsigned char index_block[4];
	fseek(f, file_desc->offset, SEEK_SET);
	fread(index_block, 1, 4, f);
	int index_block_num = (unsigned int)index_block[2] * 256 + (unsigned int)index_block[3];

	// FINDING FILE SIZE
	unsigned char * size = malloc(4*sizeof(unsigned char));
	fseek(f, 2048*index_block_num, SEEK_SET);
	fread(size, 1, 4, f);
	int file_size = (int)size[0] * 16777216 + (int)size[1] * 65536 + (int)size[2] * 256 + (int)size[3];

	// FINDING NUMBER OF HARDLINKS
	unsigned char hardlinks[4];
	fseek(f, 2048*index_block_num + 4, SEEK_SET);
	fread(hardlinks, 1, 4, f);
	int num_hardlinks = (int)hardlinks[0]*16777216 + (int)hardlinks[1] * 65536 + (int)hardlinks[2] * 256 + (int)hardlinks[3];

	// HOW MANY BYTES TO READ
	if (nbytes > file_size) {
		nbytes = file_size;
	}

	int num_blocks =  ceil(nbytes/2048.0);
	unsigned char * punteros = malloc(4*num_blocks*sizeof(unsigned char));
	unsigned char * buffer1 = malloc(2048*sizeof(unsigned char));
	int to_read = 2048;

	for(int i = 0;i<num_blocks;i++){
		if (i == num_blocks - 1){
			to_read = nbytes - 2048*i;
		}
		fseek(f, index_block_num*2048 + 8 + i*4, SEEK_SET);
		fread(punteros, 1, 4, f);
		int offset = (int)punteros[2] * 256 + (int)punteros[3];

		fseek(f, 2048*offset, SEEK_SET);
		fread(buffer1, 1, to_read, f);
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

	FILE * f = fopen(disk_path, "r+b");

	// FINDING INDEX BLOCK
	unsigned char index_block[4];
	fseek(f, file_desc->offset, SEEK_SET);
	fread(index_block, 1, 4, f);
	int index_block_num = (unsigned int)index_block[2] * 256 + (unsigned int)index_block[3];
	printf("INDEX BLOCK: %d\n", index_block_num);

	// FINDING FILE SIZE
	unsigned char * size = malloc(4*sizeof(unsigned char));
	fseek(f, 2048*index_block_num, SEEK_SET);
	fread(size, 1, 4, f);
	int file_size = (int)size[0] * 16777216 + (int)size[1] * 65536 + (int)size[2] * 256 + (int)size[3] ;
	printf("FILE SIZE: %d\n", file_size);

	// HOW MANY BYTES TO WRITE IN TOTAL
	if (nbytes > 500*2048) {
		nbytes = 500*2048;
	}

	// WHERE TO START AND HOW MANY BLOCKS TO WRITE TO
	int blocks_used = ceil(file_size/2048.0);
	int num_blocks = ceil(nbytes/2048.0);


	// agregar x bloques de datos al bloque indice
	int i;
	if(num_blocks>blocks_used){
		for (i = 0; i<(num_blocks-blocks_used);i++){
			blockIndex * block_index = find_empty_block();
			fseek(f, 2048*index_block_num + 8 + (i+num_blocks)*4, SEEK_SET);
			char new_block[4] = {(block_index->block_number>>24) & 0xFF,(block_index->block_number>>16) & 0xFF,(block_index->block_number>>8) & 0xFF, (block_index->block_number) & 0xFF};
			fwrite(new_block, 1, 4, f);
		}
	}

	//Para cada bloque - escribe datos desde el buffer

	unsigned char * punteros = malloc(4*sizeof(unsigned char));
	int to_write = 2048;
	for (i = 0; i < num_blocks; i++) {
		// Encontrar bloque:
		fseek(f, 2048*index_block_num + 8 + i*4, SEEK_SET);
		fwrite(punteros, 1, 4, f); // READING PUNTERO
		int offset = (int)punteros[2] * 256 + (int)punteros[3];
		printf("INDEX: %d\n", offset);

		if(i +1 == num_blocks){to_write = nbytes - i*2048;}

		// Escribir datos al bloque
		fseek(f, 2048*offset, SEEK_SET);
		fwrite(&buffer[i*2048], 1,to_write, f);
	}

	// Actualiza tamaño del archivo

	fseek(f, index_block_num*2048 + 2, SEEK_SET);
	char new_size[2] = {(nbytes>>8) & 0xFF, (nbytes) & 0xFF};
	fwrite(new_size, 1, 2, f);
	fclose(f);

	free(size);
	free(punteros);
	return nbytes;
}

/*
Función para cerrar archivos. Cierra el archivo indicado por file desc. Debe garantizar que cuando esta funcion
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

	// REVIEW supuesto, char* orig es un archivo y char* dest un directorio.

	int existe = move_index(dest, &puntero);
	if (existe == 1){
		printf("Ya existe un hardlink con este nombre en este directorio\n");
		return 0;
	}

	FILE* file = fopen(disk_path, "r+b");
	char * buffer = malloc(sizeof(char) * 32);
	char * dir = dirfinder(orig);
	char * filename = basefinder(orig);

	// Voy al dir del hardlink y veo si puedo crearlo, move_index ya setteo el puntero.
	for(int i = 0; i < 64; i++){
		unsigned int dir_pointer = puntero.block;
		fseek(file, dir_pointer * 2048 + i * 32, SEEK_SET);
		fread(buffer, sizeof( unsigned char ), 32, file);

		if (buffer[0] != (unsigned char)2 || buffer[0] != (unsigned char)4){
			// Si no es ni archivo ni directorio, es una entrada libre.
			memcpy(&buffer[0], "4", 1);
			memcpy(&buffer[1], filename, 27);

			// Reviso el archivo al que le crearé el hardlink.
			// nuevamente move_index setea al puntero
			int archivo = move_index(orig, &puntero);

			if (archivo == 1){
				char * buffer_archivo = malloc(sizeof(char) * 4);
				// unsigned int de 4 bytes para la cantidad de hardlinks

				unsigned int int_pointer = puntero.block;
				unsigned char aux_pointer[4];
				unsigned char * bloque_archivo = itoa(int_pointer, aux_pointer, 10);
				memcpy(&buffer[27], bloque_archivo, 4);

				fseek(file, puntero.block * 2048 + 4, SEEK_SET);
				fread(buffer_archivo, 1, 4 , file);
				unsigned int hl_counter = (unsigned int)atoi(buffer_archivo);
				hl_counter++;
				// review Cuando escribo, debo escribir el numero como int o char?
				fwrite(hl_counter, 1, 4,file);
				free(buffer_archivo);
				free(buffer);
				fclose(file);
				return 1;

			} else {
				printf("\nNo existe el archivo referenciado\n");
				free(buffer);
				fclose(file);
				return 0;
			}
		}
	}
	free(buffer);
	fclose(file);
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
