#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <getopt.h>
#include <dirent.h>
#include <regex.h>

#define TABS(n) for(int i = 0; i < (n); ++i) printf("\t");

int show_pid;
int is_sort;
static struct option long_options[] = {
  {"show-pids", 0, 0, 'p'},
  {"numeric-sort", 0, 0, 'n'},
  {"version", 0, 0, 'V'},
};

struct ps{
  int pid;
  char name[64];
  int pps, left, right; //left is brother, right is child
}ps_pool[512];
int pool_size = 1;

int option_index;

int is_num(char * str){
  while(*str != '\0'){
    if(*str < '0' || *str > '9')
      return 0;
    str++;
  }
  return 1;
}

void insert_child(int parent, int child){
  if(ps_pool[parent].right == 0){
    ps_pool[parent].right = child;
  }else{
    int p = ps_pool[parent].right;
    while(ps_pool[p].left){
      p = ps_pool[p].left;
    }
    ps_pool[p].left = child; 
  }
}

void store_ps(char * dir_name){
  ps_pool[pool_size].pid = atoi(dir_name);

  char addr[64] = "/proc/";
  strcat(addr, dir_name);
  strcat(addr, "/status");

  FILE * fp = fopen(addr, "r");
  char buffer[128], name[32], value[32];
  if(fp){
    while(1){
      fgets(buffer, 128, fp);
      sscanf(buffer,"%s %s", name, value);
      if(strcmp(name, "Name:") == 0){
        strcpy(ps_pool[pool_size].name, value);
      }
      else if(strcmp(name, "PPid:") == 0){
        int ppid = atoi(value);
        for(int i = 1; i < pool_size; ++i ){
          if(ps_pool[i].pid == ppid){
            insert_child(i , pool_size);
            break;
          }
        }
       break;   //message done; 
      }
    }
    
  }else{
    fprintf(stderr, "dir error");
    assert(fp);
  }
  
  pool_size ++;
  
}

void get_files(){
  DIR *d;
  struct dirent *dir;
  d = opendir("/proc");
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      if(is_num(dir->d_name)){
        store_ps(dir->d_name);
      }
    }
    closedir(d);
  }
}

void print_tree(int p,int depth){
  if(p == 0) return;
  for(int i = 0 ; i < depth; ++i){
    printf("\t");
  }
  if(show_pid){
    printf("%s(%d)\n", ps_pool[p].name, ps_pool[p].pid);
  } else {
    printf("%s\n",ps_pool[p].name);
  }
  print_tree(ps_pool[p].right, depth+1);
  print_tree(ps_pool[p].left, depth);
}

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  int c;
  while((c = getopt_long(argc, argv, "pnV", long_options, &option_index))!= -1 ){
    switch(c){
    case 'p':
      show_pid = 1;
      break;
    case 'n':
      is_sort = 1;
      break;
    case 'V':
      //showVersion();
      exit(0);
    default:
    }
  }

  get_files();
  print_tree(1, 0);
  return 0;
}
