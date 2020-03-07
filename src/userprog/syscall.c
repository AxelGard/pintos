#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "filesys/filesys.h"
#include "userprog/filelist.h"
#include "devices/input.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

static void syscall_handler (struct intr_frame *);
static void halt(void);
static int exit(int status);
static bool create(char *name, unsigned size);
static int open(char *name);
static void close(int fd);
static int read(int fd, void *buffer, unsigned size);
static int write(int fd, const void *buffer, unsigned size);
static int execute(char *cmdline);
static void seek(int fd, unsigned position);
static unsigned tell(int fd);
static int filesize(int fd);
static bool remove_file(const char *file_name);

static bool valid_ptr(void *ptr);
static bool valid_str(char *str, unsigned size);
static bool valid_buf(void *buf, unsigned size);
static bool valid_unfixed_str(char *str);

bool valid_ptr(void *ptr){
  if (ptr == NULL) return false;
  if (is_kernel_vaddr(ptr)) return false;
  if (pagedir_get_page(thread_current()->pagedir, ptr) == NULL) return false;
  return true;
}

bool valid_str(char *str, unsigned size){
  for (unsigned i = 0; i < size; i++){
    if (!valid_ptr((str + i))) return false;
  }
  return true;
}

bool valid_buf(void *buf, unsigned size){
  for (unsigned i = 0; i < size; i++){
    if (!valid_ptr((buf + i))) return false;
  }
  return true;
}

bool valid_unfixed_str(char *str){
  if (!valid_ptr(str)) return false;
  for (unsigned i = 0; str[i]; i++){
    if (!valid_ptr((str + i))) return false;
  }
  return true;
}


void
syscall_init (void)
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void halt(void){
  power_off();
}

int exit(int status){
  struct file_list *file_list = thread_current()->file_list;
  free_filelist(file_list);
  thread_current()->relation->exit_status = status;
  thread_exit();
  return status;
}

bool create(char *name, unsigned size){
  bool created = filesys_create (name, (off_t)size);
  return created;
}

int open(char *name){
  struct file *file = filesys_open(name);
  if (file != NULL) {
    struct file_list *file_list = thread_current()->file_list;
    return insert(file_list, file);
  } else {
    return -1;
  }
}

void close(int fd){
  struct file_list *file_list = thread_current()->file_list;
  remove(file_list, fd);
}

int read(int fd, void *buffer, unsigned size){
  if (fd == STDOUT_FILENO) {
    return -1;
  } else if (fd == STDIN_FILENO){
    char* charBuffer = (char*)buffer;
    unsigned i = 0;
    for(; i < size; i++){
      charBuffer[i] = input_getc();
    }
    return i;
  } else {
    struct file_list *file_list = thread_current()->file_list;
    struct file *file = get(file_list, fd);
    if (file != NULL) {
      int size_read = file_read(file, buffer, size);
      return size_read;
    } else {
      return -1;
    }
  }
}

int write(int fd, const void *buffer, unsigned size){
  if (fd == STDIN_FILENO) {
    return -1;
  } else if (fd == STDOUT_FILENO){
    putbuf((char*)buffer, size); /* lib/kernel/stdio.h */
    return size;
  } else {
    struct file_list *file_list = thread_current()->file_list;
    struct file *file = get(file_list, fd);
    if (file != NULL){
      int size_written = file_write(file, buffer, size);
      return size_written;
    } else {
      return -1;
    }
  }
}

int execute(char *cmdline){
  return process_execute(cmdline);
}

void seek(int fd, unsigned position){
  struct file_list *file_list = thread_current()->file_list;
  struct file *file = get(file_list, fd);
  file_seek(file, position);
}

unsigned tell(int fd){
  struct file_list *file_list = thread_current()->file_list;
  struct file *file = get(file_list, fd);
  return file_tell(file);
}

int filesize(int fd){
  struct file_list *file_list = thread_current()->file_list;
  struct file *file = get(file_list, fd);
  return file_length(file);
}

bool remove_file (const char *file_name){
  return filesys_remove(file_name);
}


static void
syscall_handler (struct intr_frame *f UNUSED) {
  //printf("syscall!!!\n\n\n");
  if (!valid_ptr(f->esp)) exit(-1);
  unsigned size = 0;
  int fd = -1;
  char *str = NULL;
  void *buf = NULL;
  switch (*((int*)f->esp)) {
    case SYS_HALT: halt(); break;
    case SYS_EXIT:
      if (!valid_ptr((f->esp + 4))) exit(-1);
      fd = *((int*)(f->esp+4));
      f->eax = exit(fd);
      break;
    case SYS_CREATE:
      if (!valid_ptr((f->esp + 4))) exit(-1);
      if (!valid_ptr((f->esp + 8))) exit(-1);
      size = *(unsigned*)(f->esp+8);
      str = *(char**)(f->esp+4);
      if (!valid_unfixed_str(str)) exit(-1);
      f->eax = create(str, size);
      break;
    case SYS_OPEN:
      if (!valid_ptr((f->esp + 4))) exit(-1);
      str = *(char**)(f->esp+4);
      if (!valid_unfixed_str(str)) exit(-1);
      f->eax = open(str);
      break;
    case SYS_CLOSE:
      if (!valid_ptr((f->esp + 4))) exit(-1);
      fd = *((int*)(f->esp+4));
      close(fd);
      break;
    case SYS_READ:
      if (!valid_ptr((f->esp + 4))) exit(-1);
      if (!valid_ptr((f->esp + 8))) exit(-1);
      if (!valid_ptr((f->esp + 12))) exit(-1);
      size = *(unsigned*)(f->esp+12);
      buf = *(void**)(f->esp+8);
      fd = *((int*)(f->esp+4));
      if (!valid_buf(buf, size)) exit(-1);
      f->eax = read(fd, buf, size);
      break;
    case SYS_WRITE:
      if (!valid_ptr((f->esp + 4))) exit(-1);
      if (!valid_ptr((f->esp + 8))) exit(-1);
      if (!valid_ptr((f->esp + 12))) exit(-1);
      size = *(unsigned*)(f->esp+12);
      buf = *(void**)(f->esp+8);
      fd = *((int*)(f->esp+4));
      if (!valid_buf(buf, size)) exit(-1);
      f->eax = write(fd, buf, size);
      break;
    case SYS_EXEC:
      if (!valid_ptr((f->esp + 4))) exit(-1);
      str = *(char**)(f->esp+4);
      if (!valid_unfixed_str(str)) exit(-1);
      f->eax = execute(str);
      break;
    case SYS_WAIT:
      if (!valid_ptr((f->esp + 4))) exit(-1);
      tid_t tid = *((int*)(f->esp+4));
      f->eax = process_wait(tid);
      break;
    case SYS_SEEK:
      if (!valid_ptr((f->esp + 4))) exit(-1);
      if (!valid_ptr((f->esp + 8))) exit(-1);
      fd = *((int*)(f->esp+4));
      unsigned position = *(unsigned*)(f->esp+8);
      seek(fd, position);
      break;
    case SYS_TELL:
      if (!valid_ptr((f->esp + 4))) exit(-1);
      fd = *((int*)(f->esp+4));
      f->eax = tell(fd);
      break;
    case SYS_FILESIZE:
      if (!valid_ptr((f->esp + 4))) exit(-1);
      fd = *((int*)(f->esp+4));
      f->eax = filesize(fd);
      break;
    case SYS_REMOVE:
      if (!valid_ptr((f->esp + 4))) exit(-1);
      str = *(char**)(f->esp+4);
      if (!valid_unfixed_str(str)) exit(-1);
      f->eax = remove_file(str);
      break;

  }
  //printf ("system call!\n");
  //thread_exit ();
}
