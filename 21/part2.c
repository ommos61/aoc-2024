
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH 1024
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output
#define MAX_NUMBERS 10
char *numbers[MAX_NUMBERS];
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

        if (strlen(line) > 0) {
            // store the data
            assert(number_count < MAX_NUMBERS);
            char *tmp = malloc(strlen(line) + 1);
            assert(tmp != NULL);
            strcpy(tmp, line);
            numbers[number_count] = tmp;
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

//
//  7 8 9
//  4 5 6
//  1 2 3
//  X 0 A
//
struct num_transition {
    char from;
    struct num_target {
        char to;
        struct direction {
            char *a, *b;
        } directions;
    } targets[11];
} num_transitions[] = {
    {'A', {{'A', {"", NULL}}, {'0', {"<", NULL}}, {'1', {"^<<", NULL}}, {'2', {"^<", "<^"}}, {'3', {"^", NULL}}, {'4', {"^^<<", NULL}},
                              {'5', {"^^<", "<^^"}}, {'6', {"^^", NULL}  }, {'7', {"^^^<<", NULL} }, {'8', {"^^^<", "<^^^"}}, {'9', {"^^^", NULL}}}}, 
    {'0', {{'A', {">", NULL}}, {'0', {"", NULL}}, {'1', {"^<", NULL}}, {'2', {"^", NULL}}, {'3', {"^>", ">^"}}, {'4', {"^^<", NULL}},
                              {'5', {"^^", NULL}}, {'6', {"^^>", ">^^"}  }, {'7', {"^^^<", NULL} }, {'8', {"^^^", NULL}}, {'9', {"^^^>", ">^^^"}}}}, 
    {'1', {{'A', {">>v", NULL}}, {'0', {">v", NULL}}, {'1', {"", NULL}}, {'2', {">", NULL}}, {'3', {">>", NULL}}, {'4', {"^", NULL}},
                              {'5', {"^>", ">^"}}, {'6', {"^>>", ">>^"}  }, {'7', {"^^", NULL} }, {'8', {"^^>", ">^^"}}, {'9', {"^^>>", ">>^^"}}}}, 
    {'2', {{'A', {"v>", ">v"}}, {'0', {"v", NULL}}, {'1', {"<", NULL}}, {'2', {"", NULL}}, {'3', {">", NULL}}, {'4', {"<^", "^<"}},
                              {'5', {"^", NULL}}, {'6', {"^>", ">^"}  }, {'7', {"<^^", "^^<"} }, {'8', {"^^", NULL}}, {'9', {"^^>", ">^^"}}}}, 
    {'3', {{'A', {"v", NULL}}, {'0', {"<v", "v<"}}, {'1', {"<<", NULL}}, {'2', {"<", NULL}}, {'3', {"", NULL}}, {'4', {"<<^", "^<<"}},
                              {'5', {"<^", "^<"}}, {'6', {"^", NULL}  }, {'7', {"<<^^", "^^<<"} }, {'8', {"<^^", "^^<"}}, {'9', {"^^", NULL}}}}, 
    {'4', {{'A', {">>vv", NULL}}, {'0', {">vv", NULL}}, {'1', {"v", NULL}}, {'2', {"v>", ">v"}}, {'3', {"v>>", ">>v"}}, {'4', {"", NULL}},
                              {'5', {">", NULL}}, {'6', {">>", NULL}  }, {'7', {"^", NULL} }, {'8', {"^>", ">^"}}, {'9', {"^>>", ">>^"}}}}, 
    {'5', {{'A', {">vv", "vv>"}}, {'0', {"vv", NULL}}, {'1', {"<v", "v<"}}, {'2', {"v", NULL}}, {'3', {"v>", ">v"}}, {'4', {"<", NULL}},
                              {'5', {"", NULL}}, {'6', {">", NULL}  }, {'7', {"<^", "^<"} }, {'8', {"^", NULL}}, {'9', {"^>", ">^"}}}}, 
    {'6', {{'A', {"vv", NULL}}, {'0', {"vv<", "<vv"}}, {'1', {"<<v", "v<<"}}, {'2', {"<v", "v<"}}, {'3', {"v", NULL}}, {'4', {"<<", NULL}},
                              {'5', {"<", NULL}}, {'6', {"", NULL}  }, {'7', {"<<^", "^<<"} }, {'8', {"<^", "^<"}}, {'9', {"^", NULL}}}}, 
    {'7', {{'A', {">>vvv", NULL}}, {'0', {">vvv", NULL}}, {'1', {"vv", NULL}}, {'2', {"vv>", ">vv"}}, {'3', {"vv>>", ">>vv"}}, {'4', {"v", NULL}},
                              {'5', {"v>", ">v"}}, {'6', {"v>>", ">>v"}  }, {'7', {"", NULL} }, {'8', {">", NULL}}, {'9', {">>", NULL}}}}, 
    {'8', {{'A', {"vvv>", ">vvv"}}, {'0', {"vvv", NULL}}, {'1', {"<vv", "vv<"}}, {'2', {"vv", NULL}}, {'3', {"v>", NULL}}, {'4', {"<v", "v<"}},
                              {'5', {"v", NULL}}, {'6', {"v>", ">v"}  }, {'7', {"<", NULL} }, {'8', {"", NULL}}, {'9', {">", NULL}}}}, 
    {'9', {{'A', {"vvv", NULL}}, {'0', {"vvv<", "<vvv"}}, {'1', {"<<vv", "vv<<"}}, {'2', {"<vv", "vv<"}}, {'3', {"vv", NULL}}, {'4', {"<<v", "v<<"}},
                              {'5', {"<v", "v<"}}, {'6', {"v", NULL}  }, {'7', {"<<", NULL} }, {'8', {"<", NULL}}, {'9', {"", NULL}}}}, 
};
struct direction find_num_target(char from, char to) {
    struct direction dirs = {0};
    for (unsigned i = 0; i < array_count(num_transitions); i++ ) {
        if (num_transitions[i].from == from) {
            for (unsigned j = 0; j < array_count(num_transitions[i].targets); j++) {
                if (num_transitions[i].targets[j].to == to) {
                    dirs = num_transitions[i].targets[j].directions;
                    break;
                }
            }
            break;
        }
    }
    return dirs;
}

struct possibilities {
    char **list;
    unsigned count;
    unsigned capacity;
};
#define PP_INITIAL_SIZE 10
void add_possibility(struct possibilities *poslist, char *str) {
    if (poslist->count == 0) {
        assert(poslist->list == NULL);
        poslist->list = malloc(PP_INITIAL_SIZE * sizeof(char *));
        poslist->capacity = PP_INITIAL_SIZE;

        poslist->list[0] = malloc(strlen(str) + 1);
        strcpy(poslist->list[0], str);
        poslist->count = 1;
    } else {
        if (poslist->count >= poslist->capacity) {
            assert(0 && "resizing of possibilities list not suppported (yet)");
        }
        poslist->list[poslist->count] = malloc(strlen(str) + 1);
        assert(poslist->list[poslist->count] != NULL);
        strcpy(poslist->list[poslist->count], str);
        poslist->count += 1;
    }
}

void append_possibility(struct possibilities *poslist, unsigned index, char *str) {
    assert(index < poslist->count);
    assert(poslist->list[index] != NULL);
    char *tmp = malloc(strlen(poslist->list[index]) + strlen(str) + 1);
    assert(tmp != NULL);
    strcpy(tmp, poslist->list[index]); strcat(tmp, str);
    free(poslist->list[index]);
    poslist->list[index] = tmp;
}

struct possibilities new_possibilities(void) {
    struct possibilities poslist = {NULL, 0, 0};
    add_possibility(&poslist, "");
    return poslist;
}

struct possibilities numpad2dirs(char *code) {
    struct possibilities poslist = new_possibilities();
    char *p = code;
    char current = 'A';
    while (*p != 0) {
        struct direction dirs = find_num_target(current, *p);
        unsigned oldcount = poslist.count;
        if (dirs.b != NULL) {
            char buf[1024];
            for (unsigned i = 0; i < oldcount; i++) {
                strcpy(buf, poslist.list[i]);
                strcat(buf, dirs.b);
                strcat(buf, "A");
                add_possibility(&poslist, buf);
            }
        }
        for (unsigned i = 0; i < oldcount; i++) {
            char buf[20];
            strcpy(buf, dirs.a);
            strcat(buf, "A");
            append_possibility(&poslist, i, buf);
        }

        current = *p;
        p += 1;
    }
    return poslist;
}

char *numchars[] = { "789", "456", "123", " 0A" };
#define MAX_BUFFER 1024
char sim_num_buffer[MAX_BUFFER];
char *simulate_num_pushes(char *pushes) {
    //if (debug) printf("simulating '%s'\n", pushes);
    sim_num_buffer[0] = 0;
    char *p = pushes;
    int curx = 2, cury = 3;
    while (*p != 0) {
        //if (debug) printf("'%s' + '%c': (curx = %d, cury = %d, c = '%s')\n", sim_num_buffer, *p, curx, cury, numchars[cury]);
        assert((curx >= 0) && (curx <= 2));
        assert((cury >= 0) && (cury <= 3));
        assert(numchars[cury][curx] != ' ');
        switch (*p) {
        case 'A': {
            char buf[2] = { 0 };
            buf[0] = numchars[cury][curx];
            strcat(sim_num_buffer, buf);
        } break;
        case '^':
            cury -= 1;
            break;
        case '<':
            curx -= 1;
            break;
        case 'v':
            cury += 1;
            break;
        case '>':
            curx += 1;
            break;
        default:
            printf("unknown char '%c' in push list\n", *p);
            break;
        }
        p += 1;
    }

    return sim_num_buffer;
}

char *dirchar_seq = "A^<v>";
struct dir_transition {
    char from;
    struct dir_target {
        char to;
        struct direction directions;
    } targets[5];
} dir_transitions[] = {
    {'A', {{'A', {"", NULL}}, {'^', {"<", NULL}}, {'<', {"v<<", NULL}}, {'v', {"v<", "<v"}}, {'>', {"v", NULL}}}}, 
    {'^', {{'A', {">", NULL}},  {'^', {"", NULL}}, {'<', {"v<", NULL}}, {'v', {"v", NULL}}, {'>', {"v>", ">v"}}}}, 
    {'<', {{'A', {">>^", NULL}}, {'^', {">^", NULL}}, {'<', {"", NULL}}, {'v', {">", NULL}}, {'>', {">>", NULL}}}}, 
    {'v', {{'A', {"^>", ">^"}}, {'^', {"^", NULL}}, {'<', {"<", NULL}}, {'v', {"", NULL}}, {'>', {">", NULL}}}}, 
    {'>', {{'A', {"^", NULL}}, {'^', {"<^", "^<"}}, {'<', {"<<", NULL}}, {'v', {"<", NULL}}, {'>', {"", NULL}}}}, 
};

struct dir_target find_dir_targets(char from, char to) {
    struct dir_target res = { 0 };
    for (unsigned i = 0; i < array_count(dir_transitions); i++ ) {
        if (dir_transitions[i].from == from) {
            for (unsigned j = 0; j < array_count(dir_transitions[i].targets); j++) {
                if (dir_transitions[i].targets[j].to == to) {
                    res = dir_transitions[i].targets[j];
                    break;
                }
            }
            break;
        }
    }
    return res;
}

#define LL_INITIAL_SIZE 5
struct lengths {
    long *list;
    unsigned count;
    unsigned capacity;
};
struct lengths new_lengths(void) {
    struct lengths lenlist = {NULL, 0, 0};
    lenlist.list = malloc(LL_INITIAL_SIZE * sizeof(long));
    lenlist.capacity = LL_INITIAL_SIZE;
    return lenlist;
}
void append_length(struct lengths *lenlist, long len) {
    int present = 0;
    for (unsigned i = 0; i < lenlist->count; i++) {
        if (len == lenlist->list[i]) {
            present = 1; break;
        }
    }
    if (!present) {
        if (lenlist->count >= lenlist->capacity) {
            // TODO: extend capacity
            assert(0 && "lenlist is too short");
        }
        lenlist->list[lenlist->count] = len;
        lenlist->count += 1;
    }
}

#define MAX_DEPTH 30
struct cache_entry {
    char seq[32];
    long length;
    struct cache_entry *next;
} *cache[MAX_DEPTH] = {0};
struct cache_entry *get_cache_entry(char *seq, unsigned depth) {
    struct cache_entry *res = cache[depth];
    while (res != NULL) {
        if (strcmp(res->seq, seq) == 0) {
            break;
        }
        res = res->next;
    }
    return res;
}
void add_cache_entry(char *seq, unsigned depth, long length) {
    struct cache_entry *entry = get_cache_entry(seq, depth);
    if (entry != NULL) {
        if (length < entry->length) {
            entry->length = length;
        }
    } else {
        entry = malloc(sizeof(struct cache_entry));
        assert(entry != NULL);
        strcpy(entry->seq, seq);
        entry->length = length;
        entry->next = cache[depth];
        cache[depth] = entry;
    }
}

long compute_dirlength(char *seq, unsigned depth) {
    struct cache_entry *e = get_cache_entry(seq, depth);
    if (e != NULL) return e->length;
    long length = 0;
    if (depth == 1) {
        char *s = seq;
        char current = 'A';
        while (*s != 0) {
            struct dir_target t = find_dir_targets(current, *s);
            length += strlen(t.directions.a) + 1; // add additional 'A'

            current = *s;
            s += 1;
        }
    } else {
        // recurse into the subsequences
        char *s = seq;
        char current = 'A';
        while (*s != 0) {
            char buf1[20];
            struct dir_target t = find_dir_targets(current, *s);
            strcpy(buf1, t.directions.a); strcat(buf1, "A");
            long len = compute_dirlength(buf1, depth - 1);
            if (t.directions.b != NULL) {
                char buf2[20];
                strcpy(buf2, t.directions.b); strcat(buf2, "A");
                len = MIN(len, compute_dirlength(buf2, depth - 1));
            }
            length += len;

            current = *s;
            s += 1;
        }
    }
    add_cache_entry(seq, depth, length);
    return length;
}

char *dirchars[] = { " ^A", "<v>" };
char sim_dir_buffer[MAX_BUFFER];
char *simulate_dir_pushes(char *pushes) {
    sim_dir_buffer[0] = 0;
    char *p = pushes;
    int curx = 2, cury = 0;
    while (*p != 0) {
        //printf("'%s' + '%c': curx = %d\n", sim_buffer, *p, curx);
        assert((curx >= 0) && (curx <= 2));
        assert((cury >= 0) && (cury <= 1));
        assert(dirchars[cury][curx] != ' ');
        switch (*p) {
        case 'A': {
            char buf[2] = { 0 };
            buf[0] = dirchars[cury][curx];
            strcat(sim_dir_buffer, buf);
        } break;
        case '^':
            cury -= 1;
            break;
        case '<':
            curx -= 1;
            break;
        case 'v':
            cury += 1;
            break;
        case '>':
            curx += 1;
            break;
        default:
            printf("unknown char '%c' in push list\n", *p);
            break;
        }
        p += 1;
    }

    return sim_dir_buffer;
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %u numbers to figure out\n", number_count);

    // implement algorithm
#define NUM_ROBOTS 25
    long count = 0;
    for (unsigned i = 0; i < number_count; i++) {
        long val = 0;
        if (sscanf(numbers[i], "%ld", &val) != 1) {
            printf("Error getting number from '%s'\n", numbers[i]);
            break;
        }

        struct possibilities inputs = numpad2dirs(numbers[i]);
        if (debug) printf("'%s' has %u possibilities\n", numbers[i], inputs.count);
        if (debug) for (unsigned j = 0; j < inputs.count; j++) printf("  %-3ld: %s\n", strlen(inputs.list[j]), inputs.list[j]);
        // simulate all the possibilites and check if they match the original
        for (unsigned j = 0; j < inputs.count; j++) {
            char *simulated = simulate_num_pushes(inputs.list[j]);
            if (strcmp(numbers[i], simulated) != 0) {
                printf("  '%s' doesn't result in '%s'\n", inputs.list[j], numbers[i]);
            }
        }

        // next directional robot steps
        long minlength = LONG_MAX;
        for (unsigned i = 0; i < inputs.count; i++) {
            long len = compute_dirlength(inputs.list[i], NUM_ROBOTS);
            minlength = MIN(minlength, len);
        }
        printf("minlength is %ld\n", minlength);
        count += val * minlength;
    }

    printf("The calculated number is %ld\n", count);

    printf("---- for 2 intermediate robots ----\n");
    printf("Info: the solution for the sample data should be %d\n", 126384);
    printf("Info: the solution for the actual data should be %d\n", 152942);
    printf("---- for 25 intermediate robots ----\n");
    printf("Info: the solution for the sample data should be %ld\n", 0L);
    printf("Info: the solution for the actual data should be %ld\n", 189235298434780L);
    return EXIT_SUCCESS;
}
