.text

.globl _start

_start:
	xor %rbp, %rbp
	mov %rsp, %rdi
	call _entry