# Multitasking routines

In addition to `symbos.h`, these functions can be found in `symbos/proc.h`.

## Processes

SymbOS makes a distinction between an *application ID*, which is associated with a single application, and a *process ID*, which is associated with a single process. An application may potentially open multiple processes, although most do not. *Pay close attention to which is required by a given function!*

### App_Run()

```c
unsigned short App_Run(char bank, char* path, char suppress);
```

Runs the app whose absolute path is at bank `bank`, address `path`. If `suppress` = 1, any errors that occur during loading will be supressed; otherwise, they will appear in a message box. If the path points to a non-executable file of a known type (such as `.txt`), it will be opened with its associated application, exactly like double-clicking it in SymCommander. Paths starting with `%` will be understood as starting with the system path; e.g., if the system path were `C:\SYMBOS\`, the path `%CMD.EXE` would run `C:\SYMBOS\CMD.EXE`.

*Return value*: On success, returns the application ID as the low byte and the process ID of the application's main process as the high byte. These can be extracted with:

```c
appid = result & 0xFF;
procid = result >> 8;
```

The process ID will always be greater than 3, so if the return value is greater than 3, the app was opened successfully.

On failure, returns an error code:

* `APPERR_NOFILE`: the file does not exist.
* `APPERR_UNKNOWN`: the file is not executable or of a known type.
* `APPERR_LOAD`: file system error (sets `_fileerr` with error code).
* `APPERR_NOMEM`: out of memory.

*SymbOS name*: `Program_Run_Command` (`MSC_SYS_PRGRUN`).

### App_End()

```c
void App_End(char appID);
```

Forcibly closes the app with the application ID `appID`. SymbOS will close any processes, windows, and other system resources registered to the app, but since it is possible for apps to reserve some resources (particularly file handles and banked memory) without explicitly registering them to the app, memory leaks may occur. (Treat this command like force-killing an application in the Task Manager: safe if we control the app and know exactly what it is doing, but a last-resort otherwise.)

*SymbOS name*: `Program_End_Command` (`MSC_SYS_PRGEND`).

### App_Search()

```c
unsigned short App_Search(char bank, char* idstring);
```

Searches for the running application with the first 12 bytes of the internal application name matching the string at bank `bank`, address `idstring`. (This is the application name listed in the Task Manager and which can be set with the `-N` command-line option to `cc`.) This function is mainly used for determining the process IDs of shared services such as network drivers so we can send messages to them.

*Return value*: If a matching app has been found, returns the application ID as the low byte and the process ID of the application's main process as the high byte. These can be extracted with:

```c
appid = result & 0xFF;
procid = result >> 8;
```

If no matching app can be found, returns 0.

*SymbOS name*: `Program_SharedService_Command` (`MSC_SYS_PRGSRV`).

### App_Service()

```c
unsigned short App_Service(char bank, char* idstring);
```

Similar to `App_Search()`, but used to connect with system shared services. The behavior is identical to `App_Search`, with two differences:

1) If the specified app is not currently running, SymbOS will attempt to start it from the system path. The `idstring` must have the exact format `%NNNNNNNN.EE` (`%`, eight ASCII characters for the filename, `.`, and two ASCII characters for the first part of the extension). The last character will be filled in automatically based on the current system type:

* `C` for Amstrad CPC
* `P` for Amstrad PCW
* `M` for MSX

For example, if the system path were `C:\SYMBOS` and the system were an Amstrad CPC, `App_Service(_symbank, "%GAMEDRVR.EX")` would try to load `C:\SYMBOS\GAMEDRVR.EXC`.

2) After the app is successfully located or started, an internal counter will be incremented that indicates how many apps are using the service. This allows SymbOS to automatically decide whether the service is still being used and should remain open. (We can unregister with the service with `App_Release()`.)

*Return value*: If the application could be found or was successfully loaded, returns the application ID as the low byte and the process ID of the application's main process as the high byte. These can be extracted with:

```c
appid = result & 0xFF;
procid = result >> 8;
```

The process ID will always be greater than 3, so if the return value is greater than 3, the app was found or opened successfully.

If the application could not be found or was not successfully loaded, returns an error code:

* `APPERR_NOFILE`: the file does not exist.
* `APPERR_UNKNOWN`: the file is not executable or of a known type.
* `APPERR_LOAD`: file system error (sets `_fileerr` with error code).
* `APPERR_NOMEM`: out of memory.

*SymbOS name*: `Program_SharedService_Command` (`MSC_SYS_PRGSRV`).

### App_Release()

```c
void App_Release(char appID);
```

Decrements the service counter incremented by `App_Service()`, so SymbOS knows when the shared service is no longer being used and can be closed. The parameter `appID` specifies the application ID of the service app to release.

*SymbOS name*: `Program_SharedService_Command` (`MSC_SYS_PRGSRV`).

### Proc_Add()

```c
signed char Proc_Add(unsigned char bank, void* header, unsigned char priority);
```

Launches a new process based on the information given in bank `bank`, address `header`. The process will be started with the priority `priority`, from 1 (highest) to 7 (lowest). The standard priority for application is 4.

Usually if we just want to run an executable file from disk, we should use [`App_Run()`](#app_run), not `Proc_Add()`. This function is for a lower-level operation, starting a new process running code we have already defined in memory. This may be code we have loaded from a file, or even part of our application's own main code; see [`thread_start()`](#multithreading) for a wrapper function that uses this to implement multithreading.

`header` must point to a data structure in the **transfer** segment with the struct type `ProcHeader`:

```c
typedef struct {
	unsigned short ix;  // initial value of IX register pair
	unsigned short iy;  // initial value of IY register pair
	unsigned short hl;  // initial value of HL register pair
	unsigned short de;  // initial value of DE register pair
	unsigned short bc;  // initial value of BC register pair
	unsigned short af;  // initial value of AF register pair
	void* startAddr;    // address of routine to run
	unsigned char pid;  // process ID, set by kernel
} ProcHeader;
```

In addition, when the process is launched, its internal stack will begin at the address immediately before this header and grow downwards. So, we must define space for the stack immediately before the header in the **transfer** segment. `startAddr` can be an absolute address, or (as in the example below) the address of a `void` function in our own main code to run as a separate thread. Due to a quirk in SCC's linker, which currently treats initialized and uninitialized arrays differently, this buffer must be given an initial value (such as `{0}`) to ensure that it is placed directly before the `ProcHeader` data structure in the **transfer** segment. (This may be improved in future releases.) For example:

```c
char subprocID;

void proccode(void) {
	/* ... process code, do something here ... */
	Proc_Delete(subprocID); // end the subprocess (rather than returning)
}

_transfer char procstack[256] = {0};
_transfer ProcHeader prochead = {0, 0, 0, 0, 0, 0, // initial register values
                                 proccode};        // address of routine to run
								 
int main(int argc, char* argv[]) {
	subprocID = Proc_Add(_symbank, &prochead, 4);
	/* ... */
}
```

(See [`Multithreading`](#multithreading) for a simpler way to implement threads within our own code, as well as some important discussion about behaviors to avoid when doing this.)

*Return value*: On success, returns the process ID of the newly launched process. On failure, returns -1.

*SymbOS name*: `Multitasking_Add_Process_Command` (`MSC_KRL_MTADDP`).

### Proc_Delete()

```c
void Proc_Delete(unsigned char pid);
```

Forcibly stops execution of the process with the process ID `pid`. (This is most useful for processes we have launched ourselves with `Proc_Add()`; for stopping an entire application and freeing its resources, see [`App_End()`](#app_end).)

*SymbOS name*: `Multitasking_Delete_Process_Command` (`MSC_KRL_MTDELP`).

### Proc_Sleep()

```c
void Proc_Sleep(unsigned char pid);
```

Forcibly puts the process with the process ID `pid` into "sleep" mode. It will not continue execution until it receives a message or is woken up by another system function (such as `Proc_Wake()`).

*SymbOS name*: `Multitasking_Sleep_Process_Command` (`MSC_KRL_MTSLPP`).

### Proc_Wake()

```c
void Proc_Wake(unsigned char pid);
```

Wakes up the process with the process ID `pid` from "sleep" mode.

*SymbOS name*: `Multitasking_Sleep_Process_Command` (`MSC_KRL_MTSLPP`).

### Proc_Priority()

```c
void Proc_Priority(unsigned char pid, unsigned char priority);
```

Sets the scheduler priority of the process with the process ID `pid` to `priority`, which may be from 1 (highest) to 7 (lowest). The default priority is usually 4. A process is allowed to change its own priority.

*SymbOS name*: `Multitasking_Sleep_Process_Command` (`MSC_KRL_MTSLPP`).

## Timers

SCC implements three types of clock events that fire at regular intervals: **counters**, **timers**, and **wake timers**. These can be used in cases where we need something to occur at regular intervals of real time (such as movement in a game), rather than at regular intervals of CPU time.

A **counter** is the simplest method. A byte in memory can be registered as a counter, after which it will be incremented automatically from 1 to 50 times per second. We can then check the value of this byte regularly, and if it has changed, perform whatever action needs to be done.

A **timer** is more complicated. It is essentially a new process that is only given CPU time for brief, regular intervals every 1/50th or 1/60th of a second (depending on the screen vsync frequency of the platform). See `Timer_Add()` and `Proc_Add()` for details on how to define this process.

A **wake timer** is a simplified wrapper that uses a **timer** to perform a single task: at a regular interval, it sends a [message](s_core.md#messaging) to the specified process ID. The idea is that, instead of continuously running at 100% CPU checking a **counter**, the target process can sit idling on `Msg_Sleep()` and only "wake up" every so often when the wake timer fires.

### Counter_Add()

```c
unsigned char Counter_Add(unsigned char bank, char* addr, unsigned char pid,
                          unsigned char speed);
```

Registers a new **counter** byte at bank `bank`, address `addr`, incrementing every `speed`/50 seconds. (For example, to increment the counter twice per second, set `speed` = 25, because 25/50 = 0.5 seconds) The process ID `pid` is the process to register this counter with (usually our own, `_sympid`).

*Return value*: On success, returns 0. On failure, returns 1.

*SymbOS name*: `Timer_Add_Counter_Command` (`MSC_KRL_TMADDT`).

### Counter_Delete()

```c
void Counter_Delete(unsigned char bank, char* addr);
```

Unregisters the **counter** byte at bank `bank`, address `addr` so it no longer increments automatically.

*SymbOS name*: `Timer_Delete_Counter_Command` (`MSC_KRL_TMDELT`).

### Counter_Clear()

```c
void Counter_Clear(unsigned char pid);
```

Unregisters all **counter** bytes associated with the process ID *pid*.

*SymbOS name*: `Timer_Delete_AllProcessCounters_Command` (`MSC_KRL_TMDELP`).

### Timer_Add()

```c
signed char Timer_Add(unsigned char bank, void* header);
```

Behaves identically to `Proc_Add()`, but launches the new process as a **timer**. (See [`Proc_Add()`](#proc_add) for details on how a new process is implemented.) The timer code should ideally be short to ensure that it can fully complete in the allotted time, even under higher CPU load. Typically, we want to implement the timer code as a short loop that ends by calling `Idle()`; the timer process will then execute the loop contents, finish, go to sleep, and be woken up 1/50th of a second later for another pass through the loop:

```c
void timer_loop(void) {
	while (1) {
		/* ... do something here ... */
		Idle();
	}
}
```

*Return value*: On success, returns the timer ID needed to stop the timer later with `Timer_Delete()`. On failure, returns -1.

*SymbOS name*: `Multitasking_Add_Timer_Command` (`MSC_KRL_MTADDT`).

### Timer_Wake()

*Currently only available in development builds of SCC.*

```c
signed char Timer_Wake(unsigned char pid, unsigned char msgid, unsigned short cycles);
```

A simplified wrapper around `Timer_Add()` that sets up a "wake timer". Every `cycles` iterations (in 1/50ths of a second), the timer process will send a [message](s_core.md#messaging) with the first byte `msg[0]` = `msgpid` to the process ID `pid`. For example, to wake our main process up every 5 seconds with `msg[0]` = 200:

```c
signed char wake_pid;
wake_pid = Timer_Wake(_sympid, 200, 5*50);
```

This function only supports running one wake timer at a time; for more complex behavior, see `Timer_Add()`.

*Return value*: On success, returns the timer ID needed to stop the timer later with `Timer_Delete()`. On failure, returns -1.

### Timer_Delete()

```c
void Timer_Delete(unsigned char id);
```

Stops execution of a timer previously launched by `Timer_Add()` or `Timer_Wake()`. The parameter `id` is the timer ID returned by `Timer_Add()` or `Timer_Wake()`.

*SymbOS name*: `Multitasking_Delete_Timer_Command` (`MSC_KRL_MTDELT`).

## Multithreading

Yes, **SCC supports multithreading** (!), thanks to SymbOS's elegant system for spawning subprocesses. Internally, threads are implemented as subprocesses of the main application (see [`Proc_Add()`](#proc_add) and [`Proc_Delete()`](#proc_delete), but the convenience functions `thread_start()` and `thread_quit()` are provided to reduce the amount of setup required.

In addition to `symbos.h`, these functions can be found in `symbos/threads.h`.

### thread_start()

```c
signed char thread_start(void* routine, char* env, unsigned short envlen);
```

Spawns a new thread by launching `routine`, which should be a `void` function in our code that takes no parameters. Stack and other information for the new thread will be stored in the environment buffer `env`, which must be in the **transfer** segment. The length of this buffer is passed as `envlen` and should generally be at least 256 bytes (more if the thread will make heavy use of the stack, e.g., with deep recursion or large local buffers).

Example:

```c
_transfer char env1[256];

void threadmain(void) {
	/* ...thread code goes here, which may call other functions... */
	/* ... */
	thread_quit(env1); // quit thread (see below)
}

int main(int argc, char* argv[]) {
	thread_start(threadmain, env1, sizeof(env1));
	/* ...main thread continues here... */
}
```

*Return value*: On success, returns the process ID of the new thread, which will also be stored as the last byte in `env`. On failure, returns -1.

### thread_quit()

```c
void thread_quit(char* env);
```

Quits the running thread associated with the environment buffer `env`. (This function will not return, so it should only be used inside the thread in question, to quit itself. To forcibly end a running thread from inside a different thread, use [`Proc_Delete()`](#proc_delete); however, this can leave resources hanging if the thread is interrupted in the middle of a system call, so it is usually better to have the thread quit itself with `thread_quit()`.)

### Thread safety

**Warning**: When multithreading, we have to worry about all the problems that come with multithreading---race conditions, deadlocks, concurrent access, reentrancy, etc. The current implementation of libc is also not generally designed with thread-safety in mind, so while most small utility functions (`memcpy()`, `strcat()`, etc.) are thread-safe, others are not---in particular, much of `stdio.h`. Any function that relies on temporarily storing data in a static buffer (rather than local variables) is not thread-safe and may misbehave if two threads call it at the same time. When in doubt, check the library source code to verify that a function does not rely on any static/global variables, or write your own reentrant substitute (e.g., using only local variables, or with a semaphore system that sets a global variable when the shared resource is being used and, if it is already set, loops until it is unset by whatever other thread is using the resource). Any use of 32-bit data types (**long**, **float**, **double**) is also currently not thread-safe, as these internally rely on static "extended" registers, meaning that only one thread at a time can safely use 32-bit data types.

Standard SymbOS system calls that do not use 32-bit data types (`File_Open()`, etc.) should all be thread-safe, as these use a semaphore system to ensure that only one message is passed in `_symmsg` at the same time.

### Thread-safe messaging

When using inter-process messaging functions (`Msg_Send()`, `Msg_Sleep()`, etc.) within a secondary thread, it is strongly recommended to use the utility function `_msgpid()` for the return process ID instead of `_sympid`. E.g.:

```c
Msg_Sleep(_msgpid(), -1, _symmsg);
```

If we used `_sympid`, all messages would be addressed to the application's main process. This queue can technically be read by any process, but using one queue for all messages makes it easy to end up in situations where one thread accidentally receives and discards a message intended for another thread, causing all sorts of strange bugs. It's safer to keep each thread's messages completely separate using `_msgpid()`.

(Note that the value of `_msgpid()` is *not* simply the current process ID; its behavior will depend on SymbOS version.)
