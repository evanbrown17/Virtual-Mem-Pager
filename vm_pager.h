/*
 * vm_pager.h
 *
 * Header file for the external pager
 */

#ifndef _VM_PAGER_H_
#define _VM_PAGER_H_

#include <sys/types.h>

/*
 * ******************************************
 * * Interface for student portion of pager *
 * ******************************************
 */

/*
 * vm_init
 *
 * Initializes the pager and any associated data structures. Called automatically
 * on pager startup. Passed the number of physical memory pages and the number of
 * disk blocks in the raw disk.
 */
extern void vm_init(unsigned memory_pages, unsigned disk_blocks);

/*
 * vm_create
 *
 * Notifies the pager that a new process with the given process ID has been created.
 * The new process will only run when it's switched to via vm_switch.
 */
extern void vm_create(pid_t pid);

/*
 * vm_switch
 *
 * Notifies the pager that the kernel is switching to a new process with the
 * given pid.
 */
extern void vm_switch(pid_t pid);

/*
 * vm_fault
 *
 * Handle a fault that occurred at the given virtual address. The write flag
 * is 1 if the faulting access was a write or 0 if the faulting access was a
 * read. Returns -1 if the faulting address corresponds to an invalid page
 * or 0 otherwise (having handled the fault appropriately).
 */
extern int vm_fault(void* addr, bool write_flag);

/*
 * vm_destroy
 *
 * Notifies the pager that the current process has exited and should be
 * deallocated.
 */
extern void vm_destroy();

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
extern void* vm_extend();

/*
 * vm_syslog
 *
 * Log (i.e., print) a message in the arena at the given address with the
 * given nonzero length. Returns 0 on success or -1 if the specified message
 * address or length is invalid.
 */
extern int vm_syslog(void* message, unsigned len);

/*
 * *********************************************
 * * Public interface for the disk abstraction *
 * *********************************************
 *
 * Disk blocks are numbered from 0 to (disk_blocks-1), where disk_blocks
 * is the parameter passed to vm_init.
 */

/*
 * disk_read
 *
 * Read the specified disk block into the specified physical memory page.
 */
extern void disk_read(unsigned block, unsigned ppage);

/*
 * disk_write
 *
 * Write the contents of the specified physical memory page onto the specified
 * disk block.
 */
extern void disk_write(unsigned block, unsigned ppage);

/*
 * ********************************************************
 * * Public interface for the physical memory abstraction *
 * ********************************************************
 *
 * Physical memory pages are numbered from 0 to (memory_pages-1), where
 * memory_pages is the parameter passed to vm_init.
 *
 * The pager accesses the data in physical memory through the variable
 * pm_physmem, e.g. ((char*) pm_physmem)[5] is byte 5 in physical memory.
 */
extern void* pm_physmem;

/*
 * ***********************
 * * Definition of arena *
 * ***********************
 */

/* page size (in bytes) for the machine */
#define VM_PAGESIZE 8192

/* virtual address at which application's arena starts */
#define VM_ARENA_BASEADDR    ((void*) 0x60000000)

/* virtual page number at which application's arena starts */
#define VM_ARENA_BASEPAGE    ((uintptr_t) VM_ARENA_BASEADDR / VM_PAGESIZE)

/* size (in bytes) of arena */
#define VM_ARENA_SIZE    0x20000000

/*
 * **************************************
 * * Definition of page table structure *
 * **************************************
 */

/*
 * Format of page table entry.
 *
 * ppage: the physical page (frame) for this virtual page, if applicable.
 * read_enable: bit determining whether loads to this virtual page will fault.
 *    (1 ==> fault, 0 ==> no fault)
 * write_enable: bit determining whether stores to this virtual page will fault.
 *    (1 ==> fault, 0 ==> no fault)
 */
typedef struct {
    unsigned long ppage : 51;   /* bits 0-50 of pte */
    unsigned int read_enable : 1; /* bit 51 of pte */
    unsigned int write_enable : 1;  /* bit 52 of pte */
} page_table_entry_t;

/*
 * Format of page table.  Entries start at virtual page VM_ARENA_BASEPAGE,
 * i.e. ptes[0] is the page table entry for virtual page VM_ARENA_BASEPAGE.
 */
typedef struct {
    page_table_entry_t ptes[VM_ARENA_SIZE / VM_PAGESIZE];
} page_table_t;

/*
 * MMU's page table base register.  This variable is defined by the
 * infrastructure, but it is controlled completely by the student's pager code.
 */
extern page_table_t* page_table_base_register;

#endif /* _VM_PAGER_H_ */
