typedef struct Sample Sample;
struct Sample {
    int a;
    int b;
};

int main() {
    Sample s;
    Sample* p = &s;
    s.a = 1;
    s.b = 2;

    p->a = 3;
    p->b = 4;

    return p->a + p->b;
}
