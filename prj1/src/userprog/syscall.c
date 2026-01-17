#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <stdbool.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

  /* Projects 2 and later. */
  // SYS_HALT,     /* Halt the operating system. */
  // SYS_EXIT,     /* Terminate this process. */
  // SYS_EXEC,     /* Start another process. */
  // SYS_WAIT,     /* Wait for a child process to die. */
  // SYS_CREATE,   /* Create a file. */
  // SYS_REMOVE,   /* Delete a file. */
  // SYS_OPEN,     /* Open a file. */
  // SYS_FILESIZE, /* Obtain a file's size. */
  // SYS_READ,     /* Read from a file. */
  // SYS_WRITE,    /* Write to a file. */
  // SYS_SEEK,     /* Change position in a file. */
  // SYS_TELL,     /* Report current position in a file. */
  // SYS_CLOSE,    /* Close a file. */
  
  /* Additional Implemented System Calls. */
  // SYS_FIBONNACI, 
  // SYS_MAX_OF_FOUR_INT,

void
check_invalid_address(void* vaddr, int argc)
{
  for (int i = 1; i <= argc; i++) 
  {
    if (vaddr + 4 * i == NULL || is_kernel_vaddr(vaddr + 4 * i))
      exit(-1);
    if (pagedir_get_page(thread_current()->pagedir, vaddr + 4 * i) == NULL)
      exit(-1);
  }
}

static void
syscall_handler (struct intr_frame* f)
{
  switch (*(uint32_t *)f->esp) 
  {
      /* Projects 2 and later. */
    case SYS_HALT:      // syscall0
      halt ();
      break;
    case SYS_EXIT:      // syscall1
      check_invalid_address (f->esp, 1);
      exit (*(int*)(f->esp + 4));
      break;
    case SYS_EXEC:      // syscall1
      check_invalid_address (f->esp, 1);
      f->eax = exec ((char *)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_WAIT:      // syscall1
      check_invalid_address (f->esp, 1);
      f->eax = wait (*(pid_t *)(f->esp + 4));
      break;
    case SYS_CREATE:    // syscall2
      check_invalid_address (f->esp, 2);
      f->eax = create((char *)*(uint32_t *)(f->esp + 4),
                      *(unsigned *)(f->esp + 8));
      break;
    case SYS_REMOVE:    // syscall1
      check_invalid_address (f->esp, 1);
      f->eax = remove ((char *)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_OPEN:      // syscall1
      check_invalid_address (f->esp, 1);
      f->eax = open ((char *)*(uint32_t *)(f->esp + 4));
      break;
    case SYS_FILESIZE:  // syscall1
      check_invalid_address (f->esp, 1);
      f->eax = filesize (*(int *)(f->esp + 4));
      break;
    case SYS_READ:      // syscall3
      check_invalid_address (f->esp, 3);
      f->eax = read (*(int *)(f->esp + 4),
                     (void *)*(uint32_t *)(f->esp + 8),
                     *(unsigned *)(f->esp + 12));
      break;
    case SYS_WRITE:     // syscall3
      check_invalid_address (f->esp, 3);
      f->eax = write (*(int *)(f->esp + 4),
                      (void*)*(uint32_t*)(f->esp + 8),
                      *(unsigned *)(f->esp + 12));
      break;
    case SYS_SEEK:      // syscall2
      check_invalid_address (f->esp, 1);
      seek (*(int *)(f->esp + 4), 
            *(unsigned *)(f->esp + 8));
      break;
    case SYS_TELL:      // syscall1
      check_invalid_address (f->esp, 1);
      f->eax = tell (*(int *)(f->esp + 4));
      break;
    case SYS_CLOSE:     // syscall1
      check_invalid_address (f->esp, 1);
      close (*(int *)(f->esp + 4));
      break;
    /* Additional Implemented System Calls. */
    case SYS_FIBONACCI: // syscall1
      check_invalid_address (f->esp, 1);
      f->eax = fibonacci (*(int *)(f->esp + 4));
      break;
    case SYS_MAX_OF_FOUR_INT: // syscall4
      check_invalid_address (f->esp, 4);
      f->eax = max_of_four_int (*(int *)(f->esp + 4),
                                *(int *)(f->esp + 8),
                                *(int *)(f->esp + 12),
                                *(int *)(f->esp + 16));
      break;
  }

  // thread_exit ();
}

/* Terminates Pintos by calling shutdown_power_off() (declared in
devices/shutdown.h). This should be seldom used, because you lose some
information about possible deadlock situations, etc. */
void
halt (void)
{
  shutdown_power_off ();
}

/* Terminates the current user program, returning status to the kernel. If the
process's parent waits for it (see below), this is the status that will be
returned. Conventionally, a status of 0 indicates success and nonzero values
indicate errors. void exit (int status) */
void 
exit (int status)
{
  struct thread *t = thread_current ();
  // store exit status of its exit_status variable
  t->exit_status = status;
  printf("%s: exit(%d)\n", t->name, status);
  thread_exit ();
}

/* Runs the executable whose name is given in cmd_line, passing any given
arguments, and returns the new process's program id (pid). Must return pid -1,
which otherwise should not be a valid pid, if the program cannot load or run for
any reason. Thus, the parent process cannot return from the exec until it knows
whether the child process successfully loaded its executable. You must use
appropriate synchronization to ensure this.
*/
pid_t
exec (const char* file)
{
  return (pid_t)(process_execute (file));
}

/* Waits for a child process pid and retrieves the child's exit status.
If pid is still alive, waits until it terminates. Then, returns the status that
pid passed to exit. If pid did not call exit(), but was terminated by the kernel
(e.g. killed due to an exception), wait(pid) must return -1. It is perfectly
legal for a parent process to wait for child processes that have already
terminated by the time the parent calls wait, but the kernel must still allow
the parent to retrieve its child's exit status, or learn that the child was
terminated by the kernel.

wait must fail and return -1 immediately if any of the following conditions is
true:

- pid does not refer to a direct child of the calling process. pid is a direct
child of the calling process if and only if the calling process received pid as
a return value from a successful call to exec. Note that children are not
inherited: if A spawns child B and B spawns child process C, then A cannot wait
for C, even if B is dead. A call to wait(C) by process A must fail. Similarly,
orphaned processes are not assigned to a new parent if their parent process
exits before they do.

- The process that calls wait has already called wait on pid. 
That is, a process may wait for any given child at most once. 

Processes may spawn any number of children, wait for them in any order, and 
may even exit without having waited for some or all of their children. 
Your design should consider all the ways in which waits can occur. 
All of a process's resources, including its struct thread, must be 
freed whether its parent ever waits for it or not, and regardless of 
whether the child exits before or after its parent.

You must ensure that Pintos does not terminate until the initial process exits.
The supplied Pintos code tries to do this by calling process_wait() (in
userprog/process.c) from main() (in threads/init.c). We suggest that you
implement process_wait() according to the comment at the top of the function and
then implement the wait system call in terms of process_wait().

Implementing this system call requires considerably more work than any of the
rest. */
int
wait (pid_t pid)
{
  return process_wait ((tid_t)pid);
}

bool
create (const char* file, unsigned initial_size)
{
  // to be implement
  initial_size = initial_size;
  return true;
}

bool
remove (const char* file)
{
  // to be implement
  return true;
}

int
open (const char* file)
{
  // to be implement
  return -1;
}

int
filesize (int fd)
{
  // to be implement
  return 0;
}

/* Reads size bytes from the file open as fd into buffer. Returns the number of
bytes actually read (0 at end of file), or -1 if the file could not be read (due
to a condition other than end of file). Fd 0 reads from the keyboard using
input_getc(). */
int
read (int fd, void* buffer, unsigned size)
{
  char c;
  int i;

  if (fd != 0) // not STDIN
    return -1;

  for (i = 0; i < size; i++) {
    c = input_getc();
    ((char*)buffer)[i] = c;
  }

  return i;
}

/* Writes size bytes from buffer to the open file fd. Returns the number of
bytes actually written, which may be less than size if some bytes could not be
written. Writing past end-of-file would normally extend the file, but file
growth is not implemented by the basic file system. The expected behavior is to
write as many bytes as possible up to end-of-file and return the actual number
written, or 0 if no bytes could be written at all.

Fd 1 writes to the console. Your code to write to the console should write all
of buffer in one call to putbuf(), at least as long as size is not bigger than a
few hundred bytes. (It is reasonable to break up larger buffers.) Otherwise,
lines of text output by different processes may end up interleaved on the
console, confusing both human readers and our grading scripts. */
int
write (int fd, const void* buffer, unsigned size)
{
  if (fd != 1) // not STDOUT 
    return -1;

  putbuf (buffer, size);
  return size;
}

/*
Changes the next byte to be read or written in open file fd to position,
expressed in bytes from the beginning of the file. (Thus, a position of 0 is the
file's start.) A seek past the current end of a file is not an error. A later
read obtains 0 bytes, indicating end of file. A later write extends the file,
filling any unwritten gap with zeros. (However, in Pintos files have a fixed
length until project 4 is complete, so writes past end of file will return an
error.) These semantics are implemented in the file system and do not require
any special effort in system call implementation.
*/
void
seek (int fd, unsigned position)
{
  // to be implement
  fd = fd;
  position = position;
}

unsigned
tell (int fd)
{
  // to be implement
  fd = fd;
  return (unsigned)0;
}

void
close (int fd)
{
  // to be implement
  fd = fd;
}

int
fibonacci (int n)
{
  if (n <= 1) 
    return n;
  return fibonacci (n - 1) + fibonacci (n - 2);
}

int
max_of_four_int (int a, int b, int c, int d)
{
  int max;
  max = (a > b) ? a : b;
  max = (max > c) ? max : c;
  max = (max > d) ? max : d;
  return max;
}
