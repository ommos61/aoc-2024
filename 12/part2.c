
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
struct pos *determine_area(struct pos p) {
    struct pos *area = new_pos(p.x, p.y, NULL);
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
                    area = new_pos(newx, newy, area);
                    dqpos_add(&dq, new_pos(newx, newy, NULL));
                    set_handled(newx, newy);
                }
            }
        }
        free(p);
    }
    return area;
}

int count_area(struct pos *area) {
    int count = 0;
    struct pos *p = area;
    while (p != NULL) {
        count += 1;
        p = p->next;
    }
    return count;
}

void free_area(struct pos *area) {
    struct pos *p = area;
    while (p != NULL) {
        struct pos *tmp = p;
        p = p->next;
        free(tmp);
    }
}

int *filled = NULL;
#define is_filled(x,y) (*(filled + (y) * row_width + (x)))
#define set_filled(x,y) (*(filled + (y) * row_width + (x)) = 1)
void print_filled(void) {
    for (unsigned y = 0; y < row_count; y++) {
        for (unsigned x = 0; x < row_width; x++) {
            if (is_filled(x, y)) printf("X"); else printf(".");
        }
        printf("\n");
    }
}
long determine_fences(struct pos *area) {
    (void)area;
    long fences_x = 0L;
    long fences_y = 0L;
    if (filled == NULL) {
        filled = malloc(row_width * row_count * sizeof(int));
        assert(filled != NULL);
    }
    memset(filled, 0, row_width * row_count *sizeof(int));
    struct pos *p = area;
    while (p != NULL) {
        set_filled(p->x, p->y);
        p = p->next;
    }
    //if (debug) print_filled();

    for (unsigned y = 0; y < row_count; y++) {
        int prev_top_fence = 0;
        int prev_bot_fence = 0;
        for (unsigned x = 0; x < row_width; x++) {
            if (is_filled(x, y)) {
                // check for top fence
                if ((!prev_top_fence) && ((y == 0) || !is_filled(x, y - 1))) {
                    if (debug) printf(" starting top fence at (%d, %d)\n", x, y);
                    prev_top_fence = 1;
                    fences_y += 1;
                } else if ((y != 0) && is_filled(x, y - 1)) {
                    prev_top_fence = 0;
                }
                // check for bottom fence
                if ((!prev_bot_fence) && ((y >= row_count - 1) || !is_filled(x, y + 1))) {
                    if (debug) printf(" starting bottom fence at (%d, %d)\n", x, y);
                    prev_bot_fence = 1;
                    fences_y += 1;
                } else if ((y < row_count - 1) && is_filled(x, y + 1)) {
                    prev_bot_fence = 0;
                }
            } else {
                prev_top_fence = 0;
                prev_bot_fence = 0;
            }
        }
    }
    for (unsigned x = 0; x < row_width; x++) {
        int prev_left_fence = 0;
        int prev_right_fence = 0;
        for (unsigned y = 0; y < row_count; y++) {
            if (is_filled(x, y)) {
                // check for top fence
                if ((!prev_left_fence) && ((x == 0) || !is_filled(x - 1, y))) {
                    if (debug) printf(" starting left fence at (%d, %d)\n", x, y);
                    prev_left_fence = 1;
                    fences_x += 1;
                } else if ((x != 0) && is_filled(x - 1, y)) {
                    prev_left_fence = 0;
                }
                // check for bottom fence
                if ((!prev_right_fence) && ((x >= row_width - 1) || !is_filled(x + 1, y))) {
                    if (debug) printf(" starting right fence at (%d, %d)\n", x, y);
                    prev_right_fence = 1;
                    fences_x += 1;
                } else if ((x < row_width - 1) && is_filled(x + 1, y)) {
                    prev_right_fence = 0;
                }
            } else {
                prev_left_fence = 0;
                prev_right_fence = 0;
            }
        }
    }
    return fences_y + fences_x;;
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
        struct pos *area = determine_area(start);
        if (debug) printf(" the area for (%d, %d) is %d\n", start.x, start.y, count_area(area));
        long fences = determine_fences(area);
        if (debug) printf(" the fences for (%d, %d) is %ld\n", start.x, start.y, fences);
        cost += (fences * count_area(area));
        free_area(area);
    }
    printf("the calculated total cost is %ld\n", cost);

    printf("Info: the solution for the sample data should be %ld\n", 1206L);
    printf("Info: the solution for the actual data should be %ld\n", 881182L);
    return EXIT_SUCCESS;
}

