# SymbOS C Compiler

A C compiler for [SymbOS](http://symbos.org) based on Alan Cox's Fuzix Compiler Kit
(Z80 version). Currently it runs on Windows and cross-compiles binaries for SymbOS.
(Eventually it should be possible to cross-compile itself to run natively on SymbOS.)

[Documentation and quickstart guide](doc/index.md)

SCC features:

* Full ANSI C compiler
* A proper libc port for plug-and-play compilation of existing code
* Extensive wrappers and utility functions for SymbOS system calls
* Extensive [documentation](doc/index.md) to help you get started with SymbOS programming
* Multithreading (yes, [really](doc/syscalls.md#multithreading)!)
* A complete sample application (programmer's calculator) for reference

SCC is considered stable and can be used to write both console and graphical
applications without issue, but is still under active development, so you may
encounter bugs or incompletely-implemented advanced features. In particular:

* `float` support is currently flaky (and scheduled for a complete rewrite)
* Many libc functions are under-tested.

Please report bugs on the Issues tab---although of course make sure that the bug
is actually in the compiler, not in your own code, and include code to reproduce it.

# License

Compiler is GPLv3. Libraries are LGPL except where indicated. copt is from Z88DK under
the Clarified Artistic License. cpp is from MCPP under the BSD license. Sample code and
included documentation is public domain.

