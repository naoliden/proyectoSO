#include "cr_API.h"

int main() {
    cr_mount("./simdiskfilled.bin");
    //cr_mkdir("HALLO.txt");
    //cr_ls("memes");
    //cr_bitmap();
    void * output = malloc(2000 * sizeof(unsigned int));
    crFILE* germy = cr_open("memes/cmake.png", 'r');
    cr_read(germy, output, 4000);
    free(output);

    return 0;
}
