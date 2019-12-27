#include "stack.h"
#include <memory>

stack* stack_init(int size) {
	if (size < 1) {
		printf("ERROR: stack initialization failed");
		return NULL;
	}
	stack *s = (stack *)malloc(sizeof(stack));
	if (s == NULL) {
		printf("ERROR: stack initialization failed");
		return NULL;
	}
	s->buf = (int*)malloc(sizeof(int) * size);
	if (s->buf == NULL) {
		printf("ERROR: stack initialization failed");
		return NULL;
	}
	s->sp = 0;
	s->size = size;
	return s;
}
void stack_end(stack *s) {
	free(s->buf);
	free(s);
}
void push(stack* s, int element) {
	if (s->sp == s->size) {
		printf("ERROR: stack overflow");
		return;
	}
	s->buf[(s->sp)++] = element;
}
int pull(stack* s) {
	if (s->sp == 0) {
		printf("ERROR: no element left in the stack");
		return 0;
	}
	return s->buf[--(s->sp)];
}