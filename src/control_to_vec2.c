// This file is auto generated by icy-table.
#ifndef TABLE_COMPILER_INDEX
#define TABLE_COMPILER_INDEX
#define array_element_size(array) sizeof(array[0])
#define array_count(array) (sizeof(array)/array_element_size(array))
#include "icydb.h"
#include <stdlib.h>
#endif


control_to_vec2 * control_to_vec2_create(const char * optional_name){
  static const char * const column_names[] = {(char *)"key", (char *)"value"};
  static const char * const column_types[] = {"icy_control", "vec2"};
  control_to_vec2 * instance = calloc(sizeof(control_to_vec2), 1);
  icy_table_init((icy_table * )instance, optional_name, 2, (unsigned int[]){sizeof(icy_control), sizeof(vec2)}, (char *[]){(char *)"key", (char *)"value"});
  instance->column_names = (char **)column_names;
  instance->column_types = (char **)column_types;
  return instance;
}

void control_to_vec2_insert(control_to_vec2 * table, icy_control * key, vec2 * value, size_t count){
  void * array[] = {(void* )key, (void* )value};
  icy_table_inserts((icy_table *) table, array, count);
}

void control_to_vec2_set(control_to_vec2 * table, icy_control key, vec2 value){
  void * array[] = {(void* )&key, (void* )&value};
  icy_table_inserts((icy_table *) table, array, 1);
}

void control_to_vec2_lookup(control_to_vec2 * table, icy_control * keys, size_t * out_indexes, size_t count){
  icy_table_finds((icy_table *) table, keys, out_indexes, count);
}

void control_to_vec2_remove(control_to_vec2 * table, icy_control * keys, size_t key_count){
  size_t indexes[key_count];
  size_t index = 0;
  size_t cnt = 0;
  while(0 < (cnt = icy_table_iter((icy_table *) table, keys, key_count, NULL, indexes, array_count(indexes), &index))){
    icy_table_remove_indexes((icy_table *) table, indexes, cnt);
    index = 0;
  }
}

void control_to_vec2_clear(control_to_vec2 * table){
  icy_table_clear((icy_table *) table);
}

void control_to_vec2_unset(control_to_vec2 * table, icy_control key){
  control_to_vec2_remove(table, &key, 1);
}

bool control_to_vec2_try_get(control_to_vec2 * table, icy_control * key, vec2 * value){
  void * array[] = {(void* )key, (void* )value};
  void * column_pointers[] = {(void *)table->key, (void *)table->value};
  size_t __index = 0;
  icy_table_finds((icy_table *) table, array[0], &__index, 1);
  if(__index == 0) return false;
  unsigned int sizes[] = {sizeof(icy_control), sizeof(vec2)};
  for(int i = 1; i < 2; i++){
    if(array[i] != NULL)
      memcpy(array[i], column_pointers[i] + __index * sizes[i], sizes[i]); 
  }
  return true;
}

void control_to_vec2_print(control_to_vec2 * table){
  icy_table_print((icy_table *) table);
}

size_t control_to_vec2_iter(control_to_vec2 * table, icy_control * keys, size_t keycnt, icy_control * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator){
  return icy_table_iter((icy_table *) table, keys, keycnt, optional_keys_out, indexes, cnt, iterator);

}
