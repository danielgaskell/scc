#include <symbos.h>
#include "sound.h"

// Note: STARTUP.SPM is the SymbOS 4.0 startup sound as a separate file, from
// the Sound Daemon source code (licensed GPLv3).

unsigned char _soundpid;
unsigned char _soundhw;
unsigned char _soundpref;
unsigned char _sounderr;
unsigned char _soundmus = 255;
unsigned char _soundfx[SOUND_MAX_FX];
unsigned char _soundfxhw[SOUND_MAX_FX];

extern void* _exit_hooks[];
unsigned char sound_needs_hook = 1;

/* ========================================================================== */
/* SymbOS Sound Daemon calls (common)                                       */
/* ========================================================================== */

unsigned char Sound_Command(unsigned char wait) {
    unsigned char id = _symmsg[0] + 128;
    if (_soundpid == 0) {
        _sounderr = ERR_NOSOUND;
        return ERR_NOSOUND;
    }
    while (Msg_Send(_msgpid(), _soundpid, _symmsg) == 0);
    if (wait) {
        Msg_Wait(_msgpid(), _soundpid, _symmsg, id);
        if (_symmsg[2] & 0x01) {
            _sounderr = _symmsg[3];
            return _sounderr;
        } else {
            _sounderr = 0;
            return 0;
        }
    } else {
        return 0;
    }
}

void Music_Free(void) {
    if (_soundmus != 255) {
        _msemaon();
        _symmsg[0] = 9;
        _symmsg[3] = _soundmus;
        Sound_Command(0);
        _msemaoff();
        _soundmus = 255;
    }
}

void Effect_Free(unsigned char handle) {
    if (handle > 0) {
        _msemaon();
        _symmsg[0] = 17;
        _symmsg[3] = _soundfx[handle-1];
        Sound_Command(0);
        _msemaoff();
        _soundfx[handle-1] = 0;
    }
}

// cleanup routine to free sounds on exit
void _sound_cleanup(void) {
    unsigned char i;
    if (_soundmus != 255)
        Music_Free();
    for (i = 0; i < SOUND_MAX_FX; ++i) {
        if (_soundfx[i])
            Effect_Free(i+1);
    }
}

signed char Sound_Init(void) {
    unsigned char i;

    // save address of _sound_cleanup to exit hooks (avoids it being linked unless actually needed)
	if (sound_needs_hook) {
        for (i = 0; i < 8; ++i) {
            if (!_exit_hooks[i]) {
                _exit_hooks[i] = _sound_cleanup;
                sound_needs_hook = 0;
                break;
            }
        }
	}

	// run initialization
	_soundpid = (App_Search(_symbank, "Sound Daemon") >> 8);
    if (_soundpid) {
        _msemaon();
        _symmsg[0] = 1;
        _symmsg[3] = _soundpid;
        Sound_Command(1);
        _soundhw = _symmsg[3];
        _soundpref = _symmsg[8];
        _msemaoff();
        return 0;
    }
    _sounderr = ERR_NOSOUND;
    return -1;
}
