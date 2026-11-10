# nasm-linux-syscalls-library

A readable x86-64 Linux syscall library for NASM: syscall numbers, categorized
constants, per-syscall parameter notes, struct offsets, and a TCP echo server
example.

Instead of writing magic numbers like this:

```nasm
mov rax, 41
mov rdi, 2
mov rsi, 1
mov rdx, 6
syscall
```

you write this:

```nasm
mov rax, SYS_SOCKET
mov rdi, AF_INET
mov rsi, SOCK_STREAM
mov rdx, IPPROTO_TCP
syscall
```

## Features

- **Syscall numbers** for x86-64 Linux (`SYS_READ` ... `SYS_LSM_LIST_MODULES`, 373 entries).
- **Categorized sections:** each lists its syscalls with register arguments
  (e.g. `; SYS_SOCKET  rdi=domain rsi=type rdx=protocol`), followed by the
  constants those syscalls use.
- **~440 constants** with standard C names: `STDIN/STDOUT/STDERR`, `O_*`,
  `S_I*`, `PROT_*`, `MAP_*`, `SIG*`, `AF_*`, `SOCK_*`, `SO_*`, `EPOLL*`,
  `CLOCK_*`, `E*` errno values and more.
- **Struct offsets:** `sockaddr_in`, `sockaddr_un`, `stat`, `timespec`,
  `pollfd`, `epoll_event`, `linux_dirent64`, `sigaction` and others.
- **Helper macros:** `HTONS()`, `IP4()`, `sys_exit`.
- **Zero dependencies:** two include files, no libc.

## Files

```
nasm-linux-syscalls-library/
├── syscalls.inc        # raw SYS_* syscall numbers
├── sys_library.inc     # readable layer: parameters, constants, offsets, macros
├── examples/
│   ├── echo_server.asm # TCP echo server using the library
│   └── test_echo.py    # automated check for the echo server
├── README.md
└── LICENSE
```

`sys_library.inc` includes `syscalls.inc` itself, so your program only needs:

```nasm
%include "sys_library.inc"
```

## Sections in `sys_library.inc`

| # | Section | Examples |
|---|---------|----------|
| 1 | x86-64 syscall ABI cheat sheet | register order, `r10` for 4th arg |
| 2 | Standard descriptors | `STDIN`, `STDOUT`, `STDERR`, `EXIT_SUCCESS` |
| 3 | File I/O | `O_CREAT`, `O_APPEND`, `MODE_644`, `SEEK_SET` |
| 4 | Directories / paths | `AT_FDCWD`, `DT_DIR`, `PATH_MAX` |
| 5 | Memory | `PROT_READ`, `MAP_ANONYMOUS`, `MAP_ANON_PRIVATE` |
| 6 | Pipes / dup / ioctl | `TIOCGWINSZ`, `FIONBIO` |
| 7 | Struct offsets | `SOCKADDR_IN_PORT`, `STAT_ST_SIZE` |
| 8 | Errno values | `ENOENT`, `EAGAIN`, `EADDRINUSE` |
| 9 | Process / wait / clone | `WNOHANG`, `CLONE_THREAD_FLAGS` |
| 10 | Signals | `SIGINT`, `SA_RESTORER`, `SIG_BLOCK` |
| 11 | Sockets | `AF_INET`, `SO_REUSEADDR`, `MSG_NOSIGNAL`, `SHUT_RDWR` |
| 12 | Polling / epoll / eventfd | `EPOLLIN`, `EPOLL_CTL_ADD` |
| 13 | Time | `CLOCK_MONOTONIC`, `TIMER_ABSTIME` |
| 14 | IPC / futex / misc | `IPC_CREAT`, `FUTEX_WAIT_PRIVATE` |
| 15 | Helper macros | `HTONS(8080)`, `IP4(127,0,0,1)` |

## Finding what you need

Search `sys_library.inc` for the syscall name to get its argument order, and
the constants for it sit right below:

```bash
grep -n "SYS_ACCEPT4" sys_library.inc   # parameters for one syscall
grep -n "^SO_" sys_library.inc          # all socket options
grep -n "^O_"  sys_library.inc          # all open() flags
```

Rarer syscalls that have no parameter notes are still available as numbers in
`syscalls.inc`.

## Quick example

```nasm
%include "sys_library.inc"

section .data
msg     db "Hello, syscalls!", LF
len     equ $ - msg

section .text
global _start
_start:
    mov     rax, SYS_WRITE
    mov     rdi, STDOUT
    lea     rsi, [rel msg]
    mov     rdx, len
    syscall

    sys_exit EXIT_SUCCESS
```

```bash
nasm -f elf64 hello.asm -o hello.o
ld hello.o -o hello
./hello
```

If NASM can't find the include, run it from the folder containing the `.inc`
files or pass `-I path/to/library/`.

## Echo server example

`examples/echo_server.asm` is a TCP echo server on `0.0.0.0:8080`. It uses
`socket`, `setsockopt`, `bind`, `listen`, `accept`, `read`, `sendto` and
`close`, plus `HTONS()`, `SOCKADDR_IN_SIZE` and `MSG_NOSIGNAL`.

```bash
cd examples
nasm -f elf64 -I ../ echo_server.asm -o echo_server.o
ld echo_server.o -o echo_server
./echo_server

# in another terminal:
nc 127.0.0.1 8080
python3 test_echo.py     # PASS/FAIL for short, binary and 100 KB payloads
```

## Things to know

- **Errors are negative.** Syscalls return `-errno` in `rax`, so `-2` is
  `-ENOENT`. Check with `test rax, rax` then `js`, or `cmp rax, -4095`.
- **Network byte order.** `sin_port` and `sin_addr` must be big-endian. Use
  `HTONS()` and `IP4()`. `INADDR_LOOPBACK` is already stored correctly.
- **Raw signal handlers.** On x86-64 `rt_sigaction` needs `SA_RESTORER` and a
  restorer routine that calls `SYS_RT_SIGRETURN`.
- **Syscall clobbers.** `syscall` destroys `rcx` and `r11`; other registers are
  preserved.
- **Names.** Constants use the standard C names (`PROT_READ`, `EAGAIN`, ...).
  If one collides with a label in your code, rename your label.
- **Flags are in hex**, with the octal value in a comment where useful.

## Compatibility

- Assembler: NASM
- Architecture: x86-64 Linux only (not 32-bit, not other architectures)
- Syscall table: ends at `SYS_LSM_LIST_MODULES` (461); newer kernel syscalls may be missing
- Tested on: [YOUR DISTRO], kernel [OUTPUT OF `uname -r`], NASM [OUTPUT OF `nasm -v`]

## Contributing

Issues and pull requests are welcome: missing constants, wrong values, extra
syscall parameter notes, or more examples.

## Notes on how this was made

Developed with AI assistance (Claude) and tested by the author: the library
assembles cleanly and the echo server passes `test_echo.py`. Constants are
taken from the Linux kernel headers; please open an issue if you spot a wrong
value.

## License

MIT, see [LICENSE](LICENSE).
