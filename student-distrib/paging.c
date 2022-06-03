#include "paging.h"

/**
 * @brief Initialises page directory and page table containing video memory
 *
 */
void page_init(void)
{
    // Page table and page directory iterator variables
    uint32_t pte, pde;

    // Set rest of page table as not present and having null physical address
    for (pte = 0; pte < PAGE_TABLE_SIZE; pte++)
    {
        page_table[pte].present = 0;
        page_table[pte].read_write = 1;
        page_table[pte].user_supervisor = 0;
        page_table[pte].write_through = 0;
        page_table[pte].cache_disable = 0;
        page_table[pte].accessed = 0;
        page_table[pte].dirty = 0;
        page_table[pte].page_attr_table = 0;
        page_table[pte].global = 0;
        page_table[pte].available = 0;
        page_table[pte].bits_31_12 = ZERO_ADDR;
    }

    // Initialise video memory as present and set physical memory correctly
    page_table[VIDEO_12].present = 1;
    page_table[VIDEO_12].bits_31_12 = VIDEO_12;

    // Initialize terminal video memory backup
    page_table[TERMINAL_0_VIDEO_12].present = 1;
    page_table[TERMINAL_0_VIDEO_12].bits_31_12 = TERMINAL_0_VIDEO_12;
    page_table[TERMINAL_1_VIDEO_12].present = 1;
    page_table[TERMINAL_1_VIDEO_12].bits_31_12 = TERMINAL_1_VIDEO_12;
    page_table[TERMINAL_2_VIDEO_12].present = 1;
    page_table[TERMINAL_2_VIDEO_12].bits_31_12 = TERMINAL_2_VIDEO_12;

    // Set rest of page directory as not present and as mapping to 4 MiB tables
    for (pde = 2; pde < PAGE_DIRECTORY_SIZE; pde++)
    {
        page_directory_entry_4M_t temp;
        temp.present = 0;
        temp.read_write = 0;
        temp.user_supervisor = 0;
        temp.write_through = 0;
        temp.cache_disable = 0;
        temp.accessed = 0;
        temp.dirty = 0;
        temp.page_size = 1;
        temp.global = 0;
        temp.available_3 = 0;
        temp.page_attr_table = 0;
        temp.reserved_21_13 = 0;
        temp.bits_31_22 = ZERO_ADDR;
        page_directory[pde] = temp.val;
    }

    // Set first 4 MiB of memory (0th index, containing video memory)
    // as present and as mapping to 4 KiB tables, and connect the page table
    // that was initialised earlier
    page_directory_entry_4K_t temp_video;
    temp_video.present = 1;
    temp_video.read_write = 1;
    temp_video.user_supervisor = 0;
    temp_video.write_through = 0;
    temp_video.cache_disable = 0;
    temp_video.accessed = 0;
    temp_video.available_1 = 0;
    temp_video.page_size = 0;
    temp_video.global = 0;
    temp_video.available_3 = 0;
    temp_video.bits_31_12 = (uint32_t)page_table >> TWELVE;
    page_directory[0] = temp_video.val;

    // Set kernel memory (1st index) as present and as mapping to
    // 4 MiB page tables
    page_directory_entry_4M_t temp_kernel;
    temp_kernel.present = 1;
    temp_kernel.read_write = 1;
    temp_kernel.user_supervisor = 0;
    temp_kernel.write_through = 0;
    temp_kernel.cache_disable = 0;
    temp_kernel.accessed = 0;
    temp_kernel.dirty = 0;
    temp_kernel.page_size = 1;
    temp_kernel.global = 0;
    temp_kernel.available_3 = 0;
    temp_kernel.page_attr_table = 0;
    temp_kernel.reserved_21_13 = 0;
    temp_kernel.bits_31_22 = 1;
    page_directory[1] = temp_kernel.val;

    // Stores the address of each terminal buffer; used in scheduling
    terminal_address[0] = TERMINAL_0_VIDEO;
    terminal_address[1] = TERMINAL_1_VIDEO;
    terminal_address[2] = TERMINAL_2_VIDEO;

    // Load page directory to be enabled
    load_page_dir(page_directory);
    enable_paging();
}

/**
 * @brief Tests the values in the table and directory
 *
 * @return 0 upon success, 1 upon of failure
 */
uint32_t test_page_structure(void)
{
    // Page table and page directory iterator variables
    uint32_t pte, pde;

    // Check all segments of page table
    for (pte = 0; pte < PAGE_TABLE_SIZE; pte++)
    {
        // If not video memory, should be not present
        if (pte != VIDEO_12 && pte != TERMINAL_0_VIDEO_12 && pte != TERMINAL_1_VIDEO_12 && pte != TERMINAL_2_VIDEO_12)
        {
            if (page_table[pte].present != 0)
            {
                return 1;
            }
        }
        else
        {
            // Video memory is present and address should be video
            if (page_table[pte].present != 1)
            {
                return 1;
            }
        }
    }
    // Check all the tables in directory
    for (pde = 2; pde < PAGE_DIRECTORY_SIZE; pde++)
    {
        // The other tables should be not present and page size in MB
        if (page_directory[pde] != 128)
        {
            return 1;
        }
    }
    // Should be present for table 0 and table 1
    if ((page_directory[0] & 0x1) != 0x1)
    {
        return 1;
    }
    if ((page_directory[1] & 0x1) != 0x1)
    {
        return 1;
    }
    // Should be read write for table 0 and table 1
    if ((page_directory[0] & 0x2) != 0x2)
    {
        return 1;
    }
    if ((page_directory[1] & 0x2) != 0x2)
    {
        return 1;
    }
    // Page size of table 0 is in KB
    if ((page_directory[0] & 0x80) != 0x0)
    {
        return 1;
    }
    // Page size of table 1 is in MB
    if ((page_directory[1] & 0x80) != 0x80)
    {
        return 1;
    }
    return 0;
}
