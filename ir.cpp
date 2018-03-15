struct Scope
{
	
};

struct Function
{
	char *name;
	Array<Node*> names;
	Array<Type*> types;
};

struct Variable
{
	
};

struct Value
{
	int kind;
};