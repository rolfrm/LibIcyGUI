// This file is auto generated by icy-table.
#include "icydb.h"
typedef struct _icy_method_table{
  char ** column_names;
  char ** column_types;
  size_t count;
  const bool is_multi_table;
  const int column_count;
  int (*cmp) (const icy_control * k1, const icy_control * k2);
  const size_t sizes[2];

  icy_control * id;
  method * procedure;
  icy_mem * id_area;
  icy_mem * procedure_area;
  base_control * proxy;
}icy_method_table;

icy_method_table * icy_method_table_create(const char * optional_name, const char * proxy_name);
void icy_method_table_set(icy_method_table * table, icy_control id, method procedure);
void icy_method_table_insert(icy_method_table * table, icy_control * id, method * procedure, size_t count);
void icy_method_table_lookup(icy_method_table * table, icy_control * keys, size_t * out_indexes, size_t count);
void icy_method_table_remove(icy_method_table * table, icy_control * keys, size_t key_count);
void icy_method_table_clear(icy_method_table * table);
void icy_method_table_unset(icy_method_table * table, icy_control key);
bool icy_method_table_try_get(icy_method_table * table, icy_control * id, method * procedure);
void icy_method_table_print(icy_method_table * table);
size_t icy_method_table_iter(icy_method_table * table, icy_control * keys, size_t keycnt, icy_control * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator);
