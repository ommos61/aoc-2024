
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
#define GET_POS(x,y) (rows[(y)][(x)])
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
            if (row_width == 0) {
                row_width = strlen(line);
            } else {
                assert(row_width == strlen(line));
            }
            char *tmp = malloc(row_width + 1);
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
         printf("%3d %s\n", y, rows[y]);
    }
    printf("    ");
    for (unsigned x = 0; x < row_width; x++) {
        printf("%d", x % 10);
    }
    printf("\n");
}

void find_pos(char c, unsigned *posx, unsigned *posy) {
    for (unsigned y = 0; y < row_count; y++) {
        for (unsigned x = 0; x < row_width; x++) {
            if (rows[y][x] == c) {
                *posx = x;
                *posy = y;
            }
        }
    }
}

#define DIR_EAST    0
#define DIR_NORTH   1
#define DIR_WEST    2
#define DIR_SOUTH   3
#define DIR_UNKNOWN 4
char *dirchars = ">^<v?";
struct dir { int dx, dy; } dirs[] = { { 1, 0 }, { 0, -1 }, { -1, 0 }, { 0, 1 } };
#define NEXTDIR(d) ((d + 1) % array_count(dirs))
#define PREVDIR(d) ((d + array_count(dirs) - 1) % array_count(dirs))
struct state {
    unsigned posx, posy;
    unsigned dir;
    unsigned cost;
    unsigned depth;
    struct state *next;
};
struct state *new_state(unsigned x, unsigned y, unsigned dir, unsigned cost, unsigned depth) {
    struct state *tmp = malloc(sizeof(struct state));
    assert(tmp != NULL);
    tmp->posx = x; tmp->posy = y; tmp->dir = dir; tmp->cost = cost; tmp->depth = depth; tmp->next = NULL;
    return tmp;
}
struct state *copy_state(struct state *st) {
    struct state *newstate = malloc(sizeof(struct state)); assert(new_state != NULL);
    *newstate = *st;
    return newstate;
}

unsigned *visited = NULL;
void init_visited(void) {
    if (visited == NULL) {
        unsigned count = row_count * row_width;
        visited = malloc(count * sizeof(int));
        assert(visited != NULL);
        for (unsigned i = 0; i < count; i++) visited[i] = 0;
    }
}
struct state *stack_pop(struct state **stack) {
    assert(*stack != NULL);
    struct state *top = (*stack);
    *stack = top->next;
    return top;
}
struct path {
    struct state *start;
    unsigned cost;
    struct path *next;
} *paths = NULL;
void add_path(struct state *stack) {
    struct path *p = malloc(sizeof(struct path)); assert(p != NULL);
    p->cost = stack->cost; p->start = NULL; p->next = NULL;
    unsigned depth = stack->depth;
    while (depth >= 0) {
        assert(stack->depth == depth);
        struct state *st = copy_state(stack);
        st->next = p->start; p->start = st;
        if (depth == 0) {
            break;
        } else {
            // find the one with the next depth
            depth -= 1;
            while (stack->depth != depth) stack = stack->next;
        }
    }
    p->next =paths;
    paths = p;
}
void print_paths(void) {
    struct path *p = paths;
    while (p != NULL) {
        printf("cost = %10d: ", p->cost);
        struct state *st = p->start;
        while (st != NULL) {
            printf("(%d, %d) ", st->posx, st->posy);
            st = st->next;
        }
        printf("\n");
        p = p->next;
    }
}
void print_stack(struct state *stack) {
    struct state *st = stack;
    printf("--------- stack ----------\n");
    if (st == NULL) printf(" <empty>\n");
    while (st != NULL) {
        printf("{ (%d, %d) '%c' depth=%d }\n", st->posx, st->posy, dirchars[st->dir], st->depth);
        st = st->next;
    }
}

#define GET_VISITED(x,y) (*(visited + row_width * (y) + (x)))
#define SET_VISITED(x,y,v) (*(visited + row_width * (y) + (x)) = (v))
#define COST_STRAIGHT 1
#define COST_TURN     1000
void find_path(struct state *start, struct state *end) {
    init_visited();
    struct state *stack = NULL;
    stack = new_state(start->posx, start->posy, start->dir, 0, 0);
    SET_VISITED(start->posx, start->posx, 1);
    while (stack != NULL) {
        //if (debug) { printf("hit <enter> to continue..."); getchar(); printf("\n"); }
        struct state *st = stack_pop(&stack);
        char save = rows[st->posy][st->posx];
        rows[st->posy][st->posx] = dirchars[st->dir];
        if (debug) printf("handling (%d, %d) dir = '%c', cost = %d\n", st->posx, st->posy, dirchars[st->dir], st->cost);
        if (debug) print_map();
        if (debug) print_stack(stack);
        if ((st->posx == end->posx) && (st->posy == end->posy)) {
            if (1) printf(" ========> a path reached the end for cost %d\n", st->cost);
            if (debug) { printf("hit <enter> to continue..."); getchar(); printf("\n"); }
            if (st->cost < end->cost) end->cost = st->cost;
            //print_stack(stack);
            //add_path(stack);
            if (debug) { printf("---------- paths -----------\n"); print_paths(); }
        }
        unsigned testdirs[3];
        testdirs[0] = st->dir;
        testdirs[1] = NEXTDIR(st->dir);
        testdirs[2] = PREVDIR(st->dir);
        // try the possible directions
        for (unsigned i = 0; i < array_count(testdirs); i++) {
            unsigned newdir = testdirs[i];
            unsigned newx = st->posx + dirs[newdir].dx;
            unsigned newy = st->posy + dirs[newdir].dy;
            char c = GET_POS(newx, newy);
            if (c != '#') { // we can move here
                if (debug) printf(" checking '%c' to (%d, %d) with '%c'\n", dirchars[newdir], newx, newy, c);
                unsigned newcost = st->cost + ((newdir == st->dir) ? COST_STRAIGHT : (COST_TURN + COST_STRAIGHT));
                if (GET_VISITED(newx, newy) == 0) {
                    // push new state on stack
                    struct state *newstate = new_state(newx, newy, newdir, newcost, st->depth + 1);
                    newstate->next = stack; stack = newstate;
                    if (debug) printf("  setting cost (%d, %d), %c to %u\n", newx, newy, dirchars[newdir], newcost);
                    SET_VISITED(newx, newy, 1);
                }
            }
        }
        //SET_VISITED(st->posx, st->posy, 0);
        rows[st->posy][st->posx] = save;
    }
}

struct pos {
    unsigned x, y;
    struct pos *next;
} *path = NULL;
struct pos *new_pos(unsigned x, unsigned y) {
    struct pos *p = malloc(sizeof(struct pos));
    p->x = x; p->y = y; p->next = NULL;
    return p;
}
int is_in_path(struct pos *path, unsigned x, unsigned y) {
    int present = 0;
    struct pos *p  = path;
    while (p != NULL) {
        if ((p->x == x) && (p->y == y)) {
            present = 1;
        }
        p = p->next;
    }
    return present;
}
unsigned lowest_cost = UINT_MAX;
unsigned lowest_count = 0;
int *visited2 = NULL;
int   is_visited(unsigned x, unsigned y) { return *(visited2 + row_width * y + x); }
void set_visited(unsigned x, unsigned y) { *(visited2 + row_width * y + x) = 1; }
void clr_visited(unsigned x, unsigned y) { *(visited2 + row_width * y + x) = 0;; }
unsigned *cost2 = NULL;
unsigned get_cost2(unsigned x, unsigned y) { return *(cost2 + row_width * y + x); }
void     set_cost2(unsigned x, unsigned y, unsigned c) { *(cost2 + row_width * y + x) = c; }

#define MAX_PATH 4096
#define MAX_PATHS 10
struct pos path2[MAX_PATH];
struct pos paths2[MAX_PATHS][MAX_PATH];
unsigned path_len = 0;
unsigned path_count = 0;
void print_path2(struct pos *path, unsigned len) {
    for (unsigned i = 0; i < len; i++) {
         printf("(%d, %d), ", path[i].x, path[i].y);
    }
    printf("\n");
}
unsigned count_any(void) {
    if (debug) printf("checking %u paths of length %u for commons\n", path_count, path_len);
    for (unsigned i = 0; i < path_count; i++) {
        if (debug) print_path2(paths2[i], path_len);
    }
    for (unsigned i = 0; i < path_count; i++) {
        for (unsigned j = 0; j < path_len; j++) {
            struct pos p = paths2[i][j];
            rows[p.y][p.x] = 'O';
        }
    }
    unsigned any = 0;
    for (unsigned y = 0; y < row_count; y++) {
        for (unsigned x = 0; x < row_width; x++) {
            if (rows[y][x] == 'O') any += 1;
        }
    }
    return any;
}

void find_path2(unsigned x, unsigned y, unsigned dir, unsigned endx, unsigned endy, unsigned cost, unsigned len) {
    if (visited2 == NULL) {
        visited2 = malloc(row_count * row_width * sizeof(int)); assert(visited2 != NULL);
        memset(visited2, 0, row_count * row_width * sizeof(int));
    }
    if (cost2 == NULL) {
        cost2 = malloc(row_count * row_width * sizeof(unsigned)); assert(cost2 != NULL);
        for (unsigned i = 0; i < row_count * row_width; i++) *(cost2 + i) = UINT_MAX;
        set_cost2(x, y, cost);
    }
    assert(len < MAX_PATH);
    path2[len].x = x; path2[len].y = y;
    if ((x == endx) && (y == endy)) {
        if (cost <= lowest_cost) {
            if (debug) printf("found path (len = %u) with cost %u\n", len, cost);
            //print_path2(path2, len);
            path_len = len;
            if (cost == lowest_cost) {
                lowest_count += 1;
                assert(path_count < MAX_PATHS);
                memcpy(paths2 + path_count, path2, MAX_PATH * sizeof(struct pos));
                path_count += 1;
            } else {
                lowest_count = 1;
                memcpy(paths2 + 0, path2, MAX_PATH * sizeof(struct pos));
                path_count = 1;
            }
            lowest_cost = cost;
        }
        return;
    }
    unsigned testdirs[3];
    testdirs[0] = dir;
    testdirs[1] = NEXTDIR(dir);
    testdirs[2] = PREVDIR(dir);
    // try the possible directions
    for (unsigned i = 0; i < array_count(testdirs); i++) {
        unsigned newdir = testdirs[i];
        unsigned newx = x + dirs[newdir].dx;
        unsigned newy = y + dirs[newdir].dy;
        char c = GET_POS(newx, newy);
        if (c != '#') { // we can move here
            unsigned newcost = cost + ((newdir == dir) ? COST_STRAIGHT : (COST_TURN + COST_STRAIGHT));
            if (newcost <= get_cost2(newx, newy)) {
                set_cost2(newx, newy, newcost + 1000);
                find_path2(newx, newy, newdir, endx, endy, newcost, len + 1);
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

    readData(fname);
    printf("The maze is %dx%d\n", row_width, row_count);
    unsigned startx, starty, endx, endy;
    find_pos('S', &startx, &starty);
    find_pos('E', &endx, &endy);
    printf("Path should be from (%d, %d) to (%d, %d)\n", startx, starty, endx, endy);
    if (debug) print_map();

    // implement algorithm
    //struct state *start = new_state(startx, starty, DIR_EAST, 0, 0);
    //struct state *end = new_state(endx, endy, DIR_UNKNOWN, UINT_MAX, 0);
    //find_path(start, end);
    find_path2(startx, starty, DIR_EAST, endx, endy, 0, 0);
    printf("The minimum cost to get from the start to the end is %u, with path length %u\n", lowest_cost, path_len);
    printf("The lowest cost occurs %u times\n", lowest_count);
    unsigned common = count_any() + 1; // to accomodate for the end poiunt
    printf("The number of positions common to all shortest paths is %u\n", common);

    printf("Info: the solution for the sample data should be %d\n", 45);
    printf("Info: the solution for the sample 2 data should be %d\n", 64);
    printf("Info: the solution for the actual data should be %d\n", 665);
    return EXIT_SUCCESS;
}

