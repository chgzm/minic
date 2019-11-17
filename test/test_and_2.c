int main() {
    int* p = 0;
    int  a = 1;
    if (a != 1 && *p) {
        return a;
    }

    return 100;
}
