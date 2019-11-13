int func(int v) {
    return v;
}

int main() {
    int a = 1;
    int b = func(a++);
    int c = func(a++);
    int d = func(a--);
    int e = func(a--);

    return b + c + d + e;
}
