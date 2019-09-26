int main() {
    int i;
    int a[10];

    for (i = 0; i < 10; ++i) {
        a[i] = i;
    }

    return a[0] + a[1] + a[9];
}
