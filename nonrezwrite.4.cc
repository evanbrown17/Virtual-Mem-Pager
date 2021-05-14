// nonrezwrite.4.cc - Invalid pointer to the memory array which we are writing to

#include "vm_app.h"

int main() {
    char* p;
    p = (char*) vm_extend(); // p is an address in the arena
    p += 10000;

    p[0] = 'h';
    p[1] = 'e';
    p[2] = 'l';
    p[3] = 'l';
    p[4] = 'o';
    vm_syslog(p, 5); // pager logs "hello"
    return 0;
}

