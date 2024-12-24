
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

#define MAX_COMPUTER_CONNECTIONS 20
struct computer {
    char name[NAMELEN + 1];
    char *connections[MAX_COMPUTER_CONNECTIONS];
    unsigned connection_count;
    struct computer *next;
} *computers = NULL;
char **computer_names = NULL;
unsigned computer_count = 0;

struct computer *new_computer(char *name)
    { struct computer *tmp = malloc(sizeof(struct computer)); assert(tmp != NULL); strcpy(tmp->name, name); tmp->connection_count = 0; tmp->next = NULL; return tmp; }

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
        for (unsigned i = 0; i < comp->connection_count; i++) {
            printf("%s%s", comp->connections[i], (i < comp->connection_count - 1) ? ", " : "");
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
        if (comp1 == NULL) { comp1 = new_computer(name1); comp1->next = computers; computers = comp1; computer_count += 1; }
        if (comp2 == NULL) { comp2 = new_computer(name2); comp2->next = computers; computers = comp2; computer_count += 1; }
        assert(comp1->connection_count < MAX_COMPUTER_CONNECTIONS);
        comp1->connections[comp1->connection_count] = name2; comp1->connection_count += 1;
        assert(comp2->connection_count < MAX_COMPUTER_CONNECTIONS);
        comp2->connections[comp2->connection_count] = name1; comp2->connection_count += 1;
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
        if (comp->connection_count > max_conns) max_conns = comp->connection_count;
        comp = comp->next;
    }
    return max_conns;
}

int is_connected(char *name1, char *name2) {
    struct computer *comp = find_computer(name1);
    if (comp == NULL) { printf("Unable to find computer '%s'\n", name1); exit(1); }
    for (unsigned i = 0; i < comp->connection_count; i++) {
        if (strcmp(comp->connections[i], name2) == 0) return 1;
    }
    return 0;
}

int is_historian(char *name) {
    return (name[0] == 't');
}

void add2party(char **party, unsigned *count, char *name) {
    int present = 0;
    for (unsigned i = 0; i < *count; i++) {
        if (strcmp(party[i], name) == 0) {
            present = 1;
            break;
        }
    }
    if (!present) {
        party[*count] = name;
        *count += 1;
    }
}

void print_party(char **party, unsigned count) {
    for (unsigned i = 0; i < count; i++) {
         printf("%s%s", party[i], (i < count - 1) ? "," : "");
    }
    printf("\n");
}

int check_party(char **party, unsigned count) {
    int present = 0;
    for (unsigned i = 0; i < count; i++) {
        if (party[i][0] == 't') {
            present = 1;
            break;
        }
    }
    return present;
}

int is_all_connected(char **party, unsigned count, char *name) {
    int connected = 1;
    for (unsigned i = 0; i < count; i++) {
        if (!is_connected(party[i], name)) {
            connected = 0;
            break;
        }
    }

    return connected;
}

void build_party(char **party, unsigned *party_count, char **connections, unsigned connection_count) {
    for (unsigned i = 0; i < connection_count; i++) {
        if (is_all_connected(party, *party_count, connections[i])) {
            add2party(party, party_count, connections[i]);
        }
    }

    return;
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

#define MAX_PARTY 200
    unsigned max_party_size = 0;
    char *max_party[MAX_PARTY];
    char *party[MAX_PARTY];
    unsigned party_count = 0;
    for (unsigned n = 0; n < computer_count; n++) {
        struct computer *comp = find_computer(computer_names[n]);
        assert(comp != NULL);
        party_count = 0;
        add2party(party, &party_count, comp->name);
        build_party(party, &party_count, comp->connections, comp->connection_count);
        //if (debug) { printf(" build: "); print_party(party, party_count); }

        if (check_party(party, party_count) && (party_count >= max_party_size)) {
            if (debug) printf("Found new bigger party of %u\n", party_count);
            qsort(party, party_count, sizeof(char *), namecompare);
            if (debug) { printf("  "); print_party(party, party_count); }
            max_party_size = party_count;
            for (unsigned i = 0; i < party_count; i++) max_party[i] = party[i];
        }
        comp = comp->next;
    }
    printf("max LAN party size is %u\n", max_party_size);
    qsort(max_party, max_party_size, sizeof(char *), namecompare);
    print_party(max_party, max_party_size);

    printf("Info: the solution for the sample data should be %s\n", "co,de,ka,ta");
    printf("Info: the solution for the actual data should be %s\n", "er,fh,fi,ir,kk,lo,lp,qi,ti,vb,xf,ys,yu");
    return EXIT_SUCCESS;
}

