#include <symbos.h>
#include <string.h>

signed char thread_start(void* routine, char* env, unsigned short envlen) {
    memset(env, 0, envlen);
    *(unsigned short*)env = envlen - 1; // save offset of PID relative to env, for thread_quit()
    *((unsigned short*)(env + envlen - 3)) = (unsigned short)routine;
    return Proc_Add(_symbank, env + envlen - 15, 4);
}

void thread_quit(char* env) {
    unsigned char pid = env[*(unsigned short*)env];
    if (pid)
        Proc_Delete(pid);
    while (1); // wait to be terminated
}
