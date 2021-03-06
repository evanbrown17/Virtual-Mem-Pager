// syslogstraddlepages.2.cc - calls a syslog that straddles two different pages after triggering a fault

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

    //bring p's page back in memory
    char c = p[0];
    c++;

    p[8190] = 'X';
    p[8191] = 'Y';
    vm_syslog(p + 8190, 5); //should log 'XYand' and cause a fault after XY
    return 0;
}

