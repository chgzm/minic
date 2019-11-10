int func(const char* str) {
    int h = 0;
    int pos = 0;
    while (str[pos] != '\0') {
        h += str[pos];
        ++pos;
    }

    return h;
}

int main() {
    return func("ab");
}
