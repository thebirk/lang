enum
{
	TYPE_VOID,
	TYPE_INTEGER,
	TYPE_FLOAT, // Structs??
	TYPE_NAMED,
	TYPE_POINTER,
	TYPE_ARRAY, // What about sized arrays?
};

enum
{
	BASIC_VOID = 0,
	
	BASIC_U8,
	BASIC_U16,
	BASIC_U32,
	BASIC_U64,
	
	BASIC_I8,
	BASIC_I16,
	BASIC_I32,
	BASIC_I64,
	
	BASIC_F32,
	BASIC_F64,
	
	BASIC_BOOL,
};

struct Type
{
	int kind;        // Type kind
	char *name;      // Type name
	int byte_size;   // For integers and floats, pointers are always pointer sized duh.
	bool signedness; // For integers
	Type *base_type;  // Used for pointer and array
};

Type basic_types[] = {
	{TYPE_VOID, "void", 0, false, 0},

	{TYPE_INTEGER, "u8" , 1, false, 0},
	{TYPE_INTEGER, "u16", 2, false, 0},
	{TYPE_INTEGER, "u32", 4, false, 0},
	{TYPE_INTEGER, "u64", 8, false, 0},

	{TYPE_INTEGER, "i8" , 1, true, 0},
	{TYPE_INTEGER, "i16", 2, true, 0},
	{TYPE_INTEGER, "i32", 4, true, 0},
	{TYPE_INTEGER, "i64", 8, true, 0},

	{TYPE_FLOAT, "f32", 4, false, 0},
	{TYPE_FLOAT, "f64", 8, false, 0},
	
	{TYPE_INTEGER, "bool", 4, false, 0},
};

Type *type_void = &basic_types[BASIC_VOID];

Type *type_u8 = &basic_types[BASIC_U8];
Type *type_u16 = &basic_types[BASIC_U16];
Type *type_u32 = &basic_types[BASIC_U32];
Type *type_u64 = &basic_types[BASIC_U64];

Type *type_i8 = &basic_types[BASIC_I8];
Type *type_i16 = &basic_types[BASIC_I16];
Type *type_i32 = &basic_types[BASIC_I32];
Type *type_i64 = &basic_types[BASIC_I64];

Type *type_int = type_i64;
Type *type_uint = type_u64;

Type *type_f32 = &basic_types[BASIC_F32];
Type *type_f64 = &basic_types[BASIC_F64];

Type *type_bool = &basic_types[BASIC_BOOL];

Type* BasicTypeFromName(char *name)
{
	for(int i = 0; i < sizeof(basic_types)/sizeof(basic_types[0]); i++) {
		Type *t = &basic_types[i];
		if(strcmp(t->name, name) == 0) {
			return t;
		}
	}
	
	if(strcmp("int", name) == 0) {
		return type_int;
	}
	
	if(strcmp("uint", name) == 0) {
		return type_uint;
	}
	
	return 0;
}

Type* NewType()
{
	Type *t = (Type*) malloc(sizeof(Type));
	*t = {0};
	return t;
}

Type* NewArrayType(Type *base_type)
{
	Type *t = NewType();
	
	t->kind = TYPE_ARRAY;
	t->base_type = base_type;
	
	return t;
}