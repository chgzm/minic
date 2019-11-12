#define NULL 0
#define bool int
#define true 1
#define false 0

typedef struct StrPtrMapEntry StrPtrMapEntry;
struct StrPtrMapEntry {
    char*            key;
    void*            val;
    StrPtrMapEntry* next;
};

typedef struct StrPtrMap StrPtrMap;
struct StrPtrMap {
    StrPtrMapEntry** entries;
    int               size;
    int               capacity;
};

StrPtrMap* create_strptrmap(int capacity) {
    StrPtrMap* map = calloc(1, sizeof(StrPtrMap));
    map->size      = 0;
    map->capacity  = capacity;
    map->entries   = calloc(capacity, sizeof(StrPtrMapEntry*));

    return map;
}

int calc_hash(const char* str) {
    int h = 0;
    int pos = 0;
    while (str[pos] != '\0') {
        h += str[pos];
        ++pos;
    }

    return h;
}

void strptrmap_put(StrPtrMap* map, const char* key, void* val) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;

    if (map->entries[index] == NULL) {
        map->entries[index] = calloc(1, sizeof(StrPtrMapEntry));
        map->entries[index]->key  = strdup(key);
        map->entries[index]->val  = val;
        map->entries[index]->next = NULL;
    }
    else {
        StrPtrMapEntry* current = map->entries[index];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next       = calloc(1, sizeof(StrPtrMapEntry));
        current->next->key  = strdup(key);
        current->next->val  = val;
        current->next->next = NULL;
    }

    ++(map->size);
}

int strptrmap_contains(StrPtrMap* map, const char* key) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;

    if (map->entries[index] == NULL) {
        return false;
    }
    else {
        StrPtrMapEntry* current = map->entries[index];
        while (current != NULL) {
            if (strcmp(current->key, key) == 0) {
                return true;
            }
            current = current->next;
        }

        return false;
    }
}

void* strptrmap_get(StrPtrMap* map, const char* key) {
    const int hash  = calc_hash(key);
    const int index = hash % map->capacity;

    if (map->entries[index] == NULL) {
        return NULL;
    }
    else {
        StrPtrMapEntry* current = map->entries[index];
        while (current != NULL) {
            if (strcmp(current->key, key) == 0) {
                return current->val;
            }

            current = current->next;
        }

        return NULL;
    }
}

int main() {
    StrPtrMap* map = create_strptrmap(8);

    int a = 1;
    int b = 2;
    int c = 4;

    strptrmap_put(map, "a", &a); 
    strptrmap_put(map, "b", &b); 
    strptrmap_put(map, "c", &c); 

    int r = strptrmap_contains(map, "a");
    r += strptrmap_contains(map, "b");
    r += strptrmap_contains(map, "c");
    r += strptrmap_contains(map, "d");

    int* pa = strptrmap_get(map, "a");
    int* pb = strptrmap_get(map, "b");
    int* pc = strptrmap_get(map, "c");

    r += (*pa + *pb + *pc);

    return r;
}

