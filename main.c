#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "sparse.h"

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fputs("usage: sparsetest <file>\n", stderr);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        fputs("Could not open file ", stderr);
        perror(argv[1]);
        return 1;
    }

    uint32_t length;
    struct sparse_extent *scan = scan_extents(fd, &length);
    if (scan == NULL)
    {
        fputs("ioctl() failed\n", stderr);
        close(fd);
        return 1;
    }

    for (uint32_t i = 0; i < length; i++)
    {
        printf("extent %u: start: %lu, end: %lu, length: %lu\n", i,
               scan[i].start, scan[i].start + scan[i].length, scan[i].length);
    }

    uint64_t last_end = 0;
    uint32_t hole = 0;
    for (uint32_t i = 0; i < length; i++)
    {
        if (scan[i].start > last_end)
        {
            printf("hole %u: start: %lu, end: %lu, length: %lu\n", hole++,
                   last_end, scan[i].start, scan[i].start - last_end);
        }
        last_end = scan[i].start + scan[i].length;
    }

    free(scan);
    close(fd);
    return 0;
}