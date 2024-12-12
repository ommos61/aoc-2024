
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

struct pos {
    int x;
    int y;
    struct pos *next;
};
struct pos *new_pos(int x, int y, struct pos *p) {
    struct pos *tmp = malloc(sizeof(struct pos));
    assert(tmp != NULL);
    tmp->x = x; tmp->y = y; tmp->next = p;
    return tmp;
}

int *handled = NULL;
#define is_handled(x,y) (*(handled + (y) * row_width + (x)))
#define set_handled(x,y) (*(handled + (y) * row_width + (x)) = 1)
struct pos find_first_unhandled(void) {
    struct pos start = { -1, -1, NULL };
    for (unsigned y = 0; y < row_count; y++) {
        int found = 0;
        for (unsigned x = 0; x < row_width; x++) {
            if (!is_handled(x, y)) {
                start.x = x;
                start.y = y;
                found = 1;
                break;
            }
        }
        if (found) break;
    }
    return start;
}

struct dqpos {
    struct pos *first, *end;
};
void dqpos_init(struct dqpos *dq) {
    dq->first = NULL; dq->end = NULL;
}
void dqpos_add(struct dqpos *dq, struct pos *p) {
    if (dq->first == NULL)
        dq->first = p;
    else
        dq->end->next = p;
    dq->end = p;
}
struct pos *dqpos_popleft(struct dqpos *dq) {
    struct pos *tmp_dqpos = dq->first;
    dq->first = tmp_dqpos->next;
    return tmp_dqpos;
}
int dqpos_empty(struct dqpos *dq) {
    return (dq->first == NULL);
}
struct dir {
    int dx;
    int dy;
} dirs[] = { { 0, 1 }, { 0, -1 }, { 1, 0 }, { -1, 0 } };
long determine_cost(struct pos p) {
    long area = 1;
    long fences = 0;
    char current = rows[p.y][p.x];
    set_handled(p.x, p.y);

    struct dqpos dq;
    dqpos_init(&dq);
    dqpos_add(&dq, new_pos(p.x, p.y, NULL));

    while (!dqpos_empty(&dq)) {
        struct pos *p = dqpos_popleft(&dq);
        for (unsigned d = 0; d < array_count(dirs); d++) {
            int newx = p->x + dirs[d].dx;
            int newy = p->y + dirs[d].dy;
            if ((newx >= 0) && (newx < (int)row_width) && (newy >= 0) && (newy < (int)row_count)) {
                char newchar = rows[newy][newx];
                if ((newchar == current) && !is_handled(newx, newy)) {
                    area += 1;
                    dqpos_add(&dq, new_pos(newx, newy, NULL));
                    set_handled(newx, newy);
                } else {
                    if (newchar != current) fences += 1;
                }
            } else {
                fences += 1;
            }
        }
    }
    if (debug) printf("  (%d, %d) area = %ld, fences = %ld\n", p.x, p.y, area, fences);
    return area * fences;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("The field is %dx%d\n", row_width, row_count);

    // implement algorithm
    if (handled == NULL) {
        handled = malloc(row_count * row_width * sizeof(int));
        assert(handled != NULL);
        memset(handled, 0, row_count * row_width * sizeof(int));
    }
    long cost = 0;
    while (1) {
        struct pos start = find_first_unhandled();
        if ((start.x == -1) && (start.y == -1)) { break; }
        if (debug) printf("  handling (%d, %d)\n", start.x, start.y);
        long thiscost = determine_cost(start);
        cost += thiscost;
    }
    printf("the calculated total cost is %ld\n", cost);

    printf("Info: the solution for the sample data should be %ld\n", 1930L);
    printf("Info: the solution for the actual data should be %ld\n", 1467094L);
    return EXIT_SUCCESS;
}

