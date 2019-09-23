int func() {
    int c = 5;
    return c;
}

int main() {
    int a = 5;
    int b = a * func();
    return b;
}
