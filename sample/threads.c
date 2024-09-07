#include <symbos.h>
// compile with: cc -o threads.com threads.c

_transfer char env1[256]; // environment for thread 1
_transfer char env2[256]; // environment for thread 2
unsigned char thread1running;
unsigned char thread2running;

// Thread 1 starting routine. In this example the thread runs in a single function,
// but it could also call other functions.
void thread1main(void) {
    int i, t;
    for (i = 0; i < 10; ++i) {
        Shell_Print("Thread 1!\r\n");
        for (t = 0; t < 2000; ++t); // short delay so messages don't stack up and we can actually see all three threads running
    }
    thread1running = 0;
	thread_quit(env1);
}

// Thread 2 starting routine. In this example the thread runs in a single function,
// but it could also call other functions.
void thread2main(void) {
    int i, t;
    for (i = 0; i < 10; ++i) {
        Shell_Print("Thread 2!\r\n");
        for (t = 0; t < 2000; ++t); // short delay so messages don't stack up and we can actually see all three threads running
    }
    thread2running = 0;
	thread_quit(env2);
}

// Main program entry point
int main(int argc, char* argv[]) {
    int i, t;

    // start both threads
    thread1running = 1;
	thread_start(thread1main, env1, sizeof(env1));
    thread2running = 1;
	thread_start(thread2main, env2, sizeof(env2));

	// continue main thread
    for (i = 0; i < 10; ++i) {
        Shell_Print("Main thread!\r\n");
        for (t = 0; t < 2000; ++t); // short delay so messages don't stack up and we can actually see all three threads running
    }

    // wait for other threads to finish
    while (thread1running || thread2running);
}
