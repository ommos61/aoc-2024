
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

struct num_transition {
    char from;
    struct num_target {
        char to;
        char *directions[2];
    } targets[11];
} num_transitions[] = {
    {'A', {{'A', {NULL, NULL}}, {'0', {"<", NULL}}, {'1', {"v<<", NULL}}, {'2', {"v<", "<v"}}, {'3', {"v", NULL}}, {'4', {"v", NULL}},
                                {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}}}, 
    {'0', {{'A', {NULL, NULL}}, {'0', {"<", NULL}}, {'1', {"v<<", NULL}}, {'2', {"v<", "<v"}}, {'3', {"v", NULL}}, {'4', {"v", NULL}},
                                {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}}}, 
    {'1', {{'A', {NULL, NULL}}, {'0', {"<", NULL}}, {'1', {"v<<", NULL}}, {'2', {"v<", "<v"}}, {'3', {"v", NULL}}, {'4', {"v", NULL}},
                                {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}}}, 
    {'2', {{'A', {NULL, NULL}}, {'0', {"<", NULL}}, {'1', {"v<<", NULL}}, {'2', {"v<", "<v"}}, {'3', {"v", NULL}}, {'4', {"v", NULL}},
                                {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}}}, 
    {'3', {{'A', {NULL, NULL}}, {'0', {"<", NULL}}, {'1', {"v<<", NULL}}, {'2', {"v<", "<v"}}, {'3', {"v", NULL}}, {'4', {"v", NULL}},
                                {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}}}, 
    {'4', {{'A', {NULL, NULL}}, {'0', {"<", NULL}}, {'1', {"v<<", NULL}}, {'2', {"v<", "<v"}}, {'3', {"v", NULL}}, {'4', {"v", NULL}},
                                {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}}}, 
    {'5', {{'A', {NULL, NULL}}, {'0', {"<", NULL}}, {'1', {"v<<", NULL}}, {'2', {"v<", "<v"}}, {'3', {"v", NULL}}, {'4', {"v", NULL}},
                                {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}}}, 
    {'6', {{'A', {NULL, NULL}}, {'0', {"<", NULL}}, {'1', {"v<<", NULL}}, {'2', {"v<", "<v"}}, {'3', {"v", NULL}}, {'4', {"v", NULL}},
                                {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}}}, 
    {'7', {{'A', {NULL, NULL}}, {'0', {"<", NULL}}, {'1', {"v<<", NULL}}, {'2', {"v<", "<v"}}, {'3', {"v", NULL}}, {'4', {"v", NULL}},
                                {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}}}, 
    {'8', {{'A', {NULL, NULL}}, {'0', {"<", NULL}}, {'1', {"v<<", NULL}}, {'2', {"v<", "<v"}}, {'3', {"v", NULL}}, {'4', {"v", NULL}},
                                {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}}}, 
    {'9', {{'A', {NULL, NULL}}, {'0', {"<", NULL}}, {'1', {"v<<", NULL}}, {'2', {"v<", "<v"}}, {'3', {"v", NULL}}, {'4', {"v", NULL}},
                                {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}, {'3', {"v", NULL}}}}, 
};
struct num_target find_num_targets(char from, char to) {
    struct num_target res = { 0 };
    for (unsigned i = 0; i < array_count(num_transitions); i++ ) {
        if (num_transitions[i].from == from) {
            for (unsigned j = 0; j < array_count(num_transitions[i].targets); j++) {
                if (num_transitions[i].targets[j].to == to) {
                    res = num_transitions[i].targets[j];
                    break;
                }
            }
            break;
        }
    }
    return res;
}

struct pos {
    char c;
    int x, y;
} num_positions[] = {
    { '0', 1, 3 },
    { '1', 0, 2 },
    { '2', 1, 2 },
    { '3', 2, 2 },
    { '4', 0, 1 },
    { '5', 1, 1 },
    { '6', 2, 1 },
    { '7', 0, 0 },
    { '8', 1, 0 },
    { '9', 2, 0 },
    { 'A', 2, 3 }
};
#define numchar2pos(c) (((c) == 'A') ? 10 : (c) - '0')

#define MAX_BUFFER 1024
char numpad_buffer[MAX_BUFFER];
char *numpad(char from, char to, int prefer_x) {
    numpad_buffer[0] = 0;
    struct pos from_pos = num_positions[numchar2pos(from)];
    struct pos to_pos = num_positions[numchar2pos(to)];
    int dx = to_pos.x - from_pos.x;
    int dy = to_pos.y - from_pos.y;
    //printf("(%d, %d) -> (%d, %d)\n", from_pos.x, from_pos.y, to_pos.x, to_pos.y);

    // avoid the gap
    if ((from_pos.y == 3) && (to_pos.x == 0)) {
        char *dir = (dy > 0) ? "v" : "^";
        for (int i = 0; i < abs(dy); i++) strcat(numpad_buffer, dir);
        dir = (dx > 0) ? ">" : "<";
        for (int i = 0; i < abs(dx); i++) strcat(numpad_buffer, dir);
    } else if ((from_pos.x == 0) && (to_pos.y == 3)) {
        char *dir = (dx > 0) ? ">" : "<";
        for (int i = 0; i < abs(dx); i++) strcat(numpad_buffer, dir);
        dir = (dy > 0) ? "v" : "^";
        for (int i = 0; i < abs(dy); i++) strcat(numpad_buffer, dir);
    } else {
        if (prefer_x) {
            char *dir = (dx > 0) ? ">" : "<";
            for (int i = 0; i < abs(dx); i++) strcat(numpad_buffer, dir);
            dir = (dy > 0) ? "v" : "^";
            for (int i = 0; i < abs(dy); i++) strcat(numpad_buffer, dir);
        } else {
            char *dir = (dy > 0) ? "v" : "^";
            for (int i = 0; i < abs(dy); i++) strcat(numpad_buffer, dir);
            dir = (dx > 0) ? ">" : "<";
            for (int i = 0; i < abs(dx); i++) strcat(numpad_buffer, dir);
        }
    }
    strcat(numpad_buffer, "A");

    return numpad_buffer;
}

struct pushes {
    char *seq;
    struct pushes *next;
};
struct pushes *new_pushes(char *str)
    { struct pushes *p = malloc(sizeof(struct pushes)); assert(p != NULL); p->seq = malloc(strlen(str) + 1); assert(p->seq != NULL); strcpy(p->seq, str); p->next = NULL; return p; }
void pushes_append(struct pushes *p, char *str)
    { char *tmp = malloc(strlen(p->seq) + strlen(str) + 1); assert(tmp != NULL); strcpy(tmp, p->seq); strcat(tmp, str); free(p->seq); p->seq = tmp; }
struct pushes *numpad2dirpad(char *code) {
    struct pushes *res = new_pushes("");
    char *p = code;
    char current = 'A';
    while (*p != 0) {
        char *seq = numpad(current, *p, 0);
        char seq_buffer[MAX_BUFFER]; strcpy(seq_buffer, seq);
        seq = numpad(current, *p, 1);
        if (strcmp(seq, seq_buffer) != 0) {
            struct pushes *newres = NULL;
            struct pushes *pu = res;
            while (pu != NULL) {
                struct pushes *tmp = new_pushes(pu->seq);
                tmp->next = newres; newres = tmp;
                pushes_append(tmp, seq);
                pu = pu->next;
            }
            pu = res;
            while (pu != NULL) {
                pushes_append(pu, seq_buffer);
                struct pushes *tmp = pu->next;
                pu->next = newres; newres = pu;
                pu = tmp;
            }
            res = newres;
        } else {
            struct pushes *pu = res;
            while (pu != NULL) {
                pushes_append(pu, seq);
                pu = pu->next;
            }
        }
        current = *p;
        p += 1;
    }

    return res;
}

char *numchars[] = { "789", "456", "123", " 0A" };
char sim_num_buffer[MAX_BUFFER];
char *simulate_num_pushes(char *pushes) {
    sim_num_buffer[0] = 0;
    char *p = pushes;
    int curx = 2, cury = 3;
    while (*p != 0) {
        if (debug) printf("'%s' + '%c': curx = %d\n", sim_num_buffer, *p, curx);
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

struct dir_transition {
    char from;
    struct dir_target {
        char to;
        char *directions[2];
    } targets[5];
} dir_transitions[] = {
    {'A', {{'A', {NULL, NULL}}, {'^', {"<", NULL}}, {'<', {"v<<", NULL}}, {'v', {"v<", "<v"}}, {'>', {"v", NULL}}}}, 
    {'^', {{'A', {">", NULL}},  {'^', {NULL, NULL}}, {'<', {"v<", NULL}}, {'v', {"v", NULL}}, {'>', {"v>", ">v"}}}}, 
    {'<', {{'A', {">>^", NULL}}, {'^', {">^", NULL}}, {'<', {NULL, NULL}}, {'v', {">", NULL}}, {'>', {">>", NULL}}}}, 
    {'v', {{'A', {">^", "^>"}}, {'^', {"^", NULL}}, {'<', {"<", NULL}}, {'v', {NULL, NULL}}, {'>', {">", NULL}}}}, 
    {'>', {{'A', {"^", NULL}}, {'^', {"<^", "^<"}}, {'<', {"<<", NULL}}, {'v', {"<", NULL}}, {'>', {NULL, NULL}}}}, 
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

struct pushes *dirpad2dirpad(char *dirs) {
    struct pushes *res = new_pushes("");
    char *p = dirs;
    char current = 'A';
    while (*p != 0) {
        //assert(current != *p);
        struct pushes *newres = NULL;
        struct dir_target t = find_dir_targets(current, *p);
        if (t.directions[1] != NULL) {
            //if (debug) printf(" '%c' -> '%c: (1)'%s'\n", current, *p, t.directions[1]);
            struct pushes *pu = res;
            while (pu != NULL) {
                struct pushes *tmp = new_pushes(pu->seq);
                tmp->next = newres; newres = tmp;
                pushes_append(tmp, t.directions[1]);
                pu = pu->next;
            }
        }
        if (t.directions[0] != NULL) {
            //if (debug) printf(" '%c' -> '%c: (0)'%s'\n", current, *p, t.directions[0]);
            struct pushes *pu = res;
            while (pu != NULL) {
                pushes_append(pu, t.directions[0]);
                struct pushes *tmp = pu->next;
                pu->next = newres; newres = pu;
                pu = tmp;
            }
        }
        if (newres != NULL) res = newres;
        struct pushes *pu = res;
        while (pu != NULL) {
            pushes_append(pu, "A");
            pu = pu->next;
        }

        current = *p;
        p += 1;
    }
    return res;
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
        assert(numchars[cury][curx] != ' ');
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
    long count = 0;
    for (unsigned i = 0; i < number_count; i++) {
        unsigned val = 0;
        if (sscanf(numbers[i], "%u", &val) != 1) {
            printf("Error getting number from '%s'\n", numbers[i]);
            break;
        }

        char *shortest = NULL;
        struct pushes *sequences = numpad2dirpad(numbers[i]);
        struct pushes *s = sequences;
        while (s != NULL) {
            if (debug) printf("'%s' -> '%s'\n", numbers[i], s->seq);
            if (strcmp(numbers[i], simulate_num_pushes(s->seq)) != 0) {
                printf("===> simulation doesn't doesn't match with original: '%s'\n", simulate_dir_pushes(s->seq));
            }

            struct pushes *sequences2 = dirpad2dirpad(s->seq);
            struct pushes *s2 = sequences2;
            while (s2 != NULL) {
                if (debug) printf("'%s' -> '%s'\n", s->seq, s2->seq);
                if (strcmp(s->seq, simulate_dir_pushes(s2->seq)) != 0) {
                    printf("===> simulation doesn't doesn't match with original: '%s'\n", simulate_dir_pushes(s2->seq));
                }

                struct pushes *sequences3 = dirpad2dirpad(s2->seq);
                struct pushes *s3 = sequences3;
                while (s3 != NULL) {
                    //if (debug) printf("'%s' -> '%s'\n", s2->seq, s3->seq);
                    if (strcmp(s2->seq, simulate_dir_pushes(s3->seq)) != 0) {
                        printf("===> simulation doesn't doesn't match with original: '%s'\n", simulate_dir_pushes(s3->seq));
                    }

                    if ((shortest == NULL) || (strlen(s3->seq) < strlen(shortest))) {
                        shortest = s3->seq;
                    }
                    s3 = s3->next;
                }
                s2 = s2->next;
            }
            s = s->next;
        }
        assert(shortest != NULL);
        printf("%s: adding %ld * %d\n", numbers[i], strlen(shortest), val);
        count += val * strlen(shortest);
    }
    printf("The calculated number is %ld\n", count);

    printf("Info: the solution for the sample data should be %d\n", 126384);
    printf("Info: the solution for the actual data should be %d\n", 152942);
    return EXIT_SUCCESS;
}
