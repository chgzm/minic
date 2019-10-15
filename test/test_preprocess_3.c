int main() {
    int a = 0;

#ifndef HOGE
#define HOGE
    a = 1;
#endif

#ifndef HOGE
    a = 2;
#endif

    return a;
}
