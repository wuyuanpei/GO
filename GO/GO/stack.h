#pragma once
// Notice: this stack can deal with overflow
struct stack {
	int* buf;
	int sp;
	int size;
};
stack *stack_init(int size);
void stack_end(stack *s);
void push(stack *s, int element);
int pull(stack *s);
