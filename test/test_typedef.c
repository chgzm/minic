typedef struct Sample Sample;
struct Sample {
    int a;
    int b;
};

int main() {
    Sample s;
    s.a = 1;
    s.b = 2;

    return s.a + s.b;
}
