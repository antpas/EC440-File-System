# EC440-File-System
The fifth project for a Fall 2016 EC440 Operating Systems course at BU; Objective is to implement a basic file system.

Enviroment: Written in C and compiled with the attached Makefile. Should run on most/many unix distros.

NOTE: This project requires disk.c and disk.h which were provided by the professor. These files contained functions to create an empty virtual disk, open a virtual disk, close an open virtual disk, and write/read a block from a disk given its the block number. I have not included these files since they are the professor's to distribute. As a result, this code WILL NOT compile and run properly.

Objectives: Implement a file system on a virtual disk (file). The file system itself must be able to be created, mounted, unmounted, and its files listed, while files in the file system must be able to be created, deleted, opened, closed, read from, written to, checked for file size, seeked, and truncated. The virtual file system is based on a file which must last beyond the execution of the program. There is a maximum of 64 files.

Overview of implementation:
- Making the filesystem initializes metadata and writes it to the virtual disk.
- Mounting a filesystem loads data from the virtual disk into memory. A bitmap for used blocks takes up the first two blocks while the next blocks of data are for file metadata and data loaded in a row.
- Unmounting a filesystem writes a filesystem in memory to a virtual disk. It starts by writing the file descriptors and a bit map of the used blocks then writes all the files' metadata and contents.
- Opening a file by name checks to see if it exists and if there are enough file descriptors for it. If so, it assigns it a file descriptor.
- Closing a file does the opposite of opening it. It checks to see if it exists then resets the file descriptor. 
- Creating a file is done by doing some error checking (files with the same names, too long of a name, too many files, etc.) then adding the file to the filesystem (not opening it).
- Deleting a file is done by error checking then deleting and/or resetting a files data and metadata.
- Reading a file is done by file descriptor and the number of bytes to be read. It starts at the current file offset and reads that many bytes block by block. Data is added to a buffer supplied by the caller. 
- Writing to a file writes a specified number of bytes block by block to the file creating blocks as necessary.
- Getting file size is as easy as requesting it with a given file descriptor.
- List files lists puts all the file names in an array of character array pointers for the user to iterate through.
- Seek modifies the offset of the requested file while truncate deletes blocks off the end of a file and adjusts the offset and file length.

Testing:
- The main.c file for this project is only for testing the library.

Developed by Sam Beaulieu, see accompanying license for usage.


