// negarryaccess.2.cc - Attempts to put things in unassigned parts of the memory array

#include "vm_app.h"

int main() {
    char* p;
    p = (char*) vm_extend(); // p is an address in the arena
    p[-1] = 'h';
    p[10000] = 'e';
    p[2] = 'l';
    p[3] = 'l';
    p[4] = 'o';
    vm_syslog(p, 5); // pager logs "hello"
    return 0;
}

