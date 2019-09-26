int main() {
    int i = 0;
    int a = 0;
    while (1) {
        while (1) {
           if (i == 4) {
               break;
           }

           a = a + i;
           ++i;
        }

        if (a == 6) {
            a = 100;
            break;
        }
    }

    return a;
}
