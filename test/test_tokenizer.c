int is_symbol(char p) {
    switch (p) {
    case '+': case '-':
    case '*': case '/': 
    case '%': case '=': 
    case ';': case ':':
    case '(': case ')':
    case '{': case '}':
    case '[': case ']':
    case '<': case '>':
    case '!': case '?': 
    case '&': case '^': 
    case '|': case '#': 
    case ',': case '.': 
    case '\\': {
        return 1;
    }
    default: {
        return 0;
    }
    }
}

int main() {
    char* str = malloc(sizeof(char) * 8);
    strncpy(str, "a'\"#9+ ", 8);
    const char* p = str;

    int cnt = 0;
    int pos = 0;
    while (p[pos]) {
        if (isspace(p[pos])) {
            cnt += 1;
        } 
        else if (p[pos] == '#') {
            cnt += 2;
        }
        else if (p[pos] == 39) {
            cnt += 4;
        }
        else if (p[pos] == '"') {
            cnt += 8;
        }
        else if (isalpha(p[pos])) {
            cnt += 16;
        }
        else if (isdigit(p[pos])) {
            cnt += 32;
        }
        else if (is_symbol(p[pos])) {
            cnt += 64;
        }
        else {
            printf("%c\n", p[pos]);
            return 100;
        }

        ++pos;
    }

    return cnt;
}
