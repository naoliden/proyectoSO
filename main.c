#include "cr_API.h"


int main(int argc, char *argv[]) {
  if (argc == 2){
    cr_mount(argv[1]);
  }
  else{
    cr_mount("./simdiskfilled.bin");
  }
  cr_ls("");
  printf("\n\n");
  crFILE* new = cr_open("tralala.txt", 'w');
  void * write = malloc(2000 * sizeof(unsigned int));
  char str[] = "HOLA GERMY!";
  strcpy(write, str);
  cr_write(new, write, 100);
  void * read = malloc(2000 * sizeof(unsigned int));
  cr_read(new, read, 100);
  //printf("DATA READ: %s\n", read);
  //cr_load("/Users/annewold/proyectoSO/testo.txt");
  printf("\n\n");
  cr_ls("");
  free(write);
  free(read);


  return 0;
}
