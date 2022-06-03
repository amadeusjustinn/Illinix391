#include "file_system.h"

// -------------------- FUNCTIONS TO BE USED IN THE FILE DIRECTORY SYSTEM --------------------

/**
 * @brief Setup for the file system (Called when booting)
 *        Sets up the memory for the file system along with the structs with information
 * 
 * @param start_addr Start address of file system (where boot block is to reside)
 */
void file_system_init(unsigned int start_addr){
	//Initialize Pointers based on Apendix A image of File System
	global_boot_block_t = (boot_block_t*)(start_addr);
	global_dentry_t = (dentry_t*)&(global_boot_block_t->block_entires);
	global_inode_t = (inode_t*)(start_addr + FILE_MEMORY_BLOCK_SIZE);
	global_block_t =(blocks_t*)(start_addr + (FILE_MEMORY_BLOCK_SIZE * (global_boot_block_t->inode_number + 1)));
}

/**
 * @brief Reads from file with given name and transfers data to given destination
 * 
 * @param fname File name (source)
 * @param dentry Data entry (destination)
 * @return 0 upon success, -1 otherwise 
 */
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry){
	uint32_t i;
	int length_file_name = strlen((char*)fname);
	//check for valid arguments with overlow with Magic Number 0
	if(fname == NULL || dentry == NULL || length_file_name < 0 || length_file_name > ENTRY_NAME){
		return -1;
	}

	// Loop through the blocks in boot to compare and fill in block if found
	// Use functions that have been given such as strcomp and strncpy
	for(i = 0; i < global_boot_block_t->entries_number;i++){
		//check if the values are the same
		if(strncmp((char*)fname, (char*)global_boot_block_t->block_entires[i].fileName, ENTRY_NAME) == 0){
			//copy over name, fileType and inodeNum
			memcpy((char*)dentry->fileName, (char*)global_boot_block_t->block_entires[i].fileName, ENTRY_NAME);
			dentry->fileType = global_boot_block_t->block_entires[i].fileType;
			dentry->inodeNum = global_boot_block_t->block_entires[i].inodeNum;
			return 0;
		}
	}

	// Name not found, so there is an error in reading
	return -1;
}

/**
 * @brief Reads from file with given index and transfers data to given destination
 * 
 * @param index File index (source)
 * @param dentry Data entry (destination)
 * @return 0 upon success, -1 otherwise 
 */
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry){
	uint32_t i = index;
	//check if index fits the constraints
	if (i >= global_boot_block_t->entries_number || i < 0){
		return -1;
	}

	// Copy over name, fileType and inodeNum (same code as read by name)
	memcpy((char*)dentry->fileName, (char*)global_boot_block_t->block_entires[i].fileName, ENTRY_NAME);
	dentry->fileType = global_boot_block_t->block_entires[i].fileType;
	dentry->inodeNum = global_boot_block_t->block_entires[i].inodeNum;
	return 0;
}

/**
 * @brief Reads from memory and copies file information to buffer
 *        Fills buffer with data read
 * 
 * @param inode Inode number of file
 * @param offset Offset within buffer
 * @param buf Buffer to contain data read
 * @param length Number of bytes to read (theoretical)
 * @return Number of bytes to read, 0 if end of file, -1 if bad data block number or inode 
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
	if (inode > global_boot_block_t->inode_number){
		return -1;
	}
	uint32_t i;
	inode_t* node = global_inode_t + inode;
	if (offset >= node->length){
		return 0;
	}
	uint32_t block_index = offset/FILE_MEMORY_BLOCK_SIZE;
	uint32_t num_bytes_read = 0;
	uint32_t buffer_index = 0;
	// 1st data block
	uint32_t block_num = node->inode_data[block_index];
	if (block_num > global_boot_block_t->block_number){
		return -1;
	}
	blocks_t* first_block = global_block_t + block_num;
	// If read only 1 block
	if (offset%FILE_MEMORY_BLOCK_SIZE + length <= FILE_MEMORY_BLOCK_SIZE){
		buffer_index = length;
	}else{
		buffer_index += FILE_MEMORY_BLOCK_SIZE - (offset%FILE_MEMORY_BLOCK_SIZE);
	}
	memcpy((int8_t *) buf, (int8_t *) first_block + (offset%FILE_MEMORY_BLOCK_SIZE), buffer_index);
	num_bytes_read += buffer_index;
	uint32_t cur_length = length - num_bytes_read;
	uint32_t length_left_to_read = cur_length;
	if (length_left_to_read == 0){
		return num_bytes_read;
	}
	// Iterate through each data block and check if condition is met
	// Copy data over to buffer if met
	for (i = 1; i <= (1 + (length_left_to_read/FILE_MEMORY_BLOCK_SIZE)); i++){
		uint32_t block_num = node->inode_data[block_index+i];
		if (block_num > global_boot_block_t->block_number){
			return -1;
		}
		blocks_t* data_block = global_block_t + block_num;
		if (i < (1 + (length_left_to_read/FILE_MEMORY_BLOCK_SIZE))){
			memcpy((int8_t *) (buf + buffer_index + (i-1)*FILE_MEMORY_BLOCK_SIZE), (int8_t *) data_block, FILE_MEMORY_BLOCK_SIZE);
			num_bytes_read += FILE_MEMORY_BLOCK_SIZE;
			cur_length -= FILE_MEMORY_BLOCK_SIZE;
		}else if (i == (1 + (length_left_to_read/FILE_MEMORY_BLOCK_SIZE))){
			memcpy((int8_t *) buf + buffer_index + ((i-1)*FILE_MEMORY_BLOCK_SIZE), (int8_t *) data_block, cur_length);
			num_bytes_read += cur_length;
		}
	}
	return num_bytes_read;
}

// ------------------------------ FILE FUNCTIONS ------------------------------

/**
 * @brief Opens directory for writing
 * 	      (Since file system is read-only, nothing is written)
 * 
 * @param filename Name of file to be written
 * @return 0 upon success, -1 otherwise 
 */
int32_t file_open(const uint8_t * filename){
	return 0;
}

/**
 * @brief Closes directory after writing
 * 		  (Since file system is read-only, nothing is written)
 * 
 * @param fd Index
 * @return 0 upon success, -1 otherwise 
 */
int32_t file_close(int32_t fd){
	return 0;
}

/**
 * @brief Reads from file with the given index
 * 		  Fills buffer with the file to be printed
 * 
 * @param fd Index
 * @param buf Buffer to write to
 * @param nbytes Length of buffer
 * @return Number of bytes read, 0 for end of file, -1 for fail 
 */
int32_t file_read(int32_t fd, void * buf, int32_t nbytes){
	// Check name and buffer is not NULL
	if(fd >= MAX_FD || fd < 0 || buf == NULL){
		return -1;
	}

	file_descriptor_t * file_descriptor = find_pcb(fd);
	int32_t num_byte_read = read_data(file_descriptor->inode, file_descriptor->file_position, buf, nbytes);
	file_descriptor->file_position += num_byte_read;
	return num_byte_read;
}

/**
 * @brief Writes to file
 *        (Since file system is read-only, nothing is written)
 * 
 * @param fd Index
 * @param buf Buffer to read from
 * @param nbytes Length of buffer
 * @return 0 upon success, -1 otherwise 
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes){
	return -1;
}

// ------------------------------ DIRECTORY FUNCTIONS ------------------------------

/**
 * @brief Opens directory for writing
 * 	      (Since file system is read-only, nothing is written)
 * 
 * @param filename Name of file to be opened
 * @return 0 upon success, -1 otherwise 
 */
int32_t directory_open(const uint8_t * filename){
	dir_position = 0;
	return 0;
}

/**
 * @brief Closes directory after writing
 * 		  (Since file system is read-only, nothing is written)
 * 
 * @param fd Index
 * @return 0 upon success, -1 otherwise 
 */
int32_t directory_close(int32_t fd){
	return 0;
}

/**
 * @brief Reads a dentry
 *		  Fills buffer with the directory to be printed
 * 
 * @param fd Index
 * @param buf Buffer to write to
 * @param nbytes Length of buffer
 * @return Number of characters in dentry name (0 upon failure) 
 */
int32_t directory_read(int32_t fd, void * buf, int32_t nbytes){
	uint8_t * buf_ptr = (uint8_t *) buf;
	if (dir_position > global_boot_block_t->entries_number){
		return 0;
	}

	uint32_t i;
	for (i = 0; i < ENTRY_NAME; i++){
		buf_ptr[i] = 0;
	}

	// Find the dentry by index and use the info in dentry to update the buffer
	dentry_t den;
	if (read_dentry_by_index(dir_position, &den) == -1){
		return 0;
	}

	memcpy((uint8_t *) buf, (int8_t *) den.fileName, strlen((int8_t*) den.fileName));
	dir_position++;

	// For very large text
	if (strlen((int8_t*) den.fileName) > ENTRY_NAME){
		return ENTRY_NAME;
	}

	return strlen((int8_t*) den.fileName);
}

/**
 * @brief Writes to a directory
 * 		  (Since file system is read-only, nothing is written)
 * 
 * @param fd Index
 * @param buf Buffer to write to
 * @param nbytes Length of buffer
 * @return 0 upon success, -1 otherwise
 */
int32_t directory_write(int32_t fd, const void* buf, int32_t nbytes){
	return -1;
}

