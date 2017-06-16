// This file is auto generated by icy-table.
#include "icydb.h"
typedef struct _int_to_control{
  char ** column_names;
  char ** column_types;
  size_t count;
  const bool is_multi_table;
  const int column_count;
  int (*cmp) (const int * k1, const int * k2);
  const size_t sizes[2];

  int * key;
  icy_control * value;
  icy_mem * key_area;
  icy_mem * value_area;
}int_to_control;

int_to_control * int_to_control_create(const char * optional_name);
void int_to_control_set(int_to_control * table, int key, icy_control value);
void int_to_control_insert(int_to_control * table, int * key, icy_control * value, size_t count);
void int_to_control_lookup(int_to_control * table, int * keys, size_t * out_indexes, size_t count);
void int_to_control_remove(int_to_control * table, int * keys, size_t key_count);
void int_to_control_clear(int_to_control * table);
void int_to_control_unset(int_to_control * table, int key);
bool int_to_control_try_get(int_to_control * table, int * key, icy_control * value);
void int_to_control_print(int_to_control * table);
size_t int_to_control_iter(int_to_control * table, int * keys, size_t keycnt, int * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator);
