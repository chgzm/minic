void func(int* a, int* b) {
    a[0] = 10;
    a[1] = 20;

    b[0] = 30;
    b[1] = 40;
}

int main() {
    int a[2];
    int b[2];
    a[0] = 1;
    a[1] = 2;

    b[0] = 3;
    b[1] = 4;

    func(a, b);

    return a[0] + a[1] + b[0] + b[1];
}
