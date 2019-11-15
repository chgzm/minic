int read_identifier(const char* p, int* pos) {
    int ret = 0;
    int len = 0;
    while (isdigit(p[*pos + len]) || isalpha(p[*pos + len]) || p[*pos + len] == '_') {
        ++len;
    }

    switch (p[*pos]) {
    case 'b': {
        if (len != 5) {
            break;
        }

        if (strncmp("break", &p[*pos], 5) == 0) {
            ret += 1;
        }

        break;
    }
    case 'c': {
        if (len != 4 && len != 5 && len != 8) {
            break;
        }

        if      (strncmp("char",     &p[*pos], 4) == 0) { ret += 2; }
        else if (strncmp("case",     &p[*pos], 4) == 0) { ret += 2; }
        else if (strncmp("const",    &p[*pos], 5) == 0) { ret += 2; }
        else if (strncmp("continue", &p[*pos], 8) == 0) { ret += 2; }

        break;
    }
    case 'd': {
        if (len != 6 && len != 7) {
            break;
        }

        if      (strncmp("double",  &p[*pos], 6) == 0) { ret += 3; }
        else if (strncmp("default", &p[*pos], 7) == 0) { ret += 3; }

        break;
    }
    case 'e': {
        if (len != 4) {
            break;
        }

        if      (strncmp("else",   &p[*pos], 4) == 0) { ret += 4; } 
        else if (strncmp("enum",   &p[*pos], 4) == 0) { ret += 4; }

        break;
    }
    case 'f': {
        if (len != 3) {
            break;
        }

        if (strncmp("for",&p[*pos], 3) == 0) { 
            ret += 5;
        }

        break;
    }
    case 'i': {
        if (len != 2 && len != 3) {
            break;
        }

        if      (strncmp("if",  &p[*pos], 2) == 0) { ret += 6; }
        else if (strncmp("int", &p[*pos], 3) == 0) { ret += 6; }

        break;
    }
    case 'r': {
        if (len != 6) {
            break;
        }

        if (strncmp("return", &p[*pos], 6) == 0) {
            ret += 7;
        }

        break;
    }
    case 's': {
        if (len != 6) {
            break;
        }

        if      (strncmp("struct", &p[*pos], 6) == 0) { ret += 8; }
        else if (strncmp("switch", &p[*pos], 6) == 0) { ret += 8; }
        else if (strncmp("static", &p[*pos], 6) == 0) { ret += 8; }
        else if (strncmp("sizeof", &p[*pos], 6) == 0) { ret += 8; }

        break;
    }
    case 't': {
        if (len != 7) {
            break;
        }

        if (strncmp("typedef", &p[*pos], 7) == 0) { 
            ret += 9;
        }

        break;
    }
    case 'v': {
        if (len != 4) {
            break;
        }

        if (strncmp("void", &p[*pos], 4) == 0) { 
            ret += 10;
        }

        break;
    }
    case 'w': {
        if (len != 5) {
            break;
        }

        if (strncmp("while", &p[*pos], 5) == 0) {
            ret += 11;
        }

        break;
    }
    default: {
        ret += 12;
        break;
    }
    }

    *(pos) += len;

    return ret;
}

int main() {
    char* str = malloc(sizeof(char) * 256);
    strncpy(str, "while hoge typedef struct break  char default for", 256);

    int ret = 0;
    int pos = 0;
    while (str[pos]) {
        if (isspace(str[pos])) {
            ++pos;
            continue;
        }
        ret += read_identifier(str, &pos); 
    }

    return ret; // 11 + 12 + 9 + 8 + 1 + 2 + 3 + 5
}
