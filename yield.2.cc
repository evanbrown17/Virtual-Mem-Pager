// yield.2.cc-calls vm_yield throughout the process of dealing with faults
// to expose any bugs related to process switching

#include "vm_app.h"

int main() {
    char* p;
    char* q;
    char* r;
    p = (char*) vm_extend(); // p is an address in the arena
    char x = p[0];
    vm_syslog(p, 8192);
    x++;
    q = (char*) vm_extend();
    vm_yield();
    r = (char*) vm_extend(); //p's page is evicted
    p[0] = 'h';
    p[1] = 'e';
    p[2] = 'l';
    p[3] = 'l';
    p[4] = 'o';
    vm_syslog(p, 5); // pager logs "hello"

    vm_yield();

    q[0] = 'a';
    vm_yield();
    q[1] = 'n';
    q[2] = 'd';
    vm_yield();
    vm_syslog(q, 3);
    
    vm_yield();
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

    vm_yield();
    p[8190] = 'X';
    p[8191] = 'Y';
    vm_syslog(p + 8190, 5); //should log 'XYand' and cause a fault after XY
    return 0;
}

