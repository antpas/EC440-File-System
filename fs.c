#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "disk.h"
#include "fs.h"

#define I_SIZE sizeof(int)
#define C_SIZE sizeof(char)
#define P_SIZE sizeof(void*)

// Struct for files
struct file {
	int file_id;
	int file_size;
	int file_offset;
	char file_name[16];
	int num_blocks;
	struct block* first_block;
};

// Struct for blocks
struct block{
	struct block* next;
	int addr;
};

// Keeps track of if filesystem is open
int open_f = 0;

// Array to count used blocks
static char blocks[8192];
// Array to hold all the files
static struct file all_files[64];
// Array to hold file descriptor pointers
static struct file* all_fildes[32];

// Creates a filesystem and writes initial metadata to disk
int make_fs(char *disk_name)
{

	// Create disk
	if(make_disk(disk_name) == -1)
		return -1;

	if(mount_fs(disk_name) == -1)
		return -1;

	// Clear file array
	int ii;
	for(ii = 0; ii<64; ii++)
	{
		all_files[ii].file_id = -1;
		all_files[ii].file_size = 0;
		all_files[ii].file_offset = 0;
		memset(all_files[ii].file_name, 0, 16);
		all_files[ii].num_blocks = 0;
		all_files[ii].first_block = NULL;
	}
	// Clear file descriptor array
	for(ii = 0; ii<32; ii++)
	{
		all_fildes[ii] = NULL;
	}

	// Write metadata to disk
	if(umount_fs(disk_name) == -1)
		return -1;

	return 0;
} 

// Mounts a filesystem and loads metadata to data structures
int mount_fs(char *disk_name)
{
	if(open_disk(disk_name) != 0)
		return -1;

	open_f = 1;

	// Create buffer to use for reading
	char buff[4096];
	memset(buff, 0, 4096);

	// Read back the fildes structure from the first block
	block_read(1, buff);
	int foo[32];
	int ii;
	for(ii = 0; ii < 32; ii++)
	{
		all_fildes[ii] = NULL;
		memcpy(&foo[ii], buff + 1 + ii*I_SIZE, I_SIZE);
	}

	// Read blocks 2 and 3 from disk for used blocks
	memset(buff, 0, 4096);
	block_read(2, buff);
	memcpy(blocks, buff, 4096);

	memset(buff, 0, 4096);
	block_read(3, buff);
	memcpy(blocks+4096, buff, 4096);

	ii = 0;
	int current = 4;
	int offset = 0;
	memset(buff, 0, 4096);
	block_read(current, buff);

	do
	{
		if(offset+I_SIZE > 4096)
		{
			current++;
			offset = 0;
			memset(buff, 0, 4096);
			block_read(current, buff);
		}
		memcpy(&all_files[ii].file_id, buff + offset, I_SIZE);
		offset += I_SIZE;

		if(offset+I_SIZE > 4096)
		{
			current++;
			offset = 0;
			memset(buff, 0, 4096);
			block_read(current, buff);
		}
		memcpy(&all_files[ii].file_offset, buff + offset, I_SIZE);
		offset += I_SIZE;

		if(offset+I_SIZE > 4096)
		{
			current++;
			offset = 0;
			memset(buff, 0, 4096);
			block_read(current, buff);
		}
		memcpy(&all_files[ii].file_size, buff + offset, I_SIZE);
		offset += I_SIZE;

		if(offset+I_SIZE > 4096)
		{
			current++;
			offset = 0;
			memset(buff, 0, 4096);
			block_read(current, buff);
		}
		memcpy(&all_files[ii].num_blocks, buff + offset, I_SIZE);
		offset += I_SIZE;

		if(offset+16 > 4096)
		{
			current++;
			offset = 0;
			memset(buff, 0, 4096);
			block_read(current, buff);
		}
		memcpy(all_files[ii].file_name, buff + offset, 16);
		offset += 16;

		int jj;
		if(all_files[ii].num_blocks > 0)
			all_files[ii].first_block = malloc(sizeof(struct block));
		
		struct block* current_block = all_files[ii].first_block;
		for(jj = 0; jj < all_files[ii].num_blocks; jj++)
		{
			if(offset+P_SIZE > 4096)
			{
				current++;
				offset = 0;
				memset(buff, 0, 4096);
				block_read(current, buff);
			}
			memcpy(&current_block->addr, buff + offset, P_SIZE);
			offset += P_SIZE;
			if(jj != all_files[ii].num_blocks-1)
			{
				current_block->next = malloc(sizeof(struct block));
				current_block = current_block->next;
			}
			else
				current_block->next = NULL;
		}

		ii++;

	}while(ii<64);

	// Load file pointers into file descriptor array
	for(ii = 0; ii<32; ii++)
	{
		if(foo[ii] == -1)
			all_fildes[ii] = NULL;
		else
		{
			int jj;
			for(jj = 0; jj<64; jj++)
			{
				if(foo[ii] == all_files[jj].file_id)
				{
					all_fildes[ii] = &all_files[jj];
					break;
				}
			}
		}
	}

	return 0;
}

// Unmounts a filesystm and writes datastructures to disk
int umount_fs(char *disk_name)
{
	if(open_f == 0)
		return -1;

	int ii;
	for(ii=0; ii<32; ii++)
	{
		if(all_fildes[ii] != NULL)
			fs_close(ii);
	}

	// Create buffer to use for writing
	char buff[4096];
	memset(buff, 0, 4096);

	// Write the fildes structure to the first block (32 pointers of 8 bytes each < 4096 bytes)
	// - First byte in buffer refers to 1, the file descriptor data structure
	// This structure is stored in block 1
	memset(buff, 1, 1);
	for(ii=0; ii<32; ii++)
	{
		if(all_fildes[ii] == NULL)
		{
			int foo = -1;
			memcpy(buff + 1 + ii*I_SIZE, &foo, I_SIZE);
		}
		else
			memcpy(buff + 1 + ii*I_SIZE, &all_fildes[ii]->file_id, I_SIZE);
	}
	block_write(1, buff);

	// Write the all files data structure to disk (1 structure = 4*Int + 16*Char + Block Addresses)
	// - First byte in buffer refers to 2, the file structure
	// This structure is stored in blocks 4+
	memset(buff, 0, 4096);
	int current_block = 4;
	int offset = 0;
	for(ii = 0; ii < 64; ii++)
	{
		// Check to see if data must be written and block can be copied
		if(offset + I_SIZE > 4096)
		{
			block_write(current_block, buff);
			memset(buff, 0, 4096);
			offset = 0;
			current_block++;
		}
		// Copy file id to disk
		memcpy(buff + offset, &all_files[ii].file_id, I_SIZE);
		offset += I_SIZE;

		// Check to see if data must be written and block can be copied
		if(offset + I_SIZE > 4096)
		{
			block_write(current_block, buff);
			memset(buff, 0, 4096);
			offset = 0;
			current_block++;
		}
		// Copy file offset to disk
		memcpy(buff + offset, &all_files[ii].file_offset, I_SIZE);
		offset += I_SIZE;
		
		// Check to see if data must be written and block can be copied
		if(offset + I_SIZE > 4096)
		{
			block_write(current_block, buff);
			memset(buff, 0, 4096);
			offset = 0;
			current_block++;
		}
		// Copy file size to disk
		memcpy(buff + offset, &all_files[ii].file_size, I_SIZE);
		offset += I_SIZE;

		// Check to see if data must be written and block can be copied
		if(offset + I_SIZE > 4096)
		{
			block_write(current_block, buff);
			memset(buff, 0, 4096);
			offset = 0;
			current_block++;
		}
		// Copy number blocks to disk
		memcpy(buff + offset, &all_files[ii].num_blocks, I_SIZE);
		offset += I_SIZE;

		// Check to see if data must be written and block can be copied
		if(offset + 16 > 4096)
		{
			block_write(current_block, buff);
			memset(buff, 0, 4096);
			offset = 0;
			current_block++;
		}
		// Copy file name to disk
		memcpy(buff + offset, all_files[ii].file_name, 16);
		offset += 16;

		// Copy all block addresses to buffer
		int jj;
		struct block* current = all_files[ii].first_block;
		struct block* old;
		for(jj = 0; jj < all_files[ii].num_blocks; jj++)
		{
			// Check to see if data must be written and block can be copied
			if(offset + P_SIZE > 4096)
			{
				block_write(current_block, buff);
				memset(buff, 0, 4096);
				offset = 0;
				current_block++;
			}
			memcpy(buff + offset, &current->addr, P_SIZE);
			offset += P_SIZE;
			old = current;
			current = current->next;

			// Free the block that was just written
			free(old);
		}
	}

	// One final write to get the last data to be written for the file  structure
	block_write(current_block, buff);

	// Mark proper metadata bits if they aren't already set
	memset(blocks, 1, current_block);

	// Write the used blocks to data
	// - No first byte reference
	// This structure is stored in blocks 2 and 3
	memset(buff, 0, 4096);
	memcpy(buff, blocks, 4096);
	block_write(2, buff);

	memset(buff, 0, 4096);
	memcpy(buff, blocks+4096, 4096);
	block_write(3, buff);

	if(close_disk(disk_name) == -1)
		return -1;

	open_f = 0;

	return 0;
}

// Open file specified by name
int fs_open(char *name)
{
	if(open_f == 0)
		return -1;

	int ii = -1;

	if(name == NULL)
		return -1;

	// Check for file name in the array
	for(ii = 0; ii<64; ii++)
	{
		// Check to see if file names are the same
		if(strcmp(all_files[ii].file_name, name) == 0)
		{
			break;
		}
	}

	// If the file wasn't found, return error
	if(ii == 64)
		return -1;

	// Search for available file descriptors
	int jj;
	for(jj = 0; jj<32; jj++)
	{
		if(all_fildes[jj] == NULL)
		{
			break;
		}
	}

	// If no file descriptors were available, return error
	if(jj == 32)
		return -1;

	// Connect file descriptor to file
	all_fildes[jj] = &all_files[ii];
	all_files[ii].file_offset = 0;

	return jj;
}

// Close file specified by name
int fs_close(int fildes)
{
	if(open_f == 0)
		return -1;
	if(fildes > 31)
		return -1;
	if(fildes < 0)
		return -1;

	// Check to see if the file descriptor is active
	if(all_fildes[fildes] == NULL)
		return -1;

	// If it is active, close it
	all_fildes[fildes] = NULL;

	return 0;
}

// Create a new file
int fs_create(char *name)
{
	if(open_f == 0)
		return -1;

	int ii;
	int jj=-1;

	if(name == NULL)
		return -1;

	// Check character length of the file 
	if(strlen(name) > 15)
		return -1;

	// Check to see if there is a file with the same name and get next open spot in array
	for(ii = 0; ii<64; ii++)
	{
		// Check to see if file names are the same
		if(strcmp(all_files[ii].file_name, name) == 0)
		{
			return -1;
		}

		// Find empty file locations
		if(all_files[ii].file_id == -1)
		{
			jj = ii;
		}
	}

	// Check to see if there are no available file spots
	if(jj == -1)
		return -1;

	// If no errors, add file to files array
	all_files[jj].file_id = jj;
	strcpy(all_files[jj].file_name, name);

	return 0;
}

// Delete a file
int fs_delete(char *name)
{
	if(open_f == 0)
		return -1;

	int ii;

	if(name == NULL)
		return -1;

	// Check to see if there is a file with the same name and get next open spot in array
	for(ii = 0; ii<64; ii++)
	{
		// Check to see if file names are the same
		if(strcmp(all_files[ii].file_name, name) == 0)
		{
			break;
		}
	}

	// If the file does not exist, return an error
	if(ii == 64)
		return -1;

	// Check to see if the file is open
	int jj;
	for(jj = 0; jj<32; jj++)
	{
		// If the file is open, return an error
		if(all_fildes[jj] == &all_files[ii])
		{
			return -1;
		}
	}

	// Delete the file info with the provided name
	all_files[ii].file_id = -1;
	all_files[ii].file_size = 0;
	all_files[ii].file_offset = 0;
	memset(all_files[ii].file_name, 0, 16);
	all_files[ii].num_blocks = 0;

	// Delete the file data with the provided name 
	if(all_files[ii].first_block != NULL)
	{
		struct block* temp = all_files[ii].first_block;
		struct block* old;
		blocks[temp->addr] = 0;
		while (temp->next != NULL)
		{
			old = temp;
			temp = temp->next;
			free(old);
			blocks[temp->addr] = 0;
		}
		all_files[ii].first_block = NULL;
	}
}

// Reads nbyte bytes from the file refered to by fildes
// - data is loaded into buf
int fs_read(int fildes, void *buf, size_t nbyte)
{
	if(open_f == 0)
		return -1;

	int bytes_copied = 0;

	// Check for errors; fildes not valid
	if(fildes > 31)
		return -1;
	if(fildes < 0)
		return -1;
	if(all_fildes[fildes] == NULL)
		return -1;
	struct file* current_file = all_fildes[fildes];

	// If at the end of the file, return 0
	if(current_file->file_size == current_file->file_offset)
		return 0;

	if(nbyte+current_file->file_offset > current_file->file_size)
		nbyte = current_file->file_size - current_file->file_offset;

	// Get offset in blocks and get that block
	int block_offset = current_file->file_offset / 4096;
	struct block* current_block = current_file->first_block;
	int ii;
	for(ii = 0; ii < block_offset; ii++)
		current_block = current_block->next;

	// Get the byte offset
	int byte_offset = current_file->file_offset % 4096;

	// Read data from the buffer for the first block
	char temp[4096];
	if(block_read(current_block->addr, temp) != 0)
		return -1;

	//int blocks_to_copy = (nbyte + byte_offset + 4095) / 4096;
	int blocks_to_copy = (nbyte + byte_offset + 4095) / 4096;
	int bytes_to_copy = (nbyte + byte_offset) % 4096;

	for(ii = 0; ii < blocks_to_copy; ii++)
	{
		// If the end of the file occurs in the current block and it will be read up to
		if((current_file->file_size - current_file->file_offset + 4095) / 4096 -1 == ii && current_file->file_offset + bytes_to_copy > current_file->file_size)
		{
			memcpy(buf+bytes_copied, temp+byte_offset, (current_file->file_size - current_file->file_offset) % 4096);
			bytes_copied+= (current_file->file_size - current_file->file_offset) % 4096;
			current_file->file_offset=current_file->file_size;
			return bytes_copied;
		}

		// If its the last block
		else if(ii == blocks_to_copy-1)
		{
			memcpy(buf+bytes_copied, temp+byte_offset, bytes_to_copy);
			bytes_copied+=bytes_to_copy;
			current_file->file_offset+=bytes_copied;
			return bytes_copied;
		}

		// If its any other block
		else
		{
			memcpy(buf+bytes_copied, temp+byte_offset, 4096-byte_offset);
			bytes_copied+=4096-byte_offset;
			current_file->file_offset+=4096-byte_offset;
			byte_offset = 0;
		}

		current_block = current_block->next;
		if(block_read(current_block->addr, temp) != 0)
			return -1;
	}
}

// Write nbyte bytes to a file, extends file as needed
int fs_write(int fildes, void *buf, size_t nbyte)
{
	if(open_f == 0)
		return -1;

	int bytes_written = 0;

	// Check for errors; fildes not valid
	if(fildes > 31)
		return -1;
	if(fildes < 0)
		return -1;
	if(all_fildes[fildes] == NULL)
		return -1;
	struct file* current_file = all_fildes[fildes];

	// If no bytes to write, return 0
	if(nbyte <= 0)
		return 0;

	// If no blocks allocated, create a block, otherwise, get current block
	struct block* current_block;
	if(current_file->file_offset == 0 && current_file->file_size == 0)
	{
		// First find an open block
		int ii;
		for(ii = 4096; ii<8192; ii++)
		{
			if(blocks[ii] == 0)
				break;
		}
		// If disk is full, return 0
		if(ii == 8192)
		{
			return 0;
		}

		// Allocate space for the block and set it up
		current_file->first_block = malloc(sizeof(struct block));
		current_file->num_blocks = 1;
		current_file->first_block->addr = ii;
		current_file->first_block->next = NULL;
		blocks[ii] = 1;
	}

	// Get offset data
	int byte_offset = current_file->file_offset % 4096;
	int block_offset = current_file->file_offset / 4096;

	// Move to current block
	current_block = current_file->first_block;
	int ii;
	for(ii = 0; ii < block_offset; ii++)
	{
		if(current_block->next == NULL)
		{
			int jj;
			for(jj = 4096; jj<8192; jj++)
			{
				if(blocks[jj] == 0)
					break;
			}
			// If disk is full, return 0
			if(jj == 8192)
			{
				return 0;
			}
			current_block->next = malloc(sizeof(struct block));
			current_file->num_blocks++;
			current_block->next->addr = jj;
			current_block->next->next = NULL;
			blocks[jj] = 1;
		}

		current_block = current_block->next;
	}

	// Get number of blocks to be written
	int blocks_to_write = (byte_offset+nbyte + 4095) / 4096;

	// Iterate through and write blocks
	for(ii = 0; ii<blocks_to_write; ii++)
	{
		// If there is only one block to be written
		if(blocks_to_write == 1)
		{
			char temp[4096];
			block_read(current_block->addr, temp);
			memcpy(temp+byte_offset, buf, nbyte);
			block_write(current_block->addr, temp);

			// Update file size and offset as needed
			if(current_file->file_offset+nbyte > current_file->file_size)
				current_file->file_size = current_file->file_offset + nbyte;
			current_file->file_offset += nbyte;

			return nbyte;
		}
		// First block to be written to
		// - Depends on byte_offset 
		if(ii == 0)
		{
			char temp[4096];
			block_read(current_block->addr, temp);
			memcpy(temp+byte_offset, buf, 4096-byte_offset);
			block_write(current_block->addr, temp);

			// Update file size and offset as needed
			if(current_file->file_offset + (4096-byte_offset) > current_file->file_size)
				current_file->file_size = current_file->file_offset + (4096-byte_offset);
			current_file->file_offset += 4096-byte_offset;
			bytes_written += 4096-byte_offset;
		}
		// If its the last block to be written
		else if(ii == blocks_to_write -1)
		{
			char temp[4096];
			block_read(current_block->addr, temp);
			memcpy(temp, buf+bytes_written, nbyte-bytes_written);
			block_write(current_block->addr, temp);

			// Update file size and offset as needed
			if(current_file->file_offset + nbyte-bytes_written > current_file->file_size)
				current_file->file_size = current_file->file_offset + nbyte-bytes_written;
			current_file->file_offset += nbyte-bytes_written;

			return nbyte;

		}
		// If its any other block to be written
		else
		{
			char temp[4096];
			memcpy(temp, buf+bytes_written, 4096);
			block_write(current_block->addr, buf+bytes_written);

			// Update file size and offset as needed
			if(current_file->file_offset + 4096 > current_file->file_size)
				current_file->file_size = current_file->file_offset + 4096;
			current_file->file_offset += 4096;

			bytes_written += 4096;
		}

		// If next block doesn't exist and not done writing, create a new block
		if(current_block->next == NULL && ii < blocks_to_write-1)
		{
			// Get addr of next open block
			int jj;
			for(jj = 4096; jj<8192; jj++)
			{
				if(blocks[jj] == 0)
					break;
			}
			// If disk is full, return 0
			if(jj == 8192)
			{
				return bytes_written;
			}
			current_block->next = malloc(sizeof(struct block));
			current_block = current_block->next;
			current_block->next = NULL;
			current_block->addr = jj;
			blocks[current_block->addr] = 1;
			current_file->num_blocks++;
		}
		else
		{
			current_block = current_block->next;
		}
	}
}

// Returns the current file's size
int fs_get_filesize(int fildes)
{
	if(open_f == 0)
		return -1;

	// Check to see if the file descriptor is active
	if(fildes > 31)
		return -1;
	if(fildes < 0)
		return -1;
	if(all_fildes[fildes] == NULL)
		return -1;

	// If the descriptor is active, return the file's size
	return all_fildes[fildes]->file_size;
}

// Lists al lthe files in the filesystem
int fs_listfiles(char ***files)
{
	if(open_f == 0)
		return -1;

	int ii;
	int num=0;

	// Count number of files in filesystem
	for(ii = 0; ii < 64; ii++)
	{
		if(all_files[ii].file_id >= 0)
		{
			num++;
		}
	}

	// Allocate memory for the file array and set last element to NULL
	*files = malloc(sizeof(char*)*(num+1));
	(*files)[num] = NULL;
	for(ii = 0; ii < num; ii++)
	{
		(*files)[ii] = malloc(sizeof(char)*16);
	}

	// Copy file names into the array
	int jj;
	ii = 0;
	for(jj = 63; jj>-1; jj--)
	{
		if(all_files[jj].file_id >= 0)
		{
			strcpy((*files)[ii], all_files[jj].file_name);
			ii++;
		}
	}

	return 0;
}

// Sets the offset of a file to a value
int fs_lseek(int fildes, off_t offset)
{
	if(open_f == 0)
		return -1;

	// If file descriptor is not active, return error
	if(fildes > 31)
		return -1;
	if(fildes < 0)
		return -1;
	if(all_fildes[fildes] == NULL)
		return -1;

	// If offset is beyond end of file, return error
	if(all_fildes[fildes]->file_size < offset)
		return -1;

	// If offset is less than zero, return error
	if(offset < 0) 
		return -1;

	all_fildes[fildes]->file_offset = offset;

	return 0;
}

// Truncates a file
int fs_truncate(int fildes, off_t length)
{
	if(open_f == 0)
		return -1;

	// If file is not active, return error
	if(fildes > 31)
		return -1;
	if(fildes < 0)
		return -1;
	if(all_fildes[fildes] == NULL)
		return -1;

	// If length is greater than size of file, return error
	if(length < 0)
		return -1;
	if(all_fildes[fildes]->file_size < length)
		return -1;

	// Update file pointer if necessary
	if(all_fildes[fildes]->file_offset > length)
		all_fildes[fildes]->file_offset = length;

	all_fildes[fildes]->file_size = length;

	// Get blocks of new file
	int file_blocks = (length+4095) / 4096;

	// Free any blocks in the block array and free any now unnecessary block structs
	int ii;
	struct block* current_block = all_fildes[fildes]->first_block;
	for(ii = 0; ii < file_blocks; ii++)
		current_block = current_block->next;

	for(ii = file_blocks; ii < all_fildes[fildes]->num_blocks; ii++)
	{
		struct block* next_block =  current_block->next;
		blocks[current_block->addr] = 0;
		free(current_block);
		current_block = next_block;
	}

	all_fildes[fildes]->num_blocks = file_blocks;

	return 0;
}