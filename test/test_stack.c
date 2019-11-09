#define NULL 0

typedef struct Stack Stack;
struct Stack {
    void** elements;
    int    top;
    int    capacity;
};

Stack* create_stack() {
    Stack* stack    = malloc(sizeof(Stack));
    stack->elements = malloc(sizeof(void*) * 16);
    stack->capacity = 16;
    stack->top      = -1;

    return stack;
}

void stack_push(Stack* stack, void* e) {
    ++(stack->top);
    if (stack->top == stack->capacity) {
        stack->capacity *= 2;
        stack->elements = realloc(stack->elements, sizeof(void*) * stack->capacity);
    }

    stack->elements[stack->top] = e;
}

void* stack_top(Stack* stack) {
    if (stack->top < 0) {
        return NULL;
    }

    return stack->elements[stack->top];
}

void stack_pop(Stack* stack) {
    --(stack->top);
}

int main() {
    Stack* stack = create_stack();

    int a = 1;
    int b = 2;
    int c = 4;

    stack_push(stack, &a);
    stack_push(stack, &b);
    stack_push(stack, &c);
   
    int* pc = stack_top(stack);
    stack_pop(stack);

    int* pb = stack_top(stack);
    stack_pop(stack);

    return *pc + *pb;
}
