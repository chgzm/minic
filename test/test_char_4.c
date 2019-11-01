char* func() {
    char* ret = malloc(sizeof(char) * 4);
    ret[0] = 'h';
    ret[1] = 'e';
    ret[2] = 'y';
    ret[3] = '\0';

    return ret;
}

int main() {
    char* str = func();
    printf("%s\n", str);
    
    return 0;
}
