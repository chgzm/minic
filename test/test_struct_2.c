struct Sample {
    int a;
    int b;
};

struct Sample2 {
    int a;
    int b;
    int c;
};

int main() {
    struct Sample s1;
    struct Sample2 s2;  

    s1.a = 1;
    s1.b = 2;
    s2.a = 3;
    s2.b = 4;
    s2.c = 5;

    return s1.a + s1.b + s2.a + s2.b + s2.c;
}
