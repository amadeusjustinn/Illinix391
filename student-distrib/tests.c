#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "paging.h"
#include "rtc.h"
#include "keyboard.h"
#include "file_system.h"
#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER \
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result) \
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure()
{
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}

/* Checkpoint 1 tests */

/* IDT Test
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test()
{
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i)
	{
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL))
		{
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* Paging Dereference Test
 *
 * Asserts that the dereferencing memory in kernel and video memory
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Halts System
 * Coverage:
 * Files: paging.h/c
 */
int paging_dereference_test()
{
	TEST_HEADER;
	char value;
	char *ptr;
	unsigned int ptr_index;
	// Dereference all of kernel memory
	for (ptr_index = 0x400000; ptr_index < 0x800000; ptr_index++)
	{
		ptr = (char *)ptr_index;
		value = *ptr;
	}
	// Dereference all of video memory
	for (ptr_index = 0xb8000; ptr_index < 0xb9000; ptr_index++)
	{
		ptr = (char *)ptr_index;
		value = *ptr;
	}
	// If any page faults, fail
	return PASS;
}

/* Paging Dereference Test Fail
 *
 * Asserts that the dereferencing memory outside of memory; should page fault
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Halts System
 * Coverage:
 * Files: paging.h/c
 */
int paging_dereference_test_fail()
{
	TEST_HEADER;
	char value;
	char *ptr;
	// Outside of memory
	unsigned int ptr_index = 0x01;
	ptr = (char *)ptr_index;
	value = *ptr;
	// If no page fault, fail
	return FAIL;
}

/* Test cases for each of the exceptions
 *
 * Asserts that each exception thrown prompts blue screen of death
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Halts System with Blue Screen of Death
 * Coverage: Exceptions
 * Files: x86_desc.h/S
 */

/* Test case for divide by 0 */
int divide_by_zero_exp_test()
{
	TEST_HEADER;
	asm volatile("int $0");
	return FAIL;
}
/* Test case for debugging */
int debug_test()
{
	TEST_HEADER;
	asm volatile("int $1");
	return FAIL;
}
/* Test case for non_maskable interrupt */
int non_maskable_interrupt_test()
{
	TEST_HEADER;
	asm volatile("int $2");
	return FAIL;
}
/* Test case for setting breakpoint */
int breakpoint_test()
{
	TEST_HEADER;
	asm volatile("int $3");
	return FAIL;
}
/* Test case for when overflow occures */
int overflow_test()
{
	TEST_HEADER;
	asm volatile("int $4");
	return FAIL;
}
/* Test case for outof bound range */
int bound_range_exceeded_test()
{
	TEST_HEADER;
	asm volatile("int $5");
	return FAIL;
}
/* Test case for invalid opcode */
int invalid_opcode_test()
{
	TEST_HEADER;
	asm volatile("int $6");
	return FAIL;
}
/* Test case for unavailible device */
int device_not_available_test()
{
	TEST_HEADER;
	asm volatile("int $7");
	return FAIL;
}
/* Test case for double fault */
int double_fault_test()
{
	TEST_HEADER;
	asm volatile("int $8");
	return FAIL;
}
/* Test case for segment overrun of coprocessor */
int coprocessor_segment_overrun_test()
{
	TEST_HEADER;
	asm volatile("int $9");
	return FAIL;
}
/* Test case for invalid TSS */
int invalid_TSS_test()
{
	TEST_HEADER;
	asm volatile("int $10");
	return FAIL;
}
/* Test case for invalid segment */
int segment_not_present_test()
{
	TEST_HEADER;
	asm volatile("int $11");
	return FAIL;
}
/* Test case for segment fault */
int stack_segment_fault_test()
{
	TEST_HEADER;
	asm volatile("int $12");
	return FAIL;
}
/* Test case for protection fault */
int general_protection_fault_test()
{
	TEST_HEADER;
	asm volatile("int $13");
	return FAIL;
}
/* Test case for pagefault */
int page_fault_fault_test()
{
	TEST_HEADER;
	asm volatile("int $14");
	return FAIL;
}
/* Test case for floating point exception */
int x87_floating_point_exception_test()
{
	TEST_HEADER;
	asm volatile("int $16");
	return FAIL;
}
/* Test case for alignment check */
int alignment_check_test()
{
	TEST_HEADER;
	asm volatile("int $17");
	return FAIL;
}
/* Test case for machine check*/
int machine_check_test()
{
	TEST_HEADER;
	asm volatile("int $18");
	return FAIL;
}
/* Test case for simd floating point expection*/
int simd_floating_point_exception_test()
{
	TEST_HEADER;
	asm volatile("int $19");
	return FAIL;
}
/* Test case for system call */
int system_call_test()
{
	TEST_HEADER;
	asm volatile("int $128");
	return PASS;
}

/* Test cases for invalid inputs to functions to catch edge cases
 *
 * Asserts that each function handler functions with edge cases and does not accept invalid inputs
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Halts System if invalid input is processed by function
 * Coverage: function inputs
 * Files: all files
 */

/* Test case for invalid input to enable irq */
int enable_irq_test()
{
	TEST_HEADER;
	if (enable_irq(20) == -1){
		return PASS;
	}
	return FAIL;
}
/* Test case for invalid input to disable irq */
int disable_irq_test()
{
	TEST_HEADER;
	if (disable_irq(20) == -1){
		return PASS;
	}
	return FAIL;
}
/* Test case for invalid input to send EOI */
int send_eoi_test()
{
	TEST_HEADER;
	if (send_eoi(20) == -1){
		return PASS;
	}
	return FAIL;
}

/* Checkpoint 2 tests */

// Check if changing frequency and read and write works
// Coverage: rtc open, read, write
void test_rtc_print()
{
	TEST_HEADER;
	unsigned int i, j, freq = 2;
	rtc_open((uint8_t *) "NULL");
	uint8_t * buf;
	for (i = 0; i < 9; i++)
	{
		for (j = 0; j < 20; j++)
		{
			rtc_read(0, buf, 0);
			printf("1");
		}
		freq <<= 1;
		rtc_write(0 ,&freq, 0);
		clear();
	}
	rtc_close(0);
}

// Check freq that is not a power of 2
int test_rtc_invalid_write(){
	TEST_HEADER;
	unsigned int freq = 3;
	rtc_open((uint8_t *) "NULL");
	if (rtc_write(0, &freq, 0) == -1){
		return PASS;
	}
	return FAIL;
}

/* Test case for testing read and write terminal functions; on enter, it will output what user typed in buffer */
void terminal_read_write_test(){
	unsigned char buffer[128];
	while (1)
	{
		int num_char = terminal_read(0, buffer, 128);
		terminal_write(0, buffer, num_char);
	}
}

/* Test ouput NULL character */
int putc_null_test(){
	TEST_HEADER;
	putc('\0');
	return PASS;
}

/* Test terminal write with null characters */
int terminal_write_null_test(){
	TEST_HEADER;
	unsigned char buffer [] = {'\0', '\0'};
	terminal_write(0, buffer, 2);
	return PASS;
}

/* Test terminal write with size bigger than buffer */
int terminal_write_size_test(){
	TEST_HEADER;
	unsigned char buffer [] = {'C', '4'};
	terminal_write(0, buffer, 20);
	return PASS;
}

//FILE TESTS
//Coverage: open, close, and write directory
//test the open functionality of files
int file_open_test(){
	TEST_HEADER;
	if (file_open((uint8_t *)"NULL") == 0){
		return PASS;
	}
	return FAIL;
}

//test the close functionality of files
int file_close_test(){
	TEST_HEADER;
	if (file_close(0) == 0){
		return PASS;
	}
	return FAIL;
}

//test the write functionality of files
int file_write_test(){
	TEST_HEADER;
	uint8_t * buf;
	if (file_write(0, buf, 0) == -1){
		return PASS;
	}
	return FAIL;
}

//test the open functionality of directories
int directory_open_test(){
	TEST_HEADER;
	if (directory_open((uint8_t *)"NULL") == 0){
		return PASS;
	}
	return FAIL;
}

//test the close functionality of directories
int directory_close_test(){
	TEST_HEADER;
	if (directory_close(0) == 0){
		return PASS;
	}
	return FAIL;
}

//test the write functionality of directories
int directory_write_test(){
	TEST_HEADER;
	uint8_t * buf;
	if (directory_write(0, buf, 0) == -1){
		return PASS;
	}
	return FAIL;
}

//test to print the directory files to the terminal as a list
//Coverage: open, close, read, and write directory
int directory_read_test(){
	TEST_HEADER;
	unsigned int i;
	for (i = 0; i < 62; i++){
		int temp = i;
		unsigned char buffer [32];
		if (directory_read(0, buffer, 0) == 0){
			return PASS;
		}
		i = temp;
		terminal_write(0, buffer, 32);
		buffer [0] = '\n';
		terminal_write(0, buffer, 1);
	}
	return PASS;
}

//test to print the file to the terminal as a list
//Coverage: open, close, read, and write file
int file_read_test1(){
	TEST_HEADER;
	create_pcb(0);
	char * fname = "frame0.txt";
	int32_t file_fd = open((uint8_t *) fname);
	unsigned char buffer[400];
	unsigned int num = file_read(file_fd, buffer, 400);
	terminal_write(file_fd, buffer, num);
	close(file_fd);
	return PASS;
}

//test the file system for the length of the larger name of files
//Coverage: open, close, read, and write directory
int file_read_test2(){
	TEST_HEADER;
	create_pcb(0);
	//char * fname = "verylargetextwithverylongname.tx";
	char * fname = "fish";
	int32_t file_fd = open((uint8_t *) fname);
	unsigned char buffer[40000];
	unsigned int num = file_read(file_fd, buffer, 40000);
	terminal_write(file_fd, buffer, num);
	close(file_fd);
	return PASS;
}

/* Checkpoint 3 tests */
// Check for invalid input into PCB
int find_pcb_test(){
	TEST_HEADER;
	if (find_pcb(9) == 0){
		return PASS;
	}
	return FAIL;
}

/* Test if parsing cmd works*/
int parse_cmd_test(){
	TEST_HEADER;
	uint8_t cmd[MAX_CMD_SIZE];
	if (parse_cmd((uint8_t *) "  shell  ", cmd) != 0){
		return FAIL;
	}
	if (strncmp((int8_t *) "shell", (int8_t *) cmd, 5) == 0){
		return PASS;
	}
	return FAIL;
}

/* Check if text fail is exe */
int exe_check_test_fail(){
	TEST_HEADER;
	uint8_t buffer[10000];
	int8_t * cmd = "frame0.txt";
	if (exe_check((uint8_t *) cmd, buffer) == -1){
		return PASS;
	}
	return FAIL;
}

/* Check if shell is exe */
int exe_check_test_pass(){
	TEST_HEADER;
	uint8_t buffer[10000];
	int8_t * cmd = "shell";
	if (exe_check((uint8_t *) cmd, buffer) > 0){
		return PASS;
	}
	return FAIL;
}

//Check that open and close works
int open_close_test(){
	TEST_HEADER;
	create_pcb(0);
	char * fname = "verylargetextwithverylongname.tx";
	//char * fname = "fish";
	int32_t file_fd = open((uint8_t *) fname);
	file_descriptor_t * fd = find_pcb(file_fd);
	if (fd->file_operations_table_ptr->open != file_open){
		return FAIL;
	}
	if (fd->file_operations_table_ptr->close != file_close){
		return FAIL;
	}
	if (fd->file_operations_table_ptr->read != file_read){
		return FAIL;
	}
	if (fd->file_operations_table_ptr->write != file_write){
		return FAIL;
	}
	if (fd->flags != 1){
		return FAIL;
	}
	close(file_fd);
	if (fd->file_operations_table_ptr != 0){
		return FAIL;
	}
	if (fd->flags != 0){
		return FAIL;
	}
	return PASS;
}

// Test that read/write works
int read_write_test(){
	TEST_HEADER;
	create_pcb(0);
	char * fname = "frame1.txt";
	int32_t file_fd = open((uint8_t *) fname);
	unsigned char buffer[400];
	unsigned int num = read(file_fd, buffer, 400);
	write(1, buffer, num);
	close(file_fd);
	return PASS;
}

/* Checkpoint 4 tests */
// Test get arg
int get_arg_test(){
	TEST_HEADER;
	create_pcb(0);
	uint8_t * fname = (uint8_t *) "cat fish";
	parse_second_arg(fname);
	uint8_t buf[1024];
	getargs(buf, 4);
	if (strncmp((int8_t *)buf, (int8_t *)"fish", 4) == 0){
		return PASS;
	}
	return FAIL;
}

/* Checkpoint 5 tests */

/* Paging Structure Test
 *
 * Asserts that the paging structure and kernal space is correct
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Halts System
 * Coverage:
 * Files: paging.h/c
 */
int paging_structure_test()
{
	TEST_HEADER;
	if (test_page_structure() == 1)
	{
		return FAIL;
	}
	return PASS;
}

/* Terminal switch test
 *
 * Test terminal swtich with invalid input
 * Inputs: None
 * Outputs: PASS/FAIL
 */
int terminal_switch_test()
{
	TEST_HEADER;
	if (terminal_switch(4) == -1){
		return PASS;
	}
	return FAIL;
}

/* Test suite entry point */
void launch_tests()
{
	// Checkpoint 1 Tests: Uncomment the ones that you want to test
	/* IDT TEST */
	// TEST_OUTPUT("idt_test", idt_test());

	/* EXCEPTION TESTS; Doesn't work due to the fact that they halt instead of while loop */
	// TEST_OUTPUT("divide_by_zero_exp_test", divide_by_zero_exp_test());
	// TEST_OUTPUT("non_maskable_interrupt_test", non_maskable_interrupt_test());
	// TEST_OUTPUT("breakpoint_test", breakpoint_test());
	// TEST_OUTPUT("overflow_test", overflow_test());
	// TEST_OUTPUT("debug_test", debug_test());
	// TEST_OUTPUT("bound_range_exceeded_test", bound_range_exceeded_test());
	// TEST_OUTPUT("invalid_opcode_test", invalid_opcode_test());
	// TEST_OUTPUT("device_not_available_test", device_not_available_test());
	// TEST_OUTPUT("double_fault_test", double_fault_test());
	// TEST_OUTPUT("coprocessor_segment_overrun_test", coprocessor_segment_overrun_test());
	// TEST_OUTPUT("invalid_TSS_test", invalid_TSS_test());
	// TEST_OUTPUT("segment_not_present_test", segment_not_present_test());
	// TEST_OUTPUT("stack_segment_fault_test", stack_segment_fault_test());
	// TEST_OUTPUT("general_protection_fault_test", general_protection_fault_test());
	// TEST_OUTPUT("page_fault_fault_test", page_fault_fault_test());
	// TEST_OUTPUT("x87_floating_point_exception_test", x87_floating_point_exception_test());
	// TEST_OUTPUT("alignment_check_test", alignment_check_test());
	// TEST_OUTPUT("machine_check_test", machine_check_test());
	// TEST_OUTPUT("simd_floating_point_exception_test", simd_floating_point_exception_test());

	/* SYSTEM CALL TEST */
	// TEST_OUTPUT("system_call_test", system_call_test());

	/* PIC TESTS */
	// TEST_OUTPUT("enable irq test", enable_irq_test());
	// TEST_OUTPUT("disable irq test", disable_irq_test());
	// TEST_OUTPUT("send eoi test", send_eoi_test());

	/* RTC TEST */
	// test_rtc_print();
	// TEST_OUTPUT("invalid frequency rtc test", test_rtc_invalid_write());

	/* PAGING TEST */
	// TEST_OUTPUT("paging_dereference_test", paging_dereference_test());
	// TEST_OUTPUT("paging_dereference_test_fail", paging_dereference_test_fail());
	
	/* TERMINAL TEST */
	// terminal_read_write_test();
	// TEST_OUTPUT("putc null character test", putc_null_test());
	// TEST_OUTPUT("terminal write null character test", terminal_write_null_test());
	// TEST_OUTPUT("terminal write size test", terminal_write_size_test());

	/*FILE TEST*/
	// TEST_OUTPUT("print the directory list", directory_read_test());
	// TEST_OUTPUT("print the file list", file_read_test1());
	// TEST_OUTPUT("print the file list2", file_read_test2());
	// TEST_OUTPUT("test file_open", file_open_test());
	// TEST_OUTPUT("test file close", file_close_test());
	// TEST_OUTPUT("test file write", file_write_test());
	// TEST_OUTPUT("test directory open", directory_open_test());
	// TEST_OUTPUT("test directory close", directory_close_test());
	// TEST_OUTPUT("test directory write", directory_write_test());

	/* PCB TEST */
	// TEST_OUTPUT("invalid fd into find pcb", find_pcb_test());

	/* EXECUTE TEST */
	// TEST_OUTPUT("parse cmd", parse_cmd_test());
	// TEST_OUTPUT("non executable into check exe", exe_check_test_fail());
	// TEST_OUTPUT("non executable into check exe", exe_check_test_pass());

	/* SYSTEM CALL TEST */
	// TEST_OUTPUT("open/close test", open_close_test());
	// TEST_OUTPUT("read/write test", read_write_test());
	// TEST_OUTPUT("get arg test", get_arg_test());

	/* PAGE MAPPING TEST */
	// TEST_OUTPUT("paging_structure_test", paging_structure_test());

	/* TERMINAL SWITCH TEST */
	// TEST_OUTPUT("terminal_switch_test", terminal_switch_test());
}
