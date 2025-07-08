#include <symbos.h>

/* ========================================================================== */
/* File Manager                                                               */
/* ========================================================================== */

void Sys_DriveInfo(char letter, Drive_Info* obj) {
    int result;
    _msemaon();
    _symmsg[1] = 42;
    _symmsg[3] = letter;
    _symmsg[4] = 0;
    result = File_Command();
    if (result == 0) {
        obj->status = _symmsg[3];
        obj->fs = _symmsg[4];
        obj->type = _symmsg[5];
        obj->sectors = _symmsg[7];
        obj->clusters = *(unsigned long*)&_symmsg[10];
        obj->removeable = _symmsg[5] >> 7;
    }
    _msemaoff();
}

unsigned long Sys_DriveFree(char letter) {
    int result;
    unsigned long size;
    _msemaon();
    _symmsg[1] = 42;
    _symmsg[3] = letter;
    _symmsg[4] = 1;
    result = File_Command();
    if (result == 0) {
        size = *(unsigned long*)&_symmsg[6];
        _msemaoff();
        return size;
    }
    _msemaoff();
    return 0;
}
