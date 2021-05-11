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

    char c = p[0]; // forces a disk read because this one gets evicted
    c++; //to avoid the error of not using c


    char b = q[1]; //evicts the page corresponding to r
    b++;

    char d = r[6]; //evicts pages corresponding to c but make sure it is not written to disk because it is not dirty
    d++;

    return 0;
}

