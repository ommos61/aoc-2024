
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_BYTES 4000
struct pos {
    unsigned x, y;
} bytes[MAX_BYTES];
unsigned byte_count = 0;
unsigned memory_size = 0;
unsigned corrupt_bytes = 0;

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        int x = 0, y = 0;
        if (sscanf(line, "%d,%d", &x, &y) == 2) {
            // store the data
            bytes[byte_count].x = x;
            bytes[byte_count].y = y;
            byte_count += 1;
        } else if (errno != 0) {
            perror("sscanf");
        } else {
            fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

char *memory = NULL;
char memory_get(unsigned x, unsigned y) { return *(memory + (y) * memory_size + (x)); }
void memory_set(unsigned x, unsigned y, char c) { *(memory + (y) * memory_size + (x)) = (c); }
void init_memory(void) {
    if (memory == NULL) {
        memory = malloc(memory_size * memory_size * sizeof(char));
        assert(memory != NULL);
    }
    for (unsigned i = 0; i < memory_size * memory_size; i++) memory[i] = '.';
}
void print_memory(void) {
    for (unsigned y = 0; y < memory_size; y++) {
        for (unsigned x = 0; x < memory_size; x++) {
            printf("%c", memory_get(x, y));
        }
        printf("\n");
    }
}

void corrupt_memory(unsigned count) {
    for (unsigned i = 0; i < count; i++) {
        memory_set(bytes[i].x, bytes[i].y, '#');
    }
}

//=================================================================
// deque
struct deque_entry {
    void *value;
    struct deque_entry *next;
};
struct deque {
    struct deque_entry *first, *last;
    unsigned count;
};
struct deque *deque_init(void) {
    struct deque *dq = malloc(sizeof(struct deque));
    assert(dq != NULL);
    dq->first = NULL; dq->last = NULL; dq->count = 0;
    return dq;
}
void deque_add(struct deque *dq, void *val) {
    struct deque_entry *e = malloc(sizeof(struct deque_entry));
    assert(e != NULL);
    e->value = val; e->next = NULL;
    if (dq->first == NULL) dq->first = e;
    if (dq->last != NULL) dq->last->next = e;
    dq->last = e;
    dq->count += 1;
}
void *deque_popleft(struct deque *dq) {
    assert((dq->count > 0) && (dq->first != NULL));
    struct deque_entry *e = dq->first;
    void *val = e->value;
    dq->first = e->next;
    if (dq->first == NULL) dq->last = NULL;
    dq->count -= 1;

    return val;
}
unsigned deque_length(struct deque *dq) {
    return dq->count;
}
//=================================================================

struct state {
    unsigned x, y, cost;
};
struct state *new_state(int x, int y, unsigned cost) {
    struct state *p = malloc(sizeof(struct pos));
    assert(p != NULL);
    p->x = x; p->y = y; p->cost = cost;
    return p;
}

struct dir {
    int dx, dy;
} dirs[] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };

unsigned *costs = NULL;
void init_costs(void) {
    if (costs == NULL) {
        costs = malloc(memory_size * memory_size * sizeof(int));
        assert(costs != NULL);
    }
    for (unsigned i = 0; i <  memory_size * memory_size; i++) costs[i] = UINT_MAX;
}
unsigned cost_get(unsigned x, unsigned y) {
    return *(costs + (y) * memory_size + (x));
}
void cost_set(unsigned x, unsigned y, unsigned c) {
    *(costs + (y) * memory_size + (x)) = (c);
}

unsigned find_path(struct pos start, struct pos end) {
    unsigned len = UINT_MAX;

    struct deque *dq = deque_init();
    deque_add(dq, new_state(start.x, start.y, 0));
    init_costs();
    while (deque_length(dq) != 0) {
        struct state *s = deque_popleft(dq);
        ///if (debug) printf("Handling (%u, %u)\n", s->x, s->y);
        if ((s->x == end.x) && (s->y == end.y)) {
            // found a path to the end
            if (debug) printf(" found path with length %u\n", s->cost);
            if (s->cost < len) len = s->cost;
            continue;
        }
        for (unsigned d = 0; d < array_count(dirs); d++) {
            int newx = s->x + dirs[d].dx;
            int newy = s->y + dirs[d].dy;
            if ((newx >= 0) && (newx < (int)memory_size) && (newy >= 0) && (newy < (int)memory_size)) {
                if (memory_get(newx, newy) != '#') {
                    unsigned newcost = s->cost + 1;
                    if ((newcost) < cost_get(newx, newy)) {
                        deque_add(dq, new_state(newx, newy, newcost));
                        cost_set(newx, newy, newcost);
                    }
                }
            }
        }
        free(s);
    }

    return len;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %u bytes\n", byte_count);
    if (strcmp(fname, "input.txt") == 0) {
        memory_size = 71;
        corrupt_bytes = 1024;
    } else {
        memory_size = 7;
        corrupt_bytes = 12;
    }
    printf("Memory is %ux%u, with %u corrupt bytes\n", memory_size, memory_size, corrupt_bytes);

    // TODO: implement algorithm
    init_memory();
    struct pos start = { 0, 0 };
    struct pos end = { memory_size - 1, memory_size - 1};
    while (corrupt_bytes < byte_count) {
        if (debug) printf("doing %u corrupt bytes\n", corrupt_bytes);
        corrupt_memory(corrupt_bytes);
        unsigned len = find_path(start, end);
        if (len == UINT_MAX) break;
        corrupt_bytes += 1;
    }
    printf("The blocking byte is (%u,%u)\n", bytes[corrupt_bytes - 1].x, bytes[corrupt_bytes - 1].y);

    printf("Info: the solution for the sample data should be %u,%u\n", 6, 1);
    printf("Info: the solution for the actual data should be %u,%u\n", 25, 6);
    return EXIT_SUCCESS;
}

