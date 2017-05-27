
// Intern a string, get the unique ID that the string gets.
// interned strings will get the ID 1...N. 0 is not a valid value for an interned string
// 
size_t icy_intern(const char * string);

// Get the string representing an interned ID.
const char * icy_intern_get(size_t id);

// A GUI control.
typedef struct{
  unsigned int Id;
}icy_control;


// internal things:
#define ICY_HIDDEN __attribute__((visibility("hidden")))
