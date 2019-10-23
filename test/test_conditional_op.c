int main() {
    int r = 0;
    int a = 1;

    r += (a == 1) ? 1 : 2;
    r += (a == 2) ? 4 : 8;

    return r;
}
