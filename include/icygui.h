
// Intern a string, get the unique ID that the string gets.
// interned strings will get the ID 1...N. 0 is not a valid value for an interned string
// 
size_t icy_intern(const char * string);

// Writes string representing an interned ID into 'buffer' of size 'size' returns the number of written bytes. If it failed to find 'id' it will return 0.
size_t icy_intern_get(size_t id, char * buffer, size_t size);
// A GUI control.
typedef struct{
  unsigned int id;
}icy_control;


// internal things:
#define ICY_HIDDEN __attribute__((visibility("hidden")))
