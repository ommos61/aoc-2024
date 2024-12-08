
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 0; // 0 = no debug output, 1 = some extra debug output
#define MAX_ROWS 100
char *rows[MAX_ROWS];
unsigned row_width = 0;
unsigned row_count = 0;

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

        unsigned len = strlen(line);
        if (len > 0) {
            // store the data
            if (row_width == 0) {
                row_width = len;
            } else {
                assert(row_width == len);
            }
            char *tmp = malloc(len + 1);
            strcpy(tmp ,line);
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
        printf("%s\n", rows[y]);
    }
}

#define MAX_ANTENNAS 128
struct pos {
    int x;
    int y;
    struct pos *next;
};
struct antenna {
    char frequency;
    struct pos *positions;
} antennas[MAX_ANTENNAS];

void build_antenna_list(void) {
    for (unsigned i = 0; i < MAX_ANTENNAS; i++) {
        antennas[i].frequency = i;
        antennas[i].positions = NULL;
    }
    for (unsigned y = 0; y < row_count; y++) {
        for (unsigned x = 0; x < row_count; x++) {
            int frequency = rows[y][x];
            if (isalpha(frequency) || isdigit(frequency)) {
                struct pos *p = malloc(sizeof(struct pos));
                p->x = x; p->y = y; p->next = antennas[frequency].positions;
                antennas[frequency].positions = p;
            }
        }
    }
}

int mark_antinode(int x, int y) {
    if ((x >= 0) && (x < (int)row_width) && (y >= 0) && (y < (int)row_count)) {
        rows[y][x] = '#';
        return 1;
    }
    return 0;
}

void generate_antinodes(void) {
    for (unsigned c = 0; c < MAX_ANTENNAS; c++) {
        struct pos *p = antennas[c].positions;
        if ((p != NULL) && (p->next != NULL)) {
            if (debug) printf("there are more than 2 antennas for '%c'\n", c);
        }
        while (p != NULL) {
            struct pos *q = p->next;
            while (q != NULL) {
                int dx = p->x - q->x, dy = p->y - q->y;
                int n = 0;
                while (mark_antinode(p->x + n * dx, p->y + n * dy)) { n += 1; }
                n = 0;
                while (mark_antinode(q->x - n * dx, q->y - n * dy)) { n += 1; }
                q = q->next;
            }
            p = p->next;
        }
    }
}

int count_antinodes(void) {
    int count = 0;
    for (unsigned y = 0; y < row_count; y++) {
        for (unsigned x = 0; x < row_count; x++) {
            if (rows[y][x] == '#') {
                count += 1;
            }
        }
    }
    return count;
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
    build_antenna_list();
    generate_antinodes();
    if (debug) print_map();
    int count = count_antinodes();
    printf("Generated number is antinodes is %d\n", count);

    printf("Info: the solution for the sample data should be %d\n", 34);
    printf("Info: the solution for the actual data should be %d\n", 813);
    return EXIT_SUCCESS;
}

