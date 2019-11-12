int main() {
    int ret = 0;
    int a = 1;
    ret += a;

    if (1) {
        int c = 2;
        ret += c;
    }

    while (1) {
        int d = 4;
        ret += d;
        break;
    }

    for (int i = 0; i < 1; ++i) {
        int e = 8;
        ret += e;
    }

    switch (a) {
    case 1: {
        int f = 16;
        ret += f;
        break;
    }
    default: {
        break;
    }
    }

    return ret;
}
