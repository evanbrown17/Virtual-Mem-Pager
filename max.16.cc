// max.16.cc - fills up a large number of pages

#include "vm_app.h"

int main() {
    char* p;
       for (int i = 0; i < 32; i++) { 
    	 p = (char*) vm_extend(); // p is an address in the arena

	p[0] = 'h';
    	p[1] = 'e';
    	p[2] = 'l';
    	p[3] = 'l';
    	p[4] = 'o';
	}
    vm_syslog(p, 5); // pager logs "hello"
    return 0;
}

