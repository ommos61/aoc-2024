
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>

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
char *gatetype_chars = "&|^";
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

int find_gate(char *name) {
    int gate = -1;
    for (unsigned g = 0; g < gate_count; g++) {
        if (strcmp(name, gates[g].out) == 0) {
            gate = g; break;
        }
    }
    return gate;
}

struct swap {
    char *out1, *out2;
} swapper[10] = {
    { "z05", "bpf" },
    { "z11", "hcc" },
    { "hqc", "qcw" },
    { "z35", "fdw" },
};
unsigned swap_count = 4;

int get_value(char *name) {
    int result = VALUE_UNKNOWN;
    for (unsigned i = 0; i < input_count; i++) {
        if (strcmp(name, inputs[i].name) == 0) {
            result = inputs[i].value;
            break;
        }
    }
    // it was not an input, so check gate outputs
    if (result == VALUE_UNKNOWN) {
        char *swap_name = name;
        for (unsigned i = 0; i < swap_count; i++) {
            if (strcmp(name, swapper[i].out1) == 0) {
                swap_name = swapper[i].out2;
            } else if (strcmp(name, swapper[i].out2) == 0) {
                swap_name = swapper[i].out1;
            }
        }
        for (unsigned g = 0; g < gate_count; g++) {
            if (strcmp(swap_name, gates[g].out) == 0) {
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
//#define NON_OPTIMIZED_OR
#ifdef NON_OPTIMIZED_OR
    if ((in1 != VALUE_UNKNOWN) && (in2 != VALUE_UNKNOWN)) { result = in1 | in2; }
#else
    if ((in1 == 1) || (in2 == 1)) { result = 1; }
    if ((in1 == 0) && (in2 == 0)) { result = 0; }
#endif
    return result;
}
        
int do_xor(int in1, int in2) {
    int result = VALUE_UNKNOWN;
    if ((in1 != VALUE_UNKNOWN) && (in2 != VALUE_UNKNOWN)) { result = in1 ^ in2; }
    return result;
}
        
#define MAX_PROPAGATE_COUNT 200;
void propagate_signals(void) {
    int still_unknown = 1;
    int propagate_count = MAX_PROPAGATE_COUNT;
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
            if (res == VALUE_UNKNOWN) unknown_count += 1;
        }
        //if (debug) printf("Still %u unknown\n", unknown_count);
        still_unknown = (unknown_count != 0);
        propagate_count -= 1;
        if (still_unknown && (propagate_count == 0)) {
            break;
        }
    }
}

long collect_value(char v) {
    long value = 0;
    int bitnum = 0;
    int val = VALUE_UNKNOWN;
    do {
        char buf[20];
        sprintf(buf, "%c%02d", v, bitnum);
        val = get_value(buf);
        if (val == 1) {
            value += (1L << bitnum);
        }
        bitnum += 1;
    } while (val != VALUE_UNKNOWN);
    return value;
}

void set_input_value(char c, long value) {
    for (unsigned i = 0; i < input_count; i++) {
        if (inputs[i].name[0] == c) {
            int bitnum = (inputs[i].name[1] - '0') * 10 + (inputs[i].name[2] - '0');
            inputs[i].value = (value >> bitnum) & 1;
        }
    }
}

struct combo { long a, b; } combos[] = { { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 }};

long wrong_bits[45];
unsigned wrong_count;

int check_addition(unsigned start_bit, unsigned end_bit, int verbose) {
    wrong_count = 0;
    unsigned problem_bit = 0;

    for (unsigned b = start_bit; b < end_bit + 1; b++) {
        for (unsigned c = 0; c < array_count(combos); c++) {
            clear_gates();
            set_input_value('x', combos[c].a * (1L << b));
            set_input_value('y', combos[c].b * (1L << b));
            propagate_signals();
            long in1 = collect_value('x');
            long in2 = collect_value('y');
            long out = collect_value('z');
            long diff_bits = out ^ (in1 + in2);
            if (diff_bits != 0) {
                if (b != problem_bit) {
                    problem_bit = b;
                    if (verbose) printf("Problem with input bit %u\n", b);
                    if (wrong_count != 0) {
                        if (wrong_bits[wrong_count - 1] != diff_bits) {
                            wrong_bits[wrong_count] = diff_bits;
                            wrong_count += 1;
                        }
                    } else {
                        wrong_bits[wrong_count] = diff_bits;
                        wrong_count += 1;
                    }
                }
                if (verbose) printf("    %ld + %ld is %ld (diff is %ld)\n", in1, in2, out, out - (in1 + in2));
                if (verbose) printf("    problem bits: %012lX\n", diff_bits);
            }
        }
    }

    return (wrong_count == 0);
}

unsigned get_first_bitnumber(long val) {
    unsigned bitnum = 0;
    assert(val != 0L);
    while ((val & 1L) == 0L) {
        val = val >> 1L;
        bitnum += 1;
    }
    return bitnum;
}

void walk_gate_tree(char *outname, unsigned level, void (*action)(char *, unsigned, void *), void *data) {
    long max_depth = (long)data;
    action(outname, level, data);
    if ((max_depth != 0L) && ((long)level >= max_depth)) return;

    for (unsigned g = 0; g < gate_count; g++) {
        if (strcmp(outname, gates[g].out) == 0) {
            walk_gate_tree(gates[g].in1, level + 1, action, data);
            walk_gate_tree(gates[g].in2, level + 1, action, data);
        }
    }
}

void print_gate(char *outname, unsigned level, void *data) {
    (void)data;
    for (unsigned l = 0; l < level; l++) printf("  ");
    int gate = find_gate(outname);
    if (gate != -1) {
        printf("%s %c %s %s\n", outname, gatetype_chars[gates[gate].type], gates[gate].in1, gates[gate].in2);
    } else {
        printf("%s\n", outname);
    }
}

char *noswap_list[1024];
unsigned noswap_count = 0;
char *swap1_list[1024];
unsigned swap1_count = 0;
char *swap2_list[1024];
unsigned swap2_count = 0;

int is_present_in_list(char *outname, char **list, unsigned count) {
    int present = 0;
    for (unsigned i = 0; i < count; i++) {
        if (strcmp(outname, list[i]) == 0) {
            present = 1;
            break;
        }
    }

    return present;
}

void add2list(char *outname, unsigned level, void *data) {
    (void)level;
    if (find_gate(outname) == -1) return;
    unsigned *count = data;
    char **list = NULL;
    if (count == &noswap_count) list = noswap_list;
    if (count == &swap1_count) list = swap1_list;
    if (count == &swap2_count) list = swap2_list;
    if (!is_present_in_list(outname, list, *count)) {
        list[*count] = outname;
        *count += 1;
    }
}

void list_remove_elements(char **list, unsigned *count, char **remove_list, unsigned remove_count) {
    int new_count = 0;
    for (unsigned i = 0; i < *count; i++) {
        if (!is_present_in_list(list[i], remove_list, remove_count)) {
            list[new_count] = list[i];
            new_count += 1;
        }
    }
    *count = new_count;

}

int name_compare(const void *v1, const void *v2) {
    char *n1 = *(char **)v1;
    char *n2 = *(char **)v2;
    return strcmp(n1, n2);
}

void walk_from_input(char *input, unsigned level) {
    for (unsigned i = 0; i < level; i++) printf("..");
    printf("%s: \n", input);
    for (unsigned g = 0; g < gate_count; g++) {
        char *name1 = gates[g].in1;
        char *name2 = gates[g].in2;
        if (strcmp(name1, input) == 0) walk_from_input(gates[g].out, level + 1);
        if (strcmp(name2, input) == 0) walk_from_input(gates[g].out, level + 1);
    }
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
    swap_count = 4;
    int is_ok = check_addition(0, input_count / 2, 1);
    if (!is_ok) {
        for (unsigned i = 0; i < wrong_count; i++) {
            printf("problem bits: %012lX\n", wrong_bits[i]);
            unsigned bitnum = get_first_bitnumber(wrong_bits[i]);
            printf(" bit number is %u\n", bitnum);
            break;
        }
    }

    if (!is_ok) {
#define START_BIT 32
#define END_BIT 34
        unsigned bitnum = 0;
        char buf[20];
        unsigned gate;
        while (bitnum < END_BIT) {
            sprintf(buf, "z%02u", bitnum);
            gate = find_gate(buf);
            walk_gate_tree(gates[gate].out, 0, add2list, &noswap_count);
            bitnum += 1;
        }
        if (debug) {
            printf("noswap: ");
            for (unsigned i = 0; i < noswap_count; i++) printf("%s ", noswap_list[i]); printf("\n");
        }

        if (debug) {
            walk_gate_tree("z00", 0, print_gate, (void *)0);
            walk_gate_tree("z01", 0, print_gate, (void *)0);
            walk_gate_tree("z02", 0, print_gate, (void *)0);
            walk_gate_tree("z35", 0, print_gate, (void *)0);
            walk_gate_tree("z36", 0, print_gate, (void *)5);
        }
//        walk_gate_tree("z35", 0, print_gate, NULL);
//        walk_from_input("x35", 0);
//        walk_from_input("y35", 0);
//        exit(1);
#define NOT_GATE34
#ifdef NOT_GATE34
        sprintf(buf, "z%02u", 35); if (debug) printf("--> %s\n", buf);
        gate = find_gate(buf);
        walk_gate_tree(gates[gate].out, 0, add2list, &swap1_count);
        list_remove_elements(swap1_list, &swap1_count, noswap_list, noswap_count);

        sprintf(buf, "z%02u", 36); if (debug) printf("--> %s\n", buf);
        gate = find_gate(buf);
        walk_gate_tree(gates[gate].out, 0, add2list, &swap2_count);
        sprintf(buf, "z%02u", 37); if (debug) printf("--> %s\n", buf);
        gate = find_gate(buf);
        walk_gate_tree(gates[gate].out, 0, add2list, &swap2_count);
        list_remove_elements(swap2_list, &swap2_count, noswap_list, noswap_count);
        list_remove_elements(swap2_list, &swap2_count, swap1_list, swap1_count);
#else
        swap1_list[0] = "z35";
        swap1_count = 1;
        swap2_count = 0;
        for (unsigned g = 0; g < gate_count; g++) {
            add2list(gates[g].out, 0, &swap2_count);
        }
        //list_remove_elements(swap2_list, &swap2_count, noswap_list, noswap_count);
#endif

        if (debug) {
            printf("swap1: ");
            for (unsigned i = 0; i < swap1_count; i++) printf("%s ", swap1_list[i]); printf("\n");
            printf("swap2: ");
            for (unsigned i = 0; i < swap2_count; i++) printf("%s ", swap2_list[i]); printf("\n");
        }

        // check combinations
#if 0
        char *swap1 = NULL, *swap2 = NULL;
        for (unsigned i = 0; i < swap1_count; i++) {
            int better = 0;
            for (unsigned j = 0; j < swap2_count; j++) {
                swapper[0].out1 = swap1_list[i];
                swapper[0].out2 = swap2_list[j];
                swap_count = 1;
                printf("swapping '%s' and '%s' and checking addition\n", swapper[0].out1, swapper[0].out2);
                int is_ok = check_addition(START_BIT, END_BIT + 8, 0);
                if (!is_ok) {
                    printf("swapping '%s' and '%s' resulted in wrong_count of %u\n", swapper[0].out1, swapper[0].out2, wrong_count);
                } else {
                    swap1 = swapper[0].out1; swap2 = swapper[0].out2;
                }
            }
            if (better) break;
        }
        printf("This worked: swap('%s', '%s')\n", swap1, swap2);
#endif
    }
    char *all[8];
    all[0] = swapper[0].out1;
    all[1] = swapper[0].out2;
    all[2] = swapper[1].out1;
    all[3] = swapper[1].out2;
    all[4] = swapper[2].out1;
    all[5] = swapper[2].out2;
    all[6] = swapper[3].out1;
    all[7] = swapper[3].out2;
    qsort(all, array_count(all), sizeof(char *), name_compare);
    printf("Solution: ");
    for (unsigned i = 0; i < array_count(all); i++) {
        printf("%s%s", all[i], (i < array_count(all) - 1) ? "," : "");
    }
    printf("\n");

    printf("Info: the solution for the sample data should be '%s'\n", "");
    printf("Info: the solution for the actual data should be '%s'\n", "bpf,fdw,hcc,hqc,qcw,z05,z11,z35");
    return EXIT_SUCCESS;
}
