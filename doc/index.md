# Documentation contents

* [Introduction](#introduction) (this page)
* [Using the compiler](compiler.md)
* [SymbOS programming guide](symbos.md)
	* **Start here** for a quickstart guide to writing your first SymbOS app (both console apps to run in SymShell and windowed apps to run on the desktop).
* [System call reference](syscalls.md)
* [Control reference](s_ctrl.md)
* [Event reference](s_event.md)
* [Graphics library](graphics.md)
* [Network library](network.md)
* [Special considerations](special.md) (compiler quirks, etc.)
* [Porting tips/troubleshooting](porting.md)

# Introduction

SCC is an ANSI C compiler that produces executables for [SymbOS](https://symbos.org).

SCC is not the only way to write code for SymbOS; for a more Visual Basic-like experience with a GUI form editor and event-driven programming (albeit with sparse documentation and a lot of quirks), check out the [Quigs IDE](https://symbos.org/quigs.htm). There is also [extensive documentation and example code](https://symbos.org/download.htm) for writing SymbOS applications in pure Z80 assembler. But if you already know C and want to write SymbOS software, or you want to port existing C code to SymbOS, or you just want something more powerful than Quigs but less mind-numbing than assembly, read on.

(This documentation assumes you are familiar with standard C syntax, particularly structs, pointers, and typecasting, as well as the various weird ways you can combine them.)

## Why C?

Writing C will rot your brain. Only a true C programmer would look at the following code and think it was an "elegant" solution to anything:

```c
addr = ((_MemStruct)membank).ptr->addr & (ver > 4 ? 7 : attr);
*((char*)(++addr)) += 'A';
```

C is an awful language that is nonetheless ideal for many kinds of 8-bit programming, where we want to express ideas in a relatively high-level way while retaining the knowledge of where every byte is going and why. C is particularly good at dealing with byte-level structured data and pointers, which makes it a good fit for SymbOS. For example, SymbOS GUI controls are defined by exact blocks of bytes laid out in the correct order. These can be conveniently represented in C as structs, allowing us to reference the control's properties by name:

```c
typedef struct {
    char* text;               // address of text buffer
    unsigned short scroll;    // scroll position, in bytes
    unsigned short cursor;    // cursor position, in bytes
    signed short selection;   // number of selected characters relative to cursor
    unsigned short len;       // current text length, in bytes
    unsigned short maxlen;    // maximum text length, in bytes
    unsigned char flags;      // flags
    unsigned char textcolor;  // text color
    unsigned char linecolor;  // line color
} Ctrl_Input;
```

With this type of intrastructure in place, we can express complex ideas in just a few lines of code:

```c
// declare a 256-byte static buffer in the SymbOS "data" segment
_data char buffer[256];

// subroutine: convert the text in the specified GUI textbox (passed by reference)
// to a number, then AND it with every byte in buffer[].
void filter_buffer(Ctrl_Input* textbox) {
	int i = 0;
	char mask = atoi(textbox->text);
	while (i < sizeof(buffer)) {
		buffer[i] &= mask;
		i++;
	}
}
```

C is also a mature language with a vast amount of open-source code written in it, so there is ample material for porting or adapting and it is possible to find code examples for just about every algorithm imaginable.

## Features

* Runs natively on Windows, Linux, and SymbOS.
* Full build chain with preprocessor, object files, linker, etc.
* Standard ANSI C syntax, with good support for most typical usage.
* Standard data types and structures: `char`, `short`, `int`, `long`, `float`, `double`, `signed`, `unsigned`, `struct`, `union`, `enum`, `auto`, `static`, `register`, `extern`, `typedef`.
* A proper libc port so existing code can be compiled unmodified.
* Headers, typedefs, and support functions for most SymbOS system calls (`symbos.h`).
* Clean handling of SymbOS segments, with data, buffers, and literals located correctly in the executable without duplication (other compilers struggle with this). Keywords `_data` and `_transfer` allow specifying the segment of globals.
* Multithreading (yes, [really](s_task.md#multithreading)!)

## Limitations

* **Not all libc functions are available, well-tested, and/or correctly implemented yet**. The libc implementation is robust enough for SCC to compile itself, but you may encounter subtle incompatibilities and any code requiring less-common system functions should be tested carefully.
* Symbol matching only considers the first 15 characters (to conserve memory on native builds).
* No high-level optimizations like subexpression merging. (Just write efficiently.)
* The libc implementation is not very fast and favors portability over speed.
* For the usual Z80 reasons, 8-bit arithmetic (`char`) will always be much faster than 16-bit (`int`) and particularly floating-point (`float`, `double`) arithmetic. Declaring variables `unsigned` may also improve efficiency where applicable.
* `double` is currently treated as synonymous with `float` (32-bit IEEE 754 floating-point).
* The usual SymbOS limits apply, such as 64KB and 16KB limits on particular segments (see the [SymbOS programming guide](symbos.md)).
* No named static struct initializers like `{.x = 1, .y = 2}`---use `{1, 2}` instead and just be careful about member order.
* No K&R-style function declarations.
* Not all C99 and C11 additions are supported.
* `const` and `volatile` are accepted but don't actually do anything.
* Local variables are technically scoped to their function, not their block.
* If you run into type-mismatch problems with unusual pointer data types (like struct members that are typed as pointers to another typedef'd struct---this can happen when working with complex data structures for windows), try `void*` instead.
