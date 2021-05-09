// sample.cc - a sample application program that uses the external pager

#include "vm_app.h"

int main() {
    char* p;
    char* q;
    char* r;
    p = (char*) vm_extend(); // p is an address in the arena
    q = (char*) vm_extend();
    r = (char*) vm_extend();
    p[0] = 'h';
    p[1] = 'e';
    p[2] = 'l';
    p[3] = 'l';
    p[4] = 'o';
    vm_syslog(p, 5); // pager logs "hello"

    q[0] = 'a';
    q[1] = 'n';
    q[2] = 'd';
    vm_syslog(q, 3);

    r[0] = 'g';
    r[1] = 'o';
    r[2] = 'o';
    r[3] = 'd';
    r[4] = 'b';
    r[5] = 'y';
    r[5] = 'e';
    vm_syslog(r, 7);

    p[0] = 'h';
    p[1] = 'i';
    p[2] = ' ';
    p[3] = 'a';
    p[4] = 'g';
    p[5] = 'a';
    p[6] = 'i';
    p[7] = 'n';
    vm_syslog(p, 8); 
    return 0;
}

