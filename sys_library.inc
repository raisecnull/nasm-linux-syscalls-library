; =============================================================================
;  sys_library.inc  -  Readable Linux x86-64 syscall library for NASM
; =============================================================================
;  Usage:      %include "sys_library.inc"
;  Requires:   syscalls.inc (SYS_* numbers) in the same directory
;
;  Sections:   1. ABI cheat sheet          9.  Process / wait / clone
;              2. Standard descriptors     10. Signals
;              3. File I/O                 11. Sockets
;              4. Directories / paths      12. Polling / epoll / eventfd
;              5. Memory                   13. Time
;              6. Pipes / dup / ioctl      14. IPC / futex / misc
;              7. Struct offsets           15. Helper macros
;              8. Errno values
;
;  Every section lists its syscalls with the register arguments, followed by
;  the constants those syscalls use.
; =============================================================================

%ifndef SYS_LIBRARY_INC
%define SYS_LIBRARY_INC

%include "syscalls.inc"

; =============================================================================
; 1. x86-64 LINUX SYSCALL ABI
; =============================================================================
;   rax = syscall number            return value comes back in rax
;   args: rdi, rsi, rdx, r10, r8, r9   (NOTE: r10, not rcx, for the 4th arg)
;   `syscall` clobbers rcx and r11. All other registers are preserved.
;   Errors are returned as -errno (rax in -4095..-1), e.g. -2 = -ENOENT.
; =============================================================================


; =============================================================================
; 2. STANDARD FILE DESCRIPTORS
; =============================================================================
STDIN               equ 0
STDOUT              equ 1
STDERR              equ 2

EXIT_SUCCESS        equ 0
EXIT_FAILURE        equ 1

NULL                equ 0
TRUE                equ 1
FALSE               equ 0
LF                  equ 10
CR                  equ 13


; =============================================================================
; 3. FILE I/O
; =============================================================================
;  SYS_READ        rdi=fd  rsi=buf  rdx=count
;  SYS_WRITE       rdi=fd  rsi=buf  rdx=count
;  SYS_OPEN        rdi=path  rsi=flags  rdx=mode
;  SYS_OPENAT      rdi=dirfd  rsi=path  rdx=flags  r10=mode
;  SYS_CLOSE       rdi=fd
;  SYS_LSEEK       rdi=fd  rsi=offset  rdx=whence
;  SYS_PREAD64     rdi=fd  rsi=buf  rdx=count  r10=offset
;  SYS_PWRITE64    rdi=fd  rsi=buf  rdx=count  r10=offset
;  SYS_READV       rdi=fd  rsi=iovec*  rdx=iovcnt
;  SYS_WRITEV      rdi=fd  rsi=iovec*  rdx=iovcnt
;  SYS_STAT        rdi=path  rsi=statbuf*
;  SYS_FSTAT       rdi=fd    rsi=statbuf*
;  SYS_LSTAT       rdi=path  rsi=statbuf*
;  SYS_NEWFSTATAT  rdi=dirfd  rsi=path  rdx=statbuf*  r10=flags
;  SYS_ACCESS      rdi=path  rsi=mode (F_OK/R_OK/W_OK/X_OK)
;  SYS_TRUNCATE    rdi=path  rsi=length
;  SYS_FTRUNCATE   rdi=fd    rsi=length
;  SYS_FSYNC       rdi=fd
;  SYS_FLOCK       rdi=fd  rsi=operation (LOCK_*)
;  SYS_FCNTL       rdi=fd  rsi=cmd (F_*)  rdx=arg
;  SYS_SENDFILE    rdi=out_fd  rsi=in_fd  rdx=offset*  r10=count
;  SYS_UMASK       rdi=mask

; open() flags
O_RDONLY            equ 0x0
O_WRONLY            equ 0x1
O_RDWR              equ 0x2
O_ACCMODE           equ 0x3
O_CREAT             equ 0x40            ; 0o100
O_EXCL              equ 0x80
O_NOCTTY            equ 0x100
O_TRUNC             equ 0x200
O_APPEND            equ 0x400
O_NONBLOCK          equ 0x800
O_DSYNC             equ 0x1000
O_ASYNC             equ 0x2000
O_DIRECT            equ 0x4000
O_DIRECTORY         equ 0x10000
O_NOFOLLOW          equ 0x20000
O_NOATIME           equ 0x40000
O_CLOEXEC           equ 0x80000
O_SYNC              equ 0x101000
O_PATH              equ 0x200000
O_TMPFILE           equ 0x410000

; Common flag combos
O_CREATE_WRITE      equ (O_WRONLY | O_CREAT | O_TRUNC)   ; like fopen("w")
O_CREATE_APPEND     equ (O_WRONLY | O_CREAT | O_APPEND)  ; like fopen("a")

; lseek() whence
SEEK_SET            equ 0
SEEK_CUR            equ 1
SEEK_END            equ 2

; File permission bits (mode argument)
S_IRUSR             equ 0x100           ; 0o400
S_IWUSR             equ 0x80            ; 0o200
S_IXUSR             equ 0x40            ; 0o100
S_IRWXU             equ 0x1C0           ; 0o700
S_IRGRP             equ 0x20            ; 0o040
S_IWGRP             equ 0x10            ; 0o020
S_IXGRP             equ 0x08            ; 0o010
S_IRWXG             equ 0x38            ; 0o070
S_IROTH             equ 0x04
S_IWOTH             equ 0x02
S_IXOTH             equ 0x01
S_IRWXO             equ 0x07
S_ISUID             equ 0x800           ; 0o4000
S_ISGID             equ 0x400           ; 0o2000
S_ISVTX             equ 0x200           ; 0o1000

MODE_600            equ 0x180           ; rw-------
MODE_644            equ 0x1A4           ; rw-r--r--
MODE_666            equ 0x1B6           ; rw-rw-rw-
MODE_700            equ 0x1C0           ; rwx------
MODE_755            equ 0x1ED           ; rwxr-xr-x
MODE_777            equ 0x1FF           ; rwxrwxrwx

; st_mode file type bits
S_IFMT              equ 0xF000
S_IFSOCK            equ 0xC000
S_IFLNK             equ 0xA000
S_IFREG             equ 0x8000
S_IFBLK             equ 0x6000
S_IFDIR             equ 0x4000
S_IFCHR             equ 0x2000
S_IFIFO             equ 0x1000

; access() mode
F_OK                equ 0
X_OK                equ 1
W_OK                equ 2
R_OK                equ 4

; fcntl() commands
F_DUPFD             equ 0
F_GETFD             equ 1
F_SETFD             equ 2
F_GETFL             equ 3
F_SETFL             equ 4
F_GETLK             equ 5
F_SETLK             equ 6
F_SETLKW            equ 7
F_DUPFD_CLOEXEC     equ 1030
FD_CLOEXEC          equ 1

; flock() operations
LOCK_SH             equ 1
LOCK_EX             equ 2
LOCK_NB             equ 4
LOCK_UN             equ 8


; =============================================================================
; 4. DIRECTORIES / PATHS
; =============================================================================
;  SYS_GETDENTS64  rdi=fd  rsi=dirent*  rdx=count
;  SYS_GETCWD      rdi=buf  rsi=size
;  SYS_CHDIR       rdi=path
;  SYS_MKDIR       rdi=path  rsi=mode
;  SYS_RMDIR       rdi=path
;  SYS_UNLINKAT    rdi=dirfd  rsi=path  rdx=flags (AT_REMOVEDIR)
;  SYS_RENAME      rdi=oldpath  rsi=newpath
;  SYS_LINK        rdi=oldpath  rsi=newpath
;  SYS_SYMLINK     rdi=target   rsi=linkpath
;  SYS_READLINK    rdi=path  rsi=buf  rdx=bufsiz
;  SYS_CHMOD       rdi=path  rsi=mode
;  SYS_CHOWN       rdi=path  rsi=uid  rdx=gid

AT_FDCWD            equ -100
AT_SYMLINK_NOFOLLOW equ 0x100
AT_REMOVEDIR        equ 0x200
AT_SYMLINK_FOLLOW   equ 0x400
AT_EMPTY_PATH       equ 0x1000

; d_type values in linux_dirent64
DT_UNKNOWN          equ 0
DT_FIFO             equ 1
DT_CHR              equ 2
DT_DIR              equ 4
DT_BLK              equ 6
DT_REG              equ 8
DT_LNK              equ 10
DT_SOCK             equ 12

PATH_MAX            equ 4096


; =============================================================================
; 5. MEMORY
; =============================================================================
;  SYS_MMAP        rdi=addr  rsi=length  rdx=prot  r10=flags  r8=fd  r9=offset
;  SYS_MUNMAP      rdi=addr  rsi=length
;  SYS_MPROTECT    rdi=addr  rsi=length  rdx=prot
;  SYS_MREMAP      rdi=old_addr  rsi=old_size  rdx=new_size  r10=flags  r8=new_addr
;  SYS_MSYNC       rdi=addr  rsi=length  rdx=flags
;  SYS_MADVISE     rdi=addr  rsi=length  rdx=advice
;  SYS_BRK         rdi=new_brk (0 = query current)
;  SYS_MEMFD_CREATE rdi=name  rsi=flags

PAGE_SIZE           equ 4096

PROT_NONE           equ 0x0
PROT_READ           equ 0x1
PROT_WRITE          equ 0x2
PROT_EXEC           equ 0x4
PROT_RW             equ (PROT_READ | PROT_WRITE)
PROT_RWX            equ (PROT_READ | PROT_WRITE | PROT_EXEC)

MAP_SHARED          equ 0x01
MAP_PRIVATE         equ 0x02
MAP_FIXED           equ 0x10
MAP_ANONYMOUS       equ 0x20
MAP_GROWSDOWN       equ 0x100
MAP_NORESERVE       equ 0x4000
MAP_POPULATE        equ 0x8000
MAP_NONBLOCK        equ 0x10000
MAP_STACK           equ 0x20000
MAP_HUGETLB         equ 0x40000
MAP_FAILED          equ -1

MAP_ANON_PRIVATE    equ (MAP_PRIVATE | MAP_ANONYMOUS)   ; typical malloc-style mmap

MS_ASYNC            equ 1
MS_INVALIDATE       equ 2
MS_SYNC             equ 4

MADV_NORMAL         equ 0
MADV_RANDOM         equ 1
MADV_SEQUENTIAL     equ 2
MADV_WILLNEED       equ 3
MADV_DONTNEED       equ 4

MREMAP_MAYMOVE      equ 1
MREMAP_FIXED        equ 2


; =============================================================================
; 6. PIPES / DUP / IOCTL
; =============================================================================
;  SYS_PIPE        rdi=int fds[2]*
;  SYS_PIPE2       rdi=int fds[2]*  rsi=flags (O_CLOEXEC|O_NONBLOCK)
;  SYS_DUP         rdi=oldfd
;  SYS_DUP2        rdi=oldfd  rsi=newfd
;  SYS_DUP3        rdi=oldfd  rsi=newfd  rdx=flags
;  SYS_IOCTL       rdi=fd  rsi=request  rdx=arg
;  SYS_SPLICE      rdi=fd_in  rsi=off_in*  rdx=fd_out  r10=off_out*  r8=len  r9=flags

TCGETS              equ 0x5401
TCSETS              equ 0x5402
TIOCGWINSZ          equ 0x5413
FIONREAD            equ 0x541B
FIONBIO             equ 0x5421

; termios c_lflag bits (for raw/no-echo terminal input)
ISIG                equ 0x01
ICANON              equ 0x02
ECHO                equ 0x08


; =============================================================================
; 7. STRUCT OFFSETS (x86-64)
; =============================================================================
; struct stat (144 bytes) - filled by SYS_STAT/FSTAT/LSTAT
STAT_ST_DEV         equ 0
STAT_ST_INO         equ 8
STAT_ST_NLINK       equ 16
STAT_ST_MODE        equ 24              ; dword
STAT_ST_UID         equ 28              ; dword
STAT_ST_GID         equ 32              ; dword
STAT_ST_RDEV        equ 40
STAT_ST_SIZE        equ 48              ; qword
STAT_ST_BLKSIZE     equ 56
STAT_ST_BLOCKS      equ 64
STAT_ST_ATIME       equ 72
STAT_ST_MTIME       equ 88
STAT_ST_CTIME       equ 104
STAT_SIZE           equ 144

; struct linux_dirent64 - filled by SYS_GETDENTS64
DIRENT_D_INO        equ 0               ; qword
DIRENT_D_OFF        equ 8               ; qword
DIRENT_D_RECLEN     equ 16              ; word
DIRENT_D_TYPE       equ 18              ; byte
DIRENT_D_NAME       equ 19              ; NUL-terminated string

; struct iovec (16 bytes) - for readv/writev
IOVEC_BASE          equ 0               ; qword
IOVEC_LEN           equ 8               ; qword
IOVEC_SIZE          equ 16

; struct timespec (16 bytes)
TIMESPEC_SEC        equ 0               ; qword
TIMESPEC_NSEC       equ 8               ; qword
TIMESPEC_SIZE       equ 16

; struct timeval (16 bytes)
TIMEVAL_SEC         equ 0
TIMEVAL_USEC        equ 8
TIMEVAL_SIZE        equ 16

; struct sockaddr_in (16 bytes)
SOCKADDR_IN_FAMILY  equ 0               ; word  (AF_INET)
SOCKADDR_IN_PORT    equ 2               ; word  (network byte order!)
SOCKADDR_IN_ADDR    equ 4               ; dword (network byte order!)
SOCKADDR_IN_ZERO    equ 8               ; 8 bytes padding
SOCKADDR_IN_SIZE    equ 16

; struct sockaddr_in6 (28 bytes)
SOCKADDR_IN6_FAMILY equ 0               ; word
SOCKADDR_IN6_PORT   equ 2               ; word
SOCKADDR_IN6_FLOW   equ 4               ; dword
SOCKADDR_IN6_ADDR   equ 8               ; 16 bytes
SOCKADDR_IN6_SCOPE  equ 24              ; dword
SOCKADDR_IN6_SIZE   equ 28

; struct sockaddr_un (110 bytes)
SOCKADDR_UN_FAMILY  equ 0               ; word  (AF_UNIX)
SOCKADDR_UN_PATH    equ 2               ; 108 bytes
SOCKADDR_UN_SIZE    equ 110

; struct pollfd (8 bytes)
POLLFD_FD           equ 0               ; dword
POLLFD_EVENTS       equ 4               ; word
POLLFD_REVENTS      equ 6               ; word
POLLFD_SIZE         equ 8

; struct epoll_event (12 bytes, packed on x86-64)
EPOLL_EVENT_EVENTS  equ 0               ; dword
EPOLL_EVENT_DATA    equ 4               ; qword (unaligned)
EPOLL_EVENT_SIZE    equ 12

; struct winsize (8 bytes) - for TIOCGWINSZ
WINSIZE_ROW         equ 0               ; word
WINSIZE_COL         equ 2               ; word
WINSIZE_XPIXEL      equ 4
WINSIZE_YPIXEL      equ 6
WINSIZE_SIZE        equ 8

; struct rlimit (16 bytes)
RLIMIT_CUR          equ 0
RLIMIT_MAX          equ 8
RLIMIT_SIZE         equ 16

; struct sigaction (as used by raw SYS_RT_SIGACTION, 32 bytes)
SIGACTION_HANDLER   equ 0               ; qword
SIGACTION_FLAGS     equ 8               ; qword
SIGACTION_RESTORER  equ 16              ; qword (needs SA_RESTORER)
SIGACTION_MASK      equ 24              ; qword (sigset, 8 bytes)
SIGACTION_SIZE      equ 32
SIGSET_SIZE         equ 8               ; 4th arg (r10) of rt_sigaction/rt_sigprocmask


; =============================================================================
; 8. ERRNO VALUES (syscalls return -errno in rax)
; =============================================================================
EPERM               equ 1
ENOENT              equ 2
ESRCH               equ 3
EINTR               equ 4
EIO                 equ 5
ENXIO               equ 6
E2BIG               equ 7
ENOEXEC             equ 8
EBADF               equ 9
ECHILD              equ 10
EAGAIN              equ 11
EWOULDBLOCK         equ 11
ENOMEM              equ 12
EACCES              equ 13
EFAULT              equ 14
EBUSY               equ 16
EEXIST              equ 17
EXDEV               equ 18
ENODEV              equ 19
ENOTDIR             equ 20
EISDIR              equ 21
EINVAL              equ 22
ENFILE              equ 23
EMFILE              equ 24
ENOTTY              equ 25
EFBIG               equ 27
ENOSPC              equ 28
ESPIPE              equ 29
EROFS               equ 30
EPIPE               equ 32
ERANGE              equ 34
ENOSYS              equ 38
ENOTEMPTY           equ 39
ENOTSOCK            equ 88
EDESTADDRREQ        equ 89
EMSGSIZE            equ 90
EPROTOTYPE          equ 91
ENOPROTOOPT         equ 92
EPROTONOSUPPORT     equ 93
EOPNOTSUPP          equ 95
EAFNOSUPPORT        equ 97
EADDRINUSE          equ 98
EADDRNOTAVAIL       equ 99
ENETDOWN            equ 100
ENETUNREACH         equ 101
ECONNABORTED        equ 103
ECONNRESET          equ 104
ENOBUFS             equ 105
EISCONN             equ 106
ENOTCONN            equ 107
ETIMEDOUT           equ 110
ECONNREFUSED        equ 111
EHOSTUNREACH        equ 113
EALREADY            equ 114
EINPROGRESS         equ 115


; =============================================================================
; 9. PROCESS / WAIT / CLONE
; =============================================================================
;  SYS_FORK        (no args)
;  SYS_VFORK       (no args)
;  SYS_CLONE       rdi=flags  rsi=stack  rdx=parent_tid*  r10=child_tid*  r8=tls
;  SYS_EXECVE      rdi=path  rsi=argv[]  rdx=envp[]
;  SYS_EXIT        rdi=status
;  SYS_EXIT_GROUP  rdi=status
;  SYS_WAIT4       rdi=pid  rsi=status*  rdx=options  r10=rusage*
;  SYS_GETPID / SYS_GETPPID / SYS_GETUID / SYS_GETGID   (no args)
;  SYS_SETUID      rdi=uid
;  SYS_SETSID      (no args)
;  SYS_KILL        rdi=pid  rsi=signal
;  SYS_TGKILL      rdi=tgid  rsi=tid  rdx=signal
;  SYS_UNAME       rdi=utsname*
;  SYS_PRCTL       rdi=option  rsi=arg2  rdx=arg3  r10=arg4  r8=arg5
;  SYS_ARCH_PRCTL  rdi=code (ARCH_*)  rsi=addr
;  SYS_GETRLIMIT   rdi=resource  rsi=rlimit*
;  SYS_PRLIMIT64   rdi=pid  rsi=resource  rdx=new*  r10=old*
;  SYS_GETRANDOM   rdi=buf  rsi=buflen  rdx=flags

WNOHANG             equ 1
WUNTRACED           equ 2
WCONTINUED          equ 8
__WALL              equ 0x40000000
__WCLONE            equ 0x80000000

CLONE_VM            equ 0x00000100
CLONE_FS            equ 0x00000200
CLONE_FILES         equ 0x00000400
CLONE_SIGHAND       equ 0x00000800
CLONE_PTRACE        equ 0x00002000
CLONE_VFORK         equ 0x00004000
CLONE_PARENT        equ 0x00008000
CLONE_THREAD        equ 0x00010000
CLONE_NEWNS         equ 0x00020000
CLONE_SYSVSEM       equ 0x00040000
CLONE_SETTLS        equ 0x00080000
CLONE_PARENT_SETTID equ 0x00100000
CLONE_CHILD_CLEARTID equ 0x00200000
CLONE_CHILD_SETTID  equ 0x01000000

; Typical flags for a pthread-style thread
CLONE_THREAD_FLAGS  equ (CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM)

ARCH_SET_GS         equ 0x1001
ARCH_SET_FS         equ 0x1002
ARCH_GET_FS         equ 0x1003
ARCH_GET_GS         equ 0x1004

RLIMIT_CPU          equ 0
RLIMIT_FSIZE        equ 1
RLIMIT_DATA         equ 2
RLIMIT_STACK        equ 3
RLIMIT_CORE         equ 4
RLIMIT_NOFILE       equ 7
RLIMIT_AS           equ 9

GRND_NONBLOCK       equ 1
GRND_RANDOM         equ 2


; =============================================================================
; 10. SIGNALS
; =============================================================================
;  SYS_RT_SIGACTION    rdi=signum  rsi=act*  rdx=oldact*  r10=sigsetsize (8)
;  SYS_RT_SIGPROCMASK  rdi=how (SIG_*)  rsi=set*  rdx=oldset*  r10=sigsetsize (8)
;  SYS_RT_SIGRETURN    (no args; used as the signal restorer)
;  SYS_PAUSE           (no args)
;  SYS_ALARM           rdi=seconds
;  SYS_KILL / SYS_TGKILL   see Process section

SIGHUP              equ 1
SIGINT              equ 2
SIGQUIT             equ 3
SIGILL              equ 4
SIGTRAP             equ 5
SIGABRT             equ 6
SIGBUS              equ 7
SIGFPE              equ 8
SIGKILL             equ 9
SIGUSR1             equ 10
SIGSEGV             equ 11
SIGUSR2             equ 12
SIGPIPE             equ 13
SIGALRM             equ 14
SIGTERM             equ 15
SIGSTKFLT           equ 16
SIGCHLD             equ 17
SIGCONT             equ 18
SIGSTOP             equ 19
SIGTSTP             equ 20
SIGTTIN             equ 21
SIGTTOU             equ 22
SIGURG              equ 23
SIGXCPU             equ 24
SIGXFSZ             equ 25
SIGVTALRM           equ 26
SIGPROF             equ 27
SIGWINCH            equ 28
SIGIO               equ 29
SIGPWR              equ 30
SIGSYS              equ 31

SIG_BLOCK           equ 0
SIG_UNBLOCK         equ 1
SIG_SETMASK         equ 2

SIG_DFL             equ 0
SIG_IGN             equ 1

SA_NOCLDSTOP        equ 0x00000001
SA_NOCLDWAIT        equ 0x00000002
SA_SIGINFO          equ 0x00000004
SA_RESTORER         equ 0x04000000      ; REQUIRED on x86-64 when using raw syscalls
SA_ONSTACK          equ 0x08000000
SA_RESTART          equ 0x10000000
SA_NODEFER          equ 0x40000000
SA_RESETHAND        equ 0x80000000


; =============================================================================
; 11. SOCKETS
; =============================================================================
;  SYS_SOCKET      rdi=domain  rsi=type  rdx=protocol
;  SYS_SOCKETPAIR  rdi=domain  rsi=type  rdx=protocol  r10=int sv[2]*
;  SYS_BIND        rdi=sockfd  rsi=sockaddr*  rdx=addrlen
;  SYS_LISTEN      rdi=sockfd  rsi=backlog
;  SYS_ACCEPT      rdi=sockfd  rsi=sockaddr*  rdx=addrlen*
;  SYS_ACCEPT4     rdi=sockfd  rsi=sockaddr*  rdx=addrlen*  r10=flags (SOCK_*)
;  SYS_CONNECT     rdi=sockfd  rsi=sockaddr*  rdx=addrlen
;  SYS_SENDTO      rdi=sockfd  rsi=buf  rdx=len  r10=flags  r8=dest_addr*  r9=addrlen
;  SYS_RECVFROM    rdi=sockfd  rsi=buf  rdx=len  r10=flags  r8=src_addr*  r9=addrlen*
;  SYS_SENDMSG     rdi=sockfd  rsi=msghdr*  rdx=flags
;  SYS_RECVMSG     rdi=sockfd  rsi=msghdr*  rdx=flags
;  SYS_SHUTDOWN    rdi=sockfd  rsi=how (SHUT_*)
;  SYS_SETSOCKOPT  rdi=sockfd  rsi=level  rdx=optname  r10=optval*  r8=optlen
;  SYS_GETSOCKOPT  rdi=sockfd  rsi=level  rdx=optname  r10=optval*  r8=optlen*
;  SYS_GETSOCKNAME rdi=sockfd  rsi=sockaddr*  rdx=addrlen*
;  SYS_GETPEERNAME rdi=sockfd  rsi=sockaddr*  rdx=addrlen*
;  (read/write/close also work on sockets)

; Address families
AF_UNSPEC           equ 0
AF_UNIX             equ 1
AF_LOCAL            equ 1
AF_INET             equ 2
AF_INET6            equ 10
AF_NETLINK          equ 16
AF_PACKET           equ 17
PF_UNSPEC           equ AF_UNSPEC
PF_UNIX             equ AF_UNIX
PF_LOCAL            equ AF_LOCAL
PF_INET             equ AF_INET
PF_INET6            equ AF_INET6

; Socket types
SOCK_STREAM         equ 1
SOCK_DGRAM          equ 2
SOCK_RAW            equ 3
SOCK_RDM            equ 4
SOCK_SEQPACKET      equ 5
SOCK_NONBLOCK       equ 0x800           ; OR into type
SOCK_CLOEXEC        equ 0x80000         ; OR into type

; Protocols
IPPROTO_IP          equ 0
IPPROTO_ICMP        equ 1
IPPROTO_TCP         equ 6
IPPROTO_UDP         equ 17
IPPROTO_IPV6        equ 41
IPPROTO_RAW         equ 255

; Socket option levels
SOL_SOCKET          equ 1
SOL_IP              equ 0
SOL_TCP             equ 6

; SOL_SOCKET options
SO_DEBUG            equ 1
SO_REUSEADDR        equ 2
SO_TYPE             equ 3
SO_ERROR            equ 4
SO_DONTROUTE        equ 5
SO_BROADCAST        equ 6
SO_SNDBUF           equ 7
SO_RCVBUF           equ 8
SO_KEEPALIVE        equ 9
SO_OOBINLINE        equ 10
SO_LINGER           equ 13
SO_REUSEPORT        equ 15
SO_RCVLOWAT         equ 18
SO_SNDLOWAT         equ 19
SO_RCVTIMEO         equ 20
SO_SNDTIMEO         equ 21
SO_ACCEPTCONN       equ 30

; IPPROTO_TCP options
TCP_NODELAY         equ 1
TCP_MAXSEG          equ 2
TCP_CORK            equ 3
TCP_KEEPIDLE        equ 4
TCP_KEEPINTVL       equ 5
TCP_KEEPCNT         equ 6

; shutdown() how
SHUT_RD             equ 0
SHUT_WR             equ 1
SHUT_RDWR           equ 2

; send/recv flags
MSG_OOB             equ 0x01
MSG_PEEK            equ 0x02
MSG_DONTROUTE       equ 0x04
MSG_CTRUNC          equ 0x08
MSG_TRUNC           equ 0x20
MSG_DONTWAIT        equ 0x40
MSG_EOR             equ 0x80
MSG_WAITALL         equ 0x100
MSG_NOSIGNAL        equ 0x4000

; Addresses (already in network byte order as a little-endian dword)
INADDR_ANY          equ 0x00000000      ; 0.0.0.0
INADDR_LOOPBACK     equ 0x0100007F      ; 127.0.0.1
INADDR_BROADCAST    equ 0xFFFFFFFF      ; 255.255.255.255

SOMAXCONN           equ 4096


; =============================================================================
; 12. POLLING / EPOLL / EVENTFD
; =============================================================================
;  SYS_POLL            rdi=pollfd*  rsi=nfds  rdx=timeout_ms
;  SYS_SELECT          rdi=nfds  rsi=readfds*  rdx=writefds*  r10=exceptfds*  r8=timeval*
;  SYS_EPOLL_CREATE1   rdi=flags (EPOLL_CLOEXEC)
;  SYS_EPOLL_CTL       rdi=epfd  rsi=op (EPOLL_CTL_*)  rdx=fd  r10=epoll_event*
;  SYS_EPOLL_WAIT      rdi=epfd  rsi=events*  rdx=maxevents  r10=timeout_ms
;  SYS_EPOLL_PWAIT     rdi=epfd  rsi=events*  rdx=maxevents  r10=timeout_ms  r8=sigmask*  r9=sigsetsize
;  SYS_EVENTFD2        rdi=initval  rsi=flags (EFD_*)

POLLIN              equ 0x01
POLLPRI             equ 0x02
POLLOUT             equ 0x04
POLLERR             equ 0x08
POLLHUP             equ 0x10
POLLNVAL            equ 0x20

EPOLL_CLOEXEC       equ 0x80000
EPOLL_CTL_ADD       equ 1
EPOLL_CTL_DEL       equ 2
EPOLL_CTL_MOD       equ 3

EPOLLIN             equ 0x001
EPOLLPRI            equ 0x002
EPOLLOUT            equ 0x004
EPOLLERR            equ 0x008
EPOLLHUP            equ 0x010
EPOLLRDHUP          equ 0x2000
EPOLLEXCLUSIVE      equ 0x10000000
EPOLLONESHOT        equ 0x40000000
EPOLLET             equ 0x80000000

EFD_SEMAPHORE       equ 1
EFD_NONBLOCK        equ 0x800
EFD_CLOEXEC         equ 0x80000


; =============================================================================
; 13. TIME
; =============================================================================
;  SYS_NANOSLEEP        rdi=timespec* req  rsi=timespec* rem
;  SYS_CLOCK_NANOSLEEP  rdi=clockid  rsi=flags  rdx=req*  r10=rem*
;  SYS_CLOCK_GETTIME    rdi=clockid  rsi=timespec*
;  SYS_GETTIMEOFDAY     rdi=timeval*  rsi=timezone*
;  SYS_TIME             rdi=time_t* (or 0)
;  SYS_SETITIMER        rdi=which (ITIMER_*)  rsi=new*  rdx=old*

CLOCK_REALTIME          equ 0
CLOCK_MONOTONIC         equ 1
CLOCK_PROCESS_CPUTIME_ID equ 2
CLOCK_THREAD_CPUTIME_ID equ 3
CLOCK_MONOTONIC_RAW     equ 4
CLOCK_REALTIME_COARSE   equ 5
CLOCK_MONOTONIC_COARSE  equ 6
CLOCK_BOOTTIME          equ 7
TIMER_ABSTIME           equ 1

ITIMER_REAL         equ 0
ITIMER_VIRTUAL      equ 1
ITIMER_PROF         equ 2


; =============================================================================
; 14. IPC / FUTEX / MISC
; =============================================================================
;  SYS_SHMGET      rdi=key  rsi=size  rdx=shmflg
;  SYS_SHMAT       rdi=shmid  rsi=addr  rdx=shmflg
;  SYS_SHMCTL      rdi=shmid  rsi=cmd (IPC_*)  rdx=buf*
;  SYS_FUTEX       rdi=uaddr*  rsi=op  rdx=val  r10=timeout*  r8=uaddr2*  r9=val3
;  SYS_SYSINFO     rdi=sysinfo*

IPC_PRIVATE         equ 0
IPC_CREAT           equ 0x200           ; 0o1000
IPC_EXCL            equ 0x400           ; 0o2000
IPC_NOWAIT          equ 0x800           ; 0o4000
IPC_RMID            equ 0
IPC_SET             equ 1
IPC_STAT            equ 2
SHM_RDONLY          equ 0x1000

FUTEX_WAIT          equ 0
FUTEX_WAKE          equ 1
FUTEX_PRIVATE_FLAG  equ 128
FUTEX_WAIT_PRIVATE  equ (FUTEX_WAIT | FUTEX_PRIVATE_FLAG)
FUTEX_WAKE_PRIVATE  equ (FUTEX_WAKE | FUTEX_PRIVATE_FLAG)


; =============================================================================
; 15. HELPER MACROS
; =============================================================================
; Convert a constant 16-bit value to network byte order (for sin_port):
;     mov word [addr + SOCKADDR_IN_PORT], HTONS(8080)
%define HTONS(x)    ((((x) & 0xFF) << 8) | (((x) >> 8) & 0xFF))

; Build an IPv4 address dword in network byte order:
;     mov dword [addr + SOCKADDR_IN_ADDR], IP4(192,168,1,10)
%define IP4(a,b,c,d) (((d) << 24) | ((c) << 16) | ((b) << 8) | (a))

; Octal-free shorthand for the most common "exit" call:
;     sys_exit 0
%macro sys_exit 1
    mov     rax, SYS_EXIT_GROUP
    mov     rdi, %1
    syscall
%endmacro

%endif ; SYS_LIBRARY_INC
