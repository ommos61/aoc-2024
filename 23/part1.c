
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
#define MAX_CONNECTIONS 3500
#define NAMELEN 2
struct connection {
    char a[NAMELEN + 1], b[NAMELEN + 1];
} connections[MAX_CONNECTIONS];
unsigned connection_count = 0;

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

        char buf1[20], buf2[20];
        if (sscanf(line, "%2s-%2s", buf1, buf2) == 2) {
            // store the data
            strcpy(connections[connection_count].a, buf1);
            strcpy(connections[connection_count].b, buf2);
            connection_count += 1;
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

#define MAX_COMPUTERS (26*26)
struct computer {
    char name[NAMELEN + 1];
    struct computer *connections;
    struct computer *next;
} *computers = NULL;
char **computer_names = NULL;
unsigned computer_count = 0;

struct computer *new_computer(char *name, struct computer *connections)
    { struct computer *tmp = malloc(sizeof(struct computer)); assert(tmp != NULL); strcpy(tmp->name, name); tmp->connections = connections; tmp->next = NULL; return tmp; }

struct computer *find_computer(char *name) {
    struct computer *comp = computers;
    while (comp != NULL) {
        if (strcmp(comp->name, name) == 0) {
            // found it
            break;
        }
        comp = comp->next;
    }
    return comp;
}

int namecompare(const void *v1, const void *v2) {
    char *n1 = *(char **)v1, *n2 = *(char **)v2;
    //if (debug) printf("comparing '%s' and '%s'\n", n1, n2);
    return strcmp(n1, n2);
}

void print_computers(void) {
    for (unsigned n = 0; n < computer_count; n++) {
        struct computer *comp = find_computer(computer_names[n]);
        assert(comp != NULL);
        printf("%s: ", comp->name);
        struct computer *conns = comp->connections;
        while (conns != NULL) {
            printf("%s%s", conns->name, (conns->connections != NULL) ? ", " : "");
            conns = conns->connections;
        }
        printf("\n");
        comp = comp->next;
    }
}

void build_computer_list(void) {
    for (unsigned i = 0; i < connection_count; i++) {
        char *name1 = connections[i].a;
        char *name2 = connections[i].b;
        struct computer *comp1 = find_computer(name1);
        struct computer *comp2 = find_computer(name2);
        if (comp1 == NULL) { comp1 = new_computer(name1, NULL); comp1->next = computers; computers = comp1; computer_count += 1; }
        if (comp2 == NULL) { comp2 = new_computer(name2, NULL); comp2->next = computers; computers = comp2; computer_count += 1; }
        comp1->connections = new_computer(name2, comp1->connections);
        comp2->connections = new_computer(name1, comp2->connections);
    }
    if (computer_names == NULL) { computer_names = malloc(computer_count * sizeof(char *)); assert(computer_names != NULL); }
    struct computer *comp = computers;
    unsigned index = 0;
    while (comp != NULL) {
        computer_names[index] = comp->name;
        index += 1;
        comp = comp->next;
    }
    //for (unsigned i = 0; i < computer_count; i++) printf("%s ", computer_names[i]); printf("\n");
    qsort(computer_names, computer_count, sizeof(char *), namecompare);
    //for (unsigned i = 0; i < computer_count; i++) printf("%s ", computer_names[i]); printf("\n");
}

unsigned max_connections(void) {
    unsigned max_conns = 0;
    struct computer *comp = computers;
    while (comp != NULL) {
        unsigned conn_count = 0;
        struct computer *conn = comp->connections;
        while (conn != NULL) {
            conn_count += 1;
            conn = conn->connections;;
        }
        if (conn_count > max_conns) max_conns = conn_count;
        comp = comp->next;
    }
    return max_conns;
}

int is_connected(char *name1, char *name2) {
    struct computer *comp = find_computer(name1);
    if (comp == NULL) { printf("Unable to find computer '%s'\n", name1); exit(1); }
    struct computer *conn = comp->connections;
    while (conn != NULL) {
        if (strcmp(conn->name, name2) == 0) return 1;
        conn = conn->connections;
    }
    return 0;
}

int is_historian(char *name) {
    return (name[0] == 't');
}

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %u connections\n", connection_count);
    //for (unsigned i = 0; i < 10; i++) printf("%s -> %s\n", connections[i].a, connections[i].b);

    // implement algorithm
    build_computer_list();
    printf("There are %u computers\n", computer_count);
    printf("There are %u maximum connections to other computers\n", max_connections());
    if (debug) print_computers();

    unsigned count = 0;
    for (unsigned n = 0; n < computer_count; n++) {
        struct computer *comp = find_computer(computer_names[n]);
        assert(comp != NULL);
        //if (debug) printf("checking computer '%s'\n", comp->name);
        struct computer *conns = comp->connections;
        while (conns != NULL) {
            int cmp = strcmp(comp->name, conns->name);
            //if (debug) printf("comparing '%s' and '%s' gives %d\n", comp->name, conns->name, cmp);
            if (cmp < 0) {
                struct computer *conns2 = comp->connections;
                while (conns2 != NULL) {
                    if (strcmp(conns->name, conns2->name) < 0) {
                        if (is_connected(conns->name, conns2->name)) {
                            if (is_historian(comp->name) || is_historian(conns->name) || is_historian(conns2->name)) {
                                count += 1;
                                if (debug) printf("%s,%s,%s\n", comp->name, conns->name, conns2->name);
                            }
                        }
                    }
                    conns2 = conns2->connections;
                }
            }
            conns = conns->connections;
        }
        comp = comp->next;
    }
    printf("There are %u computer clusters where the historiaan can be\n", count);

    printf("Info: the solution for the sample data should be %d\n", 7);
    printf("Info: the solution for the actual data should be %d\n", 1323);
    return EXIT_SUCCESS;
}

