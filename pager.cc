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
#include <set>


using namespace std;

struct Page {
	unsigned page_num;
	int ref_bit;
	int dirty_bit;
	bool resident;
	bool initialized;
	bool data_on_disk;
	unsigned frame; //physmem location (if any)
	unsigned block; //disk location
	pid_t pid; //process it belongs to
};


struct Process {
	pid_t pid;
	page_table_t* process_ptbr;
	page_table_t* process_pgtable;
	page_table_entry_t* pte_ptr;
	uintptr_t valid_floor;
	uintptr_t valid_ceiling;
	list<Page*>* page_info;	
};


static list<Process*> all_processes;
static Process* curr_process;

static list<Page*> all_pages;

static set<unsigned> taken_disk_blocks;

static unsigned mem_pages;
static unsigned frame_counter;
static unsigned blocks;
static unsigned block_counter;
//unsigned framesAssigned;
//
static Page** frames_assigned;
static unsigned clock_hand;

static uintptr_t arena_base;

static int total_pages;

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

	page_table_base_register = nullptr;
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
	new_process->valid_ceiling = (uintptr_t) VM_ARENA_BASEADDR - 1;

	new_process->process_pgtable = new page_table_t;
	new_process->pte_ptr = new_process->process_pgtable->ptes;
	//new_process->pte_ptr = new page_table_entry_t [VM_ARENA_SIZE / VM_PAGESIZE];

	//new_process->process_pgtable->ptes = new page_table_entry_t[VM_ARENA_SIZE / VM_PAGESIZE];
	//new_process->process_pgtable->ptes [VM_ARENA_SIZE / VM_PAGESIZE] = *(new page_table_entry_t [VM_ARENA_SIZE / VM_PAGESIZE]);
	//new_process->process_pgtable->ptes = new page_table_entry_t;
	//
		
	for (int i = 0; i < (VM_ARENA_SIZE / VM_PAGESIZE); i++) {
		new_process->pte_ptr[i].ppage = 0;
		new_process->pte_ptr[i].read_enable = 0;
		new_process->pte_ptr[i].write_enable = 0;
	}
	

	//cerr << "First pte.ppage is " << new_process->process_pgtable->ptes[0].ppage << endl;
	

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
	cerr << "Switch called with pid " << pid << endl;
	Process* next_process = find_process(pid);
	if (next_process != nullptr) {
 		curr_process = next_process;
		page_table_base_register = curr_process->process_ptbr; //set PTBR to point to the current page table

		cerr << "New PTBR is " << page_table_base_register << endl;
		cerr << "New process page table address is " << curr_process->process_pgtable << endl;

		return;

	}	
	
	cerr << "There is no process with the given pid: " << pid << endl;
		
}

static Page* find_page(unsigned page_num) {
	Page* result;
	for (Page* p : all_pages) {
		if (p->page_num == page_num && p->pid == curr_process->pid) {
			result = p;
			return result;
		}	
	}
	return nullptr; //error
}

static void advance_clock_hand() {
	if (clock_hand == mem_pages - 1) { //clock hand is at end of list
		clock_hand = 0;
	} else {
		clock_hand++;
	}
}

static void initialize_page(Page* page, unsigned frame) {
	//initialize memory contents to 0
	for (unsigned i = (frame * VM_PAGESIZE); i < ((frame + 1) * VM_PAGESIZE); i++) {
		((char*) pm_physmem)[i] = 0;
	}
	//disk_write(page->block, frame);
	page->initialized = true;
}


static void page_replace(Page* incoming_page, int new_pg_table_index, bool write) {

	cerr << "page replace was called with page number " << incoming_page->page_num << endl;

	while (true){
	
		/*
		if (frames_assigned[clock_hand] == nullptr) {

			frames_assigned[clock_hand] = incoming_page;
			if (incoming_page->initialized) {
				disk_read(incoming_page->block, clock_hand);
			} else {
				initialize_page_and_disk(incoming_page, clock_hand);
				cerr << "INITIALIZED PAGE\n";
			}
			incoming_page->resident = true;
			incoming_page->ref_bit = 1;
			incoming_page->frame = clock_hand;
			cerr << "new frame is " << clock_hand << endl;
			curr_process->process_pgtable->ptes[new_pg_table_index].ppage = clock_hand;
			curr_process->process_pgtable->ptes[new_pg_table_index].read_enable = 1;
			if (write) {
				incoming_page->dirty_bit = 1;
				curr_process->process_pgtable->ptes[new_pg_table_index].write_enable = 1;
			}
			advance_clock_hand();
			break;
		*/

		if (frames_assigned[clock_hand]->ref_bit == 1) {

			frames_assigned[clock_hand]->ref_bit = 0;
			int old_pg_table_index = frames_assigned[clock_hand]->page_num - VM_ARENA_BASEPAGE;
			curr_process->process_pgtable->ptes[old_pg_table_index].read_enable = 0;
			curr_process->process_pgtable->ptes[old_pg_table_index].write_enable = 0;
			advance_clock_hand();

		} else if (frames_assigned[clock_hand]->ref_bit == 0) {
			//evict this page
			if (frames_assigned[clock_hand]->dirty_bit == 1) {
				unsigned write_block = frames_assigned[clock_hand]->block;
				unsigned frame = (unsigned) clock_hand;
				disk_write(write_block, frame);
				frames_assigned[clock_hand]->data_on_disk = true;
			} 

			int old_pg_table_index = frames_assigned[clock_hand]->page_num - VM_ARENA_BASEPAGE;
			Process* evicted_process = find_process(frames_assigned[clock_hand]->pid);
			frames_assigned[clock_hand]->dirty_bit = 0;
			frames_assigned[clock_hand]->resident = false;
			frames_assigned[clock_hand]->frame = -1;
			evicted_process->process_pgtable->ptes[old_pg_table_index].ppage = 0;
			evicted_process->process_pgtable->ptes[old_pg_table_index].read_enable = 0;
			evicted_process->process_pgtable->ptes[old_pg_table_index].write_enable = 0;

			frames_assigned[clock_hand] = incoming_page;
			if (incoming_page->initialized && incoming_page->data_on_disk) {
				disk_read(incoming_page->block, (unsigned) clock_hand);
			} else {
				initialize_page(incoming_page, clock_hand);
			}	
			incoming_page->resident = true;
			incoming_page->ref_bit = 1;
			incoming_page->frame = clock_hand;
			curr_process->process_pgtable->ptes[new_pg_table_index].ppage = clock_hand;
			curr_process->process_pgtable->ptes[new_pg_table_index].read_enable = 1;
			if (write) {
				incoming_page->dirty_bit = 1;
				curr_process->process_pgtable->ptes[new_pg_table_index].write_enable = 1;
			}

			advance_clock_hand();
			break;

		}


	}

}

static void load_into_memory(Page* incoming_page, unsigned new_pg_table_index, bool write) {

	for (unsigned i = 0; i < mem_pages; i++) {
		if (frames_assigned[i] == nullptr) {
			frames_assigned[i] = incoming_page;
			if (incoming_page->initialized && incoming_page->data_on_disk) {
				disk_read(incoming_page->block, i);
			} else {
				initialize_page(incoming_page, i);
				cerr << "INITIALIZED PAGE\n";
			}
			incoming_page->resident = true;
			incoming_page->ref_bit = 1;
			incoming_page->frame = i;
			cerr << "new frame is " << i << endl;
			curr_process->process_pgtable->ptes[new_pg_table_index].ppage = i;
			curr_process->process_pgtable->ptes[new_pg_table_index].read_enable = 1;
			if (write) {
				incoming_page->dirty_bit = 1;
				curr_process->process_pgtable->ptes[new_pg_table_index].write_enable = 1;
			}

			return;

		}

	}

	//no free frames, so run clock algorithm
	page_replace(incoming_page, new_pg_table_index, write);
	
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

	unsigned long long page_mask = 0x7ffffffffffff;
	unsigned page_num = ((uintptr_t) addr >> 13) & page_mask;
	Page* curr_page = find_page(page_num);
	cerr << "current page page num is " << curr_page->page_num << " and current page pid is " << curr_page->pid << endl;
	unsigned page_table_index = page_num - VM_ARENA_BASEPAGE;


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

	if (curr_page->resident) {
		cerr << "ppage is " << curr_process->process_pgtable->ptes[page_table_index].ppage << endl;
		curr_page->ref_bit = 1;
		curr_process->process_pgtable->ptes[page_table_index].read_enable = 1;
		
		if (write_flag) {
			curr_page->dirty_bit = 1;
		}

		if (curr_page->dirty_bit == 1) {
			curr_process->process_pgtable->ptes[page_table_index].write_enable = 1;
		}

		/*
		if (write_flag and curr_page->dirty_bit == 1){
			curr_page->dirty_bit = 1;
			curr_process->process_pgtable->ptes[page_table_index].write_enable = 1;
		}
	       	else {
			if (curr_page->dirty_bit == 1) {
				curr_process->process_pgtable->ptes[page_table_index].read_enable = 1;
				curr_process->process_pgtable->ptes[page_table_index].write_enable = 1;
			} else {
				
			}

		}	
	*/
	} else {
		load_into_memory(curr_page, page_table_index, write_flag);
		//page_replace(curr_page, page_table_index, write_flag);
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

	for (Page* p : *curr_process->page_info) {
		all_pages.remove(p);
		taken_disk_blocks.erase(p->block);
		bool resident = p->resident;
		if (resident) {
			unsigned page_table_index = p->page_num - VM_ARENA_BASEPAGE;
			unsigned long frame = curr_process->process_pgtable->ptes[page_table_index].ppage;
			delete p;
			frames_assigned[frame] = nullptr; //take resident page out of memory
		} else {
			delete p;
		}
	}
	
	delete curr_process->page_info;
	//delete curr_process->pte_ptr;
	delete curr_process->process_pgtable;

	//delete page_table_base_register;
	//page_table_base_register = NULL;

	//delete curr_process->process_pgtable;
	//curr_process->process_pgtable = NULL;
	//delete curr_process->process_ptbr;
	//curr_process->process_ptbr = NULL;
	page_table_base_register = NULL;

	all_processes.remove(curr_process);


	delete curr_process;
	curr_process = NULL;

	if (all_processes.empty()) {
		clock_hand = 0; //move position of clock hand back to beginning of queue
	}

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

	cerr << "Extend called.\n";
		
	if ((curr_process->valid_ceiling < (uintptr_t) VM_ARENA_BASEADDR + (uintptr_t)VM_ARENA_SIZE - 1) &&
			taken_disk_blocks.size() < blocks){ //check to make sure we haven't run out of arena or disk
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
		newPage->page_num = VM_ARENA_BASEPAGE + curr_process->page_info->size();
		//total_pages++;
		newPage->dirty_bit = 0;
		newPage->ref_bit = 0;
		newPage->frame = -1;
		unsigned potential_block = 0;
		while (potential_block < blocks) {
			if (taken_disk_blocks.find(potential_block) == taken_disk_blocks.end()) {
				newPage->block = potential_block;
				taken_disk_blocks.insert(potential_block);
				break;
			}
			potential_block++;
		}
		newPage->initialized = false;
		newPage->resident = false;
		newPage->data_on_disk = false;
		newPage->pid = curr_process->pid;
		
		all_pages.push_back(newPage);
		curr_process->page_info->push_back(newPage);

		//delay disk initialization to the first time the page is accessed

		//block_counter++; //will probably need more complexity here to write over disk entries that are no longer needed

		int pgTable_index = newPage->page_num - VM_ARENA_BASEPAGE;
		
		//curr_process->process_pgtable->ptes[pgTable_index] = new page_table_entry_t;

		curr_process->process_pgtable->ptes[pgTable_index].ppage = 0;
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
	unsigned long long page_mask = 0x7ffffffffffff;
	unsigned offset_mask = 8191;
	unsigned offset = (uintptr_t) message & offset_mask;
	unsigned prev_page_num = ((uintptr_t) message >> 13) & page_mask;
	unsigned long long curr_address = (uintptr_t) message;
	while (str.length() < len) {
		//find page number of address
		unsigned page_num = (((uintptr_t) message + str.length()) >> 13) & page_mask;
		if (page_num != prev_page_num) {
			offset = 0;
			prev_page_num = page_num;
		}
		int page_table_index = page_num - VM_ARENA_BASEPAGE;
		if (curr_process->process_pgtable->ptes[page_table_index].read_enable == 0) {
			vm_fault((void*) curr_address, false);
		}

		//unsigned offset = (uintptr_t) message & offset_mask;
		unsigned long frame = curr_process->process_pgtable->ptes[page_table_index].ppage;
		str += ((char*) pm_physmem)[(frame * VM_PAGESIZE) + offset];
		offset++;
		curr_address++;
	}

	cout << "syslog \t\t\t" << str << endl;

	return 0;

}

