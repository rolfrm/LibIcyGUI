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
#include "function_type_inv_table.h"
#include "function_type_inv_table.c"
#include "control_to_indexes.h"
#include "control_to_indexes.c"
function_type_table * function_types;
function_type_inv_table * function_types_inv;

control_to_control * defined_functions;
control_to_control * defined_function_names; // multiple functions can have the same name.
control_to_indexes * defined_function_argument_names_indexes;
icy_vector * defined_function_argument_names;

icy_vector * scope_body;

control_to_indexes * struct_slots_indexes;
typedef struct{
  icy_symbol name;
  icy_symbol type;
}struct_slot;

icy_vector * struct_slots;

void define_struct(icy_symbol name, struct_slot * slots, size_t slot_count){
  icy_indexes slots_indexes = {0};
  control_to_indexes_try_get(struct_slots_indexes, &name, &slots_indexes);
  icy_vector_resize_sequence(struct_slots, &slots_indexes, slot_count);
  struct_slot * slots2 = icy_vector_lookup_sequence(struct_slots, slots_indexes);
  memcpy(slots2, slots, sizeof(slots2[0]) * slot_count);
  control_to_indexes_set(struct_slots_indexes, name, slots_indexes);  
}

icy_symbol sym(const char * str){
  return (icy_symbol){icy_intern(str)};
}

size_t symname(icy_symbol sym, char * buffer, size_t buffer_size){
  size_t s = icy_intern_get(sym.id, buffer, buffer_size);
  return s;
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
    function_type_inv_table_set(function_types_inv, sym, tp);
  }
  return sym;
}

function_type get_function_type(icy_symbol sym){
  function_type ft;
  if(function_type_inv_table_try_get(function_types_inv, &sym, &ft))
    return ft;
  return (function_type){0};
}

void print_sym(icy_symbol sym){
  if(sym.id == 0) return;
  size_t len = symname(sym, NULL, 0);
  char buf[len + 1];
  memset(buf, 0,  len + 1);
  ASSERT(len == symname(sym, buf, len));
  logd("%s", buf);
}


void print_type(icy_symbol sym, icy_symbol decl){
  if(sym.id == 0){
    logd("void");
    return;
  }
  if(control_to_int_try_get(primitives, &sym, NULL)
     || control_to_indexes_try_get(struct_slots_indexes, &sym, NULL)){
    print_sym(sym);
    logd(" ");
    print_sym(decl);
    return;
  }
  icy_symbol pointed  = {0};
  if(control_to_control_try_get(pointer_types, &sym, &pointed)){
    print_type(pointed, (icy_symbol){0});
    logd(" *");
    return;
  }
  function_type ftype = {0};
  if(function_type_inv_table_try_get(function_types_inv, &sym, &ftype)){
    print_type(ftype.return_type, (icy_symbol){0});
    logd("(* ");
    print_sym(decl);
    logd(")(");

    size_t nargs = icy_args_get(ftype.argument_type, NULL, 0);
    icy_symbol args[nargs];
    icy_args_get(ftype.argument_type, args, nargs);
    for(size_t i = 0; i < nargs; i++){
      if(i > 0)
	logd(", ");
      print_type(args[i], (icy_symbol){0});
    }
    return;
  }
  logd("unsupported symbol");
}


void print_function_cstyle(icy_symbol sym){
  icy_symbol ftypesym;
  ASSERT(control_to_control_try_get(defined_functions, &sym, &ftypesym));
  function_type ftype;
  ASSERT(function_type_inv_table_try_get(function_types_inv, &ftypesym, &ftype));

  icy_symbol functionname;
  ASSERT(control_to_control_try_get(defined_function_names, &sym, &functionname));
  
  print_type(ftype.return_type, functionname);
  icy_indexes idx = {0};
  ASSERT(control_to_indexes_try_get(defined_function_argument_names_indexes, &sym, &idx));
  if(idx.count == 0){
    logd("()");
    return;
  }

  icy_symbol * syms = icy_vector_lookup_sequence(defined_function_argument_names, idx);
  size_t argcnt = icy_args_get(ftype.argument_type, NULL, 0);
  ASSERT(argcnt == idx.count);
  icy_symbol args[argcnt];
  icy_args_get(ftype.argument_type, args, argcnt);
  logd("(");
  for(size_t i = 0; i < argcnt; i++){
    if(i > 0)
      logd(", ");
    print_type(args[i], syms[i]);
  }
  logd(")");
}

#include "int_to_control.h"
#include "int_to_control.c"

control_to_control * sub_expressions;
int_to_control * int_const_exprs;
control_to_indexes * expression_sequence;
icy_vector * expression_sequences;

icy_symbol const_expr_int(int value){
  icy_symbol sym = {0};
  if(!int_to_control_try_get(int_const_exprs, &value, &sym))
    int_to_control_set(int_const_exprs, value, sym = (icy_symbol){icy_alloc_id()});
  return sym;
}

void set_function_body(icy_symbol fcn, icy_symbol body){
  control_to_control_set(sub_expressions, fcn, body);
}

void set_sequence(icy_symbol owner, size_t offset, icy_symbol sym){
  icy_symbol subexpr = {0};
  if(!control_to_control_try_get(sub_expressions, &owner, &subexpr))
    control_to_control_set(sub_expressions, owner, subexpr = (icy_symbol){icy_alloc_id()});
  
  icy_indexes indexes = {0};
  if(!control_to_indexes_try_get(expression_sequence, &subexpr, &indexes))
    icy_vector_resize_sequence(expression_sequences, &indexes, offset + 1);
  
  if(indexes.count <= offset)
    icy_vector_resize_sequence(expression_sequences, &indexes, offset + 1);
  control_to_indexes_set(expression_sequence, subexpr, indexes);
  icy_symbol * syms = icy_vector_lookup_sequence(expression_sequences, indexes);
  syms[offset] = sym;
}

icy_symbol get_sequence_item(icy_symbol owner, size_t offset){
  icy_symbol subexpr = {0};
  if(!control_to_control_try_get(sub_expressions, &owner, &subexpr))
    return (icy_symbol){0};
  icy_indexes indexes = {0};
  if(!control_to_indexes_try_get(expression_sequence, &subexpr, &indexes))
    return (icy_symbol){0};
  if(indexes.count <= offset)
    return (icy_symbol){0};
  icy_symbol * symbols = icy_vector_lookup_sequence(expression_sequences, indexes);
  return symbols[offset];
}

icy_symbol set_sub_sequence(icy_symbol owner, size_t offset){
  icy_symbol subexpr = get_sequence_item(owner, offset);
  return subexpr;
  
}

void icylang_init(){
  primitives = control_to_int_create("icylang.primitives");
  pointer_types = control_to_control_create("icylang.pointer_types");
  inv_pointer_types = control_to_control_create("icylang.pointer_types_inverse_lookup");
  function_types = function_type_table_create("icylang.function_types");
  function_types_inv = function_type_inv_table_create("icylang.function_types_inverse_lookup");
  struct_slots_indexes = control_to_indexes_create("icylang.slot_index");
  struct_slots = icy_vector_create("icylang.slots", sizeof(struct_slot));

  defined_functions = control_to_control_create("icylang.functions");
  defined_function_names = control_to_control_create("icylang.functions.names");

  defined_function_argument_names_indexes = control_to_indexes_create("icylang.functions.arg_names_indexes");
  defined_function_argument_names = icy_vector_create("icylang.functions.arg_names", sizeof(icy_symbol));

  sub_expressions = control_to_control_create("icylang.sub_expr");
  int_const_exprs = int_to_control_create("icylang.int_const_expr");
  expression_sequence = control_to_indexes_create("icylang.expression_sequence");
  expression_sequences = icy_vector_create("icylang.expression_sequences", sizeof(icy_symbol));
    
  control_to_int_set(primitives, sym("int"), 4);
  control_to_int_set(primitives, sym("double"), 8);
  control_to_int_set(primitives, sym("float"), 8);
}

typedef struct{
  icy_index parent;
  int index;
}octree_index;

typedef struct{
  icy_vector * type;
  icy_vector * sub_nodes;
  icy_vector * payload;
  octree_index first_node;
}octree;

octree * octree_create(){
  octree_index first = {0};
  octree o = { .type = icy_vector_create(NULL, sizeof(int)),
	       .sub_nodes = icy_vector_create(NULL, sizeof(int) * 8),
	       .payload = icy_vector_create(NULL, sizeof(int)),
	       .first_node = first};
  icy_index n1 = icy_vector_alloc(o.type);
  icy_index n2 = icy_vector_alloc(o.sub_nodes);
  icy_index n3 = icy_vector_alloc(o.payload);
  o.first_node = (octree_index) {n1, -1};
  ASSERT(n1.index == n2.index);
  ASSERT(n3.index == n2.index);
  ASSERT(n1.index > 0);
  char * typep = icy_vector_lookup(o.type, n1);
  typep[0] = 0xFF;
  return IRON_CLONE(o);
}

octree_index octree_child(octree * oct, octree_index index,  int child_index, bool create)
{
  ASSERT(index.parent.index > 0);
  ASSERT(child_index >= 0 && child_index < 8);

  char * typep = icy_vector_lookup(oct->type, index.parent);
  int types = typep[0];
  unsigned int type = (types >> child_index) & 1;
  logd("TYPE %i %i %i %i\n", type, typep[0], index.index, index.parent);
  ASSERT(type == 1 || type == 0);
   if(index.index == -1 && type == 1){
    return (octree_index){index.parent, child_index};
  }else if(index.index == -1){
    int * childids = icy_vector_lookup(oct->sub_nodes, index.parent);
    return (octree_index){(icy_index){childids[child_index]}, -1};
  }else if(index.index >= 0 && (type == 0 || (type == 1 && create))){
    int * childids = icy_vector_lookup(oct->sub_nodes, index.parent);
    if(type == 0){
      int childid = childids[child_index];
      logd("Childid: %i\n", childid);
      return (octree_index){(icy_index){childid}, -1};
    }else{
      icy_index n1 = icy_vector_alloc(oct->type);
      icy_index n2 = icy_vector_alloc(oct->sub_nodes);
      icy_index n3 = icy_vector_alloc(oct->payload);
      ASSERT(n1.index == n2.index);
      ASSERT(n3.index == n2.index);
      ASSERT(child_index >= 0);
      childids[child_index] = n1.index;
      typep[0] =  ((~(1 << child_index)) & types);
      char * subtype = icy_vector_lookup(oct->type, n1);
      subtype[0] = 0xFF;
      return (octree_index){ (icy_index){n1.index}, -1};
    } 
  }else{
    return (octree_index){0};
  }
}

void * octree_get_payload(octree * oct, octree_index index){
  if(index.index == -1){
    return icy_vector_lookup(oct->payload, index.parent);
  }
  ASSERT(index.index >= 0 || index.index < 8);
  char * typep = icy_vector_lookup(oct->type, index.parent);
  int types = typep[0];
  unsigned int type = (types >> index.index) & 1;
  int * childids = icy_vector_lookup(oct->sub_nodes, index.parent);
  if(type == 0){
    return icy_vector_lookup(oct->payload, (icy_index){childids[index.index]});
  }else{
    return &childids[index.index];
  }
}

void octree_debug_print(octree * oct, octree_index index){
  
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
    logd("test icy args: %i %i %i %i %i\n", s1, s2, s3, s4, s5);
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
  
  { // function types:
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
    function_type f1_type = get_function_type(f1);
    ASSERT(f1_type.return_type.id == ret.id);
  }

  { // struct types. There can be only one struct with a specific name.
    icy_symbol name  = sym("vec2");
    struct_slot slots[] = {(struct_slot){ sym("x"), sym("float") },
			   (struct_slot){ sym("y"), sym("float") }};
    define_struct(name, slots, array_count(slots));
  }

  { // functions
    icy_symbol ret = sym("int");
    icy_symbol args[] = {sym("int"), sym("int")};
    icy_symbol f1 = define_function_type(ret, args, array_count(args));
    icy_symbol testf1 = sym("test.f1");
    control_to_control_set(defined_functions, testf1, f1);
    icy_indexes idx = {0};
    control_to_indexes_try_get(defined_function_argument_names_indexes, &testf1, &idx);
    icy_vector_resize_sequence(defined_function_argument_names, &idx, 2);
    icy_symbol * argnames = icy_vector_lookup_sequence(defined_function_argument_names, idx);
    argnames[0] = sym("asd");
    argnames[1] = sym("bsd");

    char buf[4] = {0};
    symname(argnames[0], buf, 4);
    ASSERT(strcmp(buf, "asd") == 0);

    print_sym(argnames[0]); print_sym(argnames[1]);
    control_to_indexes_set(defined_function_argument_names_indexes, testf1, idx);
    control_to_control_set(defined_function_names, testf1, sym("add2int"));

    //function_type tp = get_function_type(f1);
    logd("\n");

    ASSERT(const_expr_int(4).id == const_expr_int(4).id);
    ASSERT(const_expr_int(5).id != const_expr_int(4).id);
    ASSERT(const_expr_int(5).id == const_expr_int(5).id);
    //set_function_body(testf1, const_expr_int(4));

    set_sequence(testf1, 0, sym("+"));
    set_sequence(testf1, 1, const_expr_int(4));
    set_sequence(testf1, 2, const_expr_int(10));
    set_sequence(testf1, 3, const_expr_int(15));
    set_sequence(testf1, 4, const_expr_int(25));
    //icy_symbol s = set_sub_sequence(testf1, 1);
    
    //print_function_cstyle(testf1); logd("\n");
    
    //icy_symbol seq = set_sequence(testf1, 0, const_expr_i32(4));
    //logd("\n");
    
  }

  { // global varibles


  }

  { //  octree
    octree * o = octree_create();
    octree_index o1 = octree_child(o, o->first_node, 0, true);
    logd("%i %i \n", o1.index, o1.parent);
    octree_index o2 = octree_child(o, o1, 0, true);
    logd("O3\n");
    octree_index o3 = octree_child(o, o->first_node, 0, true);
    octree_index o4 = octree_child(o, o3, 0, true);
    logd("PP %i %i %i %i\n", o1, o2, o3, o4);
    {
      octree_index o11 = octree_child(o, o->first_node, 1, true);
      octree_index o12 = octree_child(o, o->first_node, 2, true);
      octree_index o13 = octree_child(o, o->first_node, 3, true);
      logd("%i %i %i\n", o11.index, o12.index, o13.index);
    }
    {
      octree_index o11 = octree_child(o, o->first_node, 1, true);
      octree_index o12 = octree_child(o, o->first_node, 2, true);
      octree_index o13 = octree_child(o, o->first_node, 3, true);
      logd("%i %i %i\n", o11.index, o12.index, o13.index);
    }
    {
      octree_index o11 = octree_child(o, o->first_node, 4, true);
      octree_index o12 = octree_child(o, o->first_node, 5, true);
      octree_index o13 = octree_child(o, o->first_node, 6, true);
      logd("%i %i %i\n", o11.index, o12.index, o13.index);
    }
    
    {
      octree_index o11 = octree_child(o, o1, 4, true);
      octree_index o12 = octree_child(o, o1, 5, true);
      octree_index o13 = octree_child(o, o1, 6, true);
      logd("%i %i %i\n", o11.index, o12.index, o13.index);
    }
    octree_index o20;
    {
      octree_index o11 = octree_child(o, o1, 4, true);
      octree_index o12 = octree_child(o, o1, 5, true);
      octree_index o13 = octree_child(o, o1, 6, true);
      logd("%i %i %i\n", o11.parent, o12.parent, o13.parent);
      o20 = o13;
    }
    
    for(int i = 0; i < 8; i++){
      octree_index o21 = octree_child(o, o20, 4, true);
      octree_index o22 = octree_child(o, o20, 5, true);
      octree_index o23 = octree_child(o, o20, 6, true);
      o20 = o23;
      logd("%i %i %i\n", o21.parent, o22.parent, o23.parent);
      int * pt = octree_get_payload(o, o22);
      *pt = 5;
    }    
  }
}
