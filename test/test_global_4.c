typedef struct Sample Sample;
struct Sample {
    int a;
    int b;
};

Sample* s;

int main() {
    s = calloc(1, sizeof(Sample));
    s->a = 1;
    s->b = 2;

    return s->a + s->b;
}
