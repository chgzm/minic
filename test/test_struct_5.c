typedef struct Sample Sample;
struct Sample {
    int a;
    Sample* next;
};

int func(Sample* s) {
    Sample* current = s;
    int cnt = 0;
    while (current != 0) {
        cnt += current->a;
        current = current->next;
    }

    return cnt;
}

int main() {
    Sample s;
    Sample s2;
    Sample s3;
    s.a  = 1;
    s2.a = 2;
    s3.a = 3;
    
    s.next = &s2;
    s2.next = &s3;
    s3.next = 0;

    return func(&s);
}
