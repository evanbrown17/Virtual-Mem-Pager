// sample.cc - a sample application program that uses the external pager

#include "vm_app.h"
#include <iostream>

using namespace std;

int main() {
    char* p;
    int i = 0;
    while (i < 7) {
	p = (char*) vm_extend(); // p is an address in the arena
	i++;
    }
    p[0] = 'h';
    p[1] = 'e';
    p[2] = 'l';
    p[3] = 'l';
    p[4] = 'o';
    vm_syslog(p, 5); // pager logs "hello"

    //cout << p[5] << endl; //should be zero
    return 0;
}

