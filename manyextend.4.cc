// sample.cc - a sample application program that uses the external pager

#include "vm_app.h"

int main() {
    char* p;
    char* q;
    char* r;
    p = (char*) vm_extend(); // p is an address in the arena
    q = (char*) vm_extend();
    r = (char*) vm_extend();


    char a = p[0];
    a = p[5];
    a++;

    char b = q[0]; 
    b = q[200];
    b++;

    char c = r[0]; 
    c = r[8191];
    c++;


    p[0] = 'h';
    p[1] = 'e';
    p[2] = 'l';
    p[3] = 'l';
    p[4] = 'o';
    vm_syslog(p, 5); // pager logs "hello"

    c = r[8000];

    p[0] = 'p';
    p[1] = 'a';
    p[2] = 'g';
    p[3] = 'e';
    p[4] = 'r';
    vm_syslog(p, 5);

    char* z = (char*) vm_extend();

    char y = z[8];
    y = z[9];
    y++;

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

    vm_syslog(p, 5);

    p[0] = 'h';
    p[1] = 'i';
    p[2] = ' ';
    p[3] = 'a';
    p[4] = 'g';
    p[5] = 'a';
    p[6] = 'i';
    p[7] = 'n';
    vm_syslog(p, 8);

    int* k = (int*) vm_extend();
    k[0] = 9;
    vm_syslog(k, 4);
    return 0;
}

