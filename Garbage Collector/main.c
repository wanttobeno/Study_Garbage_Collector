
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

typedef enum {
	OBJ_INIT,
	OBJ_PAIR
} ObjectType;


typedef struct sObject {
	// 链表结构
	struct sObject* next;
	// 标记
	unsigned char marked;  

	ObjectType type;

	union {
		/* OBJ_INT */
		int value;

		/* OBJ_PAIR */
		struct {
			struct sObject* head;
			struct sObject* tail;
		};
	};
}Object;

// 小虚拟机
#define  STACK_MAX 256
#define  INITIAL_GC_THRESHOLD 128

typedef struct {
	// 统计数量
	int numObjects;
	int maxObjects;
	// 链表结构
	Object* firstObject;
	Object* stack[STACK_MAX];
	int stackSize;
}VM;

void gc(VM* vm);

// 初始化虚拟机

VM* newVM() {
	VM* vm = calloc(1,sizeof(VM));
	vm->stackSize = 0;

	vm->numObjects = 0;
	vm->maxObjects = INITIAL_GC_THRESHOLD;
	return vm;
}

// 释放虚拟机
void  freeVM(VM *vm) {
	vm->stackSize = 0;
	gc(vm);
	free(vm);
}

// 操作虚拟机堆栈

void push(VM* vm,Object* value){
	assert(vm->stackSize < STACK_MAX,"Stack overflow!");
	vm->stack[vm->stackSize++] = value;
}

Object* pop(VM* vm) {
	assert(vm->stackSize > 0,"Stack underflow!");
	return vm->stack[--vm->stackSize];
}

// 新建对象
Object* newObject(VM* vm,ObjectType type) {
	// 判断是否需要释放
	if (vm->numObjects == vm->maxObjects)
		gc(vm);
	{
		Object* object = (Object*)calloc(1,sizeof(Object));
		object->type = type;
		object->marked = 0;

		object->next = vm->firstObject;
		vm->firstObject = object;

		vm->numObjects ++;

		return object;
	}
}

// 编写方法将每种类型的对象压到虚拟机的栈上
void pushInt(VM* vm,int intValue){
	Object* object = newObject(vm,OBJ_INIT);
	object->value = intValue;

	push(vm,object);
}

Object* pushPair(VM* vm) {
	Object* object = newObject(vm,OBJ_PAIR);
	object->tail = pop(vm);
	object->head = pop(vm);

	push(vm,object);
	return object;
}

// 标记,标记后表示该对象被删除了，当内存并没有完全删除
void mark(Object* object) {
	// 避免陷入嵌套
	if (object->marked)
		return;

	object->marked = 1;
	if (object->type == OBJ_PAIR)
	{
		mark(object->head);
		mark(object->tail);
	}
}

void markAll(VM* vm){
	int i=0;
	for(;i<vm->stackSize;i++)
		mark(vm->stack[i]);
}

// 清理标记的内存
void sweep(VM* vm)
{
	Object** object = &vm->firstObject;
	while(*object) {
		if (!(*object)->marked) {
			// 移除
			Object* unreached = *object;

			*object = unreached->next;
			free(unreached);

			vm->numObjects--;
		}
		else {
			(*object)->marked = 0;
			object = &(*object)->next;
		}
	} 
}

// 回收，并动态增加
void gc(VM* vm) {
	int numObjects = vm->numObjects;

	markAll(vm);
	sweep(vm);

	vm->maxObjects = vm->numObjects * 2;
}

void test1() {
	printf("Test 1: Objects on stack are preserved.\n");
	{
		VM* vm = newVM();
		pushInt(vm, 1);
		pushInt(vm, 2);

		gc(vm);
		assert(vm->numObjects == 2, "Should have preserved objects.");
		freeVM(vm);
	}
}

void test2() {
	printf("Test 2: Unreached objects are collected.\n");
	{
		VM* vm = newVM();
		pushInt(vm, 1);
		pushInt(vm, 2);
		pop(vm);
		pop(vm);

		gc(vm);
		assert(vm->numObjects == 0, "Should have collected objects.");
		freeVM(vm);
	}
}

void test3() {
	printf("Test 3: Reach nested objects.\n");
	{
		VM* vm = newVM();
		pushInt(vm, 1);
		pushInt(vm, 2);
		pushPair(vm);
		pushInt(vm, 3);
		pushInt(vm, 4);
		pushPair(vm);
		pushPair(vm);

		gc(vm);
		assert(vm->numObjects == 7, "Should have reached objects.");
		freeVM(vm);
	}
}

void test4() {
	printf("Test 4: Handle cycles.\n");
	{
		VM* vm = newVM();
		pushInt(vm, 1);
		pushInt(vm, 2);

		{
			Object* a = pushPair(vm);
			pushInt(vm, 3);
			pushInt(vm, 4);

			{
				Object* b = pushPair(vm);
				/* Set up a cycle, and also make 2 and 4 unreachable and collectible. */
				a->tail = b;
				b->tail = a;
			}
		}

		gc(vm);
		assert(vm->numObjects == 4, "Should have collected objects.");
		freeVM(vm);
	}
}

void perfTest() {
	printf("Performance Test.\n");
	{
		VM* vm = newVM();
		int i = 0;
		int j = 0;
		int k = 0;
		for (; i < 1000; i++) {
			for (; j < 20; j++) {
				pushInt(vm, i);
			}

			for (; k < 20; k++) {
				pop(vm);
			}
		}
		freeVM(vm);
	}
}

int main(int argc, const char * argv[]) {
	test1();
	test2();
	test3();
	test4();
	perfTest();

	return 0;
}