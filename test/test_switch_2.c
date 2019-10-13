int func(int a) {
    switch (a) {
    case 1:
        return 1;
    case 2: {
        return 2 * 2;
    }
    case 3: {
        return 3 * 3;
    }
    default: {
        return 0;            
    }
    }
}

int main() {
    return func(1) + func(2) + func(3) + func(4);
}
