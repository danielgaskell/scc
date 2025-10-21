# Kernel routines

In addition to `symbos.h`, these functions can be found in `symbos/core.h`.

## Contents

* [Messaging](#messaging)
* [Memory management](#memory-management)
* [Memory read/write](#memory-readwrite)
* [System status](#system-status)

## Messaging

### Msg_Send()

```c
unsigned char Msg_Send(char rec_pid, char send_pid, char* msg);
```

Sends the message in `*msg` to process ID `send_pid`. `rec_pid` is the process ID that should receive the response, if any; usually this should be our own process ID (`_sympid`). `*msg` must be in the **transfer** segment.

*Return value*: 0 = message queue is full; 1 = message sent successfully; 2 = receiver process does not exist.

*SymbOS name*: `Message_Send` (`MSGSND`).

### Msg_Receive()

```c
unsigned short Msg_Receive(char rec_pid, char send_pid, char* msg);
```

Checks for a message sent from process ID `send_pid` to process ID `rec_pid` and, if one is waiting, stores it in `*msg`. Usually `rec_pid` should be our own process ID (`_sympid`). If `send_pid` is -1, checks for messages from any process. `*msg` must be in the **transfer** segment.

*Return value*: Low byte: 0 = no message available, 1 = message received. High byte: sender process ID. Extract with, e.g.,

```c
rec = result & 0xFF;
pid = result >> 8;
```

*SymbOS name*: `Message_Receive` (`MSGGET`).

### Msg_Sleep()

```c
unsigned short Msg_Sleep(char rec_pid, char send_pid, char* msg);
```

Checks for a message sent from process ID `send_pid` to process ID `rec_pid`. If one is waiting, stores it in `*msg`. If there is no message, returns CPU time to SymbOS and waits until a message is available or the process is woken up for another reason. Usually `rec_pid` should be our own process ID (`_sympid`). If `send_pid` is -1, checks for messages from any process. `*msg` must be in the **transfer** segment.


*Return value*: Low byte: 0 = no message available, 1 = message received. High byte: sender process ID. Extract with, e.g.,

```c
rec = result & 0xFF;
pid = result >> 8;
```

Note that processes can be "woken up" for multiple reasons, so returning from `Msg_Sleep()` does not necessarily mean that the desired message has been received. We must check the return value or the contents of `*msg` to be sure. For example, to loop until a message is actually received:

```c
while (!(Msg_Sleep(_sympid, -1, _symmsg) & 0x01));
```

*SymbOS name*: `Message_Sleep_And_Receive` (`MSGSLP`).

### Msg_Wait()

*Currently only available in development builds of SCC.*

```c
unsigned char Msg_Wait(char rec_pid, char send_pid, char* msg, char id);
```

A utility function that idles on `Msg_Sleep()` until a message from process ID `send_pid` with the first byte `msg[0]` = `id` arrives. Any other messages received in the meantime will remain on the queue. (This is useful for command/response pairs where we want to send a message to a service and wait for its response.)

*Return value*: sender process ID.

### Idle()

```c
void Idle(void);
```

Finishes the current multitasking timeslot early, returning any remaining CPU time to the kernel so other processes can use it. When the process gets its next multitasking timeslot, execution will continue from this point.

It is recommended to use `Idle()` whenever the program is in a loop waiting for something external to happen. This way, the program will only check once per multitasking timeslot and give back the rest of its CPU time to other programs:

```c
while (Mouse_X() < 125) {
    Idle();
}
```

(Note that this "Idle" state is different from the "Sleep" state triggered by `Msg_Sleep()`, which only ends when the process receives a new message or is otherwise woken up by another process.)

*SymbOS name*: `Multitasking_SoftInterrupt` (`RST #30`).

## Memory management

Applications are able to address more that 64KB of memory by reserving additional blocks of banked memory. These blocks cannot be addressed directly using C pointers and variables, but we can read/write/copy data to them using system functions.

In addition to `symbos.h`, these functions can be found in `symbos/memory.h`.

### Mem_Reserve()

```c
unsigned char Mem_Reserve(unsigned char bank, unsigned char type, unsigned short len,
                          unsigned char* bankVar, char** addrVar);
```

Reserve a block of banked memory in bank `bank` of length `len`, in bytes. `bank` may be from 0 to 15; 0 means "any bank can be used." `type` may be one of: 0 = located anywhere; 1 = reserve within a 16KB address block (like the **data** segment); 2 = reserve within the last 16KB address block (like the **transfer** segment).

Two variables must be passed by reference to store the address of the resulting block of banked memory: `bankVar` (type `unsigned char`), which stores the bank, and `addrVar` (type `char*`), which stores the address.

**Note that, to avoid memory leaks, memory reserved with `Mem_Reserve()` ***must*** be manually released with `Mem_Release()` before program exit!** SymbOS does not have the resources to track this automatically; it is up to us.

*Return value*: 0 = success, 1 = out of memory.

*SymbOS name*: `Memory_Get` (`MEMGET`).

### Mem_Release()

```c
void Mem_Release(unsigned char bank, char* addr, unsigned short len);
```

Releases a block of banked memory previously reserved with `Mem_Reserve()`. `bank` is the bank of the reserved memory, which must be from 1 to 15; `addr` is the address; and `len` is the length of the reserved block, in bytes.

**Be careful to ensure that `bank`, `addr`, and `len` exactly match a contiguous block of memory that was previously reserved with `Mem_Reserve()`!** SymbOS does not keep track of this independently, so we can corrupt memory if we pass invalid information.

*SymbOS name*: `Memory_Free` (`MEMFRE`).

### Mem_Resize()

```c
unsigned char Mem_ResizeX(unsigned char bank, unsigned char type, char* addr,
                          unsigned short oldlen, unsigned short newlen,
                          unsigned char* bankVar, char** addrVar);
```

Attempts to resize a block of banked memory previously reserved with `Mem_Reserve()`. (This is accomplished manually by reserving a new block of the desired size, copying the old block to the new block, and releasing the old block.) `bank` is the bank of the reserved memory, which must be from 1 to 15; `addr` is the address; `oldlen` is the previous length of the reserved block, in bytes; and `newlen` is the requested new length, in bytes.

Two variables must be passed by reference to store the address of the resulting block: `bankVar` (type `unsigned char`), which stores the bank, and `addrVar` (type `char*`), which stores the address. Note that the new location of the block may be in any bank, not just the same bank as the previous block.

*Return value*: 0 = success, 1 = out of memory.

### Mem_Longest()

```c
unsigned short Mem_Longest(unsigned char bank, unsigned char type);
```

Returns (in bytes) the longest area of contiguous memory within bank `bank` that could be reserved with `Mem_Reserve()`. `bank` may be from 0 to 15; 0 means "any bank can be used." `type` may be one of: 0 = located anywhere; 1 = reserve within a 16KB address block (like the **data** segment); 2 = reserve within the last 16KB address block (like the **transfer** segment).

*SymbOS name*: `Memory_Information` (`MEMINF`).

### Mem_Free()

```c
unsigned long Mem_Free(void);
```

Returns the total amount of free memory, in bytes.

*SymbOS name*: `Memory_Summary` (`MEMSUM`).

### Mem_Banks()

```c
unsigned char Mem_Banks(void);
```

Returns the total number of existing 64KB extended RAM banks.

*SymbOS name*: `Memory_Summary` (`MEMSUM`).

## Memory read/write

In addition to `symbos.h`, these functions can be found in `symbos/memory.h`.

### Bank_ReadWord()

```c
unsigned short Bank_ReadWord(unsigned char bank, char* addr);
```

Returns the two-byte word at bank `bank`, address `addr`. `bank` must be from 1 to 15.

*SymbOS name*: `Banking_ReadWord` (`BNKRWD`).

### Bank_WriteWord()

```c
void Bank_WriteWord(unsigned char bank, char* addr, unsigned short val);
```

Writes the two-byte word `val` to memory at bank `bank`, address `addr`. `bank` must be from 1 to 15.

*SymbOS name*: `Banking_WriteWord` (`BNKWWD`).

### Bank_ReadByte()

```c
unsigned char Bank_ReadByte(unsigned char bank, char* addr);
```

Returns the byte at bank `bank`, address `addr`. `bank` must be from 1 to 15.

*SymbOS name*: `Banking_ReadByte` (`BNKRBT`).

### Bank_WriteByte()

```c
void Bank_WriteByte(unsigned char bank, char* addr, unsigned char val);
```

Writes the byte `val` to memory at bank `bank`, address `addr`. `bank` must be from 1 to 15.

*SymbOS name*: `Banking_WriteByte` (`BNKWBT`).

### Bank_Copy()

```c
void Bank_Copy(unsigned char bankDst, char* addrDst,
               unsigned char bankSrc, char* addrSrc, unsigned short len);
```

Copies `len` bytes of memory from bank `bankSrc`, address `addrSrc` to bank `bankDst`, address `addrDst`.

*SymbOS name*: `Banking_Copy` (`BNKCOP`).

### Bank_Get()

```c
unsigned char Bank_Get(void);
```

Returns the bank number in which the app's main process is running. (Normally it is easier to use the `_symbank` global for this purpose.)

*SymbOS name*: `Banking_GetBank` (`BNKGET`).

### Bank_Decompress()

```c
void Bank_Decompress(unsigned char bank, char* addrDst, char* addrSrc);
```

Decompresses the compressed data block located at bank `bank`, address `addrSrc` into memory at bank `bank`, address `addrDst`. The addresses must be arranged such that the last address of the decompressed data will be the same as the last address of the original compressed data. That is, we need to know the length of the uncompressed data ahead of time, and load the compressed data into the end of this buffer, with `addrSrc` = `addrDest` + *(uncompressed length)* - *(compressed length)*. The data will then be decompressed "in place" to fill the buffer completely from start to finish.

The structure of a compressed data block is as follows:

* 2 bytes (`unsigned short`): length of the block, minus these two bytes
* 4 bytes: the last four bytes of the data (uncompressed)
* 2 bytes (`unsigned short`): the number of uncompressed bytes before the compressed data begins (e.g., for metadata; usually 0)
* (some amount of uncompressed data, or nothing)
* (some amount of data compressed using the [ZX0 algorithm](https://github.com/einar-saukas/ZX0), minus the last four bytes given above)

**This function is only available in SymbOS 4.0 and higher.**

*SymbOS name*: `Banking_Decompress` (`BNKCPR`).

## System status

### Sys_Counter()

```c
unsigned long Sys_Counter(void);
```

Returns the system counter, which increment by two 50 times per second (so every second, the timer increases by 100). This can be used to calculate time elapsed for regulating framerates in games, etc.

*SymbOS name*: `Multitasking_GetCounter` (`MTGCNT`).

### Sys_Counter16()

```c
unsigned short Sys_Counter16(void);
```

Equivalent to `Sys_Counter()`, but returns only the low 16 bits of the system counter. (This wraps around every 11 minutes, but can be useful when [multithreading](s_task.md#multithreading) because **long** is not thread-safe.)

*SymbOS name*: `Multitasking_GetCounter` (`MTGCNT`).

### Sys_IdleCount()

```c
unsigned short Sys_IdleCount(void);
```

Returns the idle process counter, which increments every 64 microseconds. This can be used to calculate CPU usage.

*SymbOS name*: `Multitasking_GetCounter` (`MTGCNT`).
