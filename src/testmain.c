#include <stdio.h>
#include <icydb.h>
#include "icygui.h"
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>


#include "log.h"
#include "utils.h"
#include "linmath.h"
#include "icy_oop.h"
#include "fileio.h"
#include "mem.h"
#include "shader_utils.h"
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

icy_method_table * render;
#include "void_to_control.h"
#include "void_to_control.c"
#include "control_to_control.h"
#include "control_to_control.c"
#include "control_to_bool.h"
#include "control_to_bool.c"
#include "control_to_void.h"
#include "control_to_void.c"
#include "control_to_int.h"
#include "control_to_int.c"
#include "window_state.h"
#include "window_state.c"

window_state * window_state_table;

GLFWwindow * load_window(icy_control id){
  window_state * w = window_state_table;
  size_t index = 0;
  window_state_lookup(window_state_table, &id, &index, 1);
  
  if(index == 0){
    int width = 640, height = 640, x = -1, y = -1;
    window_state_insert(w, &id, &width, &height, &x, &y, 1); 
    //sprintf(w.title, "%s", "Test Window");
    window_state_lookup(w, &id, &index, 1);
  }
  if(w->height[index] <= 0) w->height[index] = 200;
  if(w->width[index] <= 0) w->width[index] = 200;
  static GLFWwindow * ctx = NULL;
  logd("Window size:  %s %i %i\n", "test title", w->width[index], w->height[index]);
  glfwWindowHint(GLFW_SAMPLES, 16);
  glfwWindowHint(GLFW_DEPTH_BITS, 32);
  GLFWwindow * window = glfwCreateWindow(w->width[index], w->height[index], "test title", NULL, ctx);
  ASSERT(window != NULL);
  if(ctx == NULL){
    ctx = window;
    glfwMakeContextCurrent(window);
    glewInit();
  }
  if(w->x[index] > 0 && w->y[index] > 0)
    glfwSetWindowPos(window, w->x[index], w->y[index]);
  glfwSetWindowSize(window, w->width[index], w->height[index]);

  /*glfwSetWindowPosCallback(window, window_pos_callback);
  glfwSetWindowSizeCallback(window, window_size_callback);
  glfwSetCursorPosCallback(window, cursor_pos_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetWindowCloseCallback(window, window_close_callback);
  glfwSetCharCallback(window, char_callback);*/
  glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
  return window;
}

void_to_control * window_lookup;
control_to_void * glfw_window_lookup;
control_to_int * background;

void set_color_rgba(control_to_int * color_table, icy_control control, float r, float g, float b, float a){
  union{
    struct {
      unsigned char r, g, b, a;
    };
    int value;
  }color;
  r = CLAMP(r, 0.0, 255.0);
  g = CLAMP(g, 0.0, 255.0);
  b = CLAMP(b, 0.0, 255.0);
  a = CLAMP(a, 0.0, 255.0);
  
  color.r = r * 255;
  color.g = g * 255;
  color.b = b * 255;
  color.a = a * 255;
  control_to_int_set(color_table, control, color.value);
}

void set_color_rgb(control_to_int * color_table, icy_control control, float r, float g, float b){
  set_color_rgba(color_table, control, r, g, b, 1);
}

void render_window(icy_control window){

  GLFWwindow * win = NULL;
  control_to_void_try_get(glfw_window_lookup, &window, (void **) &win);
  if(win == NULL){
    win = load_window(window);
    void_to_control_set(window_lookup, win, window);
    control_to_void_set(glfw_window_lookup, window, win);
  }
  window_state * w = window_state_table;
  size_t index = 0;
  window_state_lookup(w, &window, &index, 1);
  ASSERT(index > 0);
  bool last = true;
  //thickness margin = get_margin(window_id);
  //margin.left += 0.05f;
  //GLFWwindow * win = find_glfw_window(window_id);
  glfwSetWindowTitle(win, "test window");
  glfwGetWindowSize(win, w->width + index, w->height + index);
  glfwMakeContextCurrent(win);
  glViewport(0, 0, w->width[index], w->height[index]);
  //window_size = vec2_new(w->width[index], w->height[index]);
 
  //vec3 color = get_color(window_id);

  int color = 0xFFFFFFFF;
  control_to_int_try_get(background, &window, &color);
  int r = color & 0xFF;
  int g = (color >> 8) & 0xFF;
  int b = (color >> 16) & 0xFF;
  int a = (color >> 24) & 0xFF;
  float _r = 1.0 / 256.0;
  glClearColor(_r * r, _r * g, _r * b,  _r * a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //shared_offset = vec2_new(margin.left, margin.up);
  //shared_size = vec2_new(win.width - margin.left - margin.right, win.height - margin.up - margin.down);
  size_t cindex = 0;
  size_t child_index = 0;
  while(base_control_iter(child_controls, &window, 1, NULL, &child_index, 1, &cindex)){
    ASSERT(child_index != 0);
    call_method(render, child_controls->super[child_index]);
    child_index = 0;
  }
  
  bool vsync_enabled = true;
  if(vsync_enabled)
    glfwSwapInterval(last ? 1 : 0);
  else
    glfwSwapInterval(0);
  glfwSwapBuffers(win);
}
vec2 window_size;
void render_rect(vec4 color, vec2 offset, vec2 size, int tex, vec2 uv_offset, vec2 uv_size){
  static int initialized = false;
  static int shader = -1;
  static int color_loc;
  static int offset_loc;
  static int size_loc;
  static int window_size_loc;
  //static int tex_loc;
  static int mode_loc;
  static int uv_offset_loc, uv_size_loc;
  if(!initialized){
    char * vs = read_file_to_string("rect_shader.vs");
    char * fs = read_file_to_string("rect_shader.fs");
    shader = load_simple_shader(vs, strlen(vs), fs, strlen(fs));
    dealloc(vs);
    dealloc(fs);
    logd("Shader: %i\n", shader);
    initialized = true;
    color_loc = glGetUniformLocation(shader, "color");
    offset_loc = glGetUniformLocation(shader, "offset");
    size_loc = glGetUniformLocation(shader, "size");
    window_size_loc = glGetUniformLocation(shader, "window_size");
    //tex_loc = glGetUniformLocation(shader, "tex");
    mode_loc = glGetUniformLocation(shader, "mode");
    uv_offset_loc = glGetUniformLocation(shader, "uv_offset");
    uv_size_loc = glGetUniformLocation(shader, "uv_size");
  }
  glUseProgram(shader);
  if(tex != 0){
    glUniform1i(mode_loc, 1);
    glUniform2f(uv_offset_loc, uv_offset.x, uv_offset.y);
    glUniform2f(uv_size_loc, uv_size.x, uv_size.y);
    //glUniform1i(tex_loc, 0);
    glBindTexture(GL_TEXTURE_2D, tex);
  }
  else
    glUniform1i(mode_loc, 0);
  
  glUniform4f(color_loc, color.x, color.y, color.z, color.w);
  glUniform2f(offset_loc, offset.x, offset.y);
  glUniform2f(size_loc, size.x, size.y);
  glUniform2f(window_size_loc, window_size.x, window_size.y);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

#include <math.h>
void demo_window(){
  icy_control window = { icy_intern("test2/window")};
  set_method(window, render, (void *) render_window);
  set_color_rgb(background, window, 1.0, 0.6, 0.2);
  while(true){
    call_method(render, window);
  }
}


int main(){

  _some_test_method = icy_method_table_create(NULL, "some_test.vtable");
  ((bool *) &_some_test_method->is_multi_table)[0] = true;
  
  test_string_interning();
  icy_vector * iv = icy_vector_create("hello", sizeof(int));
  icy_vector_destroy(&iv); 
  window_state_table = window_state_create("window");
  base_controls = base_control_create("base-controls");

  render = icy_method_table_create(NULL, "render.vtable");
  background = control_to_int_create("background.color");
  window_lookup = void_to_control_create(NULL);
  glfw_window_lookup = control_to_void_create(NULL);
  
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
  glfwInit();
  demo_window();
  
  return 0;
}
