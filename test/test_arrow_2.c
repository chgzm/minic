typedef struct Sample Sample;
struct Sample {
    int a;
    int b;
};

void func(Sample* p) {
    p->a = 3;
    p->b = 4;
}

int main() {
    Sample s;
    s.a = 1;
    s.b = 2;

    func(&s);

    return s.a + s.b;
}
