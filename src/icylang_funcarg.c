typedef struct{
  char ** column_names;
  char ** column_types;
  size_t count;
  const bool is_multi_table;
  const int column_count;
  int (*cmp) (const icy_control * k1, const icy_control * k2);
  const size_t sizes[2];

  void * arguments;
  icy_symbol * id;
  icy_mem * arguments_area;
  icy_mem * id_area;
  
}argument_table;

static int cmp_n = 0;

static int compareN(const void * k1, const void * k2){
  return memcmp(k1, k2, cmp_n);
}

argument_table * argument_table_create(const char * optional_name, size_t len){
  static const char * const column_names[] = {(char *)"arguments", (char *)"id"};
  static const char * const column_types[] = {"void *", "size_t"};
  argument_table * instance = calloc(sizeof(argument_table), 1);
  icy_table_init((icy_table * )instance, optional_name, 2, (unsigned int[]){len, sizeof(icy_symbol)}, (char *[]){(char *)"arguments", (char *)"id"});
  instance->column_names = (char **)column_names;
  instance->column_types = (char **)column_types;
  instance->cmp = (void *)compareN;
  return instance;
}
static icy_vector * files = NULL;
static control_to_index * arglist_size_index;
static icy_mem * empty_set = NULL;
static size_t empty_id_value;
static void init_if_needed(){
  if(files == NULL){
    files = icy_vector_create(NULL, sizeof(icy_table *));
    arglist_size_index = control_to_index_create("icylang.arglist.sizeindex");
    empty_set = icy_mem_create("icylang.0args");
    if(empty_set->size < 4){
      icy_mem_realloc(empty_set, 4);
      unsigned int * p = empty_set ->ptr;
      *p = icy_alloc_id();
    }
    unsigned int * p = empty_set->ptr;
    empty_id_value =  p[0];
  }
}

static icy_index get_index_for_size(unsigned int s){
  icy_index idx = {s + 5};
  return idx;
}

static size_t get_size_for_index(icy_index idx){
  ASSERT(idx.index >= 5);
  return idx.index - 5;
}

static argument_table * get_arg_table_for_size(unsigned int s){
  init_if_needed();
  ASSERT(s > 0);
  void * pt = NULL;
  icy_index idx = get_index_for_size(s);
  while(icy_vector_contains(files, idx) == false){
    icy_vector_alloc_sequence(files, 5);
  }
  
  pt = icy_vector_lookup(files, idx);
  argument_table ** tab = pt;
  if(*tab == NULL){
    char buffer[30];
    sprintf(buffer, "icylang.arglist.%i", s);
    *tab = argument_table_create(buffer, s * sizeof(icy_symbol));    
  }
  return *tab;
}

icy_symbol icy_get_args_symbol (const icy_symbol * types, int count){

  init_if_needed();
  if(count == 0)
    return (icy_symbol){empty_id_value};
  ASSERT(types != NULL);
  argument_table * table = get_arg_table_for_size(count);
  ASSERT(table->sizes[0] == sizeof(icy_symbol) * count);
  size_t index = 0;
  cmp_n = count * sizeof(icy_symbol);
  icy_table_finds((icy_table *) table, (void *)types, &index, 1);
  if(index == 0){
    icy_symbol id = {icy_alloc_id()};
    icy_index idx = get_index_for_size(count);
    
    void * values[] = {(void *)types, &id}; 
    icy_table_inserts((icy_table *) table, values, 1);
    control_to_index_set(arglist_size_index, id, idx);
    
    icy_table_finds((icy_table *) table, (void *)types, &index, 1);
    ASSERT(table->id[index].id == id.id);
    return id;
  }else{
    return table->id[index];
  }
}


size_t icy_args_get(icy_symbol id, icy_symbol * buffer, size_t size){
  if(id.id == empty_id_value){
    return 0;
  }
  icy_index table_index = {0};
  ASSERT(control_to_index_try_get(arglist_size_index, &id, &table_index));
  if(buffer == NULL) return get_size_for_index(table_index);
  
  argument_table * table = ((argument_table **)icy_vector_lookup(files, table_index))[0];
  ASSERT(table != NULL);
  size_t index = 0;
  icy_symbol * ids = table->id;
  for(size_t i = 0; i < table->count; i++){
    if(ids[i + 1].id == id.id){
      index = i;
      break;
    }
  }
  void * ptr = table->arguments + (1 + index) * table->sizes[0];
  return mempcpy(buffer, ptr, sizeof(icy_symbol) * size) - (void *) buffer; 
}

