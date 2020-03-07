#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "filesys/filesys.h"
#include "userprog/filelist.h"
#include "devices/input.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void halt(){
  power_off();
}

void exit(struct intr_frame *f){
  struct file_list *file_list = thread_current()->file_list;
  free_filelist(file_list);
  thread_exit();
  f->eax = 0;
}

void create(struct intr_frame *f){
  char *name = *((char**)(f->esp+4));
  unsigned size = *(unsigned*)(f->esp+8);
  f->eax = filesys_create (name, (off_t)size);
}

void open(struct intr_frame *f){
  char *name = *((char**)(f->esp+4));
  struct file *file = filesys_open(name);
  if (file != NULL) {
    struct file_list *file_list = thread_current()->file_list;
    f->eax = insert(file_list, file);
  } else {
    f->eax = -1;
  }
}

void close(struct intr_frame *f){
  struct file_list *file_list = thread_current()->file_list;
  int fd = *((int*)(f->esp+4));
  if (fd >= 2) {
    remove(file_list, fd);
  }
}

void read(struct intr_frame *f){
  int fd = *((int*)(f->esp+4));
  char *buffer = *(char**)(f->esp+8);
  unsigned size = *(unsigned*)(f->esp+12);

  if (fd == STDOUT_FILENO) {
    f->eax = -1;
  } else if (fd == STDIN_FILENO){
    int i = 0;
    for(; i < size; i++){
      buffer[i] = input_getc();
    }
    f->eax = i;
  } else {
    struct file_list *file_list = thread_current()->file_list;
    struct file *file = get(file_list, fd);
    if (file != NULL) {
      f->eax = file_read(file, (void*)buffer, size);
    } else {
      f->eax = -1;
    }
  }
}

void write(struct intr_frame *f){
  int fd = *((int*)(f->esp+4));
  char *buffer = *(char**)(f->esp+8);
  unsigned size = *(unsigned*)(f->esp+12);
  if (fd == STDIN_FILENO) {
    f->eax = -1;
  } else if (fd == STDOUT_FILENO){
    putbuf(buffer, size); /* lib/kernel/stdio.h */
    f->eax = size;
  } else {
    struct file_list *file_list = thread_current()->file_list;
    struct file *file = get(file_list, fd);
    if (file != NULL){
      f->eax = file_write(file, buffer, size);
    } else {
      f->eax = -1;
    }
  }
}


static void
syscall_handler (struct intr_frame *f UNUSED) {
  switch (*((int*)f->esp)) {
    case SYS_HALT: halt(); break;
    case SYS_EXIT: exit(f); break;
    case SYS_CREATE: create(f); break;
    case SYS_OPEN: open(f); break;
    case SYS_CLOSE: close(f); break;
    case SYS_READ: read(f); break;
    case SYS_WRITE: write(f); break;
  }
  //printf ("system call!\n");
  //thread_exit ();
}
