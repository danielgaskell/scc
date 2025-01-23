#include <symbos.h>

void Time2Obj(unsigned long timestamp, SymTime* obj) {
    obj->second = (timestamp & 0x1F) * 2;
    obj->minute = (timestamp >> 5) & 0x3F;
    obj->hour   = (timestamp >> 11) & 0x1F;
    obj->day    = (timestamp >> 16) & 0x1F;
    obj->month  = (timestamp >> 21) & 0x0F;
    obj->year   = (timestamp >> 25) & 0x7F;
    obj->timezone = 0;
}

unsigned long Obj2Time(SymTime* obj) {
    return ((unsigned long)(obj->second) / 2) |
           ((unsigned long)(obj->minute) << 5) |
           ((unsigned long)(obj->hour) << 11) |
           ((unsigned long)(obj->day) << 16) |
           ((unsigned long)(obj->month) << 21) |
           ((unsigned long)(obj->year) << 25);
}
