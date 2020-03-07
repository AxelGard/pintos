#ifndef USERPROG_FILELIST_H
#define USERPROG_FILELIST_H

#include "filesys/file.h"

#define LISTSIZE 128
#define OFFSET 2

struct file_list {
  struct file** files;
  int look_from;
};

int insert(struct file_list* list, struct file* file);
struct file_list *init_filelist(void);
void remove(struct file_list* list, int fd);
struct file *get(struct file_list* list, int fd);
void free_filelist(struct file_list *filelist);
#endif
