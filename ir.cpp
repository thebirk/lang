struct Module;

struct Variable
{
	char *name;
	Type *type;
	int offset;
	bool defined;
};

struct Block
{
	int locals;
	Array<Variable> variables;
};

struct GlobalScope
{
	Block block;
	Array<Function> functions;
};

struct Argument
{
	char *name;
	Type *type;
};

struct Function
{
	char *name;
	Type *return_type;
	int num_locals;
	Scope *scope;
	Array<Argument> arguments;
};

Variable* FunctionGetVariable()
{
	
}

enum
{
	VALUE_CONSTANT,
	VALUE_VARIABLE,
	VALUE_BINARY_OP,
};

struct Value
{
	int kind;
	
	union {
		
	};
};

struct Module
{
	Parser *parser;
	Block *global_scope;
	
};

Module* MakeIrModule(Parser *parser)
{
	Module *m = ArenaPush(Module);
	
	m->global_scope = ArenaPush(Scope);
	InitGlobalScope(m);
	
	return m;
}