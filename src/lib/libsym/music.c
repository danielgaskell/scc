#include <symbos.h>
#include "sound.h"

/* ========================================================================== */
/* SymbOS Sound Daemon calls (music)                                          */
/* ========================================================================== */

signed char Music_Load(unsigned char fid, unsigned char hw) {
    if (_soundmus != 255)
        Music_Free();
    _msemaon();
    _symmsg[0] = 8;
    _symmsg[3] = fid;
    _symmsg[6] = 0; // source = file
    _symmsg[7] = hw;
    if (Sound_Command()) {
        _msemaoff();
        return -1;
    }
    _soundmus = _symmsg[3]; // music handle;
    _msemaoff();
    return 0;
}

signed char Music_Load_Mem(unsigned char bank, char* addr, unsigned short len) {
    if (_soundmus != 255)
        Music_Free();
    _msemaon();
    _symmsg[0] = 8;
    _symmsg[3] = bank;
    *((unsigned short*)(_symmsg + 4)) = len;
    *((unsigned short*)(_symmsg + 8)) = (unsigned short)addr;
    _symmsg[6] = 1; // source = memory
    _symmsg[7] = 1; // hardware = PSG
    if (Sound_Command()) {
        _msemaoff();
        return -1;
    }
    _soundmus = _symmsg[3]; // music handle;
    _msemaoff();
    return 0;
}

void Music_Start(unsigned char track) {
    if (_soundmus == 255)
        return;
    _msemaon();
    _symmsg[0] = 10;
    _symmsg[3] = _soundmus;
    _symmsg[8] = track;
    _msemaoff();
}

void Music_Stop(void) {
    if (_soundmus == 255)
        return;
    _msemaon();
    _symmsg[0] = 12;
    _symmsg[3] = _soundmus;
    _msemaoff();
}

void Music_Continue(void) {
    if (_soundmus == 255)
        return;
    _msemaon();
    _symmsg[0] = 11;
    _symmsg[3] = _soundmus;
    _msemaoff();
}

void Music_Volume(unsigned char vol) {
    if (_soundmus == 255)
        return;
    _msemaon();
    _symmsg[0] = 13;
    _symmsg[3] = _soundmus;
    _symmsg[9] = vol;
    _msemaoff();
}
