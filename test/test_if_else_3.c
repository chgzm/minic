int main() {
    int a = 2;
    if (a < 0) {
        a = 100;
    } else if (a < 10) {
        a = 200; 
    } else {
        a = 300;
    }

    return a;
}
