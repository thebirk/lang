struct Scope
{
	Scope *parent;
	
	Array<Node*> stmts;
};

struct Checker
{
	Parser *parser;
	Scope *global_scope;
};

Scope* NewScope(Scope *parent)
{
	Scope *scope = (Scope*) malloc(sizeof(Scope));
	scope->parent = parent;
	scope->stmts = make_array<Node*>();
	
	return scope;
}

bool CheckForNameCollision(Scope *scope, Node *ident)
{
	
	
	return false;
}

void AddDeclarationToScope(Scope *scope, Node *func)
{
	assert(scope);
	
	if(CheckForNameCollision(scope, func->func.ident)) {
		
	}
	
	Node *f;
	for_array(&scope->stmts, f) {
		
	}
}

bool CheckBlock(Parser *parser, Node *parent, Node *block)
{
	assert(block->kind == NODE_BLOCK);
	
	Node *stmt;
	for_array(&block->block.stmts, stmt) {
		// CheckStmt? With out without parent blocks?
	}
	
	return true;
}

bool CheckFunc(Parser *parser, Node *func)
{
	assert(func->kind == TOKEN_FUNC);
	
	// Check that return checks out with return statements(implement that aswell)
	CheckBlock(parser, func, func->func.block);
	
	return true;
}

bool CheckProgram(Checker *checker)
{
	checker->global_scope = NewScope(0);
	Scope *global_scope = checker->global_scope;
	Parser *parser = checker->parser;
	
	Node *n;
	for_array(&parser->top_level, n) {
		if(n->kind == NODE_FUNC) {
			AddDeclarationToScope(global_scope, n);
		} else if(n->kind == NODE_VAR) {
			AddDeclarationToScope(global_scope, n);
		} else {
			assert(!"Invalid top level node");
		}
	}
	
	return true;
}

/*	Checker _checker = {0};
	Checker *checker = &_checker;
	checker->parser = parser;
	printf("CheckProgram: %s\n", CheckProgram(checker) ? "true" : "false");*/