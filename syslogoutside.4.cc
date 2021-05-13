// sample.cc - a sample application program that uses the external pager

#include "vm_app.h"

int main() {
    char* p;
    p = (char*) 0x5fffffff;
    vm_syslog(p, 6);
    p = (char*) vm_extend(); // p is an address in the arena
    p[8190] = 'h';
    p[8191] = 'e';
   // p[8192] = 'l';
    //p[8193] = 'l';
    //p[8194] = 'o';
    vm_syslog(p - 1, 8191);
    vm_syslog(p + 8190, 5); // invalid length

    char* q = (char*) vm_extend();
    p[8192] = 'l';
    p[8193] = 'l';
    p[8194] = 'o';
    vm_syslog(p + 8190, 5);

    q[0] = 'x';
    return 0;
}

