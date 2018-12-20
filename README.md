

#### 学习垃圾回收算法 - 标记清除算法(mark-sweep)

---

##### 原文介绍

 Baby's First Garbage Collector

http://journal.stuffwithstuff.com/2013/12/08/babys-first-garbage-collector/

中文介绍(翻译)  一个简单的垃圾收集器

https://www.cnblogs.com/witton/p/6868966.html

本例子使用的是”标记-清除”（mark-sweep）算法，

作者Github地址：

https://github.com/munificent/mark-sweep

---

##### 垃圾回收机制

垃圾收集背后有这样一个基本的观念：编程语言(大多数的)似乎总能访问无限的内存。而开发者可以一直分配、分配再分配——像魔法一样，取之不尽用之不竭。


##### 垃圾收集算法
目前最基本的垃圾收集算法有四种：

```c++
1、标记-清除算法(mark-sweep)

2、标记-压缩算法(mark-compact)

3、复制算法(copying)

4、引用计数算法(reference counting).
```
---

##### 标记算法

```c++
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
```

##### 删除算法

```c++
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
```
