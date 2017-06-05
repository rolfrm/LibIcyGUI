typedef void (* method)(icy_control control, ...);

typedef struct _method_id{
  icy_index id;
}method_id;

#include "base_control.h"
#include "icy_vtable.h"
#include "icy_method_table.h"


method get_method(icy_control class_id, icy_vtable * _method_lookup);
void call_method(icy_method_table * mt, icy_control item);
void call_next(icy_control thing);
void set_method(icy_control id, icy_method_table * mt, method m1);
icy_control control_get_sub(icy_control parent, size_t sub_nr);
extern base_control * base_controls; 
extern base_control * child_controls;
