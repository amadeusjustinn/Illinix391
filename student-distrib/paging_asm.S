#define ASM 1

# DESCRIPTION: Loads CR3 with the address of the page directory
# INPUTS: paging_directory -- start address of page directory
# OUTPUTS: none
.globl load_page_dir
load_page_dir:
    pushl %ebp
    movl %esp, %ebp

    movl 8(%ebp), %eax
    movl %eax, %cr3

    leave
    ret


# DESCRIPTION: Enables page size extension (PSE) for 4 MiB pages,
#              and sets the paging (PG) and protection (PE) bits of CR0
# INPUTS: none
# OUTPUTS: none
.globl enable_paging
enable_paging:
    movl %cr4, %eax
    orl $0x00000010, %eax
    movl %eax, %cr4

    movl %cr0, %eax
    orl $0x80000001, %eax
    movl %eax, %cr0

    leave
    ret
