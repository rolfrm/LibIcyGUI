#include <GLFW/glfw3.h>

void_to_control * window_lookup;
void_to_control * glfw_window_lookup;
control_to_control * mouse_capture;
control_to_bool * mouse_over;
icy_control get_window(GLFWwindow * glwindow){  
  icy_control ctrl = {0};
  void_to_control_try_get(window_lookup, &glwindow, &ctrl);
  return ctrl;
}

void window_pos_callback(GLFWwindow* glwindow, int xpos, int ypos)
{
  icy_control window = get_window(glwindow);
  window_state_insert(window_state_table, &window, NULL, NULL, &xpos, &ypos, 1);
}

void window_size_callback(GLFWwindow* glwindow, int width, int height)
{
  icy_control window = get_window(glwindow);
  window_state_insert(window_state_table, &window, &width, &height, NULL, NULL, 1);
}

void gui_acquire_mouse_capture(icy_control window, icy_control control){
  icy_control capt = {0};
  ASSERT(!control_to_control_try_get(mouse_captury, &window, &capt));
  control_to_control_set(mouse_capture, window, control);
}

void gui_release_mouse_capture(icy_control window, icy_control control){
  icy_control capt = 0;
  ASSERT(control_to_control_try_get(mouse_capture, &window, &capt));
  ASSERT(capt.Id == control.Id);
  control_to_control_unset(mouse_capture, window);
}

void cursor_pos_callback(GLFWwindow * glwindow, double x, double y){
  icy_control win_id = get_window(glwindow);
  int w_width, w_height, w_x, w_y;
  ASSERT(window_state_try_get(window_state_table, &win_id, &w_width, &w_height, &w_x, &w_y));
  icy_control capture = 0;
  auto on_mouse_over = get_method(win_id, mouse_over_method);
  
  clear_is_mouse_over();
  
  GLFWwindow * glfwWin = find_glfw_window(win_id);
  glfwGetWindowSize(glfwWin, &win->width, &win->height);
  thickness margin = get_margin(win_id);
  window_size = vec2_new(win->width, win->height);
  shared_offset = vec2_new(margin.left, margin.up);
  shared_size = vec2_new(win->width - margin.left - margin.right, win->height - margin.up - margin.down);

  if(try_get_window_mouse_capture(win_id, &capture)){
    auto on_mouse_over = get_method(capture, mouse_over_method);
    if(on_mouse_over != NULL){
      on_mouse_over(capture, x, y, 0);
      return;
    }
  }
  if(on_mouse_over != NULL)
    on_mouse_over(win_id, x, y, 0);
}

void mouse_button_callback(GLFWwindow * glwindow, int button, int action, int mods){
  UNUSED(mods);
  mouse_button_action = action == GLFW_PRESS ? 1 : 0;
  mouse_button_button = button;
  if(action == GLFW_REPEAT){
    mouse_button_action = 2;
  }
  if(true || button == 0){
    u64 win_id = get_window(glwindow);
    window * win = get_window_ref(glwindow);
    auto on_mouse_over = get_method(win_id, mouse_over_method);
    if(on_mouse_over != NULL){
      double x, y;
      glfwGetCursorPos(glwindow, &x, &y);
      clear_is_mouse_over();
      thickness margin = get_margin(win_id);
      //margin.left += 0.05f;
      GLFWwindow * glfwWin = find_glfw_window(win_id);
      glfwGetWindowSize(glfwWin, &win->width, &win->height);
      window_size = vec2_new(win->width, win->height);
      shared_offset = vec2_new(margin.left, margin.up);
      shared_size = vec2_new(win->width - margin.left - margin.right, win->height - margin.up - margin.down);
      u64 capture = 0;
      if(try_get_window_mouse_capture(win_id, &capture)){
	auto on_mouse_over = get_method(capture, mouse_over_method);
	if(on_mouse_over != NULL){
	  on_mouse_over(capture, x, y, mouse_down_method);
	  return;
	}
      }
      if(on_mouse_over != NULL)
	on_mouse_over(win_id, x, y, mouse_down_method);
      //on_mouse_over(win_id, x, y, mouse_down_method);
    }
  }
}


void scroll_callback(GLFWwindow * glwindow, double x, double y){
  UNUSED(glwindow);UNUSED(x);
  edit_mode * edit = persist_alloc("edit", sizeof(edit_mode));
  main_state * main_mode = persist_alloc("main_mode", sizeof(main_state));
  if(main_mode->mode == MODE_EDIT){
    int scroll = y > 0 ? 1 : -1;
    edit->scroll_amount = scroll;
  }
}


void key_callback(GLFWwindow* glwindow, int key, int scancode, int action, int mods){
  UNUSED(scancode);
  u64 win_id = get_window(glwindow);
  u64 focused = get_focused_element(win_id);
  if(focused == 0) return;
  method m = get_method(focused, key_handler_method);
  if(m != NULL)
    m(focused, key, mods, action);
}

void char_callback(GLFWwindow * glwindow, u32 codepoint){
  if(0 == codepoint_to_utf8(codepoint,NULL, 10))
    return; // WTF! Invalid codepoint!
  u64 win_id = get_window(glwindow);
  u64 focused = get_focused_element(win_id);
  if(focused == 0) return;
  
  method m = get_method(focused, char_handler_method);
  if(m != NULL)
    m(focused, codepoint, 0);
    
}

void window_close_callback(GLFWwindow * glwindow){
  u64 win_id = get_window(glwindow);
  auto close_method = get_method(win_id, window_close_method);
  if(close_method != NULL)
    close_method(win_id);
}


void load_window(u64 id){
  
  window w;
  if(try_get_window_state(id, &w) == false){
    w = (window){.width = 640, .height = 640};
    sprintf(w.title, "%s", "Test Window");
  }
  if(w.height <= 0) w.height = 200;
  if(w.width <= 0) w.width = 200;
  static GLFWwindow * ctx = NULL;
  logd("Window size:  %s %i %i\n", w.title, w.width, w.height);
  glfwWindowHint(GLFW_SAMPLES, 16);
  glfwWindowHint(GLFW_DEPTH_BITS, 32);
  GLFWwindow * window = glfwCreateWindow(w.width, w.height, w.title, NULL, ctx);
  ASSERT(window != NULL);
  if(ctx == NULL){
    ctx = window;
    glfwMakeContextCurrent(window);
    glewInit();
  }
  glfwSetWindowPos(window, w.x, w.y);
  glfwSetWindowSize(window, w.width, w.height);
  list_push(windows.window_id, windows.cnt, id);
  list_push2(windows.glfw_window, windows.cnt, window);

  glfwSetWindowPosCallback(window, window_pos_callback);
  glfwSetWindowSizeCallback(window, window_size_callback);
  glfwSetCursorPosCallback(window, cursor_pos_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetKeyCallback(window, key_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetWindowCloseCallback(window, window_close_callback);
  glfwSetCharCallback(window, char_callback);
  glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1);
  insert_window_state(&id, &w, 1);
}

void render_window(icy_control window){
  {
    GLFWwindow * win = find_glfw_window(window);
    if(win == NULL)
      load_window(window);
  }
  
  bool last = true;
  thickness margin = get_margin(window_id);
  //margin.left += 0.05f;
  GLFWwindow * glfwWin = find_glfw_window(window_id);
  glfwSetWindowTitle(glfwWin, win.title);
  glfwGetWindowSize(glfwWin, &win.width, &win.height);
  glfwMakeContextCurrent(glfwWin);
  glViewport(0, 0, win.width, win.height);
  window_size = vec2_new(win.width, win.height);
  
  vec3 color = get_color(window_id);
  glClearColor(color.x, color.y, color.z, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  shared_offset = vec2_new(margin.left, margin.up);
  shared_size = vec2_new(win.width - margin.left - margin.right, win.height - margin.up - margin.down);
  u64 index = 0;
  control_pair * child_control = NULL;
  while((child_control = get_control_pair_parent(window_id, &index))){
    if(child_control == NULL)
      break;
    ASSERT(child_control->child_id != 0);
  
    render_sub(child_control->child_id);
  }
  bool vsync_enabled = false;
  if(vsync_enabled)
    glfwSwapInterval(last ? 1 : 0);
  else
    glfwSwapInterval(0);
  glfwSwapBuffers(glfwWin);
}
