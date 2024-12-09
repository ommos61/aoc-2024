
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
int debug = 0; // 0 = no debug output, 1 = some extra debug output
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

int find_free_space(int *disk, unsigned size, unsigned below) {
    if (debug) printf(" finding freespace of %d blocks\n", size);
    int result = 0;
    unsigned index = 0;
    while ((result == 0) && (index < below)) {
        if (disk[index] != -1) {
            index += 1;
        } else {
            //if (debug) printf(" free space starting at %d\n", index);
            for (unsigned i = 0; i < size; i++) {
                if (disk[index + i] != -1) {
                    index += i;
                    break;
                } else if (i == (size - 1)) {
                    result = index;
                    return result;
                }
            }
        }
    }
    return result;
}

int find_start(int *disk, unsigned size, int fileid) {
    for (unsigned i = 0; i < size; i++) {
         if (disk[i] == fileid) {
            return i;
         }
    }
    return -1;
}

void print_disk(int *disk, unsigned disksize) {
    for (unsigned i = 0; i < disksize; i++) {
        if (disk[i] == -1) {
            printf(".");
        } else {
            printf("%d", disk[i]);
        }
    }
    printf("\n");
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
    int file2move = fileid - 1;
    if (debug) printf("%s\n", diskinfo);
    while (file2move > 0) {
        unsigned filesize = diskinfo[file2move * 2] - '0';
        if (debug) printf(" trying to move file %d, size %d\n", file2move, filesize);
        unsigned oldstart = find_start(disk, disksize, file2move);
        int freespace = find_free_space(disk, filesize, oldstart);
        //if (debug) printf(" found enough space at index %d\n", freespace);
        if (freespace != 0) {
            // move the actual data
            for (unsigned i = 0; i < filesize; i++) {
                disk[freespace + i] = file2move;
                disk[oldstart + i] = -1;
            }
        }
        //if (debug) print_disk(disk, disksize);
        file2move -= 1;
    }

    // calculate the checksum of the resulting disk
    long checksum = 0;
    for (unsigned i = 0; i < disksize; i++) {
        if (disk[i] != -1) {
            checksum += (i * disk[i]);
        }
    }
    printf("the calculates checksum is %ld\n", checksum);

    printf("Info: the solution for the sample data should be %ld\n", 2858L);
    printf("Info: the solution for the actual data should be %ld\n", 6363913128533L);
    return EXIT_SUCCESS;
}

