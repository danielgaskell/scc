/*
 *	Do relocation merging for Fuzix style binaries. The relocations
 *	are added to the end of data and the data extended and bss shrunk
 *	to match. If we extend too far we just grow the bss. That's a real
 *	corner case and a bit of extra bss won't matter (plus we brk back
 *	anyway after load).
 *
 *	TODO:
 *	ZP
 *	support for binaries with debug info
 *	Big endian support (host and target)
 */

// I think we need to write out static 0xFF space for BSS because the SymbOS
// relocator can't handle addresses in the "extended" space - but the new
// kernel compression scheme helps reduce binary size?

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "symhead.h"

/* 16 byte header for current style binary. We try to reflect the general
   pattern of naming in classic Unixlike systems */

/* Do not change these values. They are (or will be) shared with asm stubs
   and the toolchain */
struct exec2 {
	uint16_t a_magic;
#define EXEC_MAGIC	0x80A8		/* Just need something to id with */
	uint8_t a_cpu;
	uint8_t a_cpufeat;
	uint8_t a_base;			/* Load address page */
	uint8_t a_hints;
	uint16_t a_text;
	uint16_t a_data;
	uint16_t a_bss;
	uint8_t a_entry;		/* Entry point - 0-255 bytes in only */
	/* These are kept in pages */
	uint8_t a_size;			/* Binary memory request 0 = all */
	uint8_t a_stack;		/* Stack size hint (not yet used) */
	uint8_t a_zp;			/* Zero/Direct page space required */

	/* This isn't really part of the header but a location fixed after
	   it */
	/* uint16_t a_sigvec; */
} __attribute((packed));

static struct symbos_hdr hdr;

// Expanded read() that more consistently returns read length (seems to be a mingw problem...?)
int rread(int fd, void* buf, unsigned int maxchars) {
    int preseek, readerr;
    preseek = lseek(fd, 0, SEEK_CUR);
    readerr = read(fd, buf, maxchars);
    if (readerr)
        return readerr;
    else
        return lseek(fd, 0, SEEK_CUR) - preseek;
}

int main(int argc, char *argv[])
{
    uint8_t ar[3];
    uint16_t count;
    uint16_t addr;
    int efd, rfd;
    static const uint8_t cff = 0xFF;
    uint8_t d;

    if (argc != 3) {
        fprintf(stderr, "%s binary relocs\n", argv[0]);
        exit(1);
    }
    efd = open(argv[1], O_RDWR | O_BINARY);
    if (efd == -1) {
        perror(argv[1]);
        exit(1);
    }
    rfd = open(argv[2], O_RDONLY | O_BINARY);
    if (rfd == -1) {
        perror(argv[2]);
        exit(1);
    }
    if (rread(efd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        fprintf(stderr, "%s: not valid.\n", argv[1]);
        exit(1);
    }
    if (lseek(efd, 0, SEEK_END) < 0) {
        perror("lseek");
        exit(1);
    }
    if (lseek(rfd, 0, SEEK_SET) < 0) {
        perror("lseek");
        exit(1);
    }
    /* Non ZP block */
    // Format of the .rel file seems to basically be three-byte chunks:
    //   - 1byte = 0 if zp, 1 if not?
    //   - 2byte = addr, big-endian(!)
    count = 0;
    while(rread(rfd, ar, 3) == 3) {
        if (ar[0] == 0)
            continue;
        addr = ((ar[1] << 8) + ar[2]) - 1 + hdr.origin;
        write(efd, &addr, 2);
        ++count;
    }

    // update the header with transfer information
    printf("\nCode segment: %i", hdr.len_code);
    if (hdr.extra_code)
        printf(" (+%i)", hdr.extra_code);
    printf("\nData segment: %i", hdr.len_data);
    if (hdr.extra_data)
        printf(" (+%i)", hdr.extra_code);
    printf("\nTransfer segment: %i", hdr.len_transfer);
    if (hdr.extra_transfer)
        printf(" (+%i)", hdr.extra_code);
    printf("\nRelocation entries: %i\n", count);
    if (lseek(efd, 0, SEEK_SET) < 0) {
        perror("lseek");
        exit(1);
    }
    hdr.reloc_count = count;
    if (write(efd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        fprintf(stderr, "%s: unable to update header.\n", argv[2]);
        exit(1);
    }
    if (hdr.len_data + hdr.extra_data > 16*1024)
        fprintf(stderr, "WARNING: Data segment is >16k!\n");
    if (hdr.len_transfer + hdr.extra_transfer > 16*1024)
        fprintf(stderr, "WARNING: Transfer segment is >16k!\n");
    if (hdr.len_code + hdr.extra_code + hdr.len_data + hdr.extra_data + hdr.len_transfer + hdr.extra_transfer + 0x100 > 64*1024)
        fprintf(stderr, "WARNING: Primary file size is >64k!\n");

    close(efd);
    close(rfd);
    return 0;
}
