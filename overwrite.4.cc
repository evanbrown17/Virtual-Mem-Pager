// sample.cc - a sample application program that uses the external pager

#include "vm_app.h"

int main() {
    char* p;
    p = (char*) vm_extend(); // p is an address in the arena
    p[0] = 'h';
    p[1] = 'e';
    p[2] = 'l';
    p[3] = 'l';
    p[4] = 'o';
    vm_syslog(p, 5); // pager logs "hello"

    p[0] = 'g';
    p[1] = 'o';
    p[2] = 'o';
    p[3] = 'd';
    p[4] = 'b';
    p[5] = 'y';
    p[6] = 'e';
    vm_syslog(p,7); // pager logs "goodbye"
    return 0;
}

