#include <stdio.h>
#include <icydb.h>
#include "icygui.h"
#include <GLFW/glfw3.h>
#include "log.h"
#include "utils.h"

#include "icy_oop.h"

#include "window_state.h"
#include "window_state.c"

window_state * window_state_table;
icy_vtable * render_window_m;
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
icy_method_table * _some_test_method;
  

void test_methods(){
  
  icy_control window_base = { icy_intern("test/winbase") };
  icy_control window = {icy_intern("test/window")};
  icy_control window2 = {icy_intern("test/window2")};

  base_control_set(base_controls, window, window_base);
  base_control_set(base_controls, window2, window_base);
  
  set_method(window_base, _some_test_method, (void *) test_m1);

  set_method(window, _some_test_method,  (void *) test_m2);
  set_method(window2, _some_test_method,  (void *) test_m3);

  icy_control proxy1 = { icy_intern("test/proc1") };
  method m4 = (void*) test_m4;
  icy_method_table_insert(_some_test_method, &proxy1, &m4, 1);
  
  base_control_insert(_some_test_method->proxy, &window2, &proxy1, 1);
  base_control_insert(base_controls, &window2, &window_base, 1);

  //set_method2(_some_test_method, window, k1, test_m4);
  logd("Render window:\n");
  call_method(_some_test_method, window);
  logd("render window 2:\n");
  call_method(_some_test_method, window2);

  // allocate 3 child elements to the window.
  icy_control button1 = control_get_sub(window, 0);
  
  logd("Got button 1: %i\n", button1);

  icy_control button2 = control_get_sub(window, 1);
  logd("Got button 2: %i\n", button2);

  icy_control button3 = control_get_sub(window, 2);
  logd("Got button 3: %i\n", button3);
  ASSERT(button1.id != button2.id && button2.id != button3.id);

  // create the text child element, that is the sub control of the button.
  icy_control button1_text = control_get_sub(button1, 0);
  logd("Got button 1 text: %i\n", button1_text);
  //icy_text_set(button1_text, "hello world!");
  
  //icy_button_load(button1);
  
}


int main(){

  _some_test_method = icy_method_table_create(NULL, "some_test.vtable");
  ((bool *) &_some_test_method->is_multi_table)[0] = true;
  
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
