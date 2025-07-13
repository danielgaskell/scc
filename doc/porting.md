# Porting tips

## Contents

* [Common problems](#common-problems)

## Common problems

### "Unknown symbol" errors on compilation

This occurs when the code references a function that is not actually defined. (Because this error originates at the linking stage, an underscore will be added to the beginning of the listed name---i.e., `_funcname` for `funcname()`---and no line number will be given in the error.) If the function is a standard library function that should exist, there are two possibilities:

1. SCC may not (yet) provide the function. This is especially likely for platform-specific functions like POSIX system calls (`fork()`, `chmod()`, etc.)
2. The function may be in an external library that must be manually linked with `-l` command-line options. Notably, following C convention, most `math.h` functions (`sqrt()`, etc.) are in the external library `libm.a`. To use these functions, `cc` must be run with the option `-lm`.

### "Out of memory" error when starting the compiled app

This error is often confusing because it can occur even when there is apparently still memory available. It is important to remember that SymbOS does not work with a single large address space like modern systems, but must arrange memory in a more constrained way (see the [section on memory segments](symbos.md#memory-segments)). So, what this error really means is that SymbOS cannot allocate *the exact arrangement of memory segments requested by the app*, not necessarily that it is out of memory entirely.

The two common causes of this error are:

1. The app does not obey SymbOS's [segment size restrictions](symbos.md#memory-segments). (Usually the compiler will warn about this.) The code might simply be too large to fit in 64KB, but it's also common for code written for modern systems to declare buffers that are much larger than actually needed, because memory is considered cheap. Look through the code to see if anything can be slimmed down. (Tip: To see how much space different parts of the code are taking up, run `cc` with the option `-M mapfile.txt`. This will create a "map file" called `mapfile.txt` showing the relative addresses of different symbols in the executable. By running this file through the included command-line utility `sortmap.exe`, it is possible to see how much space each object occupies.)
2. The executable file was incompletely written or corrupted somehow. Because SymbOS reads memory-allocation information from the executable's header, if this header is corrupted, it will often trigger a misleading "out of memory" error instead of something more specific. Verify that all steps of the compilation are finishing correctly and without errors, and that the file is not being modified in the process of transferring it to the computer or emulator where it is being run.

(For out-of-memory errors raised by the app itself during normal operation, see [the section on `malloc()` under Special Considerations](special.md#the-malloc-heap).)

### Problems with `malloc()`

See [the section on `malloc()` under Special Considerations](special.md#the-malloc-heap). A lot of modern code implicitly assumes that it can `malloc()` as much memory as it needs; indeed, many smaller (or lazier) programs won't even bother to check for out-of-memory on `malloc()`, since who ever heard of getting an out-of-memory error for allocating a measely few megabytes? SymbOS, that's who!

### `printf()` formatting looks wrong

Note that, for reasons of space and performance, SCC's `printf()` implementation does not properly handle every advanced feature available on other platforms. More complex formatting may need to be simplified or rewritten. (True ANSI C `printf()` is a notoriously huge function, often requiring 20KB or more of space to implement!)

### Can't `fopen()` a file that should be openable

A few possibilities:

1. The file path may be wrong in a nonobvious way. Note that, to save memory, SymbOS does not keep track of an app's "current path" in the same way most modern systems do, so SCC's libc implementation generally assumes that the "current path" is the location of the executable (for GUI apps) or the current SymShell path (for console apps). If this is not what the app itself expects, problems may arise.
2. The file name may be wrong in a nonobvious way. SymbOS only supports files with MS-DOS 8.3-style names, whereas a lot of existing C code assumes files can have arbitrarily long names; these may need to be modified to fit the 8.3-style format.
3. Too many files may be open at once. SymbOS has a hard limit of 8 concurrent filehandles, shared across all open applications.
4. If the app uses `malloc()`, it may be running out of heap space to store the file handle (see [the section on `malloc()` under Special Considerations](special.md#the-malloc-heap)).

### `fread()` etc. won't read the end of a file (or reads extra junk at the end of a file)

This usually relates to a known limitation of AMSDOS and CP/M-format filesystems. See [the section on file sizes under Special Considerations](special.md#file-sizes) for details and workarounds.
