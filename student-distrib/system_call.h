#ifndef SYSTEM_CALL_H
#define SYSTEM_CALL_H
#include "file_system.h"
#include "lib.h"
#include "keyboard.h"
#include "x86_desc.h"
#include "system_call_linkage.h"
#include "paging.h"
#include "rtc.h"
#include "scheduling.h"

#define MAX_CMD_SIZE 32
#define MAX_FILE_NAME 32
#define MAX_FILE_SIZE 4190208 // 1023 x 4096
#define EXE_MAGIC_NUM1 0x7F
#define EXE_MAGIC_NUM2 0x45
#define EXE_MAGIC_NUM3 0x4C
#define EXE_MAGIC_NUM4 0x46
#define BOTTOM_KERNEL 0x800000 // 8 MB
#define PROCESS_SIZE 0x2000 // 8 KB
#define MAX_PROCESS 6
#define PROGRAM_IMAGE 0x08048000
#define DIVIDE_BY_4MB 22
#define FOUR_MB 0x400000
#define VIRTUAL_ADDR 0x08000000  // 128 MB
#define VIDEO_VIRTUAL 0x40000000
#define START_PROGRAM 0x8000000
#define END_PROGRAM 0x8400000
#define FD_TABLE_SIZE 8
#define EIP_BYTE1 24
#define EIP_BYTE2 25
#define EIP_BYTE3 26
#define EIP_BYTE4 27
#define BYTESHIFT1 8
#define BYTESHIFT2 16
#define BYTESHIFT3 24

// Function tables
typedef struct file_operations_table
{
    int32_t (*open)(const uint8_t *filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void *buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void *buf, int32_t nbytes);
} file_operations_table_t;

// File descriptors for each file
typedef struct file_descriptor
{
    file_operations_table_t *file_operations_table_ptr;
    uint32_t inode;
    uint32_t file_position;
    uint32_t flags;
} file_descriptor_t;

// PCB for each process
typedef struct pcb
{
    uint32_t pid;
    int32_t parent_id;
    file_descriptor_t file_descriptor_table[FD_TABLE_SIZE];
    uint32_t saved_esp;
    uint32_t saved_ebp;
    uint32_t parent_saved_esp;
    uint32_t parent_saved_ebp;
    uint8_t active;
    uint8_t arg[MAX_FILE_NAME];
    uint32_t saved_eip;
} pcb_t;

file_descriptor_t *find_pcb(uint8_t fd);

/* command parser before executing */
uint8_t parse_cmd(const uint8_t *args, uint8_t *parsed_cmd);
int32_t exe_check(uint8_t *cmd, uint8_t *buffer);
void create_pcb(int8_t terminal_num);
void map(uint32_t vaddr, uint32_t paddr);
void load_exe_data(uint8_t *buffer, uint32_t length);
uint8_t parse_second_arg(const uint8_t *args);
/* Switches the terminal */
extern int32_t terminal_switch(int32_t terminal_num);
int32_t execute_base_shell(uint8_t terminal_num);
void context_switch(void);

int32_t halt(uint16_t status);
int32_t execute(const uint8_t *command);
int32_t read(int32_t fd, void *buf, int32_t nbytes);
int32_t write(int32_t fd, const void *buf, int32_t nbytes);
int32_t open(const uint8_t *filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t *buf, int32_t nbytes);
int32_t vidmap(uint8_t **screen_start);
int32_t set_handler(int32_t signum, void *handler_address);
int32_t sigreturn(void);

#endif
