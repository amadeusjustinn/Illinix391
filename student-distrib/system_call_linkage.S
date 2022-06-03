#define ASM 1

.globl halt
.globl execute
.globl read
.globl write
.globl open
.globl close
.globl getargs
.globl vidmap
.globl set_handler
.globl sigreturn

.globl system_call_link
system_call_link:
    cli
    cmpl $1, %eax     # Check if system call # is less than 1
    jl fail
    cmpl $10, %eax    # Check if system call # is greater than 10
    jg fail
    # Push the arguments to the system call in order
    pushl %ebp
    pushl %edi
    pushl %esi

    pushl %edx
    pushl %ecx
    pushl %ebx
    sti
    # Call the system call corresponding to # in EAX
    call *jump_table(, %eax, 4)
    cli
    # Discard all arguments
    addl $12, %esp
    popl %esi
    popl %edi
    popl %ebp
    sti
    iret

fail:
    # Return -1 on fail
    movl $-1, %eax
    sti
    iret

jump_table:
	.long 0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

# Flushes the TLB
.globl flush_TLB
flush_TLB:
    movl %cr3, %eax
    movl %eax, %cr3
    ret
