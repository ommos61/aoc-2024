
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
#define MAX_ROWS 150
char *rows[MAX_ROWS];
unsigned row_count = 0;
unsigned row_width = 0;

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

        if (strlen(line) > 0) {
            // store the data
            assert(row_count < MAX_ROWS);
            if (row_width == 0) {
                row_width = strlen(line);
            } else {
                assert(row_width == strlen(line));
            }
            char *tmp = malloc(strlen(line) + 1);
            assert(tmp != NULL);
            strcpy(tmp, line);
            rows[row_count] = tmp;
            row_count += 1;
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

void print_map(void) {
    for (unsigned y = 0; y < row_count; y++) {
        printf("%3u %s\n", y, rows[y]);
    }
    printf("    ");
    for (unsigned x = 0; x < row_width; x++) {
        printf("%d", x % 10);
    }
    printf("\n");
}

struct pos {
    unsigned x, y;
};
char get_pos(unsigned x, unsigned y) {
    return rows[y][x];
}
struct pos find_pos(char c) {
    struct pos position = { 0, 0 };
    for (unsigned y = 0; y < row_count; y++) {
        int found = 0;
        for (unsigned x = 0; x < row_width; x++) {
           if (get_pos(x, y) == c) {
               position.x = x; position.y = y;
               found = 1;
               break;
           }
        }
        if (found) break;
    }
    return position;
}

unsigned *costs = NULL;
void costs_init(void) {
    if (costs == NULL) {
        costs = malloc(row_count * row_width * sizeof(int));
        assert(costs != NULL);
    }
    for (unsigned i = 0; i < row_count * row_width; i++) costs[i] = UINT_MAX;
}
void cost_set(unsigned x, unsigned y, int cost) {
    *(costs + y * row_width + x) = cost;
}
unsigned cost_get(unsigned x, unsigned y) {
    return *(costs + y * row_width + x);
}

struct dir {
    int dx, dy;
} dirs[] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };
#define MAX_PATH (10 * 1024)
struct pos path[MAX_PATH];
unsigned path_len = 0;
#define PATH_ADD(p) { assert(path_len < MAX_PATH); path[path_len] = p; path_len += 1; }
void find_path(struct pos start, struct pos end) {
    costs_init();
    int cost = 0;
    struct pos cur = start;
    PATH_ADD(cur);
    cost_set(cur.x, cur.y, cost);
    while ((cur.x != end.x) || (cur.y != end.y)) {
        //if (debug) printf("At (%u, %u) for cost %d\n", cur.x, cur.y, cost);
        for (unsigned d = 0; d < array_count(dirs); d++) {
            unsigned newx = cur.x + dirs[d].dx;
            unsigned newy = cur.y + dirs[d].dy;
            if (get_pos(newx, newy) != '#') {
                if (cost_get(newx, newy) == UINT_MAX) {
                    cost += 1;
                    cost_set(newx, newy, cost);
                    cur.x = newx; cur.y = newy;
                    PATH_ADD(cur);
                    break;
                }
            }
        }
    }
}

#define MAX_CHEATS 1024
struct cheat {
    struct pos start, end;
    unsigned cost;
} cheats[MAX_CHEATS];
unsigned cheat_count = 0;
int save_cheats = 0;
void add_cheat(struct pos p1, struct pos p2, unsigned c) {
    if (save_cheats) {
        assert(cheat_count < MAX_CHEATS);
        cheats[cheat_count].start = p1;
        cheats[cheat_count].end = p2;
        cheats[cheat_count].cost = c;
    }
    cheat_count += 1;
}
int cheat_compare(const void *v1, const void *v2) {
    struct cheat *ch1 = (struct cheat *)v1;
    struct cheat *ch2 = (struct cheat *)v2;
    return (int)ch1->cost - (int)ch2->cost;
}
void print_cheats(void) {
    unsigned index = 0;
    while (index < cheat_count) {
        unsigned count = 0;
        unsigned save = cheats[index].cost;
        while ((index < cheat_count) && (save == cheats[index].cost)) {
            count += 1;
            index += 1;
        }
        if (debug) printf("Save time %u occurred %u times\n", save, count);
        //if (debug) printf(" cheat (%u, %u) -> (%u, %u), diff = %d\n", ch.start.x, ch.start.y, ch.end.x, ch.end.y, ch.cost);
    }
}

void check_pos(struct pos p, int x, int y, unsigned distance, unsigned limit) {
    if ((x > 0) && (x < (int)row_width - 1) && (y > 0) && (y < (int)row_count - 1)) {
        if (get_pos(x, y) != '#') {
            int saved_time = cost_get(x, y) - cost_get(p.x, p.y) - distance;
            if (saved_time >= (int)limit) {
                struct pos p_end = { x, y };
                add_cheat(p, p_end, saved_time);
            }
        }
    }
}

void find_cheats(unsigned limit, unsigned cheat_time) {
    for (unsigned pi = 0; pi < path_len; pi++) {
        struct pos p = path[pi];
        //unsigned p_cost = cost_get(p.x, p.y);
        for (int dx = 0; dx <= (int)cheat_time; dx++) {
            for (int dy = 0; dy <= (int)cheat_time; dy++) {
                if (((dx + dy) != 0) && ((dx + dy) <= (int)cheat_time)) {
                    check_pos(p, p.x + dx, p.y + dy, dx + dy, limit);
                    if (dy != 0) check_pos(p, p.x + dx, p.y - dy, dx + dy, limit);
                    if (dx != 0) check_pos(p, p.x - dx, p.y + dy, dx + dy, limit);
                    if ((dy != 0) && (dx != 0)) check_pos(p, p.x - dx, p.y - dy, dx + dy, limit);
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }
    unsigned save_limit = 50;
    unsigned cheat_time = 20;
    if (strcmp(fname, "input.txt") == 0) {
        save_limit = 100;
    }

    readData(fname);
    printf("The field is %ux%u\n", row_width, row_count);
    if (debug) print_map();
    struct pos start = find_pos('S'), end = find_pos('E');
    printf("Path is from (%u, %u) to (%u, %u)\n", start.x, start.y, end.x, end.y);

    // implement algorithm
    find_path(start, end);
    printf("The cost from start to end is %d\n", cost_get(end.x, end.y));
    if (save_limit == 50) save_cheats = 1; else save_cheats = 0;
    find_cheats(save_limit, cheat_time);
    //find_cheats(1, 2); (void)cheat_time;
    if (save_limit == 50) {
        qsort(cheats, cheat_count, sizeof(struct cheat), cheat_compare);
        print_cheats();
    }
    printf("There are %u (big) cheats\n", cheat_count);

    printf("Info: the solution for the sample data should be %d\n", 285);
    printf("Info: the solution for the actual data should be %d\n", 1014683);
    return EXIT_SUCCESS;
}

