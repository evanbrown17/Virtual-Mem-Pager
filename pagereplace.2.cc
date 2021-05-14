// pagereplace.2.cc - Forces page replacement

#include "vm_app.h"

int main() {
    char* p;
    char* q;
    char* r;
    char* s;
    p = (char*) vm_extend(); // p is an address in the arena
    q = (char*) vm_extend();
    r = (char*) vm_extend();
    s = (char*) vm_extend();
    p[0] = 'h';
    p[1] = 'e';
    p[2] = 'l';
    p[3] = 'l';
    p[4] = 'o';
    vm_syslog(p, 5); // pager logs "hello"

    vm_yield();

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
    r[6] = 'e';
    vm_syslog(r, 7);

    s[0] = 'p';
    s[1] = 'a';
    s[2] = 'g';
    s[3] = 'e';
    s[4] = 'r';
    s[5] = 's';
    s[6] = '!';
    vm_syslog(s, 7);
    return 0;
}

