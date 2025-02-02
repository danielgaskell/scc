
/* Symbols live in a set of hashed lists indexed by name. Only one
   instance of each name ever exists. */
struct symbol
{
    struct symbol *next;
    struct object *definedby;
    char name[NAMELEN];
    addr_t value;
    uint16_t number;	/* Needed when doing ld -r */
    uint8_t type;
    uint8_t flags;
};

struct object {
    struct object *next;
    struct symbol **syment;
    /* We might want to store a subset of this */
    struct objhdr *oh;
    addr_t base[OSEG];	/* Base address we select for this object */
    int nsym;
    const char *path;		/* We need more for library nodes.. */
    off_t off;		/* For libraries */
};

#define NHASH	64	/* Must be a power of 2 */

/* Methods shared for use in target modules */
extern void warning(const char *p);
extern void error(const char *p);
extern unsigned io_readb(uint_fast8_t *ch);
