# `sparsetest`

A simple utility that lists extents and holes in a file. Mainly developed because I couldn't find a simple example of using the FIEMAP ioctl to find holes in a file that was self-contained and MIT-licensed.

## Usage

    sparsetest <file>

## Example

    $ dd if=/dev/urandom of=test seek=10 conv=notrunc bs=1MiB count=20
    20+0 records in
    20+0 records out
    20971520 bytes (21 MB, 20 MiB) copied, 0.120803 s, 174 MB/s
    $ sparsetest test
    extent 0: start: 10485760, end: 18874368, length: 8388608
    extent 1: start: 18874368, end: 31457280, length: 12582912
    hole 0: start: 0, end: 10485760, length: 10485760
