typedef struct Entry Entry;
struct Entry {
    int a;
    int b;
    Entry* next;
};

typedef struct Sample Sample;
struct Sample {
    Entry** entries;
};

int func(Sample* s) {
    int c = 0;
    for (int i = 0; i < 8; ++i) {
        if (s->entries[i] == 0) {
            break;
        }

        c += s->entries[i]->a;
        c += s->entries[i]->b;

        if (s->entries[i]->next != 0) {
            Entry* current = s->entries[i]->next;
            c += current->a;
            c += current->b;
        }
    }   

    return c;
}

int main() {
    Sample* sample = calloc(1, sizeof(Sample));

    sample->entries = calloc(8, sizeof(Entry*));

    sample->entries[0]       = calloc(1, sizeof(Entry));
    sample->entries[0]->a    = 1;
    sample->entries[0]->b    = 2;
    sample->entries[0]->next = calloc(1, sizeof(Entry));

    Entry* current = sample->entries[0]->next;
    current->a = 16;
    current->b = 32;
    current->next = 0;
  
    sample->entries[1]       = calloc(1, sizeof(Entry));
    sample->entries[1]->a    = 4;
    sample->entries[1]->b    = 8;
    sample->entries[1]->next = 0;

    return func(sample);
}
