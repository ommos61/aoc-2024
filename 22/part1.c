
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
#define MAX_NUMBERS 2000
long numbers[MAX_NUMBERS];
unsigned number_count = 0;

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

        long value = 0;
        if (sscanf(line, "%ld", &value) == 1) {
            // store the data
            numbers[number_count] = value;
            number_count += 1;
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

#define MIX(a, b) ((a) ^ (b))
#define PRUNE(a)  ((a) % 16777216)
long next_secret(long start_secret) {
    long secret = start_secret;
    secret = PRUNE(MIX(secret, secret * 64));
    secret = PRUNE(MIX(secret, secret / 32));
    secret = PRUNE(MIX(secret, secret * 2048));
    return secret;
}

void test_secret_numbers(void) {
    long secret = 123;
    printf("%ld\n", secret);
    for (unsigned i = 0; i < 10; i++) {
        secret = next_secret(secret);
        printf("%ld\n", secret);
    }
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %u initial secret numbers\n", number_count);

    //if (debug) test_secret_numbers();

    // implement algorithm
    long total = 0;
    for (unsigned i = 0; i < number_count; i++) {
        long secret = numbers[i];
        for (unsigned n = 0; n < 2000; n++) {
            secret = next_secret(secret);
        }
        if (debug) printf("%ld: %ld\n", numbers[i], secret);
        total += secret;
    }
    printf("The sum of all the 2000th secret numbers is %ld\n", total);

    printf("Info: the solution for the sample data should be %ld\n", 37327623L);
    printf("Info: the solution for the actual data should be %ld\n", 14622549304L);
    return EXIT_SUCCESS;
}

