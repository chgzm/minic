void func(int* a, int* b) {
    ++(*a);
    ++(*b);
}

int main() {
    int a = 5;
    int b = 10;

    func(&a, &b); 

    return a + b;
}
