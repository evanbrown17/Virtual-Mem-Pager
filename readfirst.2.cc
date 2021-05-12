// sample.cc - a sample application program that uses the external pager

#include "vm_app.h"

int main() {
    char* p;
    char* q;
    char* r;
    p = (char*) vm_extend(); // p is an address in the arena
    q = (char*) vm_extend();
    r = (char*) vm_extend();

    char c = p[0];
    c++;
    vm_syslog(p, 5);

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
    char d = p[0];
    d++;

    p[8190] = 'X';
    p[8191] = 'Y';
    vm_syslog(p + 8190, 5); //should log 'XYand' and cause a fault after XY
    return 0;
}

