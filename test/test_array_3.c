int func(int* a) {
    return a[0] + a[1];
}

int main() {
    int a[2];
    a[0] = 1;
    a[1] = 2;
    return func(a);
}
