//////// Abort on fail memory allocation
void* xmalloc(size_t size)
{
	//printf("xmalloc %lld bytes\n", size);
	void *result = malloc(size);
	if(!result) {
		assert(!"xmalloc failed!");
	}
	return result;
}

void* xrealloc(void *ptr, size_t size)
{
	ptr = realloc(ptr, size);
	if(!ptr) {
		assert(!"xrealloc failed!");
	}
	return ptr;
}

//////// Array implementation
#define for_array_ref(_array, _var) for(int it = 0; (_var) = &((_array)->data[it]),it < (_array)->len; it++)
#define for_array(_array, _var) for(int it = 0; (_var) = (_array)->data[it],it < (_array)->len; it++)
	
template<typename T>
struct Array
{
	T *data;
	int len;
	int cap;
};

template<typename T>
Array<T> make_array(int initial_cap = 32)
{
	Array<T> result = {0};
	
	result.cap = initial_cap;
	result.data = (T*) xmalloc(sizeof(T)*result.cap);
	
	return result;
}

template<typename T>
void array_append(Array<T> *array, T el)
{
	array->len++;
	if(array->len >= array->cap) {
		array->cap *= 2;
		array->data = (T*)xrealloc(array->data, sizeof(T)*array->cap);
	}
	array->data[array->len-1] = el;
}

//////// MemoryArena
#define ARENA_SIZE 1024*1024*64
static void* MemoryArena = xmalloc(ARENA_SIZE);
static size_t MemoryArenaOffset = 0;

#define ArenaPushArray(_el, _size) (_el*) _ArenaPush(sizeof(_el)* (_size))
#define ArenaPush(_el) (_el*) _ArenaPush(sizeof(_el))
void* _ArenaPush(size_t bytes)
{
	void *ptr = (uint8_t*)MemoryArena+MemoryArenaOffset;
	MemoryArenaOffset += bytes;
	assert(MemoryArenaOffset <= ARENA_SIZE);
	return ptr;
}

//////// String interning

struct InternString
{
	char *str;
	int  len;
};

static Array<InternString> interns = make_array<InternString>();

char* InternStringRange(char *start, char *end)
{
	int len = end-start;
	InternString *str;
	for_array_ref(&interns, str) {
		if(str->len == len && strncmp(str->str, start, len) == 0) {
			return str->str;
		}
	}
	
	//char *result = (char*) xmalloc(len+1);
	char *result = (char*) ArenaPushArray(char, len+1);
	memcpy(result, start, len);
	result[len] = 0;
	array_append(&interns, {result, len});
	return result;
}

char* InternStringLength(char *start, int len)
{
	InternString *str;
	for_array_ref(&interns, str) {
		if(str->len == len && strncmp(str->str, start, len) == 0) {
			return str->str;
		}
	}
	
	//char *result = (char*) xmalloc(len+1);
	char *result = (char*) ArenaPushArray(char, len+1);
	memcpy(result, start, len);
	result[len] = 0;
	array_append(&interns, {result, len});
	return result;
}