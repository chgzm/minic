typedef struct Sample Sample;
struct Sample {
    int a;
    int b;
    int c;
};

void func(Sample* s, Sample* s2) {
    s->a = 10;
    s->b = 20;
    s->c = 30;

    s2->a = 10;
    s2->b = 20;
    s2->c = 30;
}

int main() {
    struct Sample s;
    struct Sample s2;
    s.a = 1;
    s.b = 2;
    s.c = 3;

    s2.a = 1;
    s2.b = 2;
    s2.c = 3;


    func(&s, &s2);

    return s.a + s.b + s.c + s2.a + s2.b + s2.c;
}
