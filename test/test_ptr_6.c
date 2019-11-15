char* func() {
    char* s = calloc(8, sizeof(char));
    strncpy(s, "hello", 8);
    return s;
}

int main() {
    char* s = func();
    if (s == 0) {
        return 0;
    } else {
        return 1;
    }
}
