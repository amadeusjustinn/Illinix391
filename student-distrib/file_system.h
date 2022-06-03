#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "types.h"
#include "lib.h"
#include "system_call.h"

#define FILE_START_POSITION 0	    //Change if need be to change base of the start position
#define FILE_MEMORY_BLOCK_SIZE 4096 // 4kb of mem per block
#define ENTRY_SIZE 64 		    // 64b of mem per entry
#define ENTRY_NAME 32		    // 32b per name of file entry

#define USER_LEVEL_FILE_TYPE 0	    // 0 for a file giving user-level access to RTC
#define DIRECTORY_FILE_TYPE 1	    // 1 for a directory
#define REGULAR_FILE_TYPE 2	        // 2 for a regular file

#define BOOT_BLOCK_RESERVED 52	    //52b reserved in book block
#define MAX_FILES 63		        //max files that the system can hold
#define FIRST_ENTRY_RESERVED 24	    //the first entry holds the 24b from boot
#define INDEX_NUMBER 1023

#define MAX_FD 8

//structs used to manage memory
/*Blocks*/
typedef struct blocks{
	unsigned char data[FILE_MEMORY_BLOCK_SIZE];
} blocks_t;

/* Dentry struct*/
typedef struct dentry{
	unsigned char fileName[ENTRY_NAME];
	unsigned int fileType;
	unsigned int inodeNum;
	unsigned char reserved[FIRST_ENTRY_RESERVED];
} dentry_t;

/* Boot Block struct - depends on the dentry struct*/
typedef struct bootBlock{
	unsigned int entries_number;
	unsigned int inode_number;
	unsigned int block_number;
	unsigned char reserved[BOOT_BLOCK_RESERVED];
	dentry_t block_entires[MAX_FILES];
} boot_block_t;

/* inode struct*/
typedef struct inode{
	unsigned int length;
	unsigned int inode_data[INDEX_NUMBER];
} inode_t;

//Reference Table to points in memory for structs defined above and Global Variables
blocks_t* global_block_t;
dentry_t* global_dentry_t;
boot_block_t* global_boot_block_t;
inode_t* global_inode_t;

unsigned int dir_position;

//functions to be used in the file directory system
extern void file_system_init(unsigned int start_addr);
extern int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
extern int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
extern int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

//file functions
int32_t file_open(const uint8_t * filename);
int32_t file_close(int32_t fd);
int32_t file_read(int32_t fd, void * buf, int32_t nbytes);
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

//directory functions
int32_t directory_open(const uint8_t * filename);
int32_t directory_close(int32_t fd);
int32_t directory_read(int32_t fd, void * buf, int32_t nbytes);
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes);

#endif
