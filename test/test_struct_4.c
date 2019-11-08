typedef struct Sample Sample;
struct Sample {
    int a;
    Sample* next;
};

int func(Sample* s) {
    return s->next->next->a;
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
