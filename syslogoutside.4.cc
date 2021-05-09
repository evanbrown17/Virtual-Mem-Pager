// sample.cc - a sample application program that uses the external pager

#include "vm_app.h"

int main() {
    char* p;
    p = (char*) vm_extend(); // p is an address in the arena
    p[8190] = 'h';
    p[8191] = 'e';
    p[8192] = 'l';
    p[8193] = 'l';
    p[8194] = 'o';
    vm_syslog(p + 8190, 5); // pager logs "hello"
    return 0;
}

