
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
#define MAX_ITEMS 2000
int list1[MAX_ITEMS], list2[MAX_ITEMS];
unsigned item_count = 0;

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

        int value1 = 0, value2 = 0;
        if (sscanf(line, "%d %d", &value1, &value2) == 2) {
            // parse the data
            list1[item_count] = value1;
            list2[item_count] = value2;
            item_count += 1;
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

int compare_int(const void *p1, const void *p2) {
    return *(int *)p1 - *(int *)p2;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    if (debug) printf("There are %d items in both lists.\n", item_count);

    // TODO: implement algorithm
    qsort(list1, item_count, sizeof(list1[0]), compare_int);
    qsort(list2, item_count, sizeof(list1[0]), compare_int);
    int diff_sum = 0;
    for (unsigned i = 0; i < item_count; i++) {
        if (debug) printf("%d %d\n", list1[i], list2[i]);
        diff_sum += abs(list1[i] - list2[i]);
    }
    printf("The calculated sum of the differences is %d\n", diff_sum);

    printf("Info: the solution for the sample data should be %d\n", 11);
    printf("Info: the solution for the actual data should be %d\n", 1590491);
    return EXIT_SUCCESS;
}

