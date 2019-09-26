int main() {
    int i = 0;
    int a = 0;
    for (i = 0; i < 5; ++i) {
        if (i == 3) {
            continue;
        }

        a = a + i;
    }

    return a;
}
