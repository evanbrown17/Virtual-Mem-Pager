// pager.cc - implementation of a memory pager
// Evan Brown and Braden Fisher
// May 2, 2021

#include "vm_pager.h"
#include <queue>
#include <iostream>
#include <list>
#include <map>
#include <deque>
#include <string>


using namespace std;

struct Page {
	int page_num;
	int ref_bit;
	int dirty_bit;
	unsigned frame; //physmem location (if any)
	unsigned block; //disk location
};


struct Process {
	pid_t pid;
	page_table_t* process_ptbr;
	page_table_t* process_pgtable;
	uintptr_t valid_floor;
	uintptr_t valid_ceiling;
	list<Page*> page_info;	
};


list<Process*> all_processes;
Process* curr_process;

list<Page*> all_pages

unsigned mem_pages;
unsigned frame_counter;
unsigned blocks;
unsigned block_counter;
//unsigned framesAssigned;
//
int * framesAssigned;

uintptr_t arena_base;

int total_pages;

/*
 * vm_init
 *
 * Initializes the pager and any associated data structures. Called automatically
 * on pager startup. Passed the number of physical memory pages and the number of
 * disk blocks in the raw disk.
 */
void vm_init(unsigned memory_pages, unsigned disk_blocks) {
	arena_base = VM_ARENA_BASEADDR;
	frame_counter = 0;
 	mem_pages = memory_pages;
	blocks = disk_blocks;
	block_counter = 0;
	total_pages = 0;
	framesAssigned = new int [mem_pages]; //says whether the frame is full or not
	for (unsigned i = 0; i < mem_pages; i++) {
		framesAssigned[i] = 0;	
		//all frames initially empty

	}
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
	new_process->valid_floor = VM_ARENA_BASEADDR;
	new_process->valid_ceiling = VM_ARENA_BASEADDR;

	new_process->process_pgtable = new page_table_t;
	new_process->process_ptbr = new_process->process_pgtable;
	new_process->process_pgtable->ptes = new page_table_entry_t;
	cerr << "New process pid is " << new_process->pid << endl;

	//Need to initialize values in actual page table?

	all_processes.push_back(new_process);



}

/*
 * vm_switch
 *
 * Notifies the pager that the kernel is switching to a new process with the
 * given pid.
*/
void vm_switch(pid_t pid) {
 	for (Process* p : all_processes) {
		if (p->pid == pid) {
			curr_process = p;
			page_table_base_register = curr_process->process_ptbr; //set PTBR to point to the current page table

		}	
	}
	cerr << "There is no process with the given pid: " << pid << endl;
		
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
		
	if ((curr_process->validceiling != (uintptr_t) VM_ARENA_BASEADDR + (uintptr_t)VM_ARENA_SIZE - 1) &&
			block_counter < blocks){ //check to make sure we haven't run out of arena or disk
		/*
		unsigned nextFrame = frame_counter;
		frame_counter++;

		//find lowest unused frame
		for (unsigned i = 0; i < mem_pages; i++) {
			if (framesAssigned[i] == 0) {
				nextFrame = i;
				break;
			}
		}
		
		//if no unused frame, cannot alloc		
		if (nextFrame > mem_pages) {
			return nullptr;
		}
		*/

		//I think we can just put it on the disk and not in memory when it is created?
	
		Page* newPage = new Page;
		newPage->page_num = VM_ARENA_BASEPAGE + total_pages;
		total_pages++;
		newPage->dirty_bit = 0;
		newPage->ref_bit = 0;
		newPage->frame = NULL;
		newPage->block = block_counter;
		
		all_pages.push_back(new_page)
		curr_process->pageInfo.push_back(newPage);

		block_counter++; //will probably need more complexity here to write over disk entries that are no longer needed

		pgTable_index = curr_process->pageInfo.size() - 1;
		
		curr_process->process_pgtable->ptes[pgTable_index] = new page_table_entry_t;
		curr_process->process_pgtable->ptes[pgTable_index]->ppage = NULL;
		curr_process->process_pgtable->ptes[pgTable_index]->read_enable = 0;
		curr_process->process_pgtable->ptes[pgTable_index]->write_enable = 0;

		uintptr_t return_value = VM_ARENA_BASEADDR + (VM_PAGESIZE * (curr_process->pageInfo.size() - 1)); 
		
		curr_process->validceiling = return_value; //update which virtual addresses are valid
		return (void*) curr_process->validceiling;

	} else {
		return nullptr;
	}
}


/*
 * vm_syslog
 *
 * Log (i.e., print) a message in the arena at the given address with the
 * given nonzero length. Returns 0 on success or -1 if the specified message
 * address or length is invalid.
 */
int vm_syslog(void* message, unsigned len) {

	//make sure the message address is in the arena
	if ((uintptr_t) message < VM_ARENA_BASEADDR || (uintptr_t) message > curr_process->validceiling) {
		return -1;
	}

	//make sure the message address plus the length is in the arena
	if ((uintptr_t) message + len > curr_process->validceiling || len == 0) {
		return - 1;
	}

	string str;
	while (str.length() < len) {
		//find page number of address
		int page_mask = ~(-1 >> 12);
		unsigned page_num = (message >> 13) & page_mask;

		if (curr_process->process_pgtable->ptes[curr_page]->read_enable == 0) {
			vm_fault(message, false);
		}

		//add to string
	}

	return 0;

}

