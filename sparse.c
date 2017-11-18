#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sparse.h"

#ifdef __linux__
#include <linux/fiemap.h>
#include <linux/fs.h>
#include <sys/ioctl.h>

struct sparse_extent *scan_extents(int fd, uint32_t *length)
{
    union {
        struct fiemap f;
        char c[8192];
    } buf;
    struct fiemap *fiemap = &buf.f;
    struct fiemap_extent *fm_extents = &fiemap->fm_extents[0];
    // memset(fiemap, 0, sizeof(*fiemap));
    memset(&buf, 0, sizeof(buf));
    // needed for safety, see https://bugs.launchpad.net/qemu/+bug/1368815
    fiemap->fm_flags = FIEMAP_FLAG_SYNC;
    fiemap->fm_length = FIEMAP_MAX_OFFSET;
    if (ioctl(fd, FS_IOC_FIEMAP, fiemap) != 0)
    {
        return NULL;
    }

    uint32_t expected_length = fiemap->fm_mapped_extents;
    struct sparse_extent *extents = malloc(sizeof(struct sparse_extent) * expected_length);
    fiemap->fm_extent_count = (sizeof(buf) - sizeof(*fiemap)) / sizeof(*fm_extents);

    *length = 0;
    struct fiemap_extent *last;
    do
    {
        if (ioctl(fd, FS_IOC_FIEMAP, fiemap) != 0)
        {
            free(extents);
            return NULL;
        }

        uint32_t new_length = *length + fiemap->fm_mapped_extents;
        if (new_length > expected_length)
        {
            extents = realloc(extents, sizeof(struct sparse_extent) * new_length);
            expected_length = new_length;
        }

        for (uint32_t i = 0; i < fiemap->fm_mapped_extents; i++)
        {
            extents[*length].start = fm_extents[i].fe_logical;
            extents[*length].length = fm_extents[i].fe_length;
            (*length)++;
        }

        last = &fm_extents[fiemap->fm_extent_count - 1];
        fiemap->fm_start = last->fe_logical + last->fe_length;
        fiemap->fm_length = FIEMAP_MAX_OFFSET - fiemap->fm_start;
    } while (!(last->fe_flags | FIEMAP_EXTENT_LAST));
    return extents;
}

#else

#include <sys/types.h>
#include <unistd.h>

#ifdef SEEK_HOLE

struct sparse_extent *scan_extents(int fd, uint32_t *length)
{
    off_t eof_pos = lseek(fd, 0, SEEK_END);
    off_t pos = lseek(fd, 0, SEEK_HOLE);

    if (pos == eof_pos)
    {
        return NULL;
    }
    int what = pos ? SEEK_HOLE : SEEK_DATA;
    pos = 0;

    *length = 0;
    uint32_t buf_length = 4;
    struct sparse_extent *extents = malloc(sizeof(struct sparse_extent) * buf_length);

    off_t next_pos;
    for (pos = 0; pos < eof_pos; pos = next_pos)
    {
        if ((next_pos = lseek(fd, pos, what)) == (off_t)-1)
        {
            free(extents);
            return NULL;
        }
        if (*length >= buf_length)
        {
            buf_length *= 2;
            extents = realloc(extents, sizeof(struct sparse_extent) * buf_length);
        }
        extents[*length].start = pos;
        extents[*length].length = next_pos - pos;
        (*length)++;
    }
    return extents;
}

#else

struct sparse_extent *scan_extents(int fd, uint32_t *length)
{
    *length = 0;
    return NULL;
}

#endif
#endif
