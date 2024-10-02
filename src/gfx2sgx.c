#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
} Pixel;

void fatal(char* msg) {
    printf("error: %s\n", msg);
    exit(1);
}

unsigned char palette_red[16] = {0xf7, 0x06, 0xf7, 0x90, 0x06, 0x06, 0x90, 0x06, 0xf7, 0x06, 0x06, 0xf7, 0xf7, 0x90, 0xf9, 0xf7};
unsigned char palette_green[16] = {0xf7, 0x06, 0x90, 0x06, 0xf7, 0x06, 0x90, 0x06, 0xf7, 0x90, 0xf7, 0x06, 0xf7, 0x90, 0x90, 0x06};
unsigned char palette_blue[16] = {0x90, 0x06, 0x06, 0x06, 0xf7, 0x90, 0xf7, 0xf7, 0xf7, 0x06, 0x06, 0xf7, 0x06, 0x90, 0x90, 0x06};

unsigned char closest_color(Pixel* pixel, unsigned char depth, unsigned char masking) {
    unsigned char i;
    unsigned char shortest_color = 0;
    float dist, shortest_dist = 10000;
    for (i = 0; i < depth; ++i) {
        dist = sqrt((pixel->red - palette_red[i])*(pixel->red - palette_red[i]) +
                    (pixel->green - palette_green[i])*(pixel->green - palette_green[i]) +
                    (pixel->blue - palette_blue[i])*(pixel->blue - palette_blue[i]));
        if (dist < shortest_dist) {
            shortest_dist = dist;
            shortest_color = i;
        }
    }
    if (masking == 1) {
        if (pixel->alpha < 127)
            return (depth == 4) ? 3 : 15;
        else
            return 0;
    } else if (masking == 2) {
        if (pixel->alpha < 127)
            return 0;
    }
    return shortest_color;
}

unsigned char numeric(char* str) {
    while (*str >= '0' && *str <= '9')
        ++str;
    if (*str) // ended on something other than null terminator
        return 0;
    return 1;
}

unsigned short write_header(FILE* fp, int xsize, int ysize, unsigned char outcolors) {
    if (outcolors == 4) {
        fputc((unsigned char)xsize / 4, fp);
        fputc((unsigned char)xsize, fp);
        fputc((unsigned char)ysize, fp);
        return 3;
    } else {
        fputc((unsigned char)xsize / 2, fp);
        fputc((unsigned char)xsize, fp);
        fputc((unsigned char)ysize, fp);
        fputc(0x00, fp);
        fputc(0x00, fp);
        fputc(0x00, fp);
        fputc(0x00, fp);
        fputc(0x20, fp);
        fputc(0x01, fp);
        fputc(0x05, fp);
        return 10;
    }
}

void usage(void) {
    printf("usage: gfx2sgx infile [[tilewidth] [tileheight]] [-4] [-m] [-o outfile]\n\n");

    printf("Converts an image (.BMP, .JPG, .PNG, .GIF, .TGA) to a .SGX graphics asset for\n");
    printf("SCC's graphics.h library. Images are converted pixel-perfect by Euclidean\n");
    printf("similarity to the SymbOS palette colors, so any necessary scaling or recoloring\n");
    printf("must be done using other graphics software prior to conversion.\n\n");

    printf("To create an image set, specify -m or tilewidth and tileheight.\n\n");

    printf("   -4       Convert in 4 colors only (default is 16)\n");
    printf("   -m       Convert alpha channel to sprite mask\n");
    printf("            (image sets will be ordered mask-tile-mask-tile...)\n");
    printf("   -o file  Specify filename of output .SGX\n");

    exit(1);
}

int main(int argc, char* argv[]) {
    int xsize, ysize, nchannels;
    int i, x, y, xstart, ystart, tile, tiles;
    unsigned short tilelen, written;
    unsigned char outbyte, offset, color;
    unsigned char outcolors = 16;
    unsigned char tilewidth = 0;
    unsigned char tileheight = 0;
    unsigned char numbers = 0;
    unsigned char maskmode = 0, maskmax = 0, maskit;
    unsigned char* data;
    char* infile = NULL;
    char* outfile = NULL;
    char* ptr;
    FILE* fp;
    Pixel* pixel;

    // process command-line options
    i = 1;
    while (i < argc) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
            case '4':
                outcolors = 4;
                break;
            case 'm':
                maskmode = 1;
                maskmax = 2;
                break;
            case 'o':
                ++i;
                if (i < argc)
                    outfile = argv[i];
                else
                    usage();
                break;
            default:
                usage();
            }
        } else {
            if (numeric(argv[i])) {
                if (numbers == 0)
                    tilewidth = atoi(argv[i]);
                else if (numbers == 1)
                    tileheight = atoi(argv[i]);
                else
                    usage();
            } else if (!infile) {
                infile = argv[i];
            } else {
                usage();
            }
        }
        ++i;
    }

    // error checking
    if (!infile)
        usage();
    if (tilewidth % 4)
        fatal("tile width is not a multiple of 4!");

    // load input file
    data = stbi_load(infile, &xsize, &ysize, &nchannels, 4);
    if (data == NULL)
        fatal("unable to open and process image");
    printf("%s: %ix%i, %s\n", infile, xsize, ysize, ((nchannels == 1) ? "grayscale" : ((nchannels == 2) ? "grayscale + alpha" : ((nchannels == 3) ? "RGB" : "RGBA"))));
    if (tilewidth == 0) {
        tilewidth = xsize;
        tileheight = ysize;
    }
    if (tilewidth > 252 || tileheight > 255)
        fatal("image dimensions too large!");
    if (xsize % 4)
        fatal("image width is not a multiple of 4!");
    if (tilewidth % 4)
        fatal("tile width is not a multiple of 4!");
    if (((xsize % tilewidth) || (ysize % tileheight)))
        fatal("image cannot be evenly divided into tiles!");
    tiles = (xsize / tilewidth) * (ysize / tileheight);

    // load output file
    if (outfile == NULL) {
        outfile = strdup(infile);
        ptr = strrchr(outfile, '.');
        if (ptr != NULL) {
            if (tiles > 1 || maskmode != 0)
                strcpy(ptr, ".gfx");
            else
                strcpy(ptr, ".sgx");
        }
    }
    fp = fopen(outfile, "wb");
    if (fp == NULL)
        fatal("unable to open output file");

    // create set header if necessary
    tilelen = (tilewidth * tileheight / 2) + 10;
    if (tiles > 1 || maskmode != 0) {
        fputc(tilelen & 0xFF, fp);
        fputc((tilelen >> 8) & 0xFF, fp);
        fputc(maskmode ? tiles*2 : tiles, fp);
    }

    // process image
    xstart = 0;
    ystart = 0;
    written = tilelen;
    for (tile = 0; tile < tiles; ++tile) {
        for (maskit = maskmode; maskit <= maskmax; ++maskit) {
            outbyte = 0;
            offset = 0;

            // pad tile to tilelen
            while (written < tilelen) {
                fputc(0, fp);
                ++written;
            }

            // write pixels
            written = write_header(fp, tilewidth, tileheight, outcolors);
            for (y = ystart; y < ystart + tileheight; ++y) {
                for (x = xstart; x < xstart + tilewidth; ++x) {
                    pixel = (Pixel*)(data + (y*xsize + x)*4);
                    color = closest_color(pixel, outcolors, maskit);
                    if (outcolors == 16) {
                        if (offset == 0)
                            outbyte = (color << 4);
                        else
                            outbyte |= color;
                    } else {
                        if (offset == 0)
                            outbyte = (((color & 1) << 7) | ((color & 2) >> 1));
                        else if (offset == 1)
                            outbyte = (((color & 1) << 6) | ((color & 2) << 0));
                        else if (offset == 2)
                            outbyte = (((color & 1) << 5) | ((color & 2) << 1));
                        else
                            outbyte = (((color & 1) << 4) | ((color & 2) << 2));
                    }
                    ++offset;
                    if ((outcolors == 16 && offset == 2) || (outcolors == 4 && offset == 4)) {
                        fputc(outbyte, fp);
                        outbyte = 0;
                        offset = 0;
                        ++written;
                    }
                }
            }
        }

        // move to next tile
        xstart += tilewidth;
        if (xstart >= xsize) {
            xstart = 0;
            ystart += tileheight;
        }
    }

    // clean up
    stbi_image_free(data);
    fflush(fp);
    fclose(fp);
    if (maskmode)
        printf("%i tile(s) extracted (%i including masks)\n", tiles, tiles*2);
    else if (tiles > 1)
        printf("%i tiles extracted\n", tiles);
    return 0;
}
