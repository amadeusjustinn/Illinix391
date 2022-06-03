#ifndef PAGING_H
#define PAGING_H

#include "lib.h"
#include "paging_asm.h"

#define TWELVE 12                  // 12-bit offset used for fitting addresses into bits 31 to 22 of PTE and PDE
#define KERNEL 0x1000              // Kernel starts at 4 MB = 16^3 KB
#define KERNEL_12 KERNEL >> TWELVE // Location of kernel memory in paging table (KERNEL / 4096 = KERNEL >> 12)
#define VIDEO 0xB8000              // Physical address of video memory
#define VIDEO_12 VIDEO >> TWELVE   // Location of video memory in paging table (VIDEO / 4096 = VIDEO >> 12)
#define ZERO_ADDR 0x00000          // 20-bit zero address for unused PTEs and PDEs
#define PAGE_DIRECTORY_SIZE 1024   // Size of page directory (4 GiB / 4 MiB)
#define PAGE_TABLE_SIZE 1024       // Size of page table (4 MiB / 4 KiB)
#define FOUR_KB_BOUNDARIES 4096    // Pages need to be aligned on 4 KiB boundaries

// Backup video memory for the terminals
#define TERMINAL_0_VIDEO 0xB9000 // VIDEO + 4KB
#define TERMINAL_1_VIDEO 0xBA000 // VIDEO + 8KB
#define TERMINAL_2_VIDEO 0xBB000 // VIDEO + 12KB
#define TERMINAL_0_VIDEO_12 TERMINAL_0_VIDEO >> TWELVE
#define TERMINAL_1_VIDEO_12 TERMINAL_1_VIDEO >> TWELVE
#define TERMINAL_2_VIDEO_12 TERMINAL_2_VIDEO >> TWELVE

/* Number of terminals */
#define NUM_TERMINALS 3

// Backup video memory addresses for each terminal
uint32_t terminal_address[NUM_TERMINALS];

/* A 4 KiB page directory entry (goes into the 0th index of the page directory) */
typedef union page_directory_entry_4K_t
{
    uint32_t val;
    struct
    {
        uint8_t present : 1;
        uint8_t read_write : 1;
        uint8_t user_supervisor : 1;
        uint8_t write_through : 1;
        uint8_t cache_disable : 1;
        uint8_t accessed : 1;
        uint8_t available_1 : 1;
        uint8_t page_size : 1;
        uint8_t global : 1;
        uint8_t available_3 : 3;
        uint32_t bits_31_12 : 20;
    } __attribute__((packed));
} page_directory_entry_4K_t;

/* A 4 MiB page directory entry (for rest of page directory) */
typedef union page_directory_entry_4M_t
{
    uint32_t val;
    struct
    {
        uint8_t present : 1;
        uint8_t read_write : 1;
        uint8_t user_supervisor : 1;
        uint8_t write_through : 1;
        uint8_t cache_disable : 1;
        uint8_t accessed : 1;
        uint8_t dirty : 1;
        uint8_t page_size : 1;
        uint8_t global : 1;
        uint8_t available_3 : 3;
        uint8_t page_attr_table : 1;
        uint16_t reserved_21_13 : 9;
        uint16_t bits_31_22 : 10;
    } __attribute__((packed));
} page_directory_entry_4M_t;

/* A page table entry (goes into a page table) */
typedef union page_table_entry_t
{
    uint32_t val;
    struct
    {
        uint8_t present : 1;
        uint8_t read_write : 1;
        uint8_t user_supervisor : 1;
        uint8_t write_through : 1;
        uint8_t cache_disable : 1;
        uint8_t accessed : 1;
        uint8_t dirty : 1;
        uint8_t page_attr_table : 1;
        uint8_t global : 1;
        uint8_t available : 3;
        uint32_t bits_31_12 : 20;
    } __attribute__((packed));
} page_table_entry_t;

/* Page directory (array of 1024 PDEs) and page table (array of 1024 PTEs) */
uint32_t page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(FOUR_KB_BOUNDARIES)));
page_table_entry_t page_table[PAGE_TABLE_SIZE] __attribute__((aligned(FOUR_KB_BOUNDARIES)));
page_table_entry_t page_table2[PAGE_TABLE_SIZE] __attribute__((aligned(FOUR_KB_BOUNDARIES)));

/* Initialises page directory and page table containing video memory */
extern void page_init(void);

/* Tests the values in the table and directory */
extern uint32_t test_page_structure(void);

#endif
