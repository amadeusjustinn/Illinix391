#include "system_call.h"

// 1 if pid is in use; 0 if not
int8_t pid_in_use[MAX_PROCESS] = {1, 1, 1, 0, 0, 0};
// Gives the value of the lowest to assign
int32_t lowest_free_pid = 3;
// Number of active processes; base shell counts already
int8_t num_process = 3;

file_operations_table_t stdin_table;
file_operations_table_t stdout_table;
file_operations_table_t rtc_table;
file_operations_table_t dentry_table;
file_operations_table_t file_table;

int32_t halt(uint16_t status)
{
    cli();
    pcb_t * PCB_curr = (pcb_t *)(BOTTOM_KERNEL - (PROCESS_SIZE * (terminals[current_terminal_run].current_pid + 1)));
    int32_t cur_pid_temp = terminals[current_terminal_run].current_pid;
    int32_t parent_pid = PCB_curr->parent_id;
    uint32_t i;
    for (i = 0; i < FD_TABLE_SIZE; i++)
    {
        close(i);
    }
    // Base shell
    if (parent_pid == -1)
    {
        printf("Top Level Shell %d Halt\n", cur_pid_temp);
        execute_base_shell(terminals[current_terminal_run].current_pid);
    }else{
        // Non-base shell
        lowest_free_pid = terminals[current_terminal_run].current_pid;
        num_process--;
        pid_in_use[terminals[current_terminal_run].current_pid] = 0;
        terminals[current_terminal_run].current_pid = parent_pid;
    }
    // Restore parent data (ESP andd EBP)
    uint32_t parent_esp = PCB_curr->parent_saved_esp;
    uint32_t parent_ebp = PCB_curr->parent_saved_ebp;
    PCB_curr->active = 0;
    int j;
    for (j = 0; j < MAX_FILE_NAME; j++){
        PCB_curr->arg[j] = 0;
    }
    // Restore parent paging
    map(VIRTUAL_ADDR, BOTTOM_KERNEL + parent_pid * FOUR_MB);


    // Write parent process’ info back to TSS
    tss.ss0 = KERNEL_DS;
    tss.esp0 = BOTTOM_KERNEL - (PROCESS_SIZE * (parent_pid + 1));
    sti();
    // Jump to execute return address
    asm volatile("                              \n\
            movl %0, %%esp                      \n\
            movl %1, %%ebp                      \n\
            movl %2, %%eax                      \n\
            jmp RETURN                          \n\
            "
                 :
                 : "r"(parent_esp), "r"(parent_ebp), "r"((uint32_t)status)
                 : "eax", "esp", "ebp");

    // halt(256) in idt.c

    // Technically never touched but function needs to return an int32_t
    return 0;
}

/* find_pcb
 *
 *  Input: fd
 *  Output: pointer to file_descriptor
 *          0 if pcb is not found
 *  Description: Goes to the pcb corresponding to the current process id and gets the file descriptor at index fd
 */
file_descriptor_t *find_pcb(uint8_t fd)
{
    if (fd >= 8)
    {
        return 0;
    }
    pcb_t *mem_ptr = (pcb_t *)(BOTTOM_KERNEL - (PROCESS_SIZE * (terminals[current_terminal_run].current_pid + 1)));
    file_descriptor_t *file_descriptor = &mem_ptr->file_descriptor_table[fd];
    return file_descriptor;
}

/* parse_cmd
 *
 *  Input: args, parsed_cmd
 *  Output: -1 if command cannot be parsed
 *           0 if command parsing was successful
 *  Description: Parses args (the unparsed command) and replaces it with parsed_cmd.
 */
uint8_t parse_cmd(const uint8_t *args, uint8_t *parsed_cmd)
{
    uint8_t cmd_start = 0;
    uint8_t cmd_end = 0;
    uint8_t unparsed_cmd_length = 0;
    uint8_t parsed_cmd_length = 0;
    unparsed_cmd_length = strlen((int8_t *)args);

    int i;
    // Parsing leading spaces
    // Modified from: https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
    for (i = 0; i < unparsed_cmd_length; i++)
    {
        if (args[cmd_start] == ' ')
        {
            cmd_start++;
        }
        else
        {
            break;
        }
    }
    cmd_end = cmd_start;
    // Parsing trailing zeroes
    for (i = cmd_start; i < unparsed_cmd_length; i++)
    {
        if (args[cmd_end] != ' ' && args[cmd_end] != '\0' && args[cmd_end] != '\n')
        {
            cmd_end++;
        }
        else
        {
            break;
        }
    }
    parsed_cmd_length = cmd_end - cmd_start;

    if (parsed_cmd_length > MAX_CMD_SIZE)
    {
        return -1;
    }

    // Filling up the new parsed command char array
    for (i = 0; i < parsed_cmd_length; i++)
    {
        parsed_cmd[i] = (uint8_t)args[i + cmd_start];
    }
    if (i != MAX_CMD_SIZE)
    {
        parsed_cmd[i] = '\0';
    }
    return 0;
}

/* parse_second_arg
 *
 *  Input: args, parsed_cmd
 *  Output: -1 if command cannot be parsed
 *           0 if command parsing was successful
 *  Description: Parses the second argument. e.g. cat fish -> fish
 */
uint8_t parse_second_arg(const uint8_t *args)
{
    uint8_t arg1_start = 0;
    uint8_t arg1_end = 0;
    uint8_t arg2_start = 0;
    uint8_t arg2_end = 0;
    uint8_t unparsed_cmd_length = 0;
    uint8_t parsed_arg_length = 0;
    uint8_t parsed_arg[MAX_CMD_SIZE];
    unparsed_cmd_length = strlen((int8_t *)args);

    int i;
    // Parsing leading spaces
    for (i = 0; i < unparsed_cmd_length; i++)
    {
        if (args[arg1_start] == ' ')
        {
            arg1_start++;
        }
        else
        {
            break;
        }
    }
    arg1_end = arg1_start;
    // Parsing trailing zeroes
    for (i = arg1_start; i < unparsed_cmd_length; i++)
    {
        if (args[arg1_end] != ' ' && args[arg1_end] != '\0' && args[arg1_end] != '\n')
        {
            arg1_end++;
        }
        else
        {
            break;
        }
    }

    // Start of parsing second argument
    arg2_start = arg1_end;
    for (i = arg2_start; i < unparsed_cmd_length; i++)
    {
        if (args[arg2_start] == ' ')
        {
            arg2_start++;
        }
        else
        {
            break;
        }
    }

    arg2_end = arg2_start;
    for (i = arg2_start; i < unparsed_cmd_length; i++)
    {
        if (args[arg2_end] != ' ' && args[arg2_end] != '\0' && args[arg2_end] != '\n')
        {
            arg2_end++;
        }
        else
        {
            break;
        }
    }

    parsed_arg_length = arg2_end - arg2_start;
    // Arg too long so only copy 32 chars
    if (parsed_arg_length <= MAX_FILE_NAME){
        // Copy the argument
        for (i = 0; i < parsed_arg_length; i++)
        {
            parsed_arg[i] = (uint8_t)args[i + arg2_start];
        }
    }else{
        // Invalid argument
        for (i = 0; i < parsed_arg_length; i++)
        {
            parsed_arg[i] = '\0';
        }
    }

    if (i != MAX_CMD_SIZE)
    {
        parsed_arg[i] = '\0';
    }

    pcb_t *mem_ptr = (pcb_t *)(BOTTOM_KERNEL - (PROCESS_SIZE * (terminals[current_terminal_run].current_pid + 1)));
    memcpy((char *)mem_ptr->arg, (int8_t *)parsed_arg, strlen((int8_t *)parsed_arg));
    return 0;
}

/* exe_check
 *
 *  Input: cmd, buffer
 *  Output: -1 if command is not an executable file
 *           num_byte, size of the buffer, if cmd is an exe file.
 *  Description: Checks if the cmd is an exe file.
 */
int32_t exe_check(uint8_t *cmd, uint8_t *buffer)
{
    /* 3. File checks */
    dentry_t temp_dentry;
    if (read_dentry_by_name(cmd, &temp_dentry) == -1)
    {
        return -1;
    }
    inode_t *node = global_inode_t + temp_dentry.inodeNum;
    int32_t num_byte = read_data(temp_dentry.inodeNum, 0, buffer, node->length);
    if (num_byte <= 0)
    {
        return -1;
    }
    /* Checking if file is an executable */
    if (buffer[0] != EXE_MAGIC_NUM1 ||
        buffer[1] != EXE_MAGIC_NUM2 ||
        buffer[2] != EXE_MAGIC_NUM3 ||
        buffer[3] != EXE_MAGIC_NUM4)
    {
        return -1;
    }
    return num_byte;
}

/* create_pcb
 *
 *  Input: number of the terminal (0-2); -1 if not a terminal
 *  Output: none
 *  Description: Sets up the initial PCB. Initializes FD table with stdin
 *               and stdout along with 6 empty FDs. Also sets Pid, Parent id,
 *               active flags, and saves esp, ebp
 */
void create_pcb(int8_t terminal_num)
{
    // Sets PID
    pcb_t * mem_ptr;
    // Non-base shell
    if (terminal_num == -1){
        mem_ptr = (pcb_t *)(BOTTOM_KERNEL - (PROCESS_SIZE * (lowest_free_pid + 1)));
        num_process++;
        pid_in_use[lowest_free_pid] = 1;
        mem_ptr->pid = lowest_free_pid;
        mem_ptr->parent_id = terminals[current_terminal_run].current_pid;
    }else{
        // Base shell
        mem_ptr = (pcb_t *)(BOTTOM_KERNEL - (PROCESS_SIZE * (terminal_num + 1)));
        mem_ptr->pid = terminal_num;
        mem_ptr->parent_id = -1;
    }
    terminals[current_terminal_run].current_pid = mem_ptr->pid;
    int i;
    // Finds next available PID
    for (i = 0; i < MAX_PROCESS; i++)
    {
        if (pid_in_use[i] == 0)
        {
            lowest_free_pid = i;
            break;
        }
    }
    // Clear arg
    int j;
    for (j = 0; j < MAX_FILE_NAME; j++){
        mem_ptr->arg[j] = 0;
    }
    // Creates file descriptor table
    // Add fd for stdin
    mem_ptr->file_descriptor_table[0].inode = 0;
    mem_ptr->file_descriptor_table[0].file_position = 0;
    mem_ptr->file_descriptor_table[0].flags = 1;
    mem_ptr->file_descriptor_table[0].file_operations_table_ptr = &stdin_table;
    mem_ptr->file_descriptor_table[0].file_operations_table_ptr->open = &terminal_open;
    mem_ptr->file_descriptor_table[0].file_operations_table_ptr->close = &terminal_close;
    mem_ptr->file_descriptor_table[0].file_operations_table_ptr->read = &terminal_read;
    mem_ptr->file_descriptor_table[0].file_operations_table_ptr->write = 0;

    // Add fd for stdout
    mem_ptr->file_descriptor_table[1].inode = 0;
    mem_ptr->file_descriptor_table[1].file_position = 0;
    mem_ptr->file_descriptor_table[1].flags = 1;
    mem_ptr->file_descriptor_table[1].file_operations_table_ptr = &stdout_table;
    mem_ptr->file_descriptor_table[1].file_operations_table_ptr->open = &terminal_open;
    mem_ptr->file_descriptor_table[1].file_operations_table_ptr->close = &terminal_close;
    mem_ptr->file_descriptor_table[1].file_operations_table_ptr->read = 0;
    mem_ptr->file_descriptor_table[1].file_operations_table_ptr->write = &terminal_write;

    for (i = 2; i < FD_TABLE_SIZE; i++)
    {
        mem_ptr->file_descriptor_table[i].inode = 0;
        mem_ptr->file_descriptor_table[i].file_position = 0;
        mem_ptr->file_descriptor_table[i].flags = 0;
        mem_ptr->file_descriptor_table[i].file_operations_table_ptr = 0;
    }
    // Set to active
    mem_ptr->active = 1;
    return;
}

/* map
 *
 *  Input: vaddr, paddr
 *  Output: none
 *  Description: Helper function that maps a new page between virtual and physical addresses.
 *               Creates a page for the new program.
 */
void map(uint32_t vaddr, uint32_t paddr)
{
    uint32_t page_dir_idx = vaddr >> DIVIDE_BY_4MB;
    page_directory_entry_4M_t temp;
    temp.present = 1;
    temp.read_write = 1;
    temp.user_supervisor = 1;
    temp.write_through = 0;
    temp.cache_disable = 0;
    temp.accessed = 0;
    temp.dirty = 0;
    temp.page_size = 1;
    temp.global = 0;
    temp.available_3 = 0;
    temp.page_attr_table = 0;
    temp.reserved_21_13 = 0;
    temp.bits_31_22 = paddr >> DIVIDE_BY_4MB;

    page_directory[page_dir_idx] = temp.val;

    flush_TLB();
    return;
}

/* load_exe_data
 *
 *  Input: buffer, length
 *  Output: none
 *  Description: puts buffer into program image memory
 */
void load_exe_data(uint8_t *buffer, uint32_t length)
{
    uint8_t *program_ptr = (uint8_t *)PROGRAM_IMAGE;
    memcpy(program_ptr, buffer, length);
    return;
}

/* Steps to execute user level code:
 *  1. Paging helpers (?)
 *  2. Parse commands
 *  3. File checks
 *  4. Create new PCB
 *  5. Setup memory (paging)
 *  6. Read exe data
 *  7. Setup old stack and instruction pointer
 *  8. Switch to user mode (context switch)
 *
 *  Input: Command
 *  Output: -1 if command cannot be executed,
 *           0 - 255 if program executes a halt system call
 *  Description: Creates a virtual address space for execute process.
 */
int32_t execute(const uint8_t *command)
{
    cli();
    if (num_process == MAX_PROCESS)
    {
        return -1;
    }
    // MAGIC NUM: a large buffer that ensures it can fit any execute cmd given.
    uint8_t buffer[100000];
    uint8_t cmd[MAX_CMD_SIZE];
    parse_cmd(command, cmd);
    int32_t num_byte = exe_check(cmd, buffer);
    if (num_byte == -1)
    {
        return -1;
    }

    // Create new PCB
    create_pcb(-1);
    pcb_t *mem_ptr = (pcb_t *)(BOTTOM_KERNEL - (PROCESS_SIZE * (terminals[current_terminal_run].current_pid + 1)));
    
    parse_second_arg(command);

    // Set up paging
    map(VIRTUAL_ADDR, BOTTOM_KERNEL + terminals[current_terminal_run].current_pid * FOUR_MB);
    
    // Load data
    load_exe_data(buffer, num_byte);

    uint32_t byte24 = buffer[EIP_BYTE1];
    uint32_t byte25 = buffer[EIP_BYTE2];
    uint32_t byte26 = buffer[EIP_BYTE3];
    uint32_t byte27 = buffer[EIP_BYTE4];
    uint32_t prog_eip = (byte27 << BYTESHIFT3) | (byte26 << BYTESHIFT2) | (byte25 << BYTESHIFT1) | byte24;
    
    // Save parent esp
    register uint32_t saved_esp asm("esp");
    mem_ptr->parent_saved_esp = saved_esp;
    // Save parent ebp
    register uint32_t saved_ebp asm("ebp");
    mem_ptr->parent_saved_ebp = saved_ebp;
    // Save eip
    mem_ptr->saved_eip = prog_eip;
    // Start context switch to user mode
    context_switch();
    return 0;
}

int32_t execute_base_shell(uint8_t terminal_num)
{
    cli();
    // MAGIC NUM: a large buffer that ensures it can fit any execute cmd given.
    uint8_t buffer[100000];
    uint8_t cmd[MAX_CMD_SIZE];
    uint8_t * command = (uint8_t *) "shell";
    parse_cmd(command, cmd);
    int32_t num_byte = exe_check(cmd, buffer);
    if (num_byte == -1)
    {
        return -1;
    }

    // Create new PCB
    create_pcb(terminal_num);
    pcb_t *mem_ptr = (pcb_t *)(BOTTOM_KERNEL - (PROCESS_SIZE * (terminals[terminal_num].current_pid + 1)));

    parse_second_arg(command);

    // Set up paging
    map(VIRTUAL_ADDR, BOTTOM_KERNEL + terminals[terminal_num].current_pid * FOUR_MB);

    // Load data
    load_exe_data(buffer, num_byte);

    uint32_t byte24 = buffer[EIP_BYTE1];
    uint32_t byte25 = buffer[EIP_BYTE2];
    uint32_t byte26 = buffer[EIP_BYTE3];
    uint32_t byte27 = buffer[EIP_BYTE4];
    uint32_t prog_eip = (byte27 << BYTESHIFT3) | (byte26 << BYTESHIFT2) | (byte25 << BYTESHIFT1) | byte24;
    // Save parent esp
    register uint32_t saved_esp asm("esp");
    mem_ptr->parent_saved_esp = saved_esp;
    // Save parent ebp
    register uint32_t saved_ebp asm("ebp");
    mem_ptr->parent_saved_ebp = saved_ebp;
    // Save eip
    mem_ptr->saved_eip = prog_eip;
    // Start context switch to user mode
    context_switch();
    return 0;
}

void context_switch(void){
    pcb_t *mem_ptr = (pcb_t *)(BOTTOM_KERNEL - (PROCESS_SIZE * (terminals[current_terminal_run].current_pid + 1)));
    // Start context switch to user mode
    tss.ss0 = KERNEL_DS;
    tss.esp0 = BOTTOM_KERNEL - (PROCESS_SIZE * (terminals[current_terminal_run].current_pid + 1));
    sti();
    asm volatile("                              \n\
            pushl $0x002B                       \n\
            pushl $0x83FFFFC                    \n\
            pushfl                              \n\
            pushl $0x0023                       \n\
            pushl %0                            \n\
            iret                                \n\
            RETURN:                             \n\
            leave                               \n\
            ret                                 \n\
            "
                 :
                 : "r"(mem_ptr->saved_eip));
}

/*  Terminal switch; switches the terminal video memory
    Inputs: terminal_num: which terminal to switch to (0-2)
    Outputs: 0 if pass; -1 if fail
*/
int32_t terminal_switch(int32_t terminal_num){
    // Don't switch if same terminal
    if (current_terminal_view == terminal_num){
        return -1;
    }
    // Don't switch for invalid terminal
    if ((terminal_num > 2) || (terminal_num < 0)){
        return -1;
    }
    // Save the current terminal video memory to its backup
    memcpy((uint8_t *) terminal_address[current_terminal_view], (uint8_t *) VIDEO, FOUR_KB_BOUNDARIES);
    // Load the new terminal backup to video memory
    memcpy((uint8_t *) VIDEO, (uint8_t *) terminal_address[terminal_num], FOUR_KB_BOUNDARIES);
    // Switch terminal
    current_terminal_view = terminal_num;
    update_cursor(terminals[current_terminal_view].screen_x, terminals[current_terminal_view].screen_y);
    return 0;
}

/* read
 *
 * reads system call that implements a read of data
 * Inputs: the file descriptor, a buffer to store the read data and the number of bytes to read by using jump table
 * Outputs: num_bytes_read 0 if end of file, -1 if bad data block number or inode
 * Side Effects: Changes buffer
 */
int32_t read(int32_t fd, void *buf, int32_t nbytes)
{
    // Check name and buffer is not NULL
    if (fd >= FD_TABLE_SIZE || fd < 0 || buf == NULL || nbytes < 0)
    {
        return -1;
    }
    file_descriptor_t *file_descriptor_ptr = find_pcb(fd);
    // This fd is not active
    if (file_descriptor_ptr->flags == 0 || file_descriptor_ptr->file_operations_table_ptr->read == 0){
        return -1;
    }
    int32_t num_byte_read = file_descriptor_ptr->file_operations_table_ptr->read(fd, buf, nbytes);
    return num_byte_read;
}

/* write
 *
 * write system call that implements a write of data to buffer from file descriptor fd by using jump table
 * Inputs: the file descriptor, a buffer to store the read data and the number of bytes to read
 * Outputs: num_bytes_write 0 if end of file, -1 if bad data block number or inode
 * Side Effects: Changes buffer
 */
int32_t write(int32_t fd, const void *buf, int32_t nbytes)
{
    // Check name and buffer is not NULL
    if (fd >= FD_TABLE_SIZE || fd < 0 || buf == NULL || nbytes < 0)
    {
        return -1;
    }
    file_descriptor_t *file_descriptor_ptr = find_pcb(fd);
    // This fd is not active
    if (file_descriptor_ptr->flags == 0 || file_descriptor_ptr->file_operations_table_ptr->write == 0){
        return -1;
    }
    int32_t num_byte_write = file_descriptor_ptr->file_operations_table_ptr->write(fd, buf, nbytes);
    return num_byte_write;
}

/* open
 *
 * open system call that checks file type and adds to fd table
 * Inputs: the filename
 * Outputs: the fd index if success in opening, else -1 for error
 * Side Effects: Changes td array
 */
int32_t open(const uint8_t *filename)
{
    int i;
    int fd = -1;
    dentry_t den;
    file_descriptor_t *file_descriptor_ptr;
    // Check if valid input
    if (strlen((int8_t *)filename) == 0 || strlen((int8_t *)filename) > ENTRY_NAME || read_dentry_by_name(filename, &den) == -1)
    {
        return -1;
    }
    // check if the dentry has correct type
    if (den.fileType != 0 && den.fileType != 1 && den.fileType != 2)
    {
        return -1;
    }

    // correct values are used and now we find fd number by traversing
    // MAGIC NUM: 2 is start of array or 3rd index and 8 is the max number of files in array
    for (i = 2; i < FD_TABLE_SIZE; i++)
    {
        file_descriptor_ptr = find_pcb(i);

        if (file_descriptor_ptr->flags == 0)
        {
            fd = i;
            break;
        }
    }
    // if no space found return -1
    if (fd == -1)
    {
        return -1;
    }

    // initialize the descriptor
    file_descriptor_ptr->flags = 1;
    file_descriptor_ptr->file_position = 0;
    // RTC
    if (den.fileType == 0)
    {
        file_descriptor_ptr->inode = 0;
        file_descriptor_ptr->file_operations_table_ptr = &rtc_table;
        file_descriptor_ptr->file_operations_table_ptr->open = &rtc_open;
        file_descriptor_ptr->file_operations_table_ptr->close = &rtc_close;
        file_descriptor_ptr->file_operations_table_ptr->read = &rtc_read;
        file_descriptor_ptr->file_operations_table_ptr->write = &rtc_write;
    }
    // Dentry
    else if (den.fileType == 1)
    {
        file_descriptor_ptr->inode = 0;
        file_descriptor_ptr->file_operations_table_ptr = &dentry_table;
        file_descriptor_ptr->file_operations_table_ptr->open = &directory_open;
        file_descriptor_ptr->file_operations_table_ptr->close = &directory_close;
        file_descriptor_ptr->file_operations_table_ptr->read = &directory_read;
        file_descriptor_ptr->file_operations_table_ptr->write = &directory_write;
    }
    // Regular file
    else
    {
        file_descriptor_ptr->inode = den.inodeNum;
        file_descriptor_ptr->file_operations_table_ptr = &file_table;
        file_descriptor_ptr->file_operations_table_ptr->open = &file_open;
        file_descriptor_ptr->file_operations_table_ptr->close = &file_close;
        file_descriptor_ptr->file_operations_table_ptr->read = &file_read;
        file_descriptor_ptr->file_operations_table_ptr->write = &file_write;
    }

    // call open from jump table
    file_descriptor_ptr->file_operations_table_ptr->open(filename);

    return fd;
}
/* close
 *
 *  Input: fd
 *  Output: -1 if invalid fd
 *           0 if success
 *  Description: Clears the file descriptor at index fd
 */
int32_t close(int32_t fd)
{
    // Fail if fd is out of bounds of array or try to close stdin or stdout
    if (fd >= FD_TABLE_SIZE || fd < 2)
    {
        return -1;
    }
    file_descriptor_t *file_descriptor_ptr = find_pcb(fd);
    // This fd is not active
    if (file_descriptor_ptr->flags == 0){
        return -1;
    }
    file_descriptor_ptr->file_operations_table_ptr = 0;
    file_descriptor_ptr->file_position = 0;
    file_descriptor_ptr->inode = 0;
    file_descriptor_ptr->flags = 0;
    return 0;
}

/* getargs
 *
 *  Input: pointer to buffer to store arguments and the number of bytes to copy
 *  Output: 0 if succuess, else -1 if error
 *  Description: system call to copy arguments from kernal to user space
 */
int32_t getargs(uint8_t *buf, int32_t nbytes)
{
    // check if arguments are valid
    if (buf == NULL || nbytes <= 0)
    {
        return -1;
    }

    // create pointer to the pcb
    pcb_t *mem_ptr = (pcb_t *)(BOTTOM_KERNEL - (PROCESS_SIZE * (terminals[current_terminal_run].current_pid + 1)));
    if (strlen((int8_t *) mem_ptr->arg) == 0){
        return -1;
    }
    // copy the args to buffer
    memcpy(buf, mem_ptr->arg, nbytes);

    return 0;
}

/* vidmap
 *
 *  Input: pointer to address of the start of vid memory
 *  Output: the start of vid memory, else -1 if error
 *  Description: system call to change the screen start address in memory of pointer
 */
int32_t vidmap(uint8_t **screen_start)
{
    uint32_t ss32 = (uint32_t)screen_start;

    // Check if bounds are correct with MAGIC Number for lower and upper bound on video memory
    if (screen_start == NULL || ss32 < START_PROGRAM || ss32 > END_PROGRAM)
    {
        return -1;
    }

    uint32_t pte;
    for (pte = 0; pte < PAGE_TABLE_SIZE; pte++)
    {
        page_table2[pte].present = 0;
        page_table2[pte].read_write = 1;
        page_table2[pte].user_supervisor = 1;
        page_table2[pte].write_through = 0;
        page_table2[pte].cache_disable = 0;
        page_table2[pte].accessed = 0;
        page_table2[pte].dirty = 0;
        page_table2[pte].page_attr_table = 0;
        page_table2[pte].global = 0;
        page_table2[pte].available = 0;
        page_table2[pte].bits_31_12 = ZERO_ADDR;
    }

    page_table2[0].present = 1;
    page_table2[0].bits_31_12 = VIDEO_12;


    uint32_t page_dir_idx = VIDEO_VIRTUAL >> DIVIDE_BY_4MB;   // 1 GB >> 22
    page_directory_entry_4K_t temp;
    temp.present = 1;
    temp.read_write = 1;
    temp.user_supervisor = 1;
    temp.write_through = 0;
    temp.cache_disable = 0;
    temp.accessed = 0;
    temp.available_1 = 0;
    temp.page_size = 0;
    temp.global = 0;
    temp.available_3 = 0;
    temp.bits_31_12 = (uint32_t)page_table2 >> TWELVE; 
    page_directory[page_dir_idx] = temp.val;

    flush_TLB();

    // set the screen_start pointer address with the MAGIC Number for 132 MB found in discussion
    *screen_start = (uint8_t *)(VIDEO_VIRTUAL);

    return VIDEO_VIRTUAL;
}

int32_t set_handler(int32_t signum, void *handler_address)
{
    return 0;
}

int32_t sigreturn(void)
{
    return 0;
}
