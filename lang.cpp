#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include "common.cpp"

#include "types.h"

#ifdef _WIN32
#	define _strdup strdup
#	define NORETURN __declspec(noreturn)
#endif /* _WIN32 */

#define TOKENS \
X(TOKEN_IDENT, "identifier") \
X(TOKEN_INTEGER, "integer") \
X(TOKEN_PLUS, "+") \
X(TOKEN_MINUS, "-") \
X(TOKEN_SLASH, "/") \
X(TOKEN_ASTERISK, "*") \
X(TOKEN_EQUAL, "=") \
X(TOKEN_LEFTPAR, "(") \
X(TOKEN_RIGHTPAR, ")") \
X(TOKEN_LEFTBRACE, "{") \
X(TOKEN_RIGHTBRACE, "}") \
X(TOKEN_LEFTBRACKET, "[") \
X(TOKEN_RIGHTBRACKET, "]") \
X(TOKEN_CARET, "^") \
X(TOKEN_COMMA, ",") \
X(TOKEN_BOR, "|") \
X(TOKEN_BAND, "&") \
X(TOKEN_LOR, "||") \
X(TOKEN_LAND, "&&") \
X(TOKEN_SEMICOLON, ";") \
X(TOKEN_COLON, ":") \
X(TOKEN_PERCENT, "%") \
X(TOKEN_IF, "if") \
X(TOKEN_DO, "do") \
X(TOKEN_ELSE, "else") \
X(TOKEN_RETURN, "return") \
X(TOKEN_FUNC, "func") \
X(TOKEN_VAR, "var") \
X(TOKEN_FOR, "for") \
X(TOKEN_WHILE, "while") \
X(TOKEN_EOF, "eof") \
X(TOKENS_DUMMY_END, "DUMMY_END")

/*

Should we convert to ir then do semantics/type checking
or semantics/type checking first?

TODO:

- Generate ir for ast
- Do semantics checking
- Complete vm
- Generate bytecodes from ir

*/

enum 
{
_TOKEN_START=0,
#define X(en, str) en,
TOKENS
#undef X
};

char* TokenKindToString(int kind)
{
	switch(kind) {
		#define X(_kind, _str) case _kind: return _str;
		TOKENS
		#undef X
	}
	
	return "(Broken TokenKind macro)";
}

// NOTE: Given the how the lexer does lines and coloumns, the line,col variables
//       often(If not in all cases) point to the end of the token.
struct Token
{
	int kind;
	char *file;
	int line;
	int col;
	
	int64_t integer;
	char *lexeme;
	int length;
};

struct Lexer
{
	Token *tokens;
	int tokens_count;
	int tokens_cap;
	
	char *data;
	char *filename;
	int line;
	int col;
};

void InitLexer(Lexer *lexer, char *path)
{
	lexer->tokens_cap = 32;
	lexer->tokens = (Token*) xmalloc(sizeof(Token)*lexer->tokens_cap);
	lexer->tokens_count = 0;
	
	lexer->filename = strdup(path);
	lexer->line = 1;
	lexer->col = 1;
	
	FILE *f = fopen(path, "rb");
	assert(f);
	
	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	rewind(f);
	
	lexer->data = (char*) xmalloc(sizeof(char)*(size+1));
	fread(lexer->data, size+1, 1, f);
	lexer->data[size] = 0;
	
	fclose(f);
}

bool IsDigit(char c)
{
	return (c >= '0') && (c <= '9');
}

bool IsAlpha(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
}

bool IsAlnum(char c)
{
	return IsDigit(c) || IsAlpha(c);
}

bool IsValidIdentifier(char c)
{
	return IsAlnum(c) || c == '_';
}

bool IsWhitespace(char c)
{
	return (c == ' ') || (c == '\t') || (c == '\r');
}

bool IsNewline(char c)
{
	return c == '\n';
}

void AddToken(Lexer *lexer, Token t)
{
	lexer->tokens_count++;
	if(lexer->tokens_count >= lexer->tokens_cap) {
		lexer->tokens_cap *= 2;
		lexer->tokens = (Token*)xrealloc(lexer->tokens, sizeof(Token)*lexer->tokens_cap);
	}
	t.file = lexer->filename;
	t.line = lexer->line;
	t.col = lexer->col;
	
	lexer->tokens[lexer->tokens_count-1] = t;
}

// NOTE: Allocating a Token for every token is slow
//       Create a smart function that only does a few
//       that way we dont need excessive amounts of mem
void Lex(Lexer *lexer)
{
	char *ptr = lexer->data;
	
	while(*ptr) {
		if(IsNewline(*ptr)) {
			ptr++;
			lexer->line++;
			lexer->col = 1;
			continue;
		}
		
		if(IsWhitespace(*ptr)) {
			ptr++;
			lexer->col++;
			continue;
		}
		
		if(*ptr == '/' && *(ptr+1) == '/') {
			ptr += 2;
			while(*ptr && !IsNewline(*ptr)) {
				ptr++;
			}
			if(*ptr == '\n') ptr++; // Eat newline
			lexer->line++;
			lexer->col = 1;
			continue;
		}
		
		if(*ptr == '/' && *(ptr+1) == '*') {
			ptr += 2;
			int n = 1;
			while(*ptr && n > 0) {
				if(IsNewline(*ptr)) {
					lexer->line++;
					lexer->col = 1;
					ptr++;
					continue;
				}
				
				if(*ptr == '/' && *(ptr+1) == '*') {
					n++;
					lexer->col += 2;
					ptr += 2;
					continue;
				} else if(*ptr == '*' && *(ptr+1) == '/') {
					n--;
					lexer->col += 2;
					ptr += 2;
					continue;
				}
				
				ptr++;
				lexer->col++;
			}
			
			continue;
		}
		
		switch(*ptr) {
			#define BASIC_TOKEN(_char, _type) case _char: { ptr++; Token t = {0}; t.kind = _type; t.length = 0; AddToken(lexer, t); lexer->col++; continue; } break
			BASIC_TOKEN('+', TOKEN_PLUS);
			BASIC_TOKEN('-', TOKEN_MINUS);
			BASIC_TOKEN('/', TOKEN_SLASH);
			BASIC_TOKEN('*', TOKEN_ASTERISK);
			BASIC_TOKEN('%', TOKEN_PERCENT);
			BASIC_TOKEN(',', TOKEN_COMMA);
			BASIC_TOKEN(';', TOKEN_SEMICOLON);
			BASIC_TOKEN(':', TOKEN_COLON);
			BASIC_TOKEN('^', TOKEN_CARET);
			BASIC_TOKEN('(', TOKEN_LEFTPAR);
			BASIC_TOKEN(')', TOKEN_RIGHTPAR);
			BASIC_TOKEN('[', TOKEN_LEFTBRACKET);
			BASIC_TOKEN(']', TOKEN_RIGHTBRACKET);
			BASIC_TOKEN('{', TOKEN_LEFTBRACE);
			BASIC_TOKEN('}', TOKEN_RIGHTBRACE);
			BASIC_TOKEN('=', TOKEN_EQUAL);
			#undef BASIC_TOKEN
		}
		
		if(IsAlpha(*ptr) || *ptr == '_') {
			char *start = ptr;
			while(IsValidIdentifier(*ptr)) {
				ptr++;
				lexer->col++;
			}
			
			Token t = {0};
			t.kind = TOKEN_IDENT;
			t.lexeme = start;
			t.length = ptr-start;
			AddToken(lexer, t);
			continue;
		}
		
		if(IsDigit(*ptr)) {
			char *start = ptr;
			while(IsDigit(*ptr)) {
				ptr++;
				lexer->col++;
			}
			//char *num = (char*) xmalloc(sizeof(char)*(ptr-start+1));
			//memcpy(num, start, ptr-start);
			//num[ptr-start] = 0;
			// This could easily support hex numbers
			// we just have to lex them correctly
			//int64_t number = strtoll(num, 0, 0);
			//free(num);
			
			int64_t number = 0;
			for(int i = 0; i < ptr-start; i++) {
				number *= 10;
				number += start[i] - '0';
			}
			
			Token t = {0};
			t.kind = TOKEN_INTEGER;
			t.integer = number;
			AddToken(lexer, t);
			continue;
		}
		
		printf("(%s:%d:%d): Unexpected character '%c' (0x%X)!\n", lexer->filename, lexer->line, lexer->col, *ptr, *ptr);
		lexer->col++;
		ptr++;
	}
	
	for(int i = 0; i < lexer->tokens_count; i++) {
		Token *t = &lexer->tokens[i];
		if(t->kind != TOKEN_IDENT) continue;
		
		if(0) {}
#define KEYWORD(_kind, _str) else if((t->length == strlen(_str)) && (strncmp(t->lexeme, _str, t->length) == 0)) { t->kind = _kind; continue; }
		KEYWORD(TOKEN_IF, "if")
		KEYWORD(TOKEN_DO, "do")
		KEYWORD(TOKEN_ELSE, "else")
		KEYWORD(TOKEN_RETURN, "return")
		KEYWORD(TOKEN_FUNC, "func")
		KEYWORD(TOKEN_VAR, "var")
		KEYWORD(TOKEN_FOR, "for")
		KEYWORD(TOKEN_WHILE, "while")
#undef KEYWORD

	}
	
	Token t = {0};
	t.kind = TOKEN_EOF;
	AddToken(lexer, t);
}

#define NODES \
X(NODE_LITERAL, "literal") \
X(NODE_BINOP, "binop") \
X(NODE_UNARY, "unary") \
X(NODE_IF, "if") \
X(NODE_COMP, "cmp") \
X(NODE_VAR, "var") \
X(NODE_BLOCK, "block") \
X(NODE_FUNC, "func") \
X(NODE_CALL, "call") \
X(NODE_ASSIGN, "assign") \
X(NODES_DUMMY_END, "dummy_end")

enum
{
#define X(_type, _str) _type,
	NODES
#undef X
};

char* NodeKindToString(int kind)
{
	switch(kind) {
		#define X(_kind, _str) case _kind: return _str;
		NODES
		#undef X
	}
	
	return "(Broken NodeKind macro)";
}

#define LITERALS \
X(LITERAL_INTEGER, "integer_literal") \
X(LITERAL_IDENT, "identifier_literal") \
X(LITERALS_DUMMY_END, "dummy_end")

enum
{
#define X(_kind, _str) _kind,
	LITERALS
#undef X
};

char* LiteralKindToString(int kind)
{
	switch(kind) {
		#define X(_kind, _str) case _kind: return _str;
		LITERALS
		#undef X
	}
	
	return "(Broken LiteralKind macro)";
}

#define COMPOP \
X(COMP_EQUAL, "==") \
X(COMP_GTE, ">=") \
X(COMP_LTE, "<=") \
X(COMP_LT, "<") \
X(COMP_GT, ">") \
X(COMPOP_DUMMY_END, "dummy_end")

enum
{
#define X(_kind, _str) _kind,
	COMPOP
#undef X
};

char* CompOpToString(int kind)
{
	switch(kind) {
		#define X(_kind, _str) case _kind: return _str;
		COMPOP
		#undef X
	}
	
	return "(Broken CompOp macro)";
}

struct Node;
struct Node
{
	int kind;
	
	union {
		struct {
			int kind;
			Token literal;
		} literal;
		struct {
			char op;
			Node *lhs;
			Node *rhs;
		} binop;
		struct {
			Node *ident;
			Array<Node*> args;
		} call;
		struct {
			char op;
			Node *rhs;
		} unary;
		struct {
			int compop;
			Node *lhs;
			Node *rhs;
		} cmp;
		struct {
			Node *cond;
			Node *if_block;
			Node *else_block; // Could be another if
		} iff;
		struct {
			Node *ident;
			Type *type;
			Node *expr;
		} var;
		struct {
			Array<Node*> stmts;
		} block;
		struct {
			Node *lhs;
			Node *rhs;
		} assign;
		struct {
			Node *ident;
			Array<Node*> arg_names;
			Array<Type*> arg_types;
			Type *return_type;
			Node *block;
		} func;
	};
};

Node* NewNode(int kind)
{
	//Node *n = (Node*)xmalloc(sizeof(Node));
	Node *n = (Node*) ArenaPush(Node);
	
	n->kind = kind;
	
	return n;
}

Node* MakeLiteralNode(int kind, Token t)
{
	Node *n = NewNode(NODE_LITERAL);
	
	n->literal.kind = kind;
	n->literal.literal = t;
	
	return n;
}

Node* MakeBinopNode(char op, Node *lhs, Node *rhs)
{
	Node *n = NewNode(NODE_BINOP);
	
	n->binop.op = op;
	n->binop.lhs = lhs;
	n->binop.rhs = rhs;
	
	return n;
}

Node* MakeUnaryNode(char op, Node *rhs)
{
	Node *n = NewNode(NODE_UNARY);
	
	n->unary.op = op;
	n->unary.rhs = rhs;
	
	return n;
}

Node* MakeIfNode(Node *cond, Node *if_block, Node *else_block)
{
	Node *n = NewNode(NODE_IF);
	
	n->iff.cond = cond;
	n->iff.if_block = if_block;
	n->iff.else_block = else_block;
	
	return n;
}

Node* MakeCompNode(int compop, Node *lhs, Node *rhs)
{
	Node *n = NewNode(NODE_COMP);
	
	n->cmp.compop = compop;
	n->cmp.lhs = lhs;
	n->cmp.rhs = rhs;
	
	return n;
}

Node* MakeVarNode(Token ident, Type *type, Node *expr)
{
	Node *n = NewNode(NODE_VAR);
	
	n->var.ident = MakeLiteralNode(TOKEN_IDENT, ident);
	n->var.type = type;
	n->var.expr = expr;
	
	return n;
}

Node* MakeBlockNode()
{
	Node *n = NewNode(NODE_BLOCK);
	
	n->block.stmts = make_array<Node*>();
	
	return n;
}

Node* MakeFuncNode(Token ident)
{
	Node *n = NewNode(NODE_FUNC);
	
	n->func.ident = MakeLiteralNode(LITERAL_IDENT, ident);
	n->func.arg_names = make_array<Node*>();
	n->func.arg_types = make_array<Type*>();
	
	return n;
}

Node* MakeCallNode(Node *ident, Array<Node*> args)
{
	Node *n = NewNode(NODE_CALL);
	
	n->call.ident = ident;
	n->call.args = args;
	
	return n;
}

Node* MakeAssignNode(Node *lhs, Node *rhs)
{
	Node *n = NewNode(NODE_ASSIGN);
	
	n->assign.lhs = lhs;
	n->assign.rhs = rhs;
	
	return n;
}

struct Scope;
struct Parser
{
	Lexer lexer;
	Token current_token;
	int current_token_offset;
	Array<Node*> top_level;
};

void InitParser(Parser *parser, char *path)
{
	InitLexer(&parser->lexer, path);
	Lex(&parser->lexer);
	parser->top_level = make_array<Node*>();
	parser->current_token = parser->lexer.tokens[0];
	parser->current_token_offset = 0;
}

NORETURN void Error(Parser *parser, char *format, ...)
{	
	printf("%s(%d:%d): ", 
		parser->current_token.file,
		parser->current_token.line,
		parser->current_token.col
	);
	
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	
	putc('\n', stdout);
	
	exit(1);
}

Token NextToken(Parser *parser)
{
	parser->current_token_offset++;
	parser->current_token = parser->lexer.tokens[parser->current_token_offset];
	return parser->current_token;
}

bool Accept(Parser *parser, int kind)
{
	if(parser->current_token.kind == kind) {
		NextToken(parser);
		return true;
	}
	return false;
}

bool Expect(Parser *parser, int kind)
{
	if(Accept(parser, kind)) {
		return true;
	}
	Error(parser, "Unexpected token! Expected '%s' got '%s'!", TokenKindToString(kind), TokenKindToString(parser->current_token.kind));
}

Node* ParseExpr(Parser *parser);
// expr6 = <integer, ident, '(' expr ')'>
Node* expr6(Parser *parser)
{
	Token t = parser->current_token;
	if(Accept(parser, TOKEN_INTEGER)) {
		return MakeLiteralNode(LITERAL_INTEGER, t);
	} else if(Accept(parser, TOKEN_IDENT)) {
		if(Accept(parser, TOKEN_LEFTPAR)) {
			// We have a call at our hands
			Node *ident = MakeLiteralNode(LITERAL_IDENT, t);
			Array<Node*> args = make_array<Node*>(4);
			Token t = parser->current_token;
			do {
				if(t.kind == TOKEN_RIGHTPAR) break;
				Node *arg = ParseExpr(parser);
				array_append(&args, arg);
			} while(Accept(parser, TOKEN_COMMA));
			Expect(parser, TOKEN_RIGHTPAR);
			
			return MakeCallNode(ident, args);
		} else {
			// Just a normal ident
			return MakeLiteralNode(LITERAL_IDENT, t);
		}
	} else if(Accept(parser, TOKEN_LEFTPAR)) {
		Node *expr = ParseExpr(parser);
		Expect(parser, TOKEN_RIGHTPAR);
		return expr;
	} else {
		Error(parser, "Expected integer, identifier or '('!");
	}

}

// expr5 = expr6 <*, /, <<, >>, &> expr6
Node* expr5(Parser *parser)
{
	Node *lhs = expr6(parser);
	
	Token t = parser->current_token;
	while(t.kind == TOKEN_ASTERISK ||
		  t.kind == TOKEN_SLASH ||
		  t.kind == TOKEN_BAND) {
		char op = TokenKindToString(t.kind)[0];
		NextToken(parser);
		
		Node *rhs = expr6(parser);
		lhs = MakeBinopNode(op, lhs, rhs);
		
		t = parser->current_token;
	}
	
	return lhs;
}

// expr4 = expr5 <+, -, |, ...> expr5
Node* expr4(Parser *parser)
{
	Node *lhs = expr5(parser);
	
	Token t = parser->current_token;
	while(t.kind == TOKEN_PLUS ||
		  t.kind == TOKEN_MINUS ||
		  t.kind == TOKEN_BOR) {
		char op = TokenKindToString(t.kind)[0];
		NextToken(parser);
		
		Node *rhs = expr5(parser);
		lhs = MakeBinopNode(op, lhs, rhs);
		
		t = parser->current_token;
	}
	
	return lhs;
}

// expr3 = expr4 <==, <=, >=, <, >, ...> expr4
Node* expr3(Parser *parser)
{
	Node *lhs = expr4(parser);
	
	Token t = parser->current_token;
	
	return lhs;
}

// expr2 = expr3 && expr3
Node* expr2(Parser *parser)
{
	Node *lhs = expr3(parser);
	
	//TODO: Logical and
	
	return lhs;
}

// Also unary
// expr1 = expr2 || expr2
Node* expr1(Parser *parser)
{
	Node *lhs = expr2(parser);
	
	//TODO: Logical or
	
	return lhs;
}

Node* ParseExpr(Parser *parser)
{
	return expr1(parser);
}

Type* ParseType(Parser *parser)
{
	Type *top_type = 0;
	Type *type = 0;
	
	Token token = parser->current_token;
	while(1) {
		if(Accept(parser, TOKEN_CARET)) {
			Type *t = NewType();
			t->kind = TYPE_POINTER;
			if(type) {
				type->base_type = t;
				type = t;
			} else {
				type = t;
				top_type = type;
			}
		} else if(Accept(parser, TOKEN_LEFTBRACKET)) {
			Expect(parser, TOKEN_RIGHTBRACKET);
			Type *t = NewType();
			t->kind = TYPE_ARRAY;
			if(type) {
				type->base_type = t;
				type = t;
			} else {
				type = t;
				top_type = type;
			}
		} else {
			break; // No more brackets or carets so we expect a ident
		}
	}
	token = parser->current_token;
	Expect(parser, TOKEN_IDENT);
	
	char *name = InternStringLength(token.lexeme, token.length);
	Type *t = BasicTypeFromName(name);
	if(!t) {
		t = NewType();
		t->kind = TYPE_NAMED;
		t->name = InternStringLength(token.lexeme, token.length);
	}
	if(type) {
		type->base_type = t;
		type = t;
	} else {
		top_type = t;
	}
	
	return top_type;
}

Node* ParseVar(Parser *parser)
{
	NextToken(parser); // Eat 'var'
	Token ident = parser->current_token;
	Expect(parser, TOKEN_IDENT);
	Type *type = 0;
	
	if(Accept(parser, TOKEN_COLON)) {
		type = ParseType(parser);
		Expect(parser, TOKEN_EQUAL);
	} else {
		Error(parser, "No type inference for now ):");
		Expect(parser, TOKEN_EQUAL);
	}
	
	Node *expr = ParseExpr(parser);
	
	Node *var = MakeVarNode(ident, type, expr);
	
	return var;
}

Node* ParseIf(Parser *parser);
Node* ParseStmt(Parser *parser)
{
	Token t = parser->current_token;
	if(t.kind == TOKEN_VAR) {
		Node *var = ParseVar(parser);
		Expect(parser, TOKEN_SEMICOLON);
		return var;
	} else if(t.kind == TOKEN_IF) {
		Node *iff = ParseIf(parser);
		return iff;
	} else if(t.kind == TOKEN_SEMICOLON) {
		NextToken(parser);
		return ParseStmt(parser);
	} else {
		Node *expr = ParseExpr(parser);
		// Now we have either the lhs of an
		// assignment or a function call
		
		if(expr->kind == NODE_CALL) {
			Expect(parser, TOKEN_SEMICOLON);
			return expr;
		}
		
		Expect(parser, TOKEN_EQUAL);
		Node *rhs = ParseExpr(parser);
		Expect(parser, TOKEN_SEMICOLON);
		
		// Handle assignment vs function call
		return MakeAssignNode(expr, rhs);
	}
}

Node* ParseBlock(Parser *parser)
{
	if(Accept(parser, TOKEN_DO)) {
		Node *stmt = ParseStmt(parser);
		return stmt;
	} else if(Accept(parser, TOKEN_LEFTBRACE)) {
		Node *block = MakeBlockNode();
		
		Token t = parser->current_token;
		while(t.kind != TOKEN_RIGHTBRACE && t.kind != TOKEN_EOF) {
			Node *stmt = ParseStmt(parser);
			array_append(&block->block.stmts, stmt);
			t = parser->current_token;
		}
		Expect(parser, TOKEN_RIGHTBRACE);
		return block;
	} else {
		Error(parser, "Expected 'do' or '{' while parsing block!");
	}
	return 0;
}

Node* ParseIf(Parser *parser)
{
	NextToken(parser); // Eat 'if'
	Node *cond = ParseExpr(parser);
	Node *if_block = ParseBlock(parser);
	Node *else_block = 0;
	
	if(Accept(parser, TOKEN_ELSE)) {
		Token t = parser->current_token;
		if(t.kind == TOKEN_IF) {
			else_block = ParseIf(parser);
		} else {
			else_block = ParseBlock(parser);
		}
	}
	
	return MakeIfNode(cond, if_block, else_block);
}

Node* ParseFunc(Parser *parser)
{
	Token func_name = NextToken(parser); // Eat 'func'
	
	Expect(parser, TOKEN_IDENT);
	Expect(parser, TOKEN_LEFTPAR);
	// This sets the ident and makes arrays for the args
	// Remeber to set return_type and block
	Node *func = MakeFuncNode(func_name);
	Token t = parser->current_token;
	do {
		if(t.kind == TOKEN_RIGHTPAR) break;
		if(!Accept(parser, TOKEN_IDENT)) {
			Error(parser, "Expected identifier while parsing argument list!");
		}
		if(Accept(parser, TOKEN_COLON)) {
			Type *type = ParseType(parser);
			array_append(&func->func.arg_names, MakeLiteralNode(LITERAL_IDENT, t));
			array_append(&func->func.arg_types, type);
		} else {
			Error(parser, "Expected ':' while parsing argument list!");
		}
	} while(Accept(parser, TOKEN_COMMA));
	Expect(parser, TOKEN_RIGHTPAR);
	
	if(Accept(parser, TOKEN_COLON)) {
		// We have a return type
		func->func.return_type = ParseType(parser);
	} else {
		func->func.return_type = type_void;
	}
	
	func->func.block = ParseBlock(parser);
	
	return func;
}

void Parse(Parser *parser)
{
	while(1) {
		Token t = parser->current_token;
		if(t.kind == TOKEN_VAR) {
			array_append(&parser->top_level, ParseVar(parser));
			Expect(parser, TOKEN_SEMICOLON);
		} else if(t.kind == TOKEN_FUNC) {
			array_append(&parser->top_level, ParseFunc(parser));
		} else {
			break;
		}
	}
	Expect(parser, TOKEN_EOF);
}

void LexerTest()
{
	Lexer _lexer = {};
	Lexer *lexer = &_lexer;
	InitLexer(lexer, "test.lang");
	Lex(lexer);
	
	for(int i = 0; i < lexer->tokens_count; i++) {
		Token *t = &lexer->tokens[i];
		printf("Token: %s at (%s:%d:%d) Lexeme: '%.*s' Integer: '%lld'\n", TokenKindToString(t->kind), t->file, t->line, t->col, t->length, t->lexeme, t->integer);
	}
}
	
#include "checker.cpp"
	
int main(int argc, char **argv)
{
	printf("\nToy Compiler\n\n");
	
	Parser _parser = {0};
	Parser *parser = &_parser;
	
	InitParser(parser, "test.lang");
	Parse(parser);
	
	Checker _checker = {0};
	Checker *checker = &_checker;
	checker->parser = parser;
	printf("CheckProgram: %s\n", CheckProgram(checker) ? "true" : "false");
	
	return 0;
}