#include "cr_API.h"

#include "library.h"

int main(int argc, char *argv[]) {
  if (argc == 2){
    cr_mount(argv[1]);
  }
  else{
    cr_mount("./simdiskfilled.bin");
  }
  cr_ls("");
  cr_mkdir("HALLO");

  /*
  void * write = malloc(2000 * sizeof(unsigned int));
  char str[] = "HOLA GERMY!";
  strcpy(write, str);
  crFILE * germy = cr_open("germy.txt", 'r');
  cr_write(germy, write, sizeof(str));
  void * output = malloc(400 * sizeof(unsigned int));
  cr_read(germy, output, 400);
  free(output);
  */
  return 0;
}
