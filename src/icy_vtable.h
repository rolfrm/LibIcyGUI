// This file is auto generated by table_compiler
#include "icydb.h"
typedef struct _icy_vtable{
  char ** column_names;
  char ** column_types;
  size_t count;
  const bool is_multi_table;
  const int column_count;
  int (*cmp) (const icy_control * k1, const icy_control * k2);
  const size_t sizes[2];

  icy_control * control;
  method_id * methods;
  icy_mem * control_area;
  icy_mem * methods_area;
}icy_vtable;

icy_vtable * icy_vtable_create(const char * optional_name);
void icy_vtable_set(icy_vtable * table, icy_control control, method_id methods);
void icy_vtable_insert(icy_vtable * table, icy_control * control, method_id * methods, size_t count);
void icy_vtable_lookup(icy_vtable * table, icy_control * keys, size_t * out_indexes, size_t count);
void icy_vtable_remove(icy_vtable * table, icy_control * keys, size_t key_count);
void icy_vtable_clear(icy_vtable * table);
void icy_vtable_unset(icy_vtable * table, icy_control key);
bool icy_vtable_try_get(icy_vtable * table, icy_control * control, method_id * methods);
void icy_vtable_print(icy_vtable * table);
size_t icy_vtable_iter(icy_vtable * table, icy_control * keys, size_t keycnt, icy_control * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator);
