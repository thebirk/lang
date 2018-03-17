#define OPCODES \
X(NOP) \
X(CONST8) \
X(CONST16) \
X(CONST32) \
X(CONST64) \
X (I8ADD) \
X(I16ADD) \
X(I32ADD) \
X(I64ADD) \
X(F32ADD) \
X(F64ADD) \
X (I8SUB) \
X(I16SUB) \
X(I32SUB) \
X(I64SUB) \
X(F32SUB) \
X(F64SUB) \
X (I8MUL) \
X(I16MUL) \
X(I32MUL) \
X(I64MUL) \
X(F32MUL) \
X(F64MUL) \
X (I8DIV) \
X(I16DIV) \
X(I32DIV) \
X(I64DIV) \
X(F32DIV) \
X(F64DIV) \
X (I8MOD) \
X(I16MOD) \
X(I32MOD) \
X(I64MOD) \
X(F32MOD) \
X(F64MOD) \
X(JMP) \
X(CALL) \
X(I8CMP) \
X(I16CMP) \
X(I32CMP) \
X(I64CMP) \
X(F32CMP) \
X(F64CMP) \
X(JE) \
X(JL) \
X(JG) \
X(JLE) \
X(JGE) \
X(JZ) \
X(JNZ) \
X(GETLOCAL) \
X(SETLOCAL) \
X(SETGLOBAL) \
X(GETGLOBAL) \
X(LAST)

enum
{
#define X(_op) _op,
	OPCODES
#undef X
}

struct Label
{
	char *name;
	uint64_t offset;
};

struct UnresolvedJump
{
	char *name;        // Name of dest label
	uint64_t location; // Where to insert the address of the label
};

// Little-endian
struct VirtualMachine
{
	Array<uint8_t> code;
	uint8_t *stack = 0;
	uint64_t stack_size = 0;
	uint64_t stack_pointer = 0;
	
	Array<Label> labels;
	Array<UnresolvedJump> unresolved_jumps;
};

void ResolveJumps()
{
	// Go through all unresolved_jumps and replace the address with 
	// labels if they exist otherwise fail!
}

// NOTE: Endian dependent, only works on little-endian
void EmitBytes(VirtualMachine *vm, uint8_t *bytes, int count)
{
	for(int i = 0; i < count; i++) {
		array_append(&vm->code, bytes[0]);
	}
}

void Emit(VirtualMachine *vm, uint8_t opcode)
{
	arrray_append(&vm->code, opcode);
}

void Emit8(VirtualMachine *vm, uint8_t opcode, uint8_t arg)
{
	arrray_append(&vm->code, opcode);
	arrray_append(&vm->code, arg);
}

void Emit16(VirtualMachine *vm, uint8_t opcode, uint16_t arg)
{
	arrray_append(&vm->code, opcode);
	EmitBytes(&vm->code, &arg, 2);
}

void Emit32(VirtualMachine *vm, uint8_t opcode, uint32_t arg)
{
	arrray_append(&vm->code, opcode);
	EmitBytes(&vm->code, &arg, 4);
}

void Emit64(VirtualMachine *vm, uint8_t opcode, uint64_t arg)
{
	arrray_append(&vm->code, opcode);
	EmitBytes(&vm->code, &arg, 8);
}

void EmitJump(VirtualMachine *vm, uint8_t opcode, char *label)
{
	Emit64(vm, opcode, 0);
	UnresolvedJump jump = {0};
	jump.name = label;
	jump.location = vm->code->len - 8;
}

void GenFunction(VirtualMachine *vm, Module *m, Function *f)
{
	// reserve stack space for arguments and local variables
	// Figure out how to do stackframes
}

void GenBytecode(VirtualMachine *vm, Module *m)
{
	// Turn global_scope varibles into
	// global offsets
	
	Function *f;
	for_array_ref(&m->global_scope->functions, f) {
		GenFunction(vm, m, f);
	}
}