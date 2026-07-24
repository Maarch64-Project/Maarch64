.global _start
.text
_start:
    // sys_write(1, msg, 14)
    mov x0, #1          // fd = 1 (stdout)
    ldr x1, =msg        // buf
    mov x2, #14         // count
    mov x8, #64         // __NR_write on arm64 is 64
    svc #0

    // sys_exit(0)
    mov x0, #0          // status = 0
    mov x8, #93         // __NR_exit on arm64 is 93
    svc #0

.data
msg:
    .ascii "Hello, ARM64!\n"
