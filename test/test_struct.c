struct Sample {
    int a;
    int b;
};

int main() {
    struct Sample s;
    int c = 3;
    s.a = 1;
    s.b = 2;

    return s.a + s.b + c;
}
