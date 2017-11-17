#ifndef SPARSE_DEFINE_H__
#define SPARSE_DEFINE_H__

#include <stdint.h>

struct sparse_extent
{
    uint64_t start, length;
};

struct sparse_extent *scan_extents(int fd, uint32_t *length);

#endif