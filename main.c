#include "cr_API.h"


int main(int argc, char *argv[]) {
  if (argc == 2){
    cr_mount(argv[1]);
  }
  else{
    cr_mount("./simdiskfilled.bin");
  }
  cr_rm("germy.txt");

/*
void * write = malloc(2000 * sizeof(unsigned int));
char str[] = "HOLA GERMY!";
strcpy(write, str);

crFILE * germy = cr_open("germy.txt", 'r');
printf("OPEN OK\n");
cr_write(germy, write, sizeof(str));
printf("WRITE OK\n");
void * output = malloc(400 * sizeof(unsigned int));
cr_read(germy, output, 400);
printf("READ OK\n");
free(output);
*/


  return 0;
}
