int main() {
    int i = 0;
    int a = 0;
    while (i < 5) {
        if (i == 4) {
            break;
        }

        a = a + i;
        ++i;
    }

    return a;
}
