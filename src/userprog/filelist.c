#include "userprog/filelist.h"
#include "threads/malloc.h"
#include <stdio.h>

struct file_list *init_filelist(){
  struct file_list *list =  (struct file_list*) malloc(sizeof(*list));
  list->files = (struct file**) malloc(LISTSIZE * sizeof(struct file*));
  for(int i = 0; i < LISTSIZE; i++){
    list->files[i] = NULL;
  }
  list->look_from = 0;
  return list;
}

int insert(struct file_list* list, struct file* file){
  for (int i = list->look_from; i < LISTSIZE; i++){
    if (list->files[i] == NULL) {
      list->files[i] = file;
      list->look_from = i+1;
      return i + OFFSET;
    }
  }
  return -1;
}

void remove(struct file_list* list, int fd){
  if (fd < 2 || fd > 129) return;
  file_close(list->files[fd - OFFSET]);
  list->files[fd - OFFSET] = NULL;
  list->look_from = fd - OFFSET;
}

struct file *get(struct file_list* list, int fd){
  if (fd < 2 || fd > 129) return NULL;
  return list->files[fd - OFFSET];
}

void free_filelist(struct file_list *filelist){
  for (int i = 0; i < LISTSIZE; i++){
    remove(filelist, (i + OFFSET));
  }
  free(filelist->files);
  free(filelist);
}
