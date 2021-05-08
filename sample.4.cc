// sample.cc - a sample application program that uses the external pager

#include "vm_app.h"

int main() {
    char* p;
    char* t;
    char* s;
    p = (char*) vm_extend(); // p is an address in the arena
    t = (char*) vm_extend();
    s = (char*) vm_extend();
    p[0] = 'h';
    p[1] = 'e';
    p[2] = 'l';
    p[3] = 'l';
    p[4] = 'o';
    t[0] = 'B';
    s[0] = 'X';
    vm_syslog(p, 5); // pager logs "hello"
    return 0;
}

