int func(int a) {
    int b = 0;
    switch (a) {
    case 1: {
        b += a * 1;
        break;
    }
    case 2: {
        b += a * 2;
        break;
    }
    case 3: {
        b += a * 3;
        break;
    }
    default: {
        b += a * 4;
        break;
    }
    }

    return b;
}

int main() {
    return func(1) + func(2) + func(3) + func(4) + func(10);
}
