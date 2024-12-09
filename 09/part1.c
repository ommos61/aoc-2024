
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define array_count(a) (sizeof(a)/sizeof(a[0]))
#define LINE_LENGTH (50 * 1024)
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

// TODO: Global data information
int debug = 1; // 0 = no debug output, 1 = some extra debug output
char *diskinfo = NULL;
unsigned infosize = 0;

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
            if (diskinfo != NULL) {
                printf("Error: cannot read to diskinfos\n");
            } else {
                char *tmp = malloc(strlen(line) + 1);
                strcpy(tmp, line);
                diskinfo = tmp;
                infosize = strlen(diskinfo);
            }
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

int main(int argc, char *argv[]) {
    char *fname = "input.txt";

    // when another input file is specified
    if (argc != 1) {
        fname = argv[1];
    }

    readData(fname);
    printf("There are %d block infos\n", infosize);

    // implement algorithm
    unsigned disksize = 0;
    for (unsigned i = 0; i < infosize; i++) {
        disksize += (diskinfo[i] - '0');
    }
    printf("the total size of the disk is %d\n", disksize);
    int *disk = malloc(disksize * sizeof(int));
    assert(disk != NULL);
    int *d = disk;
    int fileid = 0;
    for (unsigned di = 0; di < infosize; di++) {
        if ((di % 2) == 0) {
            // file blocks
            unsigned filesize = (diskinfo[di] - '0');
            for (unsigned i = 0; i < filesize; i++) {
                *d = fileid;
                d += 1;
            }
            fileid += 1;
        } else {
            // empty space
            unsigned freesize = (diskinfo[di] - '0');
            for (unsigned i = 0; i < freesize; i++) {
                *d = -1;
                d += 1;
            }
        }
    }

    // fill up the free space
    unsigned current_free = 0;
    while (disk[current_free] != -1) current_free += 1;
    unsigned lastfile_block = disksize - 1;
    while (current_free < lastfile_block) {
        disk[current_free] = disk[lastfile_block];
        disk[lastfile_block] = -1;
        while (disk[current_free] != -1) current_free += 1;
        while (disk[lastfile_block] == -1) lastfile_block -= 1;
    }

    // calculate the checksum of the resulting disk
    long checksum = 0;
    for (unsigned i = 0; i < disksize; i++) {
        if (disk[i] == -1) {
            break;
        } else {
            checksum += (i * disk[i]);
        }
    }
    printf("the calculates checksum is %ld\n", checksum);

    printf("Info: the solution for the sample data should be %ld\n", 1928L);
    printf("Info: the solution for the actual data should be %ld\n", 6340197768906L);
    return EXIT_SUCCESS;
}

