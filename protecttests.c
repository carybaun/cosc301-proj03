#include "stat.h"
#include "user.h"
#include "fcntl.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)
#define PGROUNDUP(sz) ((((uint)sz)+PGSIZE-1) & ~(PGSIZE-1))

int ppid;

#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

int mprotect(void *addr, int len) {
	if ((int)addr%PGSIZE != 0 || (int)addr > proc->sz) {  //addr too large
		return -1;
	}
	if (len <= 0 || len > (int)addr) {		
		return -1;
	}
	return 0;
}

int munprotect(void *addr, int len) {
	if ((int)addr%PGSIZE != 0 || (int)addr > proc->sz) {
		return -1;
	}
	return 0;
}

int
main(int argc, char *argv[])
{

  ppid = getpid();

  char *brk = sbrk(0);
  sbrk(PGROUNDUP(brk) - (uint)brk);
  char *start = sbrk(0);

  // should fail for address zero, which shouldn't be mapped in
  // the process any more because of part a of project
  assert(mprotect(0, 1) == -1);
  assert(munprotect(0, 1) == -1);

  printf(1, "starting address is %d\n", (uint)start);

  assert(mprotect(start, 1) == -1);
  assert(munprotect(start, 1) == -1);

  sbrk(PGSIZE * 1);
  assert(mprotect(start, 2) == -1);
  assert(munprotect(start, 2) == -1);

  assert(mprotect(start + 1, 1) == -1);
  assert(munprotect(start + 1, 1) == -1);

  assert(mprotect(start, 0) == -1);
  assert(munprotect(start, 0) == -1);

  assert(mprotect(start, -2) == -1);
  assert(munprotect(start, -2) == -1);

  assert(mprotect(start, 1) == 0);
  assert(munprotect(start, 1) == 0);

  // protect page again to check that permissions
  // carry over on fork
  assert(mprotect(start, 1) == 0);
  int rv = fork();
  if (rv < 0) {
    printf(1, "Fork failed.  Oops.  This shouldn't happen, right?!\n");
  } else if (rv == 0) {
    printf(1, "Attempting to write to protected memory in a child process\n");
    printf(1, "This should cause the child to die if the test succeeds\n");
    *start = 55; // this should cause the child proc to DIR
    printf(1, "TEST FAILED (if you got here, child didn't crash)\n");
    exit();
  } else {
    assert(munprotect(start, 1) == 0);
    wait();
  }

   printf(1, "TEST PASSED\n");

   exit();
}
