#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "icygui.h"
#include <icydb.h>
#include "log.h"
#include "utils.h"
#include "size_t_set.h"
#include "size_t_set.c"

typedef struct{
  char ** column_names;
  char ** column_types;
  size_t count;
  const bool is_multi_table;
  const int column_count;
  int (*cmp) (const icy_control * k1, const icy_control * k2);
  const size_t sizes[2];

  void * strings;
  size_t * id;
  icy_mem * strings_area;
  icy_mem * id_area;
  
}intern_string_table;

static int cmp_n = 0;

int compareN(const void * k1, const void * k2){
  return memcmp(k1, k2, cmp_n);
}

intern_string_table * intern_string_table_create(const char * optional_name, size_t len){
  static const char * const column_names[] = {(char *)"strings", (char *)"id"};
  static const char * const column_types[] = {"void *", "size_t"};
  intern_string_table * instance = calloc(sizeof(intern_string_table), 1);
  icy_table_init((icy_table * )instance, optional_name, 2, (unsigned int[]){len, sizeof(size_t)}, (char *[]){(char *)"strings", (char *)"id"});
  instance->column_names = (char **)column_names;
  instance->column_types = (char **)column_types;
  instance->cmp = (void *)compareN;
  return instance;
}

icy_vector * files = NULL;

typedef struct{
  size_t next_id;
}intern_string_data;

intern_string_data * intern_data;

ICY_HIDDEN void init_if_needed(){
  if(files == NULL){
    files = icy_vector_create(NULL, sizeof(icy_table *));
    static icy_mem * data_area = NULL;
    if(data_area == NULL)
      data_area = icy_mem_create("intern.string.data");
    icy_mem_realloc(data_area, sizeof(intern_string_data));
    intern_data = data_area->ptr;
    if(intern_data->next_id == 0)
      intern_data->next_id = 1;
  }
}

size_t get_unique_id(){
  return intern_data->next_id++;
}
size_t_set * free_ids = NULL;
size_t icy_alloc_id(){
  if(free_ids == NULL){
    free_ids = size_t_set_create("free_ids");
  }
  if(free_ids->count > 0){
    size_t id = free_ids->key[free_ids->count];
    size_t_set_remove(free_ids, &id, 1);
    return id;
  }
  return get_unique_id();
}

void icy_free_id(size_t s){
  if(free_ids == NULL){
    free_ids = size_t_set_create("free_ids");
  }
  size_t_set_set(free_ids, s);
}

intern_string_table * get_string_table_for_size(unsigned int s){
  init_if_needed();
  void * pt = NULL;
  size_t cnt = icy_vector_count(files);
  if(cnt <= s + 5)
    icy_vector_alloc_sequence(files, s - cnt + 5);
  pt = icy_vector_lookup(files, (icy_index){s+3});
  intern_string_table ** tab = pt;
  if(*tab == NULL){
    char buffer[30];
    sprintf(buffer, "intern/string.table.%i", s);
    *tab = intern_string_table_create(buffer, s);    
  }
  return *tab;
}

size_t icy_intern(const char * string){
  ASSERT(string != NULL);
  unsigned int len = strlen(string);
  intern_string_table * table = get_string_table_for_size(len);
  size_t index = 0;
  cmp_n = len;
  icy_table_finds((icy_table *) table, (void *)string, &index, 1);
  if(index == 0){
    size_t id = get_unique_id();
    void * values[] = {(void *)string, &id}; 
    icy_table_inserts((icy_table *) table, values, 1);
    return id;
  }else{
    return table->id[index];
  }
}

size_t icy_intern_get(size_t id, char * buffer, size_t size){
  size_t c = 0;
  intern_string_table ** tables = icy_vector_all(files, &c);
  
  for(size_t i = 0; i < c; i++){
    size_t ssize = i - 3 + 1;
    intern_string_table * table = tables[i];
    if(table == NULL) continue;
    for(size_t j = 0; j < table->count; j++){
      if(table->id[j + 1] == id){
	void * ptr = table->strings + (1 + j) * ( ssize);
	if(buffer == NULL)
	  return ssize;
	return mempcpy(buffer, ptr, MIN(ssize, size)) - (void *) buffer; 
      }
    }	  
  }
  return 0;
}

