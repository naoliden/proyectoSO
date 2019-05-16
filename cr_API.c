// Librerias Std:
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>
#include <math.h>

// Librerias propias en orden de prioridad
#include "cr_API.h"
#include "library.h"


unsigned int offset;
crFILE puntero;

// review REVISAR 
//  
int move_index(char* path, crFILE* p){
	

	FILE * f = fopen(disk_path, "rb");

	p->offset = 0;

	char * folder = malloc(256*sizeof(char));
	folder = strtok(path, "/");

	unsigned char * buffer = malloc( sizeof(unsigned char) * 32 );
	FILE * f = fopen(disk_path, "rb");
	while(folder){
		for(int i = 0; i < 64; i++ ) {
			fseek(f, 32 * i, SEEK_SET );
			p->offset = 32*i;
			fread(buffer, sizeof( unsigned char ), 32, f);
			char folder_name[27];
			memcpy(folder_name, &buffer[1], 26);
			if (buffer[0] == (unsigned char)1 ){
				printf("Path invalido");
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
			// printf("%d", bits[j]);
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

// FIXME                               
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

	//fixme verificar que no exista el directorio antes de hacer todo        
	

	// Path hasta antes de la carpeta a crear
	char* path_to_dir = dirfinder(foldername);
	// Nombre de la carpeta para crear
	char* new_dir = basefinder(foldername);

	blockIndex* new_block = find_empty_block();
	move_index(path_to_dir, &puntero);

	FILE * f = fopen(disk_path, "rw");
	unsigned char * buffer = malloc( sizeof( unsigned char ) * 32 );
	
	int existe = cr_exists(foldername);
	if (existe == 0){
		printf("El directorio %s ya existe", new_dir);
		free(buffer);
		fclose(f);
		return 0;
	}

	for( int j = 0; j < 64; j++){
		fseek(f, 32 * j, SEEK_SET);
		fread(buffer, sizeof( unsigned char ), 32, f);
		if ( buffer[0] == (unsigned char)1 || buffer[0] == (unsigned char)4) {
			printf( "\nCreando directorio\n");

			//todo escribir que hay directorio en directorio padre


			change_bitmap(new_block);
			free(buffer);
			fclose(f);
			break;
		}
	}
	printf( "\nNo hay espacio suficiente en el bloque\n");
	free(buffer);
	fclose(f);



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
	else if (mode == 'w'){
		crFILE * nuevo_archivo = malloc(sizeof(crFILE));
		/*
		Crear nuevo archivo
		Cambiar bit del bloque en bitmap
		*/
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
	FILE * f = fopen(disk_path, "r");

	// FINDING INDEX block
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

	// FINDING NUMBER OF HARDLINKS
	unsigned char hardlinks[4];
	fseek(f, 2048*index_block_num + 4, SEEK_SET);
	fread(hardlinks, 1, 4, f);
	int num_hardlinks = (int)hardlinks[0]*16777216 + (int)hardlinks[1] * 65536 + (int)hardlinks[2] * 256 + (int)hardlinks[3];
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
		fseek(f, index_block_num*2048 + 8 + i*4, SEEK_SET);
		fread(punteros, 1, 4, f); // READING PUNTERO
		int offset = (int)punteros[2] * 256 + (int)punteros[3];
		printf("INDEX: %d\n", offset);
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
	FILE * f = fopen(disk_path, "r");

	// FINDING INDEX block
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
	if (nbytes > (500*2048 - file_size)) {
		nbytes = (500*2048 - file_size);
	}

	// WHERE TO START AND HOW MANY BLOCKS TO WRITE TO
	int block = ceil(file_size/2048.0);
	int index = file_size - 2048*(block-1);
	int num_blocks = ceil(nbytes/2048.0);
	printf("START WRITING BLOCK: %d\nSTART WRITING INDEX: %d\nNUM BLOCKS: %d\n", block, index, num_blocks);

	unsigned char * punteros = malloc(4*num_blocks*sizeof(unsigned char));
	int to_read = 2048;
	for(int i = (block-1);i<num_blocks;i++){
		if (i == num_blocks	 - 1){ to_read = nbytes - 2048*i;}

		if(i == (block-1)){
			// FINDING DATA BLOCK TO WRITE TO
			fseek(f, index_block_num*2048 + 8 + 4*i, SEEK_SET);
			fread(punteros, 1, 4, f); // READING PUNTERO
			fclose(f);
			int offset = (int)punteros[2] * 256 + (int)punteros[3];
			// GO TO BLOCK AND RIGHT INDEX


			FILE * wfile = fopen(disk_path, "r+b");
			fseek(wfile, offset*2048 + index, SEEK_SET);
			int wr = fwrite("HOLA GERMY!", (size_t) 1, (size_t) nbytes, wfile);
			printf("WROTE %d\n", wr);

		}
		else{
			// FIND AVAILABLE BLOCK TO WRITE TO
			// SET BITMAP TO ONE.
			// PUT POINTER TO BLOCK IN index
			// GO TO BLOCK
			// WRITE DATA TO BLOCK
		}

	}

	// CHANGE FILE SIZE OF ARCHIVE
	fseek(wfile, index_block_num*2048 + 2, SEEK_SET);
	char new_size[2] = {((file_size+nbytes)>>8) & 0xFF, (file_size+nbytes) & 0xFF};
	fwrite(new_size, 1, 2, wfile);
	fclose(wfile);

	return nbytes;
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
