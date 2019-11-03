typedef struct Sample Sample;
struct Sample {
    int a;
    int b;
    int c;
};

int main() {
    Sample s;
    int a = sizeof(struct Sample) + sizeof(Sample) + sizeof(Sample*) + sizeof(s);
    return a;
}
