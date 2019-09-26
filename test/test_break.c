int main() {
    int i = 0;
    int a = 0;
    for (i = 0; i < 5; ++i) {
        if (i == 4) {
            break;
        }

        a = a + i;
    }

    return a;
}
