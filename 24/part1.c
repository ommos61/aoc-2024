
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
#define MAX_INPUTS 100
#define MAX_GATES  500
#define VALUE_UNKNOWN 2
struct input {
    char *name;
    int value;
} inputs[MAX_INPUTS];
unsigned input_count = 0;
#define GATE_AND 0
#define GATE_OR  1
#define GATE_XOR 2
struct gate {
    char *in1, *in2, *out;
    int type;
    int value;
} gates[MAX_GATES];
unsigned gate_count = 0;

char *new_string(char *str) { char *tmp = malloc(strlen(str) + 1); assert(tmp != NULL); strcpy(tmp, str); return tmp; }

// Function to read all input data to memory
void readData(char *fname) {
    FILE *fin = fopen(fname, "r");
    if (fin == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int line_count = 0;
    char line[LINE_LENGTH];
    int doing_inputs = 1;
    while (fgets(line, LINE_LENGTH, fin) != NULL) {
        // strip line ending
        if (strlen(line) != 0) line[strlen(line) - 1] = 0;

        char buf1[5], buf2[5], buf3[5], buf4[5];
        if (strlen(line) == 0) {
            doing_inputs = 0;
        } else if (strlen(line) > 0) {
            // store the data
            if (doing_inputs) {
                char bin_value = 0;
                if (sscanf(line, "%3s: %c", buf1, &bin_value) == 2) {
                    char *tmp = new_string(buf1);
                    inputs[input_count].name = tmp;
                    inputs[input_count].value = (bin_value == '0') ? 0 : 1;
                    input_count += 1;
                } else {
                    printf("Unknown input format '%s'\n", line);
                }
            } else if (!doing_inputs) {
                if (sscanf(line, "%s %s %s -> %s", buf1, buf2, buf3, buf4) == 4) {
                    gates[gate_count].in1 = new_string(buf1);
                    gates[gate_count].in2 = new_string(buf3);
                    gates[gate_count].out = new_string(buf4);
                    if (strcmp(buf2, "AND") == 0) {
                        gates[gate_count].type = GATE_AND;
                    } else if (strcmp(buf2, "OR") == 0) {
                        gates[gate_count].type = GATE_OR;
                    } else if (strcmp(buf2, "XOR") == 0) {
                        gates[gate_count].type = GATE_XOR;
                    } else {
                        printf("Unknown gate '%s'\n", buf2);
                    }
                    gate_count += 1;
                } else {
                    printf("Unknown gate format '%s'\n", line);
                }
            }
        } else {
            fprintf(stderr, "Unexpected input format '%s'.\n", line);
        }

        line_count++;
    }

    printf("lines = %d\n", line_count);
    fclose(fin);
}

void print_inputs(void) {
    for (unsigned i = 0; i < input_count; i++) {
        printf("%s: %d\n", inputs[i].name, inputs[i].value);
    }
}

void print_gates(void) {
    for (unsigned i = 0; i < gate_count; i++) {
        printf("%s %d %s: %s\n", gates[i].in1, gates[i].type, gates[i].in2, gates[i].out);
    }
}

void clear_gates(void) {
    for (unsigned i = 0; i < gate_count; i++) {
        gates[i].value = VALUE_UNKNOWN;
    }
}

int get_value(char *name) {
    int result = VALUE_UNKNOWN;
    for (unsigned i = 0; i < input_count; i++) {
        if (strcmp(name, inputs[i].name) == 0) {
            result = inputs[i].value;
            break;
        }
    }
    if (result == VALUE_UNKNOWN) {
        for (unsigned g = 0; g < gate_count; g++) {
            if (strcmp(name, gates[g].out) == 0) {
                result = gates[g].value;
            }
        }
    }

    return result;
}

int do_and(int in1, int in2) {
    int result = VALUE_UNKNOWN;
    if ((in1 != VALUE_UNKNOWN) && (in2 != VALUE_UNKNOWN)) { result = in1 & in2; }
    return result;
}
        
int do_or(int in1, int in2) {
    int result = VALUE_UNKNOWN;
    if ((in1 == 1) || (in2 == 1)) { result = 1; }
    if ((in1 == 0) && (in2 == 0)) { result = 0; }
    return result;
}
        
int do_xor(int in1, int in2) {
    int result = VALUE_UNKNOWN;
    if ((in1 != VALUE_UNKNOWN) && (in2 != VALUE_UNKNOWN)) { result = in1 ^ in2; }
    return result;
}
        
void propagate_signals(void) {
    int still_unknown = 1;
    while (still_unknown) {
        unsigned unknown_count = 0;
        for (unsigned g = 0; g < gate_count; g++) {
            int in1 = get_value(gates[g].in1);
            int in2 = get_value(gates[g].in2);
            int res = VALUE_UNKNOWN;
            switch (gates[g].type) {
            case GATE_AND:
                res = do_and(in1, in2); break;
            case GATE_OR:
                res = do_or(in1, in2); break;
            case GATE_XOR:
                res = do_xor(in1, in2); break;
            default:
                printf("Unknown gate type '%d'\n", gates[g].type); break;
            }
            gates[g].value = res;
            if ((gates[g].out[0] == 'z') && (res == VALUE_UNKNOWN)) unknown_count += 1;
        }
        if (debug) printf("Still %u unknown\n", unknown_count);
        still_unknown = (unknown_count != 0);
    }
}

long collect_output(void) {
    long value = 0;
    int max_bitnum = 0;
    for (unsigned i = 0; i < gate_count; i++) {
        assert(gates[i].value != VALUE_UNKNOWN);
        if ((gates[i].out[0] == 'z') && (gates[i].value == 1)) {
            int bitnum = (gates[i].out[1] - '0') * 10 + (gates[i].out[2] - '0');
            value += (1L << bitnum);
            if (bitnum > max_bitnum) max_bitnum = bitnum;
        }
    }
    if (debug) printf("Maximum bitnum is %d\n", max_bitnum);
    return value;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %u inputs and %u gates\n", input_count, gate_count);
    //if (debug) print_inputs();
    //if (debug) print_gates();

    // implement algorithm
    clear_gates();
    propagate_signals();
    long value = collect_output();
    printf("The output value is %ld\n", value);

    printf("Info: the solution for the sample data should be %ld\n", 2024L);
    printf("Info: the solution for the actual data should be %ld\n", 58367545758258L);
    return EXIT_SUCCESS;
}

