int func(int a) {
    int b = 100;
    int c = a + b;
    return c;
}

int main() {
    int v = func(5) + func(10);
    return v;
}
