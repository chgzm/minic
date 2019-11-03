int main() {
    int a = sizeof(void*) + sizeof(char*) + sizeof(int*) + sizeof(double*);
    return a;
}
