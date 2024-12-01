
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

int list2_count(int num) {
    int count = 0;
    for (unsigned i = 0; i < item_count; i++) {
        if (list2[i] == num) {
            count += 1;
        } else if (list2[i] > num) {
            break;
        }
    }
    return count;
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
    int simularity_score = 0;
    int last_number = 0;
    int count = 0;
    for (unsigned i = 0; i < item_count; i++) {
        if (list1[i] != last_number) {
            count = list2_count(list1[i]);
        }
        if (debug) printf("%d\n", list1[i] * count);
        simularity_score += list1[i] * count;
    }
    printf("The calculated simularity score is %d\n", simularity_score);

    printf("Info: the solution for the sample data should be %d\n", 31);
    printf("Info: the solution for the actual data should be %d\n", 22588371);
    return EXIT_SUCCESS;
}

