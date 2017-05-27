// This file is auto generated by table_compiler
#include "icydb.h"
typedef struct _render_method{
  char ** column_names;
  char ** column_types;
  size_t count;
  const bool is_multi_table;
  const int column_count;
  int (*cmp) (const icy_control * k1, const icy_control * k2);
  const size_t sizes[2];

  icy_control * control;
  render_control * render;
  icy_mem * control_area;
  icy_mem * render_area;
}render_method;

render_method * render_method_create(const char * optional_name);
void render_method_set(render_method * table, icy_control control, render_control render);
void render_method_insert(render_method * table, icy_control * control, render_control * render, size_t count);
void render_method_lookup(render_method * table, icy_control * keys, size_t * out_indexes, size_t count);
void render_method_remove(render_method * table, icy_control * keys, size_t key_count);
void render_method_clear(render_method * table);
void render_method_unset(render_method * table, icy_control key);
bool render_method_try_get(render_method * table, icy_control * control, render_control * render);
void render_method_print(render_method * table);
size_t render_method_iter(render_method * table, icy_control * keys, size_t keycnt, icy_control * optional_keys_out, size_t * indexes, size_t cnt, size_t * iterator);