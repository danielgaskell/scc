# Special considerations

## The `malloc()` heap

## Quirks of `stdio.h`

Due to a limitation of the filesystem, files stored on AMSDOS filesystems (e.g., CPC floppy disks) will sometimes be terminated with an EOF character 0x1A and then some garbage padding ([see above](#file-access)). To improve compatibility, `stdio.h` functions treat character 0x1A as EOF. If we need to read a binary file that includes legitimate 0x1A characters, the file should be opened in binary (`b`) mode, e.g.:

```c
f = fopen("data.dat", "rb");
```

...with the tradeoff being that we now need to pay attention to the fact that there may be garbage data at the end of the file. (This problem does not apply to the FAT filesystems used by most mass storage devices.)

## Building SCC

The current primary build target for SCC is Windows. Install MinGW, ensure that its `bin` folder is in the system path, and then run the `make.bat` batch files found throughout the SCC source tree to compile the relevant parts of the codebase. (This really ought to transition to proper Makefiles, but whatever.)
