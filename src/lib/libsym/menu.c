#include <symbos.h>

void Menu_Icon(Menu_Entry* entry, char* icon) {
    entry->flags |= MENU_ICON;
    entry->text[0] = 0x06;
    entry->text[1] = 0x80;
    entry->text[2] = _symbank;
    *(char**)&entry->text[3] = icon + 1;
    *(char**)&icon[3] = icon + 10;
    *(char**)&icon[5] = icon + 9;
}
