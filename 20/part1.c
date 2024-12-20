
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
void add_cheat(struct pos p1, struct pos p2, unsigned c) {
#ifdef SAVE_CHEATS
    assert(cheat_count < MAX_CHEATS);
    cheats[cheat_count].start = p1;
    cheats[cheat_count].end = p2;
    cheats[cheat_count].cost = c;
#else
    (void)p1; (void)p2; (void)c;
#endif
    cheat_count += 1;
}
int cheat_compare(const void *v1, const void *v2) {
    struct cheat *ch1 = (struct cheat *)v1;
    struct cheat *ch2 = (struct cheat *)v2;
    return (int)ch2->cost - (int)ch1->cost;
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

void find_cheats(int limit) {
    for (unsigned p = 0; p < path_len; p++) {
        for (unsigned d1 = 0; d1 < array_count(dirs); d1++) {
            unsigned newx = path[p].x + dirs[d1].dx;
            unsigned newy = path[p].y + dirs[d1].dy;
            if (get_pos(newx, newy) != '#') continue;
            if ((newx != 0) && (newx != row_width - 1) && (newy != 0) && (newy != row_count - 1)) {
                for (unsigned d2 = 0; d2 < array_count(dirs); d2++) {
                    //unsigned d2 = d1;
                    unsigned new2x = newx + dirs[d2].dx;
                    unsigned new2y = newy + dirs[d2].dy;
                    if (((newx != new2x) || (newy != new2y)) && get_pos(new2x, new2y) != '#') {
                        // check if the cost at the new pos higher than the old poss 
                        struct pos new2 = { new2x, new2y };
                        int save = (int)cost_get(new2x, new2y) - (int)cost_get(path[p].x, path[p].y) - 2;
                        if (save >= limit) {
                            add_cheat(path[p], new2, save);
                            //if (debug) printf(" add cheat (%u, %u) -> (%u, %u), diff = %d\n", path[p].x, path[p].y, new2x, new2y, save);
                        }
                    }
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
    unsigned save_limit = 1;
    if (strcmp(fname, "input.txt") == 0) save_limit = 100;

    readData(fname);
    printf("The field is %ux%u\n", row_width, row_count);
    if (debug) print_map();
    struct pos start = find_pos('S'), end = find_pos('E');
    printf("Path is from (%u, %u) to (%u, %u)\n", start.x, start.y, end.x, end.y);

    // implement algorithm
    find_path(start, end);
    printf("The cost from start to end is %d\n", cost_get(end.x, end.y));
    find_cheats(save_limit);
    //qsort(cheats, cheat_count, sizeof(struct cheat), cheat_compare);
    //print_cheats();
    printf("There are %u (big) cheats\n", cheat_count);

    printf("Info: the solution for the sample data should be %d\n", 44);
    printf("Info: the solution for the actual data should be %d\n", 1417);
    return EXIT_SUCCESS;
}

