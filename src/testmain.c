#include <icydb.h>
int main(){
  icy_vector * iv = icy_vector_create("hello", sizeof(int));
  icy_vector_destroy(&iv);
  return 0;
}
