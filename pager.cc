// pager.cc - implementation of a memory pager
// Evan Brown and Braden Fisher
// May 2, 2021

#include "vm_pager.h"
#include <queue>
#include <iostream>

using namespace std;

struct Process {
	pid_t pid;
	page_table_t* process_ptbr;
	page_table_t process_pgtable;
};

queue<Process*> all_processes;
Process* curr_process;

/*
 * vm_init
 *
 * Initializes the pager and any associated data structures. Called automatically
 * on pager startup. Passed the number of physical memory pages and the number of
 * disk blocks in the raw disk.
 */
void vm_init(unsigned memory_pages, unsigned disk_blocks) {
  
}

/*
 * vm_create
 *
 * Notifies the pager that a new process with the given process ID has been created.
 * The new process will only run when it's switched to via vm_switch.
 */
void vm_create(pid_t pid) {
  
	Process* new_process = new Process;
	new_process->pid = pid;
	cerr << "New process pid is " << new_process->pid << endl;



}

/*
 * vm_switch
 *
 * Notifies the pager that the kernel is switching to a new process with the
 * given pid.
 */
void vm_switch(pid_t pid) {
  // TODO
}

/*
 * vm_fault
 *
 * Handle a fault that occurred at the given virtual address. The write flag
 * is 1 if the faulting access was a write or 0 if the faulting access was a
 * read. Returns -1 if the faulting address corresponds to an invalid page
 * or 0 otherwise (having handled the fault appropriately).
 */
int vm_fault(void* addr, bool write_flag) {
  // TODO
  return -1;
}

/*
 * vm_destroy
 *
 * Notifies the pager that the current process has exited and should be
 * deallocated.
 */
void vm_destroy() {
  // TODO
}

/*
 * vm_extend
 *
 * Declare as valid the lowest invalid virtual page in the current process's
 * arena. Returns the lowest-numbered byte of the newly valid virtual page.
 * For example, if the valid part of the arena before calling vm_extend is
 * 0x60000000-0x60003FFF, vm_extend will return 0x60004000 and the resulting
 * valid part of the arena will be 0x60000000-0x60005FFF. The newly-allocated
 * page is allocated a disk block in swap space and should present a zero-filled
 * view to the application. Returns null if the new page cannot be allocated.
 */
void* vm_extend() {
  // TODO
  return nullptr;
}

/*
 * vm_syslog
 *
 * Log (i.e., print) a message in the arena at the given address with the
 * given nonzero length. Returns 0 on success or -1 if the specified message
 * address or length is invalid.
 */
int vm_syslog(void* message, unsigned len) {
  // TODO
  return -1;
}

