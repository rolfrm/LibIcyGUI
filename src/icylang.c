#include <stdio.h>
#include <icydb.h>
#include <icygui.h>

#include "log.h"
#include "utils.h"
#include "icy_oop.h"
#include "fileio.h"
#include "mem.h"

#include "control_to_int.h"
#include "control_to_control.h"
#include "control_to_index.h"
#include "control_to_index.c"
#include "control_set.h"
#include "control_set.c"
typedef icy_control icy_symbol;

#include "icylang_funcarg.c"

control_to_int * primitives;
control_to_control * pointer_types;
control_to_control * inv_pointer_types;

typedef struct{
  icy_symbol return_type;
  icy_symbol argument_type;
}function_type;

icy_symbol no_type = {0};
#include "function_type_table.h"
#include "function_type_table.c"
function_type_table * function_types; 
icy_symbol sym(const char * str){
  return (icy_symbol){icy_intern(str)};
}

icy_symbol get_pointer_to(icy_symbol type){
  icy_symbol out = {0};
  
  if(control_to_control_try_get(inv_pointer_types, &type, &out))
  return out;
  if(control_to_control_try_get(pointer_types, &type, NULL)
     || control_to_int_try_get(primitives, &type, NULL)){
    out = (icy_symbol) {icy_alloc_id()};
    control_to_control_set(pointer_types, out, type);
    control_to_control_set(inv_pointer_types, type, out);
  }else{
    ERROR("unknown symbol: %i\n", type);
  }
  
  return out;  
}

icy_symbol get_pointed_to(icy_symbol ptrsym){
  icy_symbol out = {0};
  if(!control_to_control_try_get(pointer_types, &ptrsym, &out))
    ERROR("type was not a pointer %i\n", ptrsym);
  return out;
}

bool is_pointer_type(icy_symbol maybeptrsym){
  return control_to_control_try_get(pointer_types, &maybeptrsym, NULL);
}

icy_symbol define_function_type(icy_symbol return_type, icy_symbol * arguments, size_t arg_count){
  icy_symbol args = icy_get_args_symbol (arguments, arg_count);
  function_type tp = {return_type, args};
  icy_symbol sym;
  if(!function_type_table_try_get(function_types, &tp, &sym)){
    sym = (icy_symbol){icy_alloc_id()};
    function_type_table_set(function_types, tp, sym);
  }
  return sym;
}
 

void icylang_init(){
  primitives = control_to_int_create("icylang.primitives");
  pointer_types = control_to_control_create("icylang.pointer_types");
  inv_pointer_types = control_to_control_create("icylang.pointer_types_inverse_lookup");
  function_types = function_type_table_create("icylang.function_types");
  
  control_to_int_set(primitives, sym("int"), 4);
  control_to_int_set(primitives, sym("double"), 8);
  control_to_int_set(primitives, sym("float"), 8);
}

void icylang_test(){
  icylang_init();
  { // test pointer stuff.
    icy_symbol s1 = sym("int");
    icy_symbol s = get_pointer_to(s1);
    ASSERT(s.id != s1.id);
    icy_symbol s2 = get_pointed_to(s);
    ASSERT(s1.id == s2.id);
  }

  {
    ASSERT(get_arg_table_for_size(1) == get_arg_table_for_size(1));
    ASSERT(get_arg_table_for_size(2) != get_arg_table_for_size(1));
    ASSERT(get_arg_table_for_size(2) == get_arg_table_for_size(2));
  }
  
  { // test icy args
    icy_symbol args[] = {sym("int"), sym("double")};

    icy_symbol args2[] = {sym("int"), sym("double"), sym("int")};
    icy_symbol args3[] = {};
    icy_symbol args4[] = {sym("int"), sym("int")};
    icy_symbol s1 = icy_get_args_symbol (args, array_count(args));
    icy_symbol s2 = icy_get_args_symbol (args, array_count(args));
    icy_symbol s3 = icy_get_args_symbol (args2, array_count(args2));
    icy_symbol s4 = icy_get_args_symbol (args3, array_count(args3));
    icy_symbol s5 = icy_get_args_symbol (args4, array_count(args4));
    icy_get_args_symbol (args, array_count(args));
    icy_get_args_symbol (args, array_count(args));
    icy_get_args_symbol (args2, array_count(args2));
    icy_get_args_symbol (args3, array_count(args3));
    icy_get_args_symbol (args4, array_count(args));
    logd("%i %i %i %i %i\n", s1, s2, s3, s4, s5);
    ASSERT(s1.id == s2.id);
    ASSERT(s1.id != s3.id);
    ASSERT(s4.id != s1.id);
    ASSERT(s5.id != s1.id);

    size_t nargs4 = icy_args_get(s5, NULL, 0);
    ASSERT(nargs4 == array_count(args4));
    icy_symbol args4_2[nargs4];
    icy_args_get(s5, args4_2, nargs4);
    for(size_t i = 0; i < nargs4; i++)
      ASSERT(args4_2[i].id == args4[i].id); 
  }
  
  {
    icy_symbol ret = sym("int");
    icy_symbol args[] = {sym("int"), sym("int")};
    icy_symbol args2[] = {sym("int"), sym("int"), sym("int")};
    icy_symbol f1 = define_function_type(ret, args, array_count(args));
    icy_symbol f11 = define_function_type(ret, args, array_count(args));
    ASSERT(f1.id == f11.id);
    icy_symbol f2 = define_function_type(sym("int"), args2, array_count(args2));
    icy_symbol f22 = define_function_type(ret, args2, array_count(args2));
    ASSERT(f2.id == f22.id);
    ASSERT(f2.id != f1.id);
    icy_symbol f3 = define_function_type(no_type, NULL, 0);
    icy_symbol f33 = define_function_type(no_type, NULL, 0);
    ASSERT(f3.id == f33.id);
    ASSERT(f3.id != f2.id);
    
  }
  
}
