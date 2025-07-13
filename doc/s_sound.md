# Sound

Sound capabilities are only available if an appropriate sound daemon is running (in SymbOS 4.0 or later).

SymbOS supports two incompatible sound chips, PSG and OPL4. PSG refers to the standard 3-voice "chiptune" Programmable Sound Generator (e.g., AY-3-8912) integrated into most classic platforms SymbOS can run on. The OPL4 is a more powerful "wavetable" chip that supports recorded audio playback and is usually only available in expansion cards.

The SymbOS sound daemon makes a distinction between "music" and "sound effects." In general, only one music track can be playing at a time, but multiple shorter sound effects may be triggered and mixed together on top of it as it plays. Music and effects are loaded as part of "collections," which contain multiple subsongs or effects that can be referred to by numerical ID.

Sound errors are recorded in the global variable `_sounderr`, documented [here](s_ref.md#error-codes). An example sound application (`snddemo.c`) is included in SCC's `sample` folder.

In addition to `symbos.h`, these functions can be found in `symbos/sound.h`.

## Contents

* [Creating/getting sounds](#creatinggetting-sounds)
* [Sound functions](#sound-functions)

## Creating/getting sounds

For PSG, music/effect collections consist of packaged and compressed Arkos Tracker II .AKG/.AKS files (for music) or .AKX files (for sound effects). For OPL4, music/effect collections consist of packaged and compressed Amiga MOD files (for music) or PCM WAV files (for sound effects). Tools for packaging raw files into collections are described [here](https://github.com/Prodatron/symbos-wiki/wiki/SoundD-%E2%80%90-creating-sound-files).

An easy way to add sound effects without creating your own collection is to use the default system effects collection. This is always loaded and can be accessed using the resource handle 0. The effects in this collection have standard names and are intended for use by programs: `FX_CLICK1`, `FX_CLICK2`, `FX_BEEP1`, `FX_BEEP2`, `FX_RING1`, `FX_RING2`, `FX_ALERT1`, `FX_ALERT2`, `FX_SLIDE1`, `FX_SLIDE2`, `FX_RAISE`, `FX_LOWER`, `FX_POPUP`, `FX_SHRINK`, `FX_TIC1`, `FX_TIC2`, `FX_SHOOT`, `FX_EXPLODE`, `FX_STEP`, `FX_LOSE`, `FX_WIN`, `FX_CAR`, and `FX_PLANE`. Note that the user can theoretically change what collection is used for the system sounds, so there is no guarantee of exactly what a given effect will sound like on every computer. However, the system effects should be stable enough that (e.g.) `FX_BEEP1` can always be expected to be some kind of beep.

## Sound functions

### Sound_Init()

```c
signed char Sound_Init(void);
```

Initializes the sound interface, if present. This should be called before using any other sound functions.

After initialization, the process ID of the sound daemon will be stored in `_soundpid`, and the available audio hardware will be stored in `_soundhw` as an OR'd bitmask:

* `_soundhw & SOUND_PSG` will be nonzero if the user has a PSG-compatible sound chip.
* `_soundhw & SOUND_OPL4` will be nonzero if the user has an OPL4-compatible sound chip.

The preferred audio hardware will be stored in `_soundpref` (one of `SOUND_NONE`, `SOUND_PSG`, or `SOUND_OPL4`).

*Return value*: On success, sets the variables above and returns 0. On failure, sets `_sounderr` and returns -1.

### Music_Load()

```c
signed char Music_Load(unsigned char fid, unsigned char hw);
```

Loads a music collection from the open file handle `fid` into memory, for device `hw` (one of `SOUND_PSG` or `SOUND_OPL4`). Only one music collection can be loaded at a time, so if the program has already loaded a collection, it will first be freed with `Music_Free()`.

Note that this function takes a file *handle*, not a file *path*. That is, the file must first be opened with `File_Open()`, then read with `Music_Load()`, and then closed with `File_Close()`. (The idea is that multiple sound assets might be stored in the same file, so it's up to the program to decide how to manage this file; `Music_Load()` will just start reading from the file's current seek position.)

*Return value*: On success, returns 0. On failure, sets `_sounderr` and returns -1.

### Music_Load_Mem()

```c
signed char Music_Load_Mem(unsigned char bank, char* addr, unsigned short len);
```

Loads a music collection from the memory location at bank `bank`, address `addr`, where the music data is `len` bytes long. (Only PSG music can be loaded in this way.) Only one music collection can be loaded at a time, so if the program has already loaded a collection, it will first be freed with `Music_Free()`.

*Return value*: On success, returns 0. On failure, sets `_sounderr` and returns -1.

### Music_Free()

```c
void Music_Free(void);
```

Unloads the currently loaded music collection, if any. (This is also done automatically on program exit.)

### Music_Start()

```c
void Music_Start(unsigned char track);
```

Starts playing (from the beginning) track number `track` from the currently loaded music collection.

### Music_Stop()

```c
void Music_Stop(void);
```

Pauses and mutes the currently playing track from the currently loaded music collection.

### Music_Continue()

```c
void Music_Continue(void);
```

Resumes playing the last-played music track from the currently loaded music collection.

### Music_Volume()

```c
void Music_Volume(unsigned char vol);
```

Sets the playback volume for music, from 0 (silent) to 255 (loud). Note that volume is reset automatically to 255 (loud) after loading a music collection.

### Effect_Load()

```c
signed char Effect_Load(unsigned char fid, unsigned char hw);
```

Loads an effect collection from the open file handle `fid` into memory, for device `hw` (one of `SOUND_PSG` or `SOUND_OPL4`).

Note that this function takes a file *handle*, not a file *path*. That is, the file must first be opened with `File_Open()`, then read with `Effect_Load()`, and then closed with `File_Close()`. (The idea is that multiple sound assets might be stored in the same file, so it's up to the program to decide how to manage this file; `Effect_Load()` will just start reading from the file's current seek position.)

Up to 16 effect collections may be loaded simultaneously, distinguished by their resource handles.

*Return value*: On success, returns the resource handle of the effect collection (1-16). On failure, sets `_sounderr` and returns -1.

### Effect_Load_Mem()

```c
signed char Effect_Load_Mem(unsigned char bank, char* addr, unsigned short len);
```

Loads an effect collection from the memory location at bank `bank`, address `addr`, where the effect data is `len` bytes long. (Only PSG effects can be loaded in this way.)

Up to 16 effect collections may be loaded simultaneously, distinguished by their resource handles.

*Return value*: On success, returns the resource handle of the effect collection (1-16). On failure, sets `_sounderr` and returns -1.

### Effect_Free()

```c
void Effect_Free(unsigned char handle);
```

Unloads the effect collection with the resource handle `handle`. (This is also done automatically on program exit.)

### Effect_Play()

```c
void Effect_Play(unsigned char handle, unsigned char id, unsigned char volume,
                 unsigned char priority, unsigned char pan, int pitch) ;
```

Starts playing effect number `id` from the loaded effect collection with the resource handle `handle`. `volume` goes from 0 (silent) to 255 (loud).

`priority` specifies how the effect should be played (use 0 for the defaults specified below). For PSG effect collections, the available priorities are:

* `FX_ANY` - force playing on any channel (default)
* `FX_FORCE` - force playing on the specified channel
* `FX_OPTIONAL` - play only if a channel is free
* `FX_OPTCH` - play only if the specified channel is free
* `FX_ONLY` - play only if no other effect is active

For OPL4 effect collections, the available priorities are:

* `FX_PLAY` - play (default)
* `FX_SINGLE` - first stop any other instance of the same effect
* `FX_SOLO` - first stop any other effects from the same collection

`pan` can be one of `PAN_LEFT`, `PAN_MIDDLE`, or `PAN_RIGHT`. For OPL4 effect collections, `pan` can also be specified as a number from 0 (left) to 255 (right).

`pitch` is only meaningful for OPL4 effect collections and can be used to change the effect pitch. 0 = standard pitch.

### Effect_Stop()

```c
void Effect_Stop(unsigned char handle, unsigned char id);
```

Stops playing all instances of effect number `id` from the loaded effect collection with the resource handle `handle`.
