struct symbos_hdr {
    uint16_t len_code;
    uint16_t len_data;
    uint16_t len_transfer;
    uint16_t origin;
    uint16_t reloc_count;
    uint16_t stack_offset;
    uint16_t unused1;
    uint8_t bank;
    char name[25];
    uint8_t flags;
    uint16_t icon16_addr;
    char unused2[5];
    char exeid[8];
    uint16_t extra_code;
    uint16_t extra_data;
    uint16_t extra_transfer;
    char unused3[26];
    uint8_t minor_version;
    uint8_t major_version;
    char icon_small[19];
    char icon_large[147];
} __attribute((packed));

