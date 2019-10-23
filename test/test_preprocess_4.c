int main() {
    int a = 0;

#ifndef HOGE
#define HOGE
    a += 1;
#else 
    a += 2;
#endif

#ifndef HOGE
    a += 4;
#else
    a += 8;
#endif

    return a;
}
