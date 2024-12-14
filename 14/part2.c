
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output
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

static int *space = NULL;
#define SPACE_GET(x,y) (*(space + (y) * width + (x)))
#define SPACE_SET(x,y,c) (*(space + (y) * width + (x)) == c)
void render_space(void) {
    if (space == NULL) space = malloc(width * height * sizeof(int));
    assert(space != NULL);
    memset(space, 0, width * height * sizeof(int));
    for (unsigned r = 0; r < robot_count; r++) {
        SPACE_GET(robots[r].x, robots[r].y) += 1;
    }
}

void print_space(FILE *f) {
    render_space();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int count = *(space + y * width + x);
            if (count == 0) {
                fprintf(f, ".");
            } else {
                fprintf(f, "%d", count % 10);
            }
        }
        fprintf(f, "\n");
    }
    free(space); space = NULL;
}

int positive_modulo(int a, int n) {
    return (a % n + n) % n;
}

void move_robots(int moves) {
    for (unsigned r = 0; r < robot_count; r++) {
        int newx = robots[r].x + moves *robots[r].vx;
        int newy = robots[r].y + moves * robots[r].vy;
        robots[r].x = positive_modulo(newx, width);
        robots[r].y = positive_modulo(newy, height);
    }
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
    long t = 0;
    long time_inc = 1;
    while (1) {
        move_robots(time_inc);
        t += time_inc;
        if (t == 12) time_inc = 103;
        if (((t - 12) % 103) == 0) {
            if (debug) printf("\033[2J"); // clear screen
            if (debug) print_space(stdout);
            if (debug) printf("time = %ld ('q' to stop, <Enter> for next): ", t);
            int c = getchar();
            if (c == 'q') break;
        }
    }
    printf("\nThe time is %ld\n", t);
    FILE *result = fopen("result.txt", "w");
    print_space(result);
    fclose(result);

    printf("Info: the solution for the sample data should be %d\n", 0);
    printf("Info: the solution for the actual data should be %ld\n", 6398L);
    return EXIT_SUCCESS;
}

