typedef struct Vector Vector;
struct Vector {
    void** elements;
    int    size;
    int    capacity;
};

Vector* create_vector() {
    Vector* vec    = malloc(sizeof(Vector));
    vec->elements  = malloc(sizeof(void*) * 16);
    vec->capacity  = 16;
    vec->size      = 0;

    return vec;
}

void vector_push_back(Vector* vec, void* e) {
    if (vec->size == vec->capacity) {
        vec->capacity *= 2;
        vec->elements = realloc(vec->elements, sizeof(void*) * vec->capacity);
    }

    // void** elements = vec->elements;
    vec->elements[vec->size] = e;
    ++vec->size;
}

int main() {
    Vector* vec = create_vector();

    int a = 10;
    int b = 20;

    vector_push_back(vec, &a);
    vector_push_back(vec, &b);

    void** elements = vec->elements;

    int* pa = elements[0];
    int* pb = elements[1];

    return *pa + *pb;
}
