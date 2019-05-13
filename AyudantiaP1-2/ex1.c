#include <stdio.h>
#include <stdlib.h>

int main(){
  /* Haremos uso de fseek para ver la cantidad de bytes de un archivo y
     leerlo */
  FILE* f = fopen("ex1.txt", "rb"); // Lectura binaria.
  fseek(f, 0, SEEK_END); // Ponemos el puntero al final.
  int size_f = ftell(f); // Retorna la posición del puntero.
  fseek(f, 0, SEEK_SET); // Lo devolvemos al inicio.
  unsigned char buffer[size_f];          // Buffer que almacenará los bytes de f.
  fread(buffer, 1, size_f, f); // Leemos los bytes y los almacenamos en el
                               // buffer.
  fclose(f); // Cerramos el archivo.

  /* Haremos lo mismo, pero para escribir un nuevo archivo. */
  FILE* f_new = fopen("ex1_same.txt", "wb"); // Escritura binaria.
  fwrite(buffer, 1, size_f, f_new);  // Escribimos la misma cantidad de bytes
                                     // desde el buffer.
  fclose(f_new); // Cerramos el archivo.
  return 0;
}
