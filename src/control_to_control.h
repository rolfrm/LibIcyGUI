// This file is auto generated by icy-table.
#include "icydb.h"
typedef struct _control_to_control{
  char ** column_names;
  char ** column_types;
  size_t count;
  const bool is_multi_table;
  const int column_count;
  int (*cmp) (const icy_control * k1, const icy_control * k2);
  const size_t sizes[2];

  icy_control * key;
  icy_control * value;
  icy_mem * key_area;
  icy_mem * value_area;
}control_to_control;

control_to_control * control_to_control_create(const char * optional_name);
void control_to_control_set(control_to_control * table, icy_control key, icy_control value);
void control_to_control_insert(control_to_control * table, icy_control * key, icy_control * value, size_t count);
void control_to_control_lookup(control_to_control * table, icy_control * keys, size_t * out_indexes, size_t count);
void control_to_control_remove(control_to_control * table, icy_control * keys, size_t key_count);
void control_to_control_clear(control_to_control * table);
void control_to_control_unset(control_to_control * table, icy_control key);
bool control_to_control_try_get(control_to_control * table, icy_control * key, icy_control * value);
void control_to_control_print(control_to_control * table);
size_t control_to_control_iter(control_to_control * table, icy_control * keys, size_t keycnt, icy_control * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator);
