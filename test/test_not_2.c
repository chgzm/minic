int func() {
    return 0;
}

int main() {
    int a = 0;
    if (a == 0 && !func()) {
        return 0;
    } else {
        return 1;
    }
}
