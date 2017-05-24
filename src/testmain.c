#include <stdio.h>


#include <icydb.h>
#include <GLFW/glfw3.h>
typedef struct{
  unsigned int Id;
}icy_control;


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

void render_window_base(icy_control win){
  printf("Rendering base class%i\n", win.Id);
}

void render_window(icy_control win){
  printf("Rendering %i\n", win.Id);
}

int main(){
  icy_vector * iv = icy_vector_create("hello", sizeof(int));
  icy_vector_destroy(&iv);
  window_state_table = window_state_create("window");
  base_controls = base_control_create("base-controls");
  icy_control window_base = {6};
  icy_control window = {5};
  render_methods = render_method_create(NULL);
  render_method_set(render_methods, window, render_window);
  render_method_set(render_methods, window_base, render_window_base);
  base_control_set(base_controls, window, window_base);
  icy_control bs;
  base_control_try_get(base_controls, &window, &bs);
  printf("Base class: %i\n", bs.Id);
  
  //glfwInit();
  return 0;
}
