int main() {
    int a = 0;

#ifdef HOGE
    a += 1;
#else 
#define HOGE
    a += 2;
#endif

#ifdef HOGE
    a += 4;
#else
    a += 8;
#endif

    return a;
}
