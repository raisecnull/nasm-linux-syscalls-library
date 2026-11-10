; =============================================================================
;  echo_server.asm - TCP echo server using sys_library.inc
; =============================================================================
;  Listens on 0.0.0.0:8080, accepts one client at a time and sends back
;  everything it receives until the client disconnects.
;
;  Build:   nasm -f elf64 echo_server.asm -o echo_server.o
;           ld echo_server.o -o echo_server
;  Run:     ./echo_server
;  Test:    nc 127.0.0.1 8080        (type text, it comes back)
;           python3 test_echo.py     (automated check)
;
;  Exercises: SYS_SOCKET, SYS_SETSOCKOPT, SYS_BIND, SYS_LISTEN, SYS_ACCEPT,
;             SYS_READ, SYS_SENDTO, SYS_CLOSE, SYS_WRITE, SYS_EXIT_GROUP
;             and HTONS(), INADDR_ANY, SOMAXCONN, MSG_NOSIGNAL, -EINTR handling
; =============================================================================

%include "sys_library.inc"

PORT        equ 8080
BUF_SIZE    equ 1024

section .data

server_addr:                            ; struct sockaddr_in
    dw  AF_INET                         ; sin_family
    dw  HTONS(PORT)                     ; sin_port  (network byte order)
    dd  INADDR_ANY                      ; sin_addr  (0.0.0.0)
    dq  0                               ; sin_zero

optval      dd 1                        ; value for SO_REUSEADDR

msg_start   db "Echo server listening on 0.0.0.0:8080", LF
msg_start_len equ $ - msg_start

msg_err     db "error: a syscall failed", LF
msg_err_len equ $ - msg_err

section .bss

buf         resb BUF_SIZE

section .text
global _start

_start:
    ; --- listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) ---------------
    mov     rax, SYS_SOCKET
    mov     rdi, AF_INET
    mov     rsi, SOCK_STREAM
    mov     rdx, IPPROTO_TCP
    syscall
    test    rax, rax
    js      fail
    mov     r12, rax                    ; r12 = listening socket

    ; --- setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, 4) ----------------
    mov     rax, SYS_SETSOCKOPT
    mov     rdi, r12
    mov     rsi, SOL_SOCKET
    mov     rdx, SO_REUSEADDR
    lea     r10, [rel optval]
    mov     r8, 4
    syscall
    test    rax, rax
    js      fail

    ; --- bind(fd, &server_addr, 16) ------------------------------------------
    mov     rax, SYS_BIND
    mov     rdi, r12
    lea     rsi, [rel server_addr]
    mov     rdx, SOCKADDR_IN_SIZE
    syscall
    test    rax, rax
    js      fail

    ; --- listen(fd, SOMAXCONN) -----------------------------------------------
    mov     rax, SYS_LISTEN
    mov     rdi, r12
    mov     rsi, SOMAXCONN
    syscall
    test    rax, rax
    js      fail

    ; --- print banner --------------------------------------------------------
    mov     rax, SYS_WRITE
    mov     rdi, STDOUT
    lea     rsi, [rel msg_start]
    mov     rdx, msg_start_len
    syscall

.accept_loop:
    ; --- client_fd = accept(fd, NULL, NULL) ----------------------------------
    mov     rax, SYS_ACCEPT
    mov     rdi, r12
    xor     esi, esi                    ; don't care about peer address
    xor     edx, edx
    syscall
    cmp     rax, -EINTR                 ; interrupted by a signal? retry
    je      .accept_loop
    test    rax, rax
    js      fail
    mov     r13, rax                    ; r13 = client socket

.echo_loop:
    ; --- n = read(client, buf, BUF_SIZE) -------------------------------------
    mov     rax, SYS_READ
    mov     rdi, r13
    lea     rsi, [rel buf]
    mov     rdx, BUF_SIZE
    syscall
    test    rax, rax
    jle     .close_client               ; 0 = client closed, <0 = error
    mov     r14, rax                    ; r14 = bytes read
    xor     r15d, r15d                  ; r15 = bytes sent so far

.send_loop:                             ; handle partial sends
    mov     rax, SYS_SENDTO
    mov     rdi, r13
    lea     rsi, [rel buf]
    add     rsi, r15                    ; buf + sent
    mov     rdx, r14
    sub     rdx, r15                    ; remaining
    mov     r10, MSG_NOSIGNAL           ; no SIGPIPE if the client vanished
    xor     r8d, r8d                    ; no dest address
    xor     r9d, r9d
    syscall
    test    rax, rax
    jle     .close_client
    add     r15, rax
    cmp     r15, r14
    jb      .send_loop
    jmp     .echo_loop

.close_client:
    mov     rax, SYS_CLOSE
    mov     rdi, r13
    syscall
    jmp     .accept_loop

fail:
    mov     rax, SYS_WRITE
    mov     rdi, STDERR
    lea     rsi, [rel msg_err]
    mov     rdx, msg_err_len
    syscall
    sys_exit EXIT_FAILURE
