#include <stdio.h>
#include <icydb.h>
#include "icygui.h"
#include <GLFW/glfw3.h>
#include "log.h"
#include "utils.h"

#include "window_state.h"
#include "window_state.c"
#include "base_control.h"
#include "base_control.c"

window_state * window_state_table;
base_control * base_controls; 
typedef void (* render_control)(icy_control control);
#include "render_method.h"
#include "render_method.c"

render_method * render_methods;

typedef struct _method_id{
  icy_index id;
}method_id;

typedef void (* method)(icy_control control, ...);
#include "icy_vtable.h"
#include "icy_vtable.c"

#include "icy_method_table.h"
#include "icy_method_table.c"

icy_vtable * render_window_m;
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

bool test_string_interning(){

  const char * strings[] = {"--__--", "µasdµ", "Hello?", "Hello? 2", "asd", "dsa", "asddsa", "dsaasd", "12", "123", "-123", "           ddd        ", "              aaa          "};
  size_t ids[array_count(strings)];
  for(size_t i = 0; i < array_count(strings); i++){
    ids[i] = icy_intern(strings[i]);
    logd("ID: %i\n", ids[i]);
  }
  for(size_t i = 1; i < array_count(strings) - 1; i++){
    ASSERT(ids[i] == icy_intern(strings[i]));
    ASSERT(ids[i + 1] != icy_intern(strings[i]));
    ASSERT(ids[i - 1] != icy_intern(strings[i]));
    char buffer[30] = {0};
    size_t t = icy_intern_get(ids[i], buffer, sizeof(buffer));
    ASSERT(t == strlen(buffer));
    ASSERT(strcmp(buffer, strings[i]) == 0);
    logd("'%s'\n", buffer);
    char buffer2[5] = {0};
    size_t t2 = icy_intern_get(ids[i], buffer2, sizeof(buffer2));
    ASSERT(t2 <= 5);
    for(size_t j = 0; j < t2; j++)
      ASSERT(buffer[j] == buffer2[j]);
    ASSERT(icy_intern_get(0xFFFFFF1, buffer, sizeof(buffer)) == 0);
    
  }
    return true;
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

void test_m1(icy_control thing){
  logd("m1 called %i\n", thing.id);
}

void test_m2(icy_control self){
  logd("m2 called %i\n", self.id);
  call_next(self);
}

void test_m3(icy_control self){
  logd("m3 called %i\n", self);
  call_next(self);
}

void test_m4(icy_control self){
  logd("m4 called %i\n", self);
  call_next(self);
}
icy_method_table * _render_method;
  
void set_method(icy_control id, icy_method_table * mt, method m1){
  ASSERT(mt != NULL);
  icy_method_table_insert(mt, &id, &m1, 1);
}
/*
void set_method2(icy_method_table * mt, icy_control control, method_id id, method m){
  icy_vtable_insert(mt->vtable, &control, &id);
  icy_method_table_insert(mt, &id, &m, 1);
  }*/

void test_methods(){
  
  icy_control window_base = {icy_intern("test/winbase")};
  icy_control window = {icy_intern("test/window")};
  icy_control window2 = {icy_intern("test/window2")};

  base_control_set(base_controls, window, window_base);
  base_control_set(base_controls, window2, window_base);
  
  set_method(window_base, _render_method, (void *) test_m1);

  set_method(window, _render_method,  (void *) test_m2);
  set_method(window2, _render_method,  (void *) test_m3);

  icy_control proxy1 = { icy_intern("test/proc1") };
  method m4 = (void*) test_m4;
  icy_method_table_insert(_render_method, &proxy1, &m4, 1);
  
  base_control_insert(_render_method->proxy, &window2, &proxy1, 1);
  base_control_insert(base_controls, &window2, &window_base, 1);

  //set_method2(_render_method, window, k1, test_m4);
  logd("Render window:\n");
  call_method(_render_method, window);
  logd("render window 2:\n");
  call_method(_render_method, window2);
}


int main(){

  _render_method = icy_method_table_create(NULL);
  _render_method->proxy = base_control_create("render.vtable");
  ((bool *) &_render_method->is_multi_table)[0] = true;
  
  test_string_interning();
  icy_vector * iv = icy_vector_create("hello", sizeof(int));
  icy_vector_destroy(&iv); 
  window_state_table = window_state_create("window");
  base_controls = base_control_create("base-controls");
  
  test_methods();

  size_t a = icy_alloc_id();
  size_t b = icy_alloc_id();
  size_t d = icy_alloc_id();
  icy_free_id(a);
  size_t c = icy_alloc_id();
  ASSERT(c == a);
  ASSERT(b != a);
  logd("%i %i %i %i\n", a, b, c, d);
  icy_free_id(b);
  icy_free_id(d);
  icy_free_id(c);
  //glfwInit();
  return 0;
}
