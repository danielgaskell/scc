#include <symbos.h>
#include "sound.h"

/* ========================================================================== */
/* SymbOS Sound Daemon calls (sound effects)                                          */
/* ========================================================================== */

// find a registry slot for a loaded effect collection
signed char Effect_Register(unsigned char hw) {
    unsigned char i;
    for (i = 0; i < SOUND_MAX_FX; ++i) {
        if (_soundfx[i] == 0) {
            _soundfx[i] = _symmsg[3]; // internal handle
            _soundfxhw[i] = hw;
            _msemaoff();
            return i+1; // SCC handle
        }
    }
    _msemaoff();
    _sounderr = ERR_TOOMANY;
    return -1;
}

signed char Effect_Load(unsigned char fid, unsigned char hw) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 16;
    _symmsg[3] = fid;
    _symmsg[6] = 0; // source = file
    _symmsg[7] = hw;
    result = Sound_Command(1);
    if (result) {
        _msemaoff();
        return -1;
    }
    return Effect_Register(hw);
}

signed char Effect_Load_Mem(unsigned char bank, char* addr, unsigned short len) {
    unsigned char result;
    _msemaon();
    _symmsg[0] = 16;
    _symmsg[3] = bank;
    *((unsigned short*)(_symmsg + 4)) = len;
    *((unsigned short*)(_symmsg + 8)) = (unsigned short)addr;
    _symmsg[6] = 1; // source = memory
    _symmsg[7] = 1; // hardware = PSG
    result = Sound_Command(1);
    if (result) {
        _msemaoff();
        return -1;
    }
    return Effect_Register(1);
}

void Effect_Play(unsigned char handle, unsigned char id, unsigned char volume,
                 unsigned char priority, unsigned char pan, int pitch) {
    if (_soundfxhw[handle-1] == SOUND_PSG) {
        // PSG-specific defaults
        if (pan == PAN_LEFT)
            pan = 0;
        else if (pan == PAN_RIGHT)
            pan = 2;
        else
            pan = 1; // PAN_MIDDLE
        if (!priority)
            priority = FX_ANY;
    } else if (!priority) {
        // OPL4-specific defaults
        priority = FX_PLAY;
    }
    _msemaon();
    _symmsg[0] = 18;
    _symmsg[3] = (handle > 0) ? _soundfx[handle-1] : 0;
    _symmsg[4] = pan;
    _symmsg[5] = priority;
    *((int*)(_symmsg + 6)) = pitch;
    _symmsg[8] = id;
    _symmsg[9] = volume;
    Sound_Command(0);
    _msemaoff();
}

void Effect_Stop(unsigned char handle, unsigned char id) {
    _msemaon();
    _symmsg[0] = 19;
    _symmsg[3] = (handle > 0) ? _soundfx[handle-1] : 0;
    _symmsg[8] = id;
    Sound_Command(0);
    _msemaoff();
}
