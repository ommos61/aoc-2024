
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
#define MAX_MACHINES 500
struct pos {
    int x, y;
};
struct machine {
    struct pos A, B;
    struct pos target;
} machines[MAX_MACHINES];
unsigned machine_count = 0;

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
        // store the data
        if (sscanf(line, "Button A: X%d, Y%d", &x, &y) == 2) {
            machines[machine_count].A.x = x;
            machines[machine_count].A.y = y;
        } else if (sscanf(line, "Button B: X%d, Y%d", &x, &y) == 2) {
            machines[machine_count].B.x = x;
            machines[machine_count].B.y = y;
        } else if (sscanf(line, "Prize: X=%d, Y=%d", &x, &y) == 2) {
            machines[machine_count].target.x = x;
            machines[machine_count].target.y = y;
            machine_count += 1;
        } else if (strlen(line) == 0) {
            // skip empty lines
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

void print_machine(struct machine m) {
    printf("A = (%d, %d), B = (%d, %d), P = (%d, %d)\n",
           m.A.x, m.A.y, m.B.x, m.B.y, m.target.x, m.target.y);
}

#define MAX_PUSHES 100
long determine_cost(struct machine m) {
    long acount = 0, bcount = 0;
    for (int a = 0; a <= MAX_PUSHES; a++) {
        for (int b = 0; b <= MAX_PUSHES; b++) {
            if ((m.target.x == (a * m.A.x + b * m.B.x)) && (m.target.y == (a * m.A.y + b * m.B.y))) {
                //printf(" found prize\n");
                acount = a; bcount = b;
                break;
            }
        }
        if ((acount != 0) || (bcount != 0)) break;
    }
    if (debug) print_machine(m);
    if (debug) printf(" %ld times A, %ld times B\n", acount, bcount);
    return ((3 * acount) + (1 * bcount));
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %d machines\n", machine_count);

    // TODO: implement algorithm
    long totalcost = 0;
    for (unsigned i = 0; i < machine_count; i++) {
        long cost = determine_cost(machines[i]);
        if (debug) printf("The cost for machine %d is %ld\n", i + 1, cost);
        totalcost += cost;
    }
    printf("The total cost is %ld\n", totalcost);

    printf("Info: the solution for the sample data should be %ld\n", 480L);
    printf("Info: the solution for the actual data should be %ld\n", 38839L);
    return EXIT_SUCCESS;
}

