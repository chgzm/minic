enum {
    SAMPLE_1,
    SAMPLE_2
};

enum Test {
    TEST_1,
    TEST_2,
    TEST_3,
};

int main() {
    int a = SAMPLE_1 + SAMPLE_2;
    a += (TEST_1 + TEST_2 + TEST_3);
    return a;
}
