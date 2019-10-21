int main() {
    int a = 0;
    int ret = 0;
    if (a == 1 && a == 2) {
        ret += 2;
    }

    if (a == 0 && a == 1) {
        ret += 4;
    }

    if (a == 1 && a == 0) {
        ret += 8;
    }

    if (a == 0 && a < 1) {
        ret += 16;
    }

    return ret;
}
