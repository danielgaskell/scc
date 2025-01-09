#define MAX_SYMBOLS 2048
#define SYMBOL_LEN 15

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned short addr;
    unsigned char segment;
    char name[SYMBOL_LEN + 2];
} SYMBOL;

int compare_addr(const void* a, const void* b) {
    return (((SYMBOL*)a)->addr > ((SYMBOL*)b)->addr) -
           (((SYMBOL*)a)->addr < ((SYMBOL*)b)->addr);
}

SYMBOL symbols[MAX_SYMBOLS];
char buffer[SYMBOL_LEN + 10];
int count = 0;

int main(int argc, char* argv[]) {
    int i;
    char* ptr;
    FILE* f;

    if (argc != 2) {
        printf("sortmap: sorts a .map file by address and calculates symbol size.\n");
        printf("usage: sortmap file.map\n");
        exit(1);
    }

    // read file
    f = fopen(argv[1], "r");
    if (f == NULL) {
        printf("sortmap: unable to open %s\n", argv[1]);
        exit(1);
    }
    while (!feof(f)) {
        fgets(buffer, sizeof(buffer), f);
        if (strlen(buffer)) {
            buffer[4] = 0; // zero-terminate for strtol()
            symbols[count].addr = strtol(buffer, &ptr, 16);
            symbols[count].segment = buffer[5];
            strcpy(symbols[count].name, buffer + 7);
            ++count;
        }
    }
    fclose(f);

    // sort and output
    qsort(symbols, count, sizeof(SYMBOL), compare_addr);
    symbols[count].addr = symbols[count-1].addr - 1; // so the last symbol (whose size is unknown) displays as -1
    for (i = 0; i < count; ++i) {
        printf("%04x %c (%i) %s", symbols[i].addr,
                                  symbols[i].segment,
                                  symbols[i+1].addr - symbols[i].addr,
                                  symbols[i].name);
    }
}
