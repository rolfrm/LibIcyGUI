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

static icy_control method_implementor = {0};
void render_thing2(icy_control impl, icy_control control){
  while(true){
    render_control rm1 = NULL;
    render_method_try_get(render_methods, &impl, &rm1);
    if(rm1 != NULL){
      icy_control prev = method_implementor;
      method_implementor = impl;
      rm1(control);
      method_implementor = prev;
      break;
    }
    if(!base_control_try_get(base_controls, &impl, &impl))
      break;
  }
}

void render_thing(icy_control control){
  render_thing2(control, control);
}

void render_next(icy_control control){
  icy_control impl = method_implementor;
  
  if(base_control_try_get(base_controls, &impl, &impl))
    render_thing2(impl, control);
}

void render_window_base(icy_control win){
  printf("Rendering base class %i\n", win.Id);
}

void render_window(icy_control win){
  printf("Rendering %i\n", win.Id);
  render_next(win);
}

typedef struct _method_id{
  icy_index id;
}method_id;

typedef void (* method)(icy_control control, ...);
#include "icy_vtable.h"
#include "icy_vtable.c"

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

int main(){
  
  
  test_string_interning();
  icy_vector * iv = icy_vector_create("hello", sizeof(int));
  icy_vector_destroy(&iv);
  window_state_table = window_state_create("window");
  base_controls = base_control_create("base-controls");
  icy_control window_base = {6};
  icy_control window = {5};
  render_methods = render_method_create(NULL);

  render_method_set(render_methods, window_base, render_window_base);
  render_method_set(render_methods, window, render_window);
  base_control_set(base_controls, window, window_base);
  
  render_thing(window);
  
  //glfwInit();
  return 0;
}
