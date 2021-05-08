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
#include <climits>


using namespace std;

struct Page {
	unsigned page_num;
	int ref_bit;
	int dirty_bit;
	bool resident;
	bool initialized;
	unsigned frame; //physmem location (if any)
	unsigned block; //disk location
	pid_t pid; //process it belongs to
};


struct Process {
	pid_t pid;
	page_table_t* process_ptbr;
	page_table_t* process_pgtable;
	uintptr_t valid_floor;
	uintptr_t valid_ceiling;
	list<Page*>* page_info;	
};


list<Process*> all_processes;
Process* curr_process;

list<Page*> all_pages;

unsigned mem_pages;
unsigned frame_counter;
unsigned blocks;
unsigned block_counter;
//unsigned framesAssigned;
//
Page** frames_assigned;
unsigned clock_hand;

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
	arena_base = (uintptr_t) VM_ARENA_BASEADDR;
	frame_counter = 0;
	clock_hand = 0;
 	mem_pages = memory_pages;
	blocks = disk_blocks;
	block_counter = 0;
	total_pages = 0;
	frames_assigned = new Page* [mem_pages]; //says whether the frame is full or not
	for (unsigned i = 0; i < mem_pages; i++) {
		frames_assigned[i] = nullptr;	
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
	new_process->valid_floor = (uintptr_t) VM_ARENA_BASEADDR;
	new_process->valid_ceiling = (uintptr_t) VM_ARENA_BASEADDR;

	new_process->process_pgtable = new page_table_t;
	new_process->process_ptbr = new_process->process_pgtable;
	//new_process->process_pgtable->ptes = new page_table_entry_t;
	cerr << "New process pid is " << new_process->pid << endl;

	new_process->page_info = new list<Page*>;

	//Need to initialize values in actual page table?

	all_processes.push_back(new_process);



}

Process* find_process(pid_t pid) {
	for (Process* p : all_processes) {
		if (p->pid == pid) {
			return p;
		}
	}
	return nullptr; //can't find process
}

/*
 * vm_switch
 *
 * Notifies the pager that the kernel is switching to a new process with the
 * given pid.
*/
void vm_switch(pid_t pid) {
	Process* next_process = find_process(pid);
	if (next_process != nullptr) {
 		curr_process = next_process;
		page_table_base_register = curr_process->process_ptbr; //set PTBR to point to the current page table
		return;

	}	
	
	cerr << "There is no process with the given pid: " << pid << endl;
		
}

Page* find_page(unsigned page_num) {
	Page* result;
	for (Page* p : all_pages) {
		if (p->page_num == page_num && p->pid == curr_process->pid) {
			result = p;
			return result;
		}	
	}
	return nullptr; //error
}

void advance_clock_hand() {
	if (clock_hand == mem_pages - 1) { //clock hand is at end of list
		clock_hand = 0;
	} else {
		clock_hand++;
	}
}

void initialize_page_and_disk(Page* page, unsigned frame) {
	//initialize memory contents to 0
	for (unsigned i = (frame * VM_PAGESIZE); i < ((frame + 1) * VM_PAGESIZE); i++) {
		((int*) pm_physmem)[i] = 0;
	}
	disk_write(page->block, frame);
	page->initialized = true;
}


void page_replace(Page* incoming_page, int new_pg_table_index) {

	while (true){
	
		if (frames_assigned[clock_hand] == nullptr) {

			frames_assigned[clock_hand] = incoming_page;
			if (incoming_page->initialized) {
				disk_read(incoming_page->block, clock_hand);
			} else {
				initialize_page_and_disk(incoming_page, clock_hand);
			}
			incoming_page->resident = true;
			incoming_page->ref_bit = 1;
			incoming_page->frame = clock_hand;
			curr_process->process_pgtable->ptes[new_pg_table_index].ppage = clock_hand;
			curr_process->process_pgtable->ptes[new_pg_table_index].read_enable = 1;
			advance_clock_hand();
			break;

		} else if (frames_assigned[clock_hand]->ref_bit == 1) {

			frames_assigned[clock_hand]->ref_bit = 0;
			advance_clock_hand();

		} else if (frames_assigned[clock_hand]->ref_bit == 0) {
			//evict this page
			if (frames_assigned[clock_hand]->dirty_bit == 1) {
				unsigned write_block = frames_assigned[clock_hand]->block;
				unsigned frame = (unsigned) clock_hand;
				disk_write(write_block, frame);
			} 

			int old_pg_table_index = frames_assigned[clock_hand]->page_num - VM_ARENA_BASEPAGE;
			Process* evicted_process = find_process(frames_assigned[clock_hand]->pid);
			frames_assigned[clock_hand]->dirty_bit = 0;
			frames_assigned[clock_hand]->resident = false;
			frames_assigned[clock_hand]->frame = -1;
			evicted_process->process_pgtable->ptes[old_pg_table_index].ppage = UINT_MAX;
			evicted_process->process_pgtable->ptes[old_pg_table_index].read_enable = 0;
			evicted_process->process_pgtable->ptes[old_pg_table_index].write_enable = 0;

			frames_assigned[clock_hand] = incoming_page;
			if (incoming_page->initialized) {
				disk_read(incoming_page->block, (unsigned) clock_hand);
			} else {
				initialize_page_and_disk(incoming_page, clock_hand);
			}	
			incoming_page->resident = true;
			incoming_page->ref_bit = 1;
			incoming_page->frame = clock_hand;
			curr_process->process_pgtable->ptes[new_pg_table_index].ppage = clock_hand;
			curr_process->process_pgtable->ptes[new_pg_table_index].read_enable = 1;
			advance_clock_hand();
			break;

		}


	}

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
	
	if ((uintptr_t) addr < (uintptr_t) VM_ARENA_BASEADDR || (uintptr_t) addr > curr_process->valid_ceiling) {	
		return -1;
	}

	unsigned page_mask = ~(-1 >> 12);
	unsigned page_num = (uintptr_t) addr & page_mask;
	Page* curr_page = find_page(page_num);
	int page_table_index = page_num - VM_ARENA_BASEPAGE;

/*
	if (!write_flag) { //fault occurred on read
		//check residency
		if (curr_process->process_pgtable->ptes[page_table_index].ppage != UINT_MAX) {
			//resident
			curr_page->ref_bit = 1;
			curr_process->process_pgtable->ptes[page_table_index].read_enable = 1;

		} else {
			//non-resident
			page_replace(curr_page, page_table_index);
		}
	} else { //fault occurred on write
		if (curr_process->process_pgtable->ptes[page_table_index].ppage != UINT_MAX) {
			curr_page->ref_bit = 1;
			curr_page->dirty_bit = 1;
			curr_process->process_pgtable->ptes[page_table_index].write_enable = 1;
		} else {
			page_replace(curr_page, page_table_index);
		}
	
	}
	*/

	if (curr_process->process_pgtable->ptes[page_table_index].ppage != UINT_MAX) {
		curr_page->ref_bit = 1;
		if (!write_flag){
			curr_process->process_pgtable->ptes[page_table_index].read_enable = 1;
		} else {
			curr_page->dirty_bit = 1;
			curr_process->process_pgtable->ptes[page_table_index].write_enable = 1;
		}	
	
	} else {
		page_replace(curr_page, page_table_index);
	}

	return 0;
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
		
	if ((curr_process->valid_ceiling < (uintptr_t) VM_ARENA_BASEADDR + (uintptr_t)VM_ARENA_SIZE - 1) &&
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
		newPage->frame = -1;
		newPage->block = block_counter;
		newPage->initialized = false;
		newPage->resident = false;
		newPage->pid = curr_process->pid;
		
		all_pages.push_back(newPage);
		curr_process->page_info->push_back(newPage);

		//delay disk initialization to the first time the page is accessed

		block_counter++; //will probably need more complexity here to write over disk entries that are no longer needed

		int pgTable_index = newPage->page_num - VM_ARENA_BASEPAGE;
		
		//curr_process->process_pgtable->ptes[pgTable_index] = new page_table_entry_t;
		curr_process->process_pgtable->ptes[pgTable_index].ppage = UINT_MAX;
		curr_process->process_pgtable->ptes[pgTable_index].read_enable = 0;
		curr_process->process_pgtable->ptes[pgTable_index].write_enable = 0;

		uintptr_t return_value = (uintptr_t)VM_ARENA_BASEADDR + (VM_PAGESIZE * (curr_process->page_info->size() - 1)); 
		
		curr_process->valid_ceiling = return_value + VM_PAGESIZE - 1; //update which virtual addresses are valid
		return (void*) return_value;

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
	if ((uintptr_t) message < (uintptr_t) VM_ARENA_BASEADDR || (uintptr_t) message > curr_process->valid_ceiling) {
		return -1;
	}

	//make sure the message address plus the length is in the arena and the length is not zero
	if ((uintptr_t) message + len > curr_process->valid_ceiling || len == 0) {
		return - 1;
	}

	string str = "";
	unsigned page_mask = ~(-1 >> 12);
	unsigned offset_mask = 8191;
	while (str.length() < len) {
		//find page number of address
		unsigned page_num = ((uintptr_t) message >> 13) & page_mask;
		int page_table_index = page_num - VM_ARENA_BASEPAGE;
		if (curr_process->process_pgtable->ptes[page_table_index].read_enable == 0) {
			vm_fault(message, false);
		}

		unsigned offset = (uintptr_t) message & offset_mask;
		unsigned long frame = curr_process->process_pgtable->ptes[page_table_index].ppage;
		str += ((char*) pm_physmem)[(frame * VM_PAGESIZE) + offset + str.length()];

	}

	cout << "syslog \t\t\t" << str << endl;

	return 0;

}

