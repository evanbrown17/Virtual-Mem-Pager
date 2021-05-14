// syslogstraddlepages.6.cc - calls a syslog that straddles two different pages

#include "vm_app.h"

int main() {
    char* p;
    char* q;
    char* r;
    p = (char*) vm_extend(); // p is an address in the arena
    q = (char*) vm_extend();
    r = (char*) vm_extend();
    int i = 0;
    while (i < 8192) {
	    p[i] = 'X';
	    i++;
    }

    vm_syslog(p, 5); // pager logs "hello"

    vm_yield();
    i = 0;
    while (i < 8192) {
	    q[i] = 'Y';
	    i++;
    }
    vm_syslog(q, 3);
    i = 0;
    while (i < 8192) {
	    r[i] = 'Z';
	    i++;
    }
    vm_syslog(r, 7);

    char c = p[0];
    c++;

    vm_syslog(p + 8190, 6);
    return 0;
}

