
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
#define MAX_ROBOTS 500
struct robot {
    int x, y;
    int vx, vy;
} robots[MAX_ROBOTS];
unsigned robot_count = 0;
int maxx = 0, maxy = 0;
int width = 11, height = 7;

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
        int vx = 0, vy = 0;
        if (sscanf(line, "p=%d,%d v=%d,%d", &x, &y, &vx, &vy) == 4) {
            // store the data
            assert(robot_count < MAX_ROBOTS);
            robots[robot_count].x = x;
            robots[robot_count].y = y;
            robots[robot_count].vx = vx;
            robots[robot_count].vy = vy;
            maxx = MAX(maxx, x);
            maxy = MAX(maxy, y);
            robot_count += 1;
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

void print_space(void) {
    int *space = malloc(width * height * sizeof(int));
    assert(space != NULL);
    memset(space, 0, width * height * sizeof(int));
    for (unsigned r = 0; r < robot_count; r++) {
        *(space + robots[r].y * width + robots[r].x) += 1;
    }
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int count = *(space + y * width + x);
            if (count == 0) {
                printf(".");
            } else {
                printf("%d", count % 10);
            }
        }
        printf("\n");
    }
}

void move_robots(void) {
    for (unsigned r = 0; r < robot_count; r++) {
        int newx = robots[r].x + robots[r].vx;
        int newy = robots[r].y + robots[r].vy;
        if (newx < 0) newx += width;
        if (newx >= (int)width) newx -= width;
        if (newy < 0) newy += height;
        if (newy >= (int)height) newy -= height;
        robots[r].x = newx;
        robots[r].y = newy;
    }
}

unsigned countup_quadrants(void) {
    unsigned counts[4] = {0};
    for (unsigned r = 0; r < robot_count; r++) {
        unsigned index = 0;
        if (robots[r].x > (width / 2)) index += 1;
        if (robots[r].y > (height / 2)) index += 2;
        if ((robots[r].x != (width / 2)) && (robots[r].y != (height / 2))) {
            counts[index] += 1;
        }
    }
    if (debug) printf("quadrants: %d, %d, %d, %d\n", counts[0], counts[1], counts[2], counts[3]);
    return counts[0] * counts[1] * counts[2] * counts[3];
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    if (maxx > width) { width = 101; height = 103; }
    printf("The guard area is %dx%d\n", width, height);

    // implement algorithm
    unsigned max_time = 100;
    for (unsigned t = 0; t < max_time; t++) {
        move_robots();
    }
    if (debug) print_space();
    unsigned score = countup_quadrants();
    printf("The safety factor is %d\n", score);

    printf("Info: the solution for the sample data should be %d\n", 12);
    printf("Info: the solution for the actual data should be %d\n", 219512160);
    return EXIT_SUCCESS;
}

