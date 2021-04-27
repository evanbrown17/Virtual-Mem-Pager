/*
 * vm_app.h
 *
 * Public routines for clients of the external pager
 */

#ifndef _VM_APP_H_
#define _VM_APP_H_

/* page size (in bytes) for the machine */
#define VM_PAGESIZE 8192

/*
 * vm_extend
 *
 * Ask for the lowest invalid virtual page in the process's arena to
 * be declared valid.  Returns the lowest-numbered byte of the newly
 * valid virtual page.  For example, if the valid part of the arena
 * before calling vm_extend is 0x60000000-0x60003FFF, the return value
 * will be 0x60004000, and the resulting valid part of the arena will
 * be 0x60000000-0x60005FFF. The newly-allocated page is initialized to
 * all zero bytes. Returns null if the new page cannot be allocated.
 */
extern void* vm_extend();

/* 
 * vm_syslog
 *
 * Ask external pager to log a message of nonzero length len. Message data
 * must be in the part of the address space controlled by the pager.
 * Returns 0 on success or -1 on failure.
 */
extern int vm_syslog(void* message, unsigned len);

/* 
 * vm_yield
 *
 * Ask operating system to yield the CPU to another process. The
 * infrastructure's scheduler is non-preemptive, so a process runs
 * until it calls vm_yield or exits.
 */
extern void vm_yield();

#endif /* _VM_APP_H_ */
