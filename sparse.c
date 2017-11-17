#include "sparse.h"
#include <linux/fiemap.h>
#include <linux/fs.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#ifdef __linux__
#include <linux/fiemap.h>

struct sparse_extent *scan_extents(int fd, uint32_t *length)
{
    union {
        struct fiemap f;
        char c[8192];
    } buf;
    struct fiemap *fiemap = &buf.f;
    struct fiemap_extent *extents = &fiemap->fm_extents[0];
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
    struct sparse_extent *holes =
        malloc(sizeof(struct sparse_extent) * expected_length);
    fiemap->fm_extent_count =
        (sizeof(buf) - sizeof(*fiemap)) / sizeof(*extents);

    *length = 0;
    struct fiemap_extent *last;
    do
    {
        if (ioctl(fd, FS_IOC_FIEMAP, fiemap) != 0)
        {
            free(holes);
            return NULL;
        }

        uint32_t new_length = *length + fiemap->fm_mapped_extents;
        if (new_length > expected_length)
        {
            holes = realloc(holes, sizeof(struct sparse_extent) * new_length);
            expected_length = new_length;
        }

        for (uint32_t i = 0; i < fiemap->fm_mapped_extents; i++)
        {
            holes[*length].start = extents[i].fe_logical;
            holes[*length].length = extents[i].fe_length;
            (*length)++;
        }

        last = &extents[fiemap->fm_extent_count - 1];
        fiemap->fm_start = last->fe_logical + last->fe_length;
        fiemap->fm_length = FIEMAP_MAX_OFFSET - fiemap->fm_start;
    } while (!(last->fe_flags | FIEMAP_EXTENT_LAST));
    return holes;
}

    //#elif defined(SEEK_HOLE)
    // solaris?

#else
// no sparse support

#endif
