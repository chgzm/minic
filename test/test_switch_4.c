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
    int ret = 0;
    char c = 'a';
    if (is_symbol(c)) {
        ret += 1;
    } else {
        ret += 2;
    }

    char d = '+';
    if (is_symbol(d)) {
        ret += 4;
    } else {
        ret += 8;
    }

    return ret;
}
