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

int is_line_comment(const char* p, int pos) {
    return (strncmp(&p[pos], "//", 2) == 0);
}

int is_block_comment_begin(const char* p, int pos) {
    return (strncmp(&p[pos], "/*", 2) == 0);
}

int is_block_comment_end(const char* p, int pos) {
    return (strncmp(&p[pos], "*/", 2) == 0);
}

int main() {
    char* str = malloc(sizeof(char) * 256);
    strncpy(str, "### // aaaa \n bbbb\0", 256);
    const char* p = str;

    int cnt = 0;
    int pos = 0;
    while (p[pos]) {
        if (isspace(p[pos])) {
            cnt += 1;
        } 
        else if (is_line_comment(p, pos)) {
            while (p[pos] != '\n') {
                ++pos;
            } 
            cnt += 2;
            continue;
        }
        else if (is_block_comment_begin(p, pos)) {
            while (!is_block_comment_end(p, pos)) {
                ++pos;
            }
            pos += 2;
            cnt += 3;
            continue;
        }
        else if (p[pos] == '#') {
            cnt += 4;
        }
        else if (p[pos] == 39) {
            cnt += 5;
        }
        else if (p[pos] == '"') {
            cnt += 6;
        }
        else if (isalpha(p[pos])) {
            cnt += 7;
        }
        else if (isdigit(p[pos])) {
            cnt += 8;
        }
        else if (is_symbol(p[pos])) {
            cnt += 9;
        }
        else {
            printf("%c\n", p[pos]);
            return 10;
        }

        ++pos;
    }

    return cnt;
}
