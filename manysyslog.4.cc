// sample.cc - a sample application program that uses the external pager

#include "vm_app.h"
#include <iostream>

using namespace std;

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
    r[6] = 'e';
    vm_syslog(r, 7);

    char c = q[0]; // forces a disk read because this one gets evicted
    c++;

    vm_syslog(p, 19920);

    char* s = (char*) vm_extend();
    int i = 0;
    while(i < 500) {
	    s[i] = 'x';
            i++;
    }

    vm_syslog(s, 10);
    vm_syslog(s + 200, 20);
    vm_syslog(p, 24600);

    p[5] = ' ';
    p[6] = 'p';
    p[7] = 'a';
    p[8] = 'g';
    p[9] = 'e';
    p[10] = 'r';
    p[11] = 's';
    p[12] = '!';
    vm_syslog(p, 100);
    vm_syslog(p, 20);

    vm_syslog(q, 16400);
    vm_syslog(p, 24600);
    return 0;
}

