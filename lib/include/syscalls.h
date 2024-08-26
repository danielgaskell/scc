/* SYSCALLS.H */
#ifndef __SYSCALLS_H
#define __SYSCALLS_H
#ifndef __TYPES_H
#include <types.h>
#endif
#ifndef __SIGNAL_H
//#include <signal.h>
#endif
#include <sys/stat.h>

//#include <sys/userstructs.h>

extern int errno;

struct _uzisysinfoblk {
  uint8_t infosize;		/* For expandability */
  uint8_t banks;		/* Banks in our 64K (and thus pagesize) */
  uint8_t max_open;
  uint8_t nproc;		/* Number of processes */
  uint16_t ticks;		/* Tick rate in HZ */
  uint16_t memk;		/* Memory in KB */
  uint16_t usedk;		/* Used memory in KB */
  uint16_t config;		/* Config flag mask */
  uint16_t loadavg[3];
  uint16_t swapk;
  uint16_t swapusedk;
  uint8_t cputype;		/* CPU type information */
  uint8_t cpu[3];		/* CPU type specific data */
  uint16_t spare[8];
};

/*
 *	This is actually overlaid over a blkbuf holding the actual
 *	record in question, and pinned until we umount the fs.
 */
#define __FILESYS_TABSIZE 50

struct _uzifilesys {
    int16_t       s_mounted;
    uint16_t      s_isize;
    uint16_t      s_fsize;
    uint16_t      s_nfree;
    uint16_t      s_free[__FILESYS_TABSIZE];
    int16_t       s_ninode;
    uint16_t      s_inode[__FILESYS_TABSIZE];
    uint8_t       s_fmod;
    uint8_t       s_timeh;
    uint32_t      s_time;
    uint16_t      s_tfree;
    uint16_t      s_tinode;
    uint8_t	  s_shift;
};

struct _sockio {
        uint16_t sio_flags;
        uint16_t sio_addr_len;
        uint8_t sio_addr[16];
};

struct hd_geometry {
	uint8_t heads;
	uint8_t sectors;
	uint16_t cylinders;
	uint32_t start;
};

typedef struct {
        uint32_t low;
        uint32_t high;
} __ktime_t;

struct sockaddr;
struct sockaddr_in;

#define HDIO_GETGEO		0x0101
#define HDIO_GET_IDENTITY	0x0102	/* Not yet implemented anywhere */
#define HDIO_TRIM 		0x0106
#define BLKGETSIZE		0x0107

/* uadmin */

#define A_SHUTDOWN		1
#define A_REBOOT		2
#define A_DUMP			3
#define A_FREEZE		4	/* Unimplemented, want for NC100? */
#define A_SWAPCTL		16	/* Unimplemented */
#define A_CONFIG		17	/* Unimplemented */
#define A_FTRACE		18	/* Unimplemented:
                                          Hook to the syscall trace debug */
#define A_SUSPEND               32	/* Suspend to RAM (optional) */

#define AD_NOSYNC		1

#define A_SC_ADD		1

/* shutdown */
#define SHUT_RD			0
#define SHUT_WR			1
#define SHUT_RDWR		2

struct times;
struct tms;
struct utimbuf;
struct utsname;
struct rlimit;

extern void _exit(int code);
extern int open(const char *path, int flags, ...);
extern int close(int fd);
extern int creat(const char *path, mode_t mode);
extern int mknod(const char *path, mode_t mode, dev_t dev); /* FIXME */
extern int link(const char *path, const char *path2); /* FIXME */
extern int unlink(const char *path); /* FIXME */
extern ssize_t read(int fd, void *buf, int len);
extern ssize_t write(int fd, const void *buf, int len);
extern int chdir(const char *path); /* FIXME */
extern void sync(void); /* FIXME */
extern int access(const char *path, int way); /* FIXME */
extern int chmod(const char *path, mode_t mode); /* FIXME */
extern int chown(const char *path, uid_t owner, gid_t group); /* FIXME */
extern int dup(int fd); /* FIXME */
extern pid_t getpid(void); /* FIXME */
extern pid_t getppid(void); /* FIXME */
extern uid_t getuid(void); /* FIXME */
extern mode_t umask(mode_t); /* FIXME */
extern int execve(const char *path, char * const argv[], char *const envp[]); /* FIXME */
extern pid_t wait(int *status); /* FIXME */
extern int setuid(uid_t uid); /* FIXME */
extern int setgid(gid_t gid); /* FIXME */
extern int ioctl(int fd, int request,...); /* FIXME */
extern int brk(void *addr); /* FIXME */
extern void *sbrk(intptr_t increment); /* FIXME */
extern pid_t _fork(uint16_t flags, void *addr); /* FIXME */
extern int mount(const char *dev, const char *path, int flags); /* FIXME */
extern int _umount(const char *dev, int flags); /* FIXME */
extern sighandler_t signal(int signum, sighandler_t sighandler); /* FIXME */
extern int dup2(int oldfd, int newfd); /* FIXME */
extern int _pause(unsigned int dsecs); /* FIXME */
extern int kill(pid_t pid, int sig); /* FIXME */
extern int pipe(int *pipefds); /* FIXME */
extern gid_t getgid(void); /* FIXME */
extern uid_t geteuid(void); /* FIXME */
extern gid_t getegid(void); /* FIXME */
extern int chroot(const char *path); /* FIXME */
extern int fcntl(int fd, int cmd, ...); /* FIXME */
extern int fchdir(int fd); /* FIXME */
extern int fchmod(int fd, mode_t mode); /* FIXME */
extern int fchown(int fd, uid_t owner, gid_t group); /* FIXME */
extern int mkdir(const char *path, mode_t mode); /* FIXME */
extern int rmdir(const char *path); /* FIXME */
extern pid_t setpgrp(void); /* FIXME */
extern pid_t waitpid(pid_t pid, int *status, int options); /* FIXME */
extern int uadmin(int cmd, int ctrl, void *ptr); /* FIXME */
extern int nice(int prio); /* FIXME */
extern int rename(const char *path, const char *newpath); /* FIXME */
extern int flock(int fd, int op); /* FIXME */
extern pid_t getpgrp(void); /* FIXME */
extern int sched_yield(void); /* FIXME */
extern int acct(const char *filename); /* FIXME */
extern int setgroups(size_t size, const gid_t *groups); /* FIXME */
extern int getgroups(int size, gid_t *groups); /* FIXME */
extern int getrlimit(int resource, struct rlimit *rlim); /* FIXME */
extern int setrlimit(int resource, const struct rlimit *rlim); /* FIXME */
extern int setpgid(pid_t pid, pid_t pgrp); /* FIXME */
extern pid_t setsid(void); /* FIXME */
extern pid_t getsid(pid_t pid); /* FIXME */
extern unsigned int _alarm(unsigned int); /* FIXME */

/* asm syscall hooks with C wrappers */
extern int _getdirent(int fd, void *buf, int len); /* FIXME */
extern int _stat(const char *path, struct _uzistat *s); /* FIXME */
extern int _fstat(int fd, struct _uzistat *s); /* FIXME */
extern int _statfs(const char *path, uint8_t *fs); /* FIXME */
extern int _time(__ktime_t *t, uint16_t clock); /* FIXME */
extern int _stime(const __ktime_t *t, uint16_t clock); /* FIXME */
extern int _times(struct tms *t); /* FIXME */
extern int _utime(const char *file, __ktime_t *buf); /* FIXME */
extern int _uname(struct _uzisysinfoblk *uzib, int len); /* FIXME */
extern int _profil(void *samples, uint16_t offset, uint16_t size, int16_t scale); /* FIXME */
extern int _lseek(int fd, off_t *offset, int mode); /* FIXME */
extern int _select(int nfd, uint16_t *base); /* FIXME */
extern int _ftruncate(int fd, off_t *offset); /* FIXME */

/* C library provided syscall emulation */
extern int stat(const char *path, struct stat *s); /* FIXME */
extern int fstat(int fd, struct stat *s); /* FIXME */
extern unsigned int alarm(unsigned int seconds); /* FIXME */
extern time_t time(time_t *t); /* FIXME */
extern int stime(const time_t *t); /* FIXME */
extern int times(struct tms *tms); /* FIXME */
extern int utime(const char *filename, const struct utimbuf *utim); /* FIXME */
extern int uname(struct utsname *buf); /* FIXME */
extern int profil(unsigned short *bufbase, size_t bufsize, unsigned long offset, /* FIXME */
                  unsigned int scale);
extern int ftruncate(int fd, off_t offset); /* FIXME */

/* Networking */
extern int __netcall(void *argbuf); /* FIXME */

#endif	/* __SYSCALLS_H */


