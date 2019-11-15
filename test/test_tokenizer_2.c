int read_directive(const char* p, int* pos) {
    int ret = 0;
    ++(*pos);

    int len = 0;
    while (p[*pos + len] != ' ' && p[*pos + len] != '\n') {
        ++len;
    }

    *pos += len; 

    int a = 0;
    while (p[*pos + a] == ' ') {
        ++a;
        ret += 1;
    }

    while (isalpha(p[*pos + a]) || p[*pos + a] == '_' || isdigit(p[*pos +a])) {
        ++a;
        ret += 1;
    }

    while (p[*pos + a] == ' ') {
        ++a;
        ret += 1;
    }

    if (p[*pos + a] != '\n') {
        ret += 1;
    }

    return *pos + len + ret;
}

int main() {
    char* str = malloc(sizeof(char) * 15);
    strncpy(str, "#define HOGE 3", 15);
    const char* p = str;

    int pos = 0;
    return read_directive(p, &pos); // 7 + 6 + 7
}
