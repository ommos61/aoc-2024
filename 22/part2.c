
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
int debug = 1; // 0 = no debug output, 1 = some extra debug output
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

#define PRICE(a) ((a) % 10)

void test_secret_prices(void) {
    long secret = 123;
    printf("%ld\n", secret);
    int old_price = PRICE(secret);
    for (unsigned i = 0; i < 10; i++) {
        secret = next_secret(secret);
        int new_price = PRICE(secret);
        printf("%10ld: %d (%d)\n", secret, new_price, new_price - old_price);
        old_price = new_price;
    }
}

#define SEQUENCE_LEN 4
struct sequence {
    int diffs[SEQUENCE_LEN];
};

#define SECRETS_COUNT 2000
long secrets[MAX_NUMBERS][SECRETS_COUNT + SEQUENCE_LEN];
void build_secrets(void) {
    for (unsigned n = 0; n < number_count; n++) {
        secrets[n][0] = numbers[n];
        secrets[n][SECRETS_COUNT] = 0;
        for (unsigned i = 0; i < SECRETS_COUNT + SEQUENCE_LEN; i++) {
             secrets[n][i + 1] = next_secret(secrets[n][i]);
        }
    }
}

int try_sequence(struct sequence sequence, long *secrets, unsigned secrets_count) {
    int bananas = 0;
    for (unsigned i = 0; i < secrets_count - SEQUENCE_LEN; i++) {
        int matched_sequence = 1;
        int old_price = PRICE(secrets[i]);
        for (unsigned j = 0; j < SEQUENCE_LEN; j++) {
            int new_price = PRICE(secrets[i + j + 1]);
            if ((new_price - old_price) != sequence.diffs[j]) {
                matched_sequence = 0;
                break;
            }
            old_price = new_price;
        }
        if (matched_sequence) {
            bananas = old_price;
            break;
        }
    }
    return bananas;
}

struct sequence index2sequence(long *secrets, unsigned start) {
    struct sequence result = {0};
    for (unsigned i = 0; i < SEQUENCE_LEN; i++) {
        int old_price = PRICE(secrets[start + i]);
        int new_price = PRICE(secrets[start + i + 1]);
        result.diffs[i] = new_price - old_price;
    }
    return result;
}

unsigned find_sequence_index(int bananas, long *secrets, unsigned secrets_count, unsigned offset) {
    unsigned result = 0;

    for (unsigned i = offset; i < secrets_count - SEQUENCE_LEN; i++) {
        if (PRICE(secrets[i + SEQUENCE_LEN]) == bananas) {
            struct sequence test = index2sequence(secrets, i);
            if (try_sequence(test, secrets, secrets_count) == bananas) {
                result = i;
                break;
            }
        }
    }

    return result;
}

int get_bananas_total(struct sequence seq) {
    int total = 0;
    for (unsigned i = 0; i < number_count; i++) {
        int bananas = try_sequence(seq, secrets[i], SECRETS_COUNT);
        total += bananas;
    }
    return total;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %u initial secret numbers\n", number_count);

//    if (debug) { test_secret_prices(); exit(0); }

    // implement algorithm
#if 0
    struct sequence seq = { -2,1,-1,3 };
    long total = 0;
    for (unsigned i = 0; i < number_count; i++) {
        build_secrets(numbers[i]);
        if (debug) printf("%ld: %ld\n", secrets[0], secrets[2000]);
        int bananas = try_sequence(seq, secrets, SECRETS_COUNT);
        if (debug) printf(" bananas: %d\n", bananas);
        total += bananas;
    }
    printf("The maximum number of bananas you can buy is %ld\n", total);
#endif

    build_secrets();

#define MAX_BANANAS_PER_BUYER 9
    int max_total = 0;
    struct sequence max_sequence = {0};
    printf("Solution is reached after about 100 lines checking\n");
    for (unsigned i = 0; i < number_count; i++) {
        printf("checking max for line %d\n", i);
        unsigned index = 0;
        while (index < SECRETS_COUNT - SEQUENCE_LEN) {
            unsigned new_index = find_sequence_index(MAX_BANANAS_PER_BUYER, secrets[i], SECRETS_COUNT, index);
            if (new_index == 0) break;
            struct sequence seq = index2sequence(secrets[i], new_index);
            //if (debug) printf("index = %-4d [ %d, %d, %d, %d ] = %ld\n",
            //                  new_index, seq.diffs[0], seq.diffs[1], seq.diffs[2], seq.diffs[3], PRICE(secrets[i][new_index + SEQUENCE_LEN]));
            int total_bananas = get_bananas_total(seq);
            //if (debug) printf(" total bananas = %d\n", total_bananas);
            if (total_bananas > max_total) {
                max_total = total_bananas;
                max_sequence = seq;
            }
            index = new_index + 1;
        }
        printf("max [ %d, %d, %d, %d ] = %d\n", max_sequence.diffs[0], max_sequence.diffs[1], max_sequence.diffs[2], max_sequence.diffs[3], max_total);
    }

    printf("Info: the solution for the sample data should be %ld\n", 23L);
    printf("Info: the solution for the actual data should be %ld\n", 1735L);
    return EXIT_SUCCESS;
}

