#include "cr_API.h"

int main() {
    cr_mount("./simdiskfilled.bin");
    //cr_mkdir("HALLO.txt");
    cr_ls("");
    //cr_bitmap();
    void * output = malloc(2000 * sizeof(unsigned int));
    crFILE* germy = cr_open("germy.txt", 'r');
    cr_read(germy, output, 4000);
    free(output);
    return 0;
}
