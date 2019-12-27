#include "stack.h"
#include <memory>

stack* stack_init(int size) {
	if (size < 1) return NULL;
	stack *s = (stack *)malloc(sizeof(stack));
	if (s == NULL) return NULL;
	s->buf = (int*)malloc(sizeof(int) * size);
	if (s->buf == NULL) return NULL;
	s->sp = 0;
	s->size = size;
	return s;
}
void stack_end(stack *s) {
	free(s->buf);
	free(s);
}
void push(stack* s, int element) {
	if (s->sp == s->size) return;
	s->buf[(s->sp)++] = element;
}
int pull(stack* s) {
	if (s->sp == 0) return 0;
	return s->buf[--(s->sp)];
}