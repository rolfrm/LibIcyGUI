#include <stdio.h>
#include <icydb.h>
#include "icygui.h"
#include "log.h"
#include "utils.h"

#include "icy_oop.h"

#include "base_control.c"

base_control * base_controls; 



#include "icy_vtable.c"
#include "icy_method_table.c"

icy_vector * method_lookup;

method get_method(icy_control class_id, icy_vtable * _method_lookup){
  method_id m;
  if(icy_vtable_try_get(_method_lookup, &class_id, &m)){
    method * proc = icy_vector_lookup(method_lookup, m.id);
    ASSERT(proc != NULL);
    return *proc;
  }
  return NULL;
}


icy_control current_id;
icy_method_table * current_method_table;
bool was_proxy = false;
void call_method2(icy_method_table * mt, icy_control item, icy_control realitem){
  method m;
  {
    icy_control proxy = item;
    if(base_control_try_get(mt->proxy, &proxy, &proxy) && !was_proxy){
      if(icy_method_table_try_get(mt, &proxy, &m)){
	icy_control previtem = proxy;
	icy_method_table * prev_mt = current_method_table;
	bool prev_was_proxy = was_proxy;
	was_proxy = true;
	current_id = item;
	current_method_table = mt;
	m(realitem);
	current_id = previtem;
	current_method_table = prev_mt;
	was_proxy = prev_was_proxy;
	return;
      }
    }

  }
  
  icy_control baseitem = item;
  if(icy_method_table_try_get(mt, &item, &m)){
    icy_control previtem = baseitem;
    icy_method_table * prev_mt = current_method_table;
    current_id = baseitem;
    current_method_table = mt;
    bool prev_was_proxy = was_proxy;
    was_proxy = false;
    m(realitem);
    current_id = previtem;
    current_method_table = prev_mt;
    was_proxy = prev_was_proxy;
    return;
  }
  
  while(base_control_try_get(base_controls, &baseitem, &baseitem)) {
    if(icy_method_table_try_get(mt, &baseitem, &m)){
      icy_control previtem = baseitem;
      icy_method_table * prev_mt = current_method_table;
      current_id = baseitem;
      current_method_table = mt;
      bool prev_was_proxy = was_proxy;
      was_proxy = false;
      m(realitem);
      current_id = previtem;
      current_method_table = prev_mt;
      was_proxy = prev_was_proxy;
      return;
    }
  }  
}

void call_method(icy_method_table * mt, icy_control item){
  call_method2(mt, item, item);
}

void call_next(icy_control thing){
  icy_control baseitem = thing;
  if(was_proxy || (base_control_try_get(base_controls, &baseitem, &baseitem) && baseitem.id != 0))
    call_method2(current_method_table, baseitem, thing);
}

void set_method(icy_control id, icy_method_table * mt, method m1){
  ASSERT(mt != NULL);
  icy_method_table_insert(mt, &id, &m1, 1);
}

base_control * child_controls;

icy_control control_get_sub(icy_control parent, size_t sub_nr){
  icy_oop_init();
  size_t keys[sub_nr + 1];
  memset(keys, 0, sizeof(keys));
  { // If element has not been allocated. alloc.
    base_control_iter(child_controls, &parent, 1, NULL, keys, array_count(keys), NULL);
    int changed = 0;
    for(size_t i = 0; i < sub_nr + 1; i++){
      if(keys[i] == 0){
	icy_control def[sub_nr - i + 1];
	icy_control p[sub_nr - i + 1];
	for(size_t j = i; j < sub_nr + 1; j++)
	  p[j - i] = parent;
	memset(def, 0, sizeof(def));
	base_control_insert(child_controls, p, def, sub_nr - i + 1);
	changed = 1;
	break;
      }
    }
    if(changed)
      base_control_iter(child_controls, &parent, 1, NULL, keys, array_count(keys), NULL);
  }
  size_t idx = keys[sub_nr];
  if(child_controls->super[idx].id == 0)
    child_controls->super[idx].id = icy_alloc_id();
  
  return child_controls->super[idx];
}

void icy_oop_init(){
  if(child_controls == NULL){
    child_controls = base_control_create("child-controls");
    ((bool *)&child_controls->is_multi_table)[0] = true;
  }
}
