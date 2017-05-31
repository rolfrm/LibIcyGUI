// This file is auto generated by icy-table.
#include "icydb.h"
typedef struct _size_t_set{
  char ** column_names;
  char ** column_types;
  size_t count;
  const bool is_multi_table;
  const int column_count;
  int (*cmp) (const size_t * k1, const size_t * k2);
  const size_t sizes[1];

  size_t * key;
  icy_mem * key_area;
}size_t_set;

size_t_set * size_t_set_create(const char * optional_name);
void size_t_set_set(size_t_set * table, size_t key);
void size_t_set_insert(size_t_set * table, size_t * key, size_t count);
void size_t_set_lookup(size_t_set * table, size_t * keys, size_t * out_indexes, size_t count);
void size_t_set_remove(size_t_set * table, size_t * keys, size_t key_count);
void size_t_set_clear(size_t_set * table);
void size_t_set_unset(size_t_set * table, size_t key);
bool size_t_set_try_get(size_t_set * table, size_t * key);
void size_t_set_print(size_t_set * table);
size_t size_t_set_iter(size_t_set * table, size_t * keys, size_t keycnt, size_t * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator);
