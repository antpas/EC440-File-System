#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fs.h"

int fs_make_test();
int fs_mount_unmount_test();
int fs_file_manage_test();
int fs_read_write();
int fs_auxilary_test();
int fs_large_file();
int fs_inclusive_test();
int fs_openread();

int main()
{
	int test;

	// Test make function
	printf("\nTesting makeing a fs\n");
    test = fs_make_test();
    printf("Test make finished with code: %d\n\n", test);
    if(test == -1) return 0;

    // Test initial mount and unmount functions
    printf("Testing initial mount and unmount functions\n");
    test = fs_mount_unmount_test();
    printf("Test initial mount, unmount finished with code: %d\n\n", test);
    if(test == -1) return 0;

    // Test createing, opening, closing and deleting files
    printf("Testing file management\n");
    test = fs_file_manage_test();
    printf("Test file management finished with code: %d\n\n", test);
    if(test == -1) return 0;

    // Test reading and writing from a file
    printf("Testing reading and writing from a file\n");
    test = fs_read_write();
    printf("Test reading and writing finished with code: %d\n\n", test);
    if(test == -1) return 0;

    // Test auxillary functions
    printf("Testing auxilary functions\n");
    test = fs_auxilary_test();
    printf("Test auxilary functions finished with code: %d\n\n", test);
    if(test == -1) return 0;

    // Test large file read/write/seek/truncate
    printf("Testing large file operations\n");
    test = fs_large_file();
    printf("Test large file operations finished with code: %d\n\n", test);
    if(test == -1) return 0;

    // Test all functions with large file
    printf("Testing all functions together with a large file\n");
    test = fs_inclusive_test();
    printf("Test all functions finished with code: %d\n\n", test);
    if(test == -1) return 0;

    // Open a file twice, close the first, and read from the second
    printf("Testing open twice, close one, and read\n");
    test = fs_openread();
    printf("Teest open twice, close one, and read finished with code: %d\n\n", test);
    if(test == -1) return 0;

    return 0;
}

int fs_openread()
{
	char file1[] = "file";
	if(mount_fs("Test") == -1)
		return -1;
	if(fs_create(file1) == -1)
		return -1;
	int fildes0 = fs_open(file1);
	if(fildes0 == -1)
		return -1;
	if(fs_write(fildes0, file1, strlen(file1)) != strlen(file1))
		return -1;
	int fildes1 = fs_open(file1);
	if(fildes1 == -1)
		return -1;
	if(fs_close(fildes0) == -1)
		return -1;
	char temp[20];
	if(fs_read(fildes1, temp, strlen(file1)) != strlen(file1))
		return -1;
	if(memcmp(temp, file1, strlen(file1)) != 0)
		return -1;

	if(fs_close(fildes1) == -1)
		return -1;
	if(fs_delete(file1) == -1)
		return -1;
	if(umount_fs("Test") == -1)
		return -1;

	return 0;
}

int fs_make_test()
{
	char name[] = "Test";

	if(make_fs(name) == -1)
		return -1;
	if(make_fs(NULL) != -1)
		return -1;

	return 0;
}

int fs_mount_unmount_test()
{
	char name[] = "Test";

	printf("--- Checking mounting errors\n");
	if(mount_fs("blah") != -1)
		return -1;
	if(mount_fs(NULL) != -1)
		return -1;

	printf("--- Checking unmounting errors\n");
	if(umount_fs(NULL) != -1)
		return -1;
	if(umount_fs("blah") != -1)
		return -1;

	printf("--- Test mounting a filesystem\n");
	if(mount_fs(name) == -1)
		return -1;

	printf("--- Test unmounting a filesystem\n");
	if(umount_fs(name) == -1)
		return -1;

	printf("--- Test mounting two filesystems\n");
	if(make_fs("Test2") == -1)
		return -1;
	if(mount_fs(name) == -1)
		return -1;
	if(mount_fs("Test2") != -1)
		return -1;

	printf("--- Cleaning up and finishing test\n");
	if(umount_fs(name) == -1)
		return -1;
}

int fs_file_manage_test()
{
	char name[] = "Test_File";
	if(mount_fs("Test") == -1)
		return -1;

	printf("--- Test opening, deleting, and closing non-existing files\n");
	if(fs_open("Blah") != -1)
		return -1;
	if(fs_open(NULL) != -1)
		return -1;
	if(fs_delete("Blah") != -1)
		return -1;
	if(fs_delete(NULL) != -1)
		return -1;
	if(fs_close(5) != -1)
		return -1;
	if(fs_close(40) != -1)
		return -1;
	if(fs_close(0) != -1)
		return -1;
	if(fs_close(30) != -1)
		return -1;
	if(fs_close(-10) != -1)
		return -1;

	printf("--- Test createing a file\n");
	if(fs_create("Test_File_tolong") != -1)
		return -1;
	if(fs_create("Test_File______") == -1)
		return -1;
	if(fs_create("Test_File_way_too_long") != -1)
		return -1;
	if(fs_create(name) == -1)
		return -1;
	if(fs_create(name) != -1)
		return -1;

	printf("--- Test opening and closing a file\n");
	int fildes = fs_open(name);
	if(fildes == -1)
		return -1;
	if(fs_close(fildes) == -1)
		return -1;
	if(fs_close(fildes) != -1)
		return -1;

	printf("--- Test opening 32+ files then closing all but one\n");
	int ii;
	int fildes2[32];
	for(ii = 0; ii<32; ii++)
	{
		fildes2[ii] = fs_open(name);
		if(fildes2[ii] == -1)
			return -1;
	}
	fildes = fs_open(name);
	if(fildes != -1)
		return -1;
	for(ii = 0; ii<31; ii++)
	{
		if(fs_close(fildes2[ii]) == -1)
			return -1;
	}
	fildes = fildes2[31];

	printf("--- Test deleting an open file\n");
	if(fs_delete(name) != -1)
		return -1;

	printf("--- Test deleting a file\n");
	if(fs_close(fildes) == -1)
		return -1;
	if(fs_delete(name) == -1)
		return -1;
	if(fs_delete("Test_File______") == -1)
		return -1;

	printf("--- Test createing more than 64 files\n");
	char names[64][16];
	for(ii = 0; ii<64; ii++)
	{
		sprintf(names[ii], "Test File %d", ii);
		if(fs_create(names[ii]) == -1)
			return -1;
	}
	if(fs_create("testing") != -1)
		return -1;

	printf("--- Cleaning up and finishing test\n");
	for(ii = 0; ii<64; ii++)
	{
		if(fs_delete(names[ii]) == -1)
			return -1;
	}
	if(umount_fs("Test") == -1)
		return -1;

	return 0;
}

int fs_read_write()
{
	char file1[] = "File 1";
	char string1[] = "Hello World! I'm being read and written!";
	char string2[] = "More bytes to be written to the file";
	char string3[] = "Hello World! I'm being read and written!More bytes to be written to the file";
	char string4[] = "Hello World! I'm being read and written!More bytes to be";
	char string5[] = " written to the file";

	printf("--- Mount filesystem, then create and open a file\n");
	if(mount_fs("Test") == -1)
		return -1;
	if(fs_create(file1) == -1)
		return -1;
	int fildes = fs_open(file1);
	if(fildes == -1)
		return -1;

	printf("--- Test writing to non-existing file\n");
	if(fs_write(20, string1, strlen(string1)) != -1)
		return -1;
	if(fs_write(40, string1, strlen(string1)) != -1)
		return -1;
	if(fs_write(-10, string1, strlen(string1)) != -1)
		return -1;
	if(fs_write(-1, string1, strlen(string1)) != -1)
		return -1;

	printf("--- Write less than one block to the file\n");
	if(fs_write(fildes, string1, strlen(string1)) != strlen(string1))
		return -1;

	printf("--- Try to read data back from file (should fail due to offset)\n");
	char temp[strlen(string1)];
	if(fs_read(fildes, temp, strlen(string1)) != 0)
		return -1;
	if(memcmp(temp, string1, strlen(string1)) == 0)
		return -1;

	printf("--- Close and open file then try read again\n");
	if(fs_close(fildes) == -1)
		return -1;
	fildes = fs_open(file1);
	if(fildes == -1)
		return -1;
	if(fs_read(fildes, temp, strlen(string1)) != strlen(string1))
		return -1;
	if(memcmp(temp, string1, strlen(string1)) != 0)
		return -1;

	printf("--- Write more bytes to the file\n");
	if(fs_write(fildes, string2, strlen(string2)) != strlen(string2))
		return -1;

	char temp2[strlen(string2)+strlen(string1)];
	printf("--- Close and Repoen file then read all bytes from the file\n");
	if(fs_close(fildes) == -1)
		return -1;
	fildes = fs_open(file1);
	if(fildes == -1)
		return -1;
	if(fs_read(fildes, temp2, strlen(string2)+strlen(string1)) != strlen(string2)+strlen(string1))
		return -1;
	if(memcmp(temp2, string3, strlen(string3)) != 0)
		return -1;

	char temp3[strlen(string4)];
	char temp4[strlen(string5) + 10];
	printf("--- Test reading up to the end of the file (open/close, read then read the remaining)\n");
	if(fs_close(fildes) == -1)
		return -1;
	fildes = fs_open(file1);
	if(fildes == -1)
		return -1;
	if(fs_read(fildes, temp3, strlen(string4)) != strlen(string4))
		return -1;
	if(memcmp(temp3, string4, strlen(string4)) != 0)
		return -1;
	if(fs_read(fildes, temp4, strlen(string5) + 10) != strlen(string5))
		return -1;
	if(memcmp(temp4, string5, strlen(string5)) != 0)
		return -1;

	printf("--- Test unmounting filesystem and remounting it then reading\n");
	if(umount_fs("Test") == -1)
		return -1;
	if(mount_fs("Test") == -1)
		return -1;
	if(fs_close(fildes) != -1)
		return -1;
	fildes = fs_open(file1);
	if(fildes == -1)
		return -1;
	if(fs_read(fildes, temp2, strlen(string2)+strlen(string1)) != strlen(string2)+strlen(string1))
		return -1;
	if(memcmp(temp2, string3, strlen(string3)) != 0)
		return -1;

	printf("--- Test deleting file\n");
	if(fs_delete(file1) != -1)
		return -1;
	if(fs_close(fildes) == -1)
		return -1;
	if(fs_delete(file1) == -1)
		return -1;

	printf("--- Test reading from deleted file\n");
	memset(temp3, 0, strlen(string4));
	if(fs_open(file1) != -1)
		return -1;
	if(fs_read(fildes, temp3, strlen(string4)) != -1)
		return -1;
	if(memcmp(temp3, string4, strlen(string4)) == 0)
		return -1;
	if(fs_read(-10, temp3, strlen(string4)) != -1)
		return -1;
	if(memcmp(temp3, string4, strlen(string4)) == 0)
		return -1;

	printf("--- Test writing to deleted file\n");
	if(fs_write(fildes, string1, strlen(string1)) != -1)
		return -1;

	printf("--- Cleaning up and finishing test\n");
	if(umount_fs("Test") == -1)
		return -1;

	return 0;
}

int fs_auxilary_test()
{
	char file1[] = "File 1";
	char file2[] = "File 2";
	char file3[] = "File 3";
	char file4[] = "File 4";
	char file5[] = "File 45";
	char string1[] = "Hello World! I'm being read and written!";
	char string2[] = "More bytes to be written to the file";
	char string3[] = "Hello World! I'm being read and written!More bytes to be written to the file";
	char string4[] = "Hello World! I'm being read and written!More bytes to be";

	char temp[strlen(string2)+strlen(string1)];
	printf("--- Mount filesystem, create and open a file, and write to the file\n");
	if(mount_fs("Test") == -1)
		return -1;
	if(fs_create(file1) == -1)
		return -1;
	int fildes1 = fs_open(file1);
	if(fildes1 == -1)
		return -1;
	if(fs_write(fildes1, string3, strlen(string3)) != strlen(string3))
		return -1;
	if(fs_close(fildes1) == -1)
		return -1;
	fildes1 = fs_open(file1);
	if(fildes1 == -1)
		return -1;
	if(fs_read(fildes1, temp, strlen(string2)+strlen(string1)) != strlen(string2)+strlen(string1))
		return -1;
	if(memcmp(temp, string3, strlen(string3)) != 0)
		return -1;

	printf("--- Create a few more files with different lengths\n");
	if(fs_create(file2) == -1)
		return -1;
	int fildes2 = fs_open(file2);
	if(fildes2 == -1)
		return -1;
	if(fs_write(fildes2, string1, strlen(string1)) != strlen(string1))
		return -1;

	if(fs_create(file3) == -1)
		return -1;
	int fildes3 = fs_open(file3);
	if(fildes3 == -1)
		return -1;
	if(fs_write(fildes3, string2, strlen(string2)) != strlen(string2))
		return -1;

	if(fs_create(file4) == -1)
		return -1;
	int fildes4 = fs_open(file4);
	if(fildes4 == -1)
		return -1;
	if(fs_write(fildes4, string4, strlen(string4)) != strlen(string4))
		return -1;

	printf("--- Test getting file sizes\n");
	if(fs_get_filesize(40) != -1)
		return -1;
	if(fs_get_filesize(20) != -1)
		return -1;
	if(fs_get_filesize(-1) != -1)
		return -1;
	if(fs_get_filesize(-15) != -1)
		return -1;
	if(fs_get_filesize(fildes1) != strlen(string3))
		return -1;
	if(fs_get_filesize(fildes2) != strlen(string1))
		return -1;
	if(fs_get_filesize(fildes3) != strlen(string2))
		return -1;
	if(fs_get_filesize(fildes4) != strlen(string4))
		return -1;

	printf("--- Test listing files\n");
	char **files;
	if(fs_listfiles(&files) == -1)
		return -1;
	int num = 0;
	while(files[num++] != NULL);
	num--;
	char *array[4];
	array[0] = file1;
	array[1] = file2;
	array[2] = file3;
	array[3] = file4;
	int ii;
	for(ii = 0; ii<num; ii++)
	{
		if(strcmp(array[ii], files[ii]) != 0)
			return -1;
		free(files[ii]);
	}
	free(files);

	printf("--- Test adding a fifth file and checking list and size\n");
	if(fs_create(file5) == -1)
		return -1;
	int fildes5 = fs_open(file5);
	if(fildes5 == -1)
		return -1;
	if(fs_get_filesize(fildes5) != 0)
		return -1;
	if(fs_listfiles(&files) == -1)
		return -1;
	num = 0;
	while(files[num++] != NULL);
	num--;
	char *array2[5];
	array2[0] = file1;
	array2[1] = file2;
	array2[2] = file3;
	array2[3] = file4;
	array2[4] = file5;
	for(ii = 0; ii<num; ii++)
	{
		if(strcmp(array2[ii], files[ii]) != 0)
			return -1;
		free(files[ii]);
	}
	free(files);

	printf("--- Test seeking errors\n");
	if(fs_lseek(fildes5, 1) != -1)
		return -1;
	if(fs_lseek(fildes5, 10) != -1)
		return -1;
	if(fs_lseek(fildes4, -11) != -1)
		return -1;
	if(fs_lseek(fildes3, -3) != -1)
		return -1;
	if(fs_lseek(fildes1, 9343) != -1)
		return -1;
	if(fs_lseek(fildes2, 541) != -1)
		return -1;
	if(fs_lseek(50, 2) != -1)
		return -1;
	if(fs_lseek(30, 2) != -1)
		return -1;
	if(fs_lseek(-1, 2) != -1)
		return -1;
	if(fs_lseek(-16, 2) != -1)
		return -1;

	printf("--- Test seeking to a spot in a file and writing and reading\n");
	if(fs_lseek(fildes1, 0) == -1)
		return -1;
	char temp3[100];
	if(fs_read(fildes1, temp3, 10) == -1)
		return -1;
	if(memcmp(temp3, string3, 10) != 0)
		return -1;
	if(fs_lseek(fildes1, strlen(string1)) == -1)
		return -1;
	char temp2[100];
	if(fs_read(fildes1, temp2, 100) != strlen(string2))
		return -1;
	if(memcmp(temp2, string2, strlen(string2)) != 0)
		return -1;

	printf("--- Test truncating errors\n");
	if(fs_truncate(fildes1, 932) != -1)
		return -1;
	if(fs_truncate(fildes5, 2) != -1)
		return -1;
	if(fs_truncate(fildes4, -5) != -1)
		return -1;
	if(fs_truncate(fildes3, -20) != -1)
		return -1;
	if(fs_truncate(50, 2) != -1)
		return -1;
	if(fs_truncate(30, 2) != -1)
		return -1;
	if(fs_truncate(-1, 2) != -1)
		return -1;
	if(fs_truncate(-167, 2) != -1)
		return -1;

	printf("--- Test truncating a file\n");
	if(fs_truncate(fildes1, 50) == -1)
		return -1;
	if(fs_get_filesize(fildes1) != 50)
		return -1;
	if(fs_lseek(fildes1, 0) == -1)
		return -1;
	char temp4[100];
	if(fs_read(fildes1, temp4, 100) != 50)
		return -1;
	if(memcmp(temp4, string3, 50) != 0)
		return -1;

	printf("--- Cleaning up and finishing test\n");
	if(fs_close(fildes1) == -1)
		return -1;
	if(fs_delete(file1) == -1)
		return -1;
	if(fs_close(fildes2) == -1)
		return -1;
	if(fs_delete(file2) == -1)
		return -1;
	if(fs_close(fildes3) == -1)
		return -1;
	if(fs_delete(file3) == -1)
		return -1;
	if(fs_close(fildes4) == -1)
		return -1;
	if(fs_delete(file4) == -1)
		return -1;
	if(fs_close(fildes5) == -1)
		return -1;
	if(fs_delete(file5) == -1)
		return -1;
	if(umount_fs("Test") == -1)
		return -1;

	return 0;
}

int fs_large_file()
{
	char file1[] = "File 1";
	char file2[] = "File 2";

	char string[] = "Testing string";

	printf("--- Mount a filesystem, create a file, and open the file\n");
	if(mount_fs("Test") == -1)
		return -1;
	if(fs_create(file1) == -1)
		return -1;
	int fildes1 = fs_open(file1);
	if(fildes1 == -1)
		return -1;

	char long_string[] = "The Project Gutenberg EBook of Adventures of Huckleberry Finn, Complete by Mark Twain (Samuel Clemens) This eBook is for the use of anyone anywhere at no cost and with almost no restrictions whatsoever. You may copy it, give it away or re-use it under the terms of the Project Gutenberg License included with this eBook or online at www.gutenberg.net Title: Adventures of Huckleberry Finn, Complete Author: Mark Twain (Samuel Clemens) Release Date: August 20, 2006 [EBook #76] Last Updated: April 18, 2015] Language: English Character set encoding: ISO-8859-1 *** START OF THIS PROJECT GUTENBERG EBOOK HUCKLEBERRY FINN *** Produced by David Widger ADVENTURES OF HUCKLEBERRY FINN (Tom Sawyer s Comrade) By Mark Twain Complete CONTENTS. CHAPTER I. Civilizing Huck.--Miss Watson.--Tom Sawyer Waits. CHAPTER II. The Boys Escape Jim.--Torn Sawyer s Gang.--Deep-laid Plans. CHAPTER III. A Good Going-over.--Grace Triumphant.-- One of Tom Sawyers s Lies . CHAPTER IV. Huck and the Judge.--Superstition. CHAPTER V. Huck s Father.--The Fond Parent.--Reform. CHAPTER VI. He Went for Judge Thatcher.--Huck Decided to Leave.--Political Economy.--Thrashing Around. CHAPTER VII. Laying for Him.--Locked in the Cabin.--Sinking the Body.--Resting. CHAPTER VIII. Sleeping in the Woods.--Raising the Dead.--Exploring the Island.--Finding Jim.--Jim s Escape.--Signs.--Balum. CHAPTER IX. The Cave.--The Floating House. CHAPTER X. The Find.--Old Hank Bunker.--In Disguise. CHAPTER XI. Huck and the Woman.--The Search.--Prevarication.--Going to Goshen. CHAPTER XII. Slow Navigation.--Borrowing Things.--Boarding the Wreck.--The Plotters.--Hunting for the Boat. CHAPTER XIII. Escaping from the Wreck.--The Watchman.--Sinking. CHAPTER XIV. A General Good Time.--The Harem.--French. CHAPTER XV. Huck Loses the Raft.--In the Fog.--Huck Finds the Raft.--Trash. CHAPTER XVI. Expectation.--A White Lie.--Floating Currency.--Running by Cairo.--Swimming Ashore. CHAPTER XVII. An Evening Call.--The Farm in Arkansaw.--Interior Decorations.--Stephen Dowling Bots.--Poetical Effusions. CHAPTER XVIII. Col. Grangerford.--Aristocracy.--Feuds.--The Testament.--Recovering the Raft.--The Wood--pile.--Pork and Cabbage. CHAPTER XIX. Tying Up Day--times.--An Astronomical Theory.--Running a Temperance Revival.--The Duke of Bridgewater.--The Troubles of Royalty. CHAPTER XX. Huck Explains.--Laying Out a Campaign.--Working the Camp--meeting.--A Pirate at the Camp--meeting.--The Duke as a Printer. CHAPTER XXI. Sword Exercise.--Hamlet s Soliloquy.--They Loafed Around Town.--A Lazy Town.--Old Boggs.--Dead. CHAPTER XXII. Sherburn.--Attending the Circus.--Intoxication in the Ring.--The Thrilling Tragedy. CHAPTER XXIII. Sold.--Royal Comparisons.--Jim Gets Home-sick. CHAPTER XXIV. Jim in Royal Robes.--They Take a Passenger.--Getting Information.--Family Grief. CHAPTER XXV. Is It Them?--Singing the Doxologer. --Awful Square--Funeral Orgies.--A Bad Investment . CHAPTER XXVI. A Pious King.--The King s Clergy.--She Asked His Pardon.--Hiding in the Room.--Huck Takes the Money. CHAPTER XXVII. The Funeral.--Satisfying Curiosity.--Suspicious of Huck,--Quick Sales and Small. CHAPTER XXVIII. The Trip to England.-- The Brute! --Mary Jane Decides to Leave.--Huck Parting with Mary Jane.--Mumps.--The Opposition Line. CHAPTER XXIX. Contested Relationship.--The King Explains the Loss.--A Question of Handwriting.--Digging up the Corpse.--Huck Escapes. CHAPTER XXX. The King Went for Him.--A Royal Row.--Powerful Mellow. CHAPTER XXXI. Ominous Plans.--News from Jim.--Old Recollections.--A Sheep Story.--Valuable Information. CHAPTER XXXII. Still and Sunday--like.--Mistaken Identity.--Up a Stump.--In a Dilemma. CHAPTER XXXIII. A Nigger Stealer.--Southern Hospitality.--A Pretty Long Blessing.--Tar and Feathers. CHAPTER XXXIV. The Hut by the Ash Hopper.--Outrageous.--Climbing the Lightning Rod.--Troubled with Witches. CHAPTER XXXV. Escaping Properly.--Dark Schemes.--Discrimination in Stealing.--A Deep Hole. CHAPTER XXXVI. The Lightning Rod.--His Level Best.--A Bequest to Posterity.--A High Figure. CHAPTER XXXVII. The Last Shirt.--Mooning Around.--Sailing Orders.--The Witch Pie. CHAPTER XXXVIII. The Coat of Arms.--A Skilled Superintendent.--Unpleasant Glory.--A Tearful Subject. CHAPTER XXXIX. Rats.--Lively Bed--fellows.--The Straw Dummy. CHAPTER XL. Fishing.--The Vigilance Committee.--A Lively Run.--Jim Advises a Doctor. CHAPTER XLI. The Doctor.--Uncle Silas.--Sister Hotchkiss.--Aunt Sally in Trouble. CHAPTER XLII. Tom Sawyer Wounded.--The Doctor s Story.--Tom Confesses.--Aunt Polly Arrives.--Hand Out Them Letters . CHAPTER THE LAST. Out of Bondage.--Paying the Captive.--Yours Truly, Huck Finn. ILLUSTRATIONS. The Widows Moses and the Bulrushers Miss Watson Huck Stealing Away They Tip-toed Along Jim Tom Sawyer s Band of Robbers Huck Creeps into his Window Miss Watson s Lecture The Robbers Dispersed Rubbing the Lamp ! ! ! ! Judge Thatcher surprised Jim Listening Pap Huck and his Father Reforming the Drunkard Falling from Grace The Widows Moses and the Bulrushers Miss Watson Huck Stealing Away They Tip-toed Along Jim Tom Sawyer s Band of Robbers Huck Creeps into his Window Miss Watson s Lecture The Robbers Dispersed Rubbing the Lamp ! ! ! ! Judge Thatcher surprised Jim Listening Pap Huck and his Father Reforming the Drunkard Falling from Grace Getting out of the Way Solid Comfort Thinking it Over Raising a Howl Git Up The Shanty Shooting the Pig Taking a Rest In the Woods Watching the Boat Discovering the Camp Fire Jim and the Ghost Misto Bradish s Nigger Exploring the Cave In the Cave Jim sees a Dead Man They Found Eight Dollars Jim and the Snake Old Hank Bunker A Fair Fit Come In Him and another Man She puts up a Snack Hump Yourself On the Raft He sometimes Lifted a Chicken Please don t, Bill It ain t Good Morals Oh! Lordy, Lordy! In a Fix Hello, What s Up? The Wreck We turned in and Slept Turning over the Truck Solomon and his Million Wives The story of Sollermun We Would Sell the Raft Among the Snags Asleep on the Raft Something being Raftsman Boy, that s a Lie Here I is, Huck Climbing up the Bank Who s There? Buck It made Her look Spidery They got him out and emptied Him The House Col. Grangerford Young Harney Shepherdson Miss Charlotte And asked me if I Liked Her Behind the Wood-pile Hiding Day-times And Dogs a-Coming By rights I am a Duke! I am the Late Dauphin Tail Piece On the Raft The King as Juliet Courting on the Sly A Pirate for Thirty Years Another little Job Practizing Hamlet s Soliloquy Gimme a Chaw A Little Monthly Drunk The Death of Boggs Sherburn steps out A Dead Head He shed Seventeen Suits Tragedy Their Pockets Bulged Henry the Eighth in Boston Harbor Harmless Adolphus He fairly emptied that Young Fellow Alas, our Poor Brother You Bet it is Leaking Making up the Deffisit Going for him The Doctor The Bag of Money The Cubby Supper with the Hare-Lip Honest Injun The Duke looks under the Bed Huck takes the Money A Crack in the Dining-room Door The Undertaker He had a Rat! Was you in my Room? Jawing In Trouble Indignation How to Find Them He Wrote Hannah with the Mumps The Auction The True Brothers The Doctor leads Huck The Duke Wrote Gentlemen, Gentlemen! Jim Lit Out The King shakes Huck The Duke went for Him Spanish Moss Who Nailed Him? Thinking He gave him Ten Cents Striking for the Back Country Still and Sunday-like She hugged him tight Who do you reckon it is? It was Tom Sawyer Mr. Archibald Nichols, I presume? A pretty long Blessing Traveling By Rail Vittles A Simple Job Witches Getting Wood One of the Best Authorities The Breakfast-Horn Smouching the Knives Going down the Lightning-Rod Stealing spoons Tom advises a Witch Pie The Rubbage-Pile Missus, dey s a Sheet Gone In a Tearing Way One of his Ancestors Jim s Coat of Arms A Tough Job Buttons on their Tails Irrigation Keeping off Dull Times Sawdust Diet Trouble is Brewing Fishing Every one had a Gun Tom caught on a Splinter Jim advises a Doctor The Doctor Uncle Silas in Danger Old Mrs. Hotchkiss Aunt Sally talks to Huck Tom Sawyer wounded The Doctor speaks for Jim Tom rose square up in Bed Hand out them Letters Out of Bondage Tom s Liberality Yours Truly EXPLANATORY IN this book a number of dialects are used, to wit: the Missouri negro dialect  the extremest form of the backwoods Southwestern dialect  the ordinary Pike County dialect  and four modified varieties of this last. The shadings have not been done in a haphazard fashion, or by guesswork  but painstakingly, and with the trustworthy guidance and support of personal familiarity with these several forms of speech. I make this explanation for the reason that without it many readers would suppose that all these characters were trying to talk alike and not succeeding. THE AUTHOR. HUCKLEBERRY FINN Scene: The Mississippi Valley Time: Forty to fifty years ago CHAPTER I. YOU don t know about me without you have read a book by the name of The Adventures of Tom Sawyer  but that ain t no matter. That book was made by Mr. Mark Twain, and he told the truth, mainly. There was things which he stretched, but mainly he told the truth. That is nothing. I never seen anybody but lied one time or another, without it was Aunt Polly, or the widow, or maybe Mary. Aunt Polly--Tom s Aunt Polly, she is--and Mary, and the Widow Douglas is all told about in that book, which is mostly a true book, with some stretchers, as I said before. Now the way that the book winds up is this: Tom and me found the money that the robbers hid in the cave, and it made us rich. We got six thousand dollars apiece--all gold. It was an awful sight of money when it was piled up. Well, Judge Thatcher he took it and put it out at interest, and it fetched us a dollar a day apiece all the year round--more than a body could tell what to do with. The Widow Douglas she took me for her son, and allowed she would sivilize me  but it was rough living in the house all the time, considering how dismal regular and decent the widow was in all her ways  and so when I couldn t stand it no longer I lit out. I got into my old rags and my sugar-hogshead again, and was free and satisfied. But Tom Sawyer he hunted me up and said he was going to start a band of robbers, and I might join if I would go back to the widow and be respectable. So I went back. The widow she cried over me, and called me a poor lost lamb, and she called me a lot of other names, too, but she never meant no harm by it. She put me in them new clothes again, and I couldn t do nothing but sweat and sweat, and feel all cramped up. Well, then, the old thing commenced again. The widow rung a bell for supper, and you had to come to time. When you got to the table you couldn t go right to eating, but you had to wait for the widow to tuck down her head and grumble a little over the victuals, though there warn t really anything the matter with them,--that is, nothing only everything was cooked by itself. In a barrel of odds and ends it is different  things get mixed up, and the juice kind of swaps around, and the things go better. After supper she got out her book and learned me about Moses and the Bulrushers, and I was in a sweat to find out all about him  but by and by she let it out that Moses had been dead a considerable long time  so then I didn t care no more about him, because I don t take no stock in dead people. Pretty soon I wanted to smoke, and asked the widow to let me. But she wouldn t. She said it was a mean practice and wasn t clean, and I must try to not do it any more. That is just the way with some people. They get down on a thing when they don t know nothing about it. Here she was a-bothering about Moses, which was no kin to her, and no use to anybody, being gone, you see, yet finding a power of fault with me for doing a thing that had some good in it. And she took snuff, too  of course that was all right, because she done it herself. Her sister, Miss Watson, a tolerable slim old maid, with goggles on, had just come to live with her, and took a set at me now with a spelling-book. She worked me middling hard for about an hour, and then the widow made her ease up. I couldn t stood it much longer. Then for an hour it was deadly dull, and I was fidgety. Miss Watson would say, Don t put your feet up there, Huckleberry  and Don t scrunch up like that, Huckleberry--set up straight  and pretty soon she would say, Don t gap and stretch like that, Huckleberry--why don t you try to behave? Then she told me all about the bad place, and I said I wished I was there. She got mad then, but I didn t mean no harm. All I wanted was to go somewheres  all I wanted was a change, I warn t particular. She said it was wicked to say what I said  said she wouldn t say it for the whole world  she was going to live so as to go to the good place. Well, I couldn t see no advantage in going where she was going, so I made up my mind I wouldn t try for it. But I never said so, because it would only make trouble, and wouldn t do no good. Now she had got a start, and she went on and told me all about the good place. She said all a body would have to do there was to go around all day long with a harp and sing, forever and ever. So I didn t think much of it. But I never said so. I asked her if she reckoned Tom Sawyer would go there, and she said not by a considerable sight. I was glad about that, because I wanted him and me to be together. Miss Watson she kept pecking at me, and it got tiresome and lonesome. By and by they fetched the niggers in and had prayers, and then everybody was off to bed. I went up to my room with a piece of candle, and put it on the table. Then I set down in a chair by the window and tried to think of something cheerful, but it warn t no use. I felt so lonesome I most wished I was dead. The stars were shining, and the leaves rustled in the woods ever so mournful  and I heard an owl, away off, who-whooing about somebody that was dead, and a whippowill and a dog crying about somebody that was going to die  and the wind was trying to whisper something to me, and I couldn t make out what it was, and so it made the cold shivers run over me. Then away out in the woods I heard that kind of a sound that a ghost makes when it wants to tell about something that s on its mind and can t make itself understood, and so can t rest easy in its grave, and has to go about that way every night grieving. I got so down-hearted and scared I did wish I had some company. Pretty soon a spider went crawling up my shoulder, and I flipped it off and it lit in the candle  and before I could budge it was all shriveled up. I didn t need anybody to tell me that that was an awful bad sign and would fetch me some bad luck, so I was scared and most shook the clothes off of me. I got up and turned around in my tracks three times and crossed my breast every time  and then I tied up a little lock of my hair with a thread to keep witches away. But I hadn t no confidence. You do that when you ve lost a horseshoe that you ve found, instead of nailing it up over the door, but I hadn t ever heard anybody say it was any way to keep off bad luck when you d killed a spider. I set down again, a-shaking all over, and got out my pipe for a smoke  for the house was all as still as death now, and so the widow wouldn t know. Well, after a long time I heard the clock away off in the town go boom--boom--boom--twelve licks  and all still again--stiller than ever. Pretty soon I heard a twig snap down in the dark amongst the trees--something was a stirring. I set still and listened. Directly I could just barely hear a me-yow! me-yow! down there. That was good! Says I, me-yow! me-yow! as soft as I could, and then I put out the light and scrambled out of the window on to the shed. Then I slipped down to the ground and crawled in among the trees, and, sure enough, there was Tom Sawyer waiting for me. CHAPTER II. WE went tiptoeing along a path amongst the trees back towards the end of the widow s garden, stooping down so as the branches wouldn t scrape our heads. When we was passing by the kitchen I fell over a root and made a noise. We scrouched down and laid still. Miss Watson s big nigger, named Jim, was setting in the kitchen door  we could see him pretty clear, because there was a light behind him. He got up and stretched his neck out about a minute, listening. Then he says: Who dah? He listened some more  then he come tiptoeing down and stood right between us  we could a touched him, nearly. Well, likely it was minutes and minutes that there warn t a sound, and we all there so close together. There was a place on my ankle that got to itching, but I dasn t scratch it  and then my ear begun to itch  and next my back, right between my shoulders. Seemed like I d die if I couldn t scratch. Well, I ve noticed that thing plenty times since. If you are with the quality, or at a funeral, or trying to go to sleep when you ain t sleepy--if you are anywheres where it won t do for you to scratch, why you will itch all over in upwards of a thousand places. Pretty soon Jim says: Say, who is you? Whar is you? Dog my cats ef I didn hear sumf n. Well, I know what I s gwyne to do: I s gwyne to set down here and listen tell I hears it agin. So he set down on the ground betwixt me and Tom. He leaned his back up against a tree, and stretched his legs out till one of them most touched one of mine. My nose begun to itch. It itched till the tears come into my eyes. But I dasn t scratch. Then it begun to itch on the inside. Next I got to itching underneath. I didn t know how I was going to set still. This miserableness went on as much as six or seven minutes  but it seemed a sight longer than that. I was itching in eleven different places now. I reckoned I couldn t stand it more n a minute longer, but I set my teeth hard and got ready to try. Just then Jim begun to breathe heavy  next he begun to snore--and then I was pretty soon comfortable again. Tom he made a sign to me--kind of a little noise with his mouth--and we went creeping away on our hands and knees. When we was ten foot off Tom whispered to me, and wanted to tie Jim to the tree for fun. But I said no  he might wake and make a disturbance, and then they d find out I warn t in. Then Tom said he hadn t got candles enough, and he would slip in the kitchen and get some more. I didn t want him to try. I said Jim might wake up and come. But Tom wanted to resk it  so we slid in there and got three candles, and Tom laid five cents on the table for pay. Then we got out, and I was in a sweat to get away  but nothing would do Tom but he must crawl to where Jim was, on his hands and knees, and play something on him. I waited, and it seemed a good while, everything was so still and lonesome. As soon as Tom was back we cut along the path, around the garden fence, and by and by fetched up on the steep top of the hill the other side of the house. Tom said he slipped Jim s hat off of his head and hung it on a limb right over him, and Jim stirred a little, but he didn t wake. Afterwards Jim said the witches be witched him and put him in a trance, and rode him all over the State, and then set him under the trees again, and hung his hat on a limb to show who done it. And next time Jim told it he said they rode him down to New Orleans  and, after that, every time he told it he spread it more and more, till by and by he said they rode him all over the world, and tired him most to death, and his back was all over saddle-boils. Jim was monstrous proud about it, and he got so he wouldn t hardly notice the other niggers. Niggers would come miles to hear Jim tell about it, and he was more looked up to than any nigger in that country. Strange niggers would stand with their mouths open and look him all over, same as if he was a wonder. Niggers is always talking about witches in the dark by the kitchen fire  but whenever one was talking and letting on to know all about such things, Jim would happen in and say, Hm! What you know bout witches? and that nigger was corked up and had to take a back seat. Jim always kept that five-center piece round his neck with a string, and said it was a charm the devil give to him with his own hands, and told him he could cure anybody with it and fetch witches whenever he wanted to just by saying something to it  but he never told what it was he said to it. Niggers would come from all around there and give Jim anything they had, just for a sight of that five-center piece  but they wouldn t touch it, because the devil had had his hands on it. Jim was most ruined for a servant, because he got stuck up on account of having seen the devil and been rode by witches. Well, when Tom and me got to the edge of the hilltop we looked away down into the village and could see three or four lights twinkling, where there was sick folks, maybe  and the stars over us was sparkling ever so fine  and down by the village was the river, a whole mile broad, and awful still and grand. We went down the hill and found Jo Harper and Ben Rogers, and two or three more of the boys, hid in the old tanyard. So we unhitched a skiff and pulled down the river two mile and a half, to the big scar on the hillside, and went ashore. We went to a clump of bushes, and Tom made everybody swear to keep the secret, and then showed them a hole in the hill, right in the thickest part of the bushes. Then we lit the candles, and crawled in on our hands and knees. We went about two hundred yards, and then the cave opened up. Tom poked about amongst the passages, and pretty soon ducked under a wall where you wouldn t a noticed that there was a hole. We went along a narrow place and got into a kind of room, all damp and sweaty and cold, and there we stopped. Tom says: Now, we ll start this band of robbers and call it Tom Sawyer s Gang. Everybody that wants to join has got to take an oath, and write his name in blood. Everybody was willing. So Tom got out a sheet of paper that he had wrote the oath on, and read it. It swore every boy to stick to the band, and never tell any of the secrets  and if anybody done anything to any boy in the band, whichever boy was ordered to kill that person and his family must do it, and he mustn t eat and he mustn t sleep till he had killed them and hacked a cross in their breasts, which was the sign of the band. And nobody that didn t belong to the band could use that mark, and if he did he must be sued  and if he done it again he must be killed. And if anybody that belonged to the band told the secrets, he must have his throat cut, and then have his carcass burnt up and the ashes scattered all around, and his name blotted off of the list with blood and never mentioned again by the gang, but have a curse put on it and be forgot forever. Everybody said it was a real beautiful oath, and asked Tom if he got it out of his own head. He said, some of it, but the rest was out of pirate-books and robber-books, and every gang that was high-toned had it. Some thought it would be good to kill the _families_ of boys that told the secrets. Tom said it was a good idea, so he took a pencil and wrote it in. Then Ben Rogers says: Here s Huck Finn, he hain t got no family  what you going to do bout him? Well, hain t he got a father? says Tom Sawyer. Yes, he s got a father, but you can t never find him these days. He used to lay drunk with the hogs in the tanyard, but he hain t been seen in these parts for a year or more. They talked it over, and they was going to rule me out, because they said every boy must have a family or somebody to kill, or else it wouldn t be fair and square for the others. Well, nobody could think of anything to do--everybody was stumped, and set still. I was most ready to cry  but all at once I thought of a way, and so I offered them Miss Watson--they could kill her. Everybody said: Oh, she ll do. That s all right. Huck can come in. Then they all stuck a pin in their fingers to get blood to sign with, and I made my mark on the paper. Now, says Ben Rogers, what s the line of business of this Gang? Nothing only robbery and murder, Tom said. But who are we going to rob?--houses, or cattle, or-- Stuff! stealing cattle and such things ain t robbery  it s burglary, says Tom Sawyer. We ain t burglars. That ain t no sort of style. We are highwaymen. We stop stages and carriages on the road, with masks on, and kill the people and take their watches and money. Must we always kill the people? Oh, certainly. It s best. Some authorities think different, but mostly it s considered best to kill them--except some that you bring to the cave here, and keep them till they re ransomed. Ransomed? What s that? I don t know. But that s what they do. I ve seen it in books  and so of course that s what we ve got to do. But how can we do it if we don t know what it is? Why, blame it all, we ve _got_ to do it. Don t I tell you it s in the books? Do you want to go to doing different from what s in the books, and get things all muddled up? Oh, that s all very fine to _say_, Tom Sawyer, but how in the nation are these fellows going to be ransomed if we don t know how to do it to them?--that s the thing I want to get at. Now, what do you reckon it is? Well, I don t know. But per aps if we keep them till they re ransomed, it means that we keep them till they re dead. Now, that s something _like_. That ll answer. Why couldn t you said that before? We ll keep them till they re ransomed to death  and a bothersome lot they ll be, too--eating up everything, and always trying to get loose. How you talk, Ben Rogers. How can they get loose when there s a guard over them, ready to shoot them down if they move a peg? A guard! Well, that _is_ good. So somebody s got to set up all night and never get any sleep, just so as to watch them. I think that s foolishness. Why can t a body take a club and ransom them as soon as they get here? Because it ain t in the books so--that s why. Now, Ben Rogers, do you want to do things regular, or don t you?--that s the idea. Don t you reckon that the people that made the books knows what s the correct thing to do? Do you reckon _you_ can learn em anything? Not by a good deal. No, sir, we ll just go on and ransom them in the regular way. All right. I don t mind  but I say it s a fool way, anyhow. Say, do we kill the women, too? Well, Ben Rogers, if I was as ignorant as you I wouldn t let on. Kill the women? No  nobody ever saw anything in the books like that. You fetch them to the cave, and you re always as polite as pie to them  and by and by they fall in love with you, and never want to go home any more. Well, if that s the way I m agreed, but I don t take no stock in it. Mighty soon we ll have the cave so cluttered up with women, and fellows waiting to be ransomed, that there won t be no place for the robbers. But go ahead, I ain t got nothing to say. Little Tommy Barnes was asleep now, and when they waked him up he was scared, and cried, and said he wanted to go home to his ma, and didn t want to be a robber any more. So they all made fun of him, and called him cry-baby, and that made him mad, and he said he would go straight and tell all the secrets. But Tom give him five cents to keep quiet, and said we would all go home and meet next week, and rob somebody and kill some people. Ben Rogers said he couldn t get out much, only Sundays, and so he wanted to begin next Sunday  but all the boys said it would be wicked to do it on Sunday, and that settled the thing. They agreed to get together and fix a day as soon as they could, and then we elected Tom Sawyer first captain and Jo Harper second captain of the Gang, and so started home. I clumb up the shed and crept into my window just before day was breaking. My new clothes was all greased up and clayey, and I was dog-tired. CHAPTER III. WELL, I got a good going-over in the morning from old Miss Watson on account of my clothes  but the widow she didn t scold, but only cleaned off the grease and clay, and looked so sorry that I thought I would behave awhile if I could. Then Miss Watson she took me in the closet and prayed, but nothing come of it. She told me to pray every day, and whatever I asked for I would get it. But it warn t so. I tried it. Once I got a fish-line, but no hooks. It warn t any good to me without hooks. I tried for the hooks three or four times, but somehow I couldn t make it work. By and by, one day, I asked Miss Watson to try for me, but she said I was a fool. She never told me why, and I couldn t make it out no way. I set down one time back in the woods, and had a long think about it. I says to myself, if a body can get anything they pray for, why don t Deacon Winn get back the money he lost on pork? Why can t the widow get back her silver snuffbox that was stole? Why can t Miss Watson fat up? No, says I to my self, there ain t nothing in it. I went and told the widow about it, and she said the thing a body could get by praying for it was spiritual gifts. This was too many for me, but she told me what she meant--I must help other people, and do everything I could for other people, and look out for them all the time, and never think about myself. This was including Miss Watson, as I took it. I went out in the woods and turned it over in my mind a long time, but I couldn t see no advantage about it--except for the other people  so at last I reckoned I wouldn t worry about it any more, but just let it go. Sometimes the widow would take me one side and talk about Providence in a way to make a body s mouth water  but maybe next day Miss Watson would take hold and knock it all down again. I judged I could see that there was two Providences, and a poor chap would stand considerable show with the widow s Providence, but if Miss Watson s got him there warn t no help for him any more. I thought it all out, and reckoned I would belong to the widow s if he wanted me, though I couldn t make out how he was a-going to be any better off then than what he was before, seeing I was so ignorant, and so kind of low-down and ornery. Pap he hadn t been seen for more than a year, and that was comfortable for me  I didn t want to see him no more. He used to always whale me when he was sober and could get his hands on me  though I used to take to the woods most of the time when he was around. Well, about this time he was found in the river drownded, about twelve mile above town, so people said. They judged it was him, anyway  said this drownded man was just his size, and was ragged, and had uncommon long hair, which was all like pap  but they couldn t make nothing out of the face, because it had been in the water so long it warn t much like a face at all. They said he was floating on his back in the water. They took him and buried him on the bank. But I warn t comfortable long, because I happened to think of something. I knowed mighty well that a drownded man don t float on his back, but on his face. So I knowed, then, that this warn t pap, but a woman dressed up in a man s clothes. So I was uncomfortable again. I judged the old man would turn up again by and by, though I wished he wouldn t. We played robber now and then about a month, and then I resigned. All the boys did. We hadn t robbed nobody, hadn t killed any people, but only just pretended. We used to hop out of the woods and go charging down on hog-drivers and women in carts taking garden stuff to market, but we never hived any of them. Tom Sawyer called the hogs ingots, and he called the turnips and stuff julery, and we would go to the cave and powwow over what we had done, and how many people we had killed and marked. But I couldn t see no profit in it. One time Tom sent a boy to run about town with a blazing stick, which he called a slogan (which was the sign for the Gang to get together), and then he said he had got secret news by his spies that next day a whole parcel of Spanish merchants and rich A-rabs was going to camp in Cave Hollow with two hundred elephants, and six hundred camels, and over a thousand sumter mules, all loaded down with di monds, and they didn t have only a guard of four hundred soldiers, and so we would lay in ambuscade, as he called it, and kill the lot and scoop the things. He said we must slick up our swords and guns, and get ready. He never could go after even a turnip-cart but he must have the swords and guns all scoured up for it, though they was only lath and broomsticks, and you might scour at them till you rotted, and then they warn t worth a mouthful of ashes more than what they was before. I didn t believe we could lick such a crowd of Spaniards and A-rabs, but I wanted to see the camels and elephants, so I was on hand next day, Saturday, in the ambuscade  and when we got the word we rushed out of the woods and down the hill. But there warn t no Spaniards and A-rabs, and there warn t no camels nor no elephants. It warn t anything but a Sunday-school picnic, and only a primer-class at that. We busted it up, and chased the children up the hollow  but we never got anything but some doughnuts and jam, though Ben Rogers got a rag doll, and Jo Harper got a hymn-book and a tract  and then the teacher charged in, and made us drop everything and cut. I didn t see no di monds, and I told Tom Sawyer so. He said there was loads of them there, anyway  and he said there was A-rabs there, too, and elephants and things. I said, why couldn t we see them, then? He said if I warn t so ignorant, but had read a book called Don Quixote, I would know without asking. He said it was all done by enchantment. He said there was hundreds of soldiers there, and elephants and treasure, and so on, but we had enemies which he called magicians  and they had turned the whole thing into an infant Sunday-school, just out of spite. I said, all right  then the thing for us to do was to go for the magicians. Tom Sawyer said I was a numskull. Why, said he, a magician could call up a lot of genies, and they would hash you up like nothing before you could say Jack Robinson. They are as tall as a tree and as big around as a church. Well, I says, s pose we got some genies to help _us_--can t we lick the other crowd then? How you going to get them? I don t know. How do _they_ get them? Why, they rub an old tin lamp or an iron ring, and then the genies come tearing in, with the thunder and lightning a-ripping around and the smoke a-rolling, and everything they re told to do they up and do it. They don t think nothing of pulling a shot-tower up by the roots, and belting a Sunday-school superintendent over the head with it--or any other man. Who makes them tear around so? Why, whoever rubs the lamp or the ring. They belong to whoever rubs the lamp or the ring, and they ve got to do whatever he says. If he tells them to build a palace forty miles long out of di monds, and fill it full of chewing-gum, or whatever you want, and fetch an emperor s daughter from China for you to marry, they ve got to do it--and they ve got to do it before sun-up next morning, too. And more: they ve got to waltz that palace around over the country wherever you want it, you understand. Well, says I, I think they are a pack of flat-heads for not keeping the palace themselves stead of fooling them away like that. And what s more--if I was one of them I would see a man in Jericho before I would drop my business and come to him for the rubbing of an old tin lamp. How you talk, Huck Finn. Why, you d _have_ to come when he rubbed it, whether you wanted to or not. What! and I as high as a tree and as big as a church? All right, then  I _would_ come  but I lay I d make that man climb the highest tree there was in the country. Shucks, it ain t no use to talk to you, Huck Finn. You don t seem to know anything, somehow--perfect saphead. I thought all this over for two or three days, and then I reckoned I would see if there was anything in it. I got an old tin lamp and an iron ring, and went out in the woods and rubbed and rubbed till I sweat like an Injun, calculating to build a palace and sell it  but it warn t no use, none of the genies come. So then I judged that all that stuff was only just one of Tom Sawyer s lies. I reckoned he believed in the A-rabs and the elephants, but as for me I think different. It had all the marks of a Sunday-school. CHAPTER IV. WELL, three or four months run along, and it was well into the winter now. I had been to school most all the time and could spell and read and write just a little, and could say the multiplication table up to six times seven is thirty-five, and I don t reckon I could ever get any further than that if I was to live forever. I don t take no stock in mathematics, anyway. At first I hated the school, but by and by I got so I could stand it. Whenever I got uncommon tired I played hookey, and the hiding I got next day done me good and cheered me up. So the longer I went to school the easier it got to be. I was getting sort of used to the widow s ways, too, and they warn t so raspy on me. Living in a house and sleeping in a bed pulled on me pretty tight mostly, but before the cold weather I used to slide out and sleep in the woods sometimes, and so that was a rest to me. I liked the old ways best, but I was getting so I liked the new ones, too, a little bit. The widow said I was coming along slow but sure, and doing very satisfactory. She said she warn t ashamed of me. One morning I happened to turn over the salt-cellar at breakfast. I reached for some of it as quick as I could to throw over my left shoulder and keep off the bad luck, but Miss Watson was in ahead of me, and crossed me off. She says, Take your hands away, Huckleberry  what a mess you are always making! The widow put in a good word for me, but that warn t going to keep off the bad luck, I knowed that well enough. I started out, after breakfast, feeling worried and shaky, and wondering where it was going to fall on me, and what it was going to be. There is ways to keep off some kinds of bad luck, but this wasn t one of them kind  so I never tried to do anything, but just poked along low-spirited and on the watch-out.";
	printf("--- Write a 4k file to the filesystem\n");
	int bytes_copied=0;
	int ii;
	for(ii = 0; ii<460; ii++)
	{
		//printf("Written %d\n", ii);

		if(fs_write(fildes1, long_string+bytes_copied, 38000) == -1)
			return -1;
	}

	printf("--- Try to create another file and write to it\n");
	if(fs_create(file2) == -1)
		return -1;
	int fildes2 = fs_open(file2);
	if(fildes2 == -1)
		return -1;
	if(fs_write(fildes2, string, 10) != 0)
		return -1;

	printf("--- Truncate the large file and then try to write to the new file\n");
	if(fs_truncate(fildes1, fs_get_filesize(fildes1)-10000) == -1)
		return -1;
	if(fs_write(fildes2, string, 10) != 10)
		return -1;

	printf("--- Read the first 38000 from the first file\n");
	char buf[38000];
	if(fs_lseek(fildes1, 0) == -1)
		return -1;
	if(fs_read(fildes1, buf, 38000) != 38000)
		return -1;
	if(memcmp(buf, long_string, 38000) != 0)
		return -1;

	printf("--- Cleaning up and finishing test\n");
	if(fs_close(fildes1) == -1)
		return -1;
	if(fs_close(fildes2) == -1)
		return -1;
	if(fs_delete(file1) == -1)
		return -1;
	if(fs_delete(file2) == -1)
		return -1;
	if(umount_fs("Test") == -1)
		return -1;

	return 0;
}

int fs_inclusive_test()
{
	// Test data
	char name1[] = "file1";
	char name2[] = "file2";
	char name3[] = "file3";
	char name4[] = "file4";
	char name5[] = "file5";

	char string1[] = "The Project Gutenberg EBook of Adventures of Huckleberry Finn, Complete by Mark Twain (Samuel Clemens) This eBook is for the use of anyone anywhere at no cost and with almost no restrictions whatsoever. You may copy it, give it away or re-use it under the terms of the Project Gutenberg License included with this eBook or online at www.gutenberg.net Title: Adventures of Huckleberry Finn, Complete Author: Mark Twain (Samuel Clemens) Release Date: August 20, 2006 [EBook #76] Last Updated: April 18, 2015] Language: English Character set encoding: ISO-8859-1 *** START OF THIS PROJECT GUTENBERG EBOOK HUCKLEBERRY FINN *** Produced by David Widger ADVENTURES OF HUCKLEBERRY FINN (Tom Sawyer s Comrade) By Mark Twain Complete CONTENTS. CHAPTER I. Civilizing Huck.--Miss Watson.--Tom Sawyer Waits. CHAPTER II. The Boys Escape Jim.--Torn Sawyer s Gang.--Deep-laid Plans. CHAPTER III. A Good Going-over.--Grace Triumphant.-- One of Tom Sawyers s Lies . CHAPTER IV. Huck and the Judge.--Superstition. CHAPTER V. Huck s Father.--The Fond Parent.--Reform. CHAPTER VI. He Went for Judge Thatcher.--Huck Decided to Leave.--Political Economy.--Thrashing Around. CHAPTER VII. Laying for Him.--Locked in the Cabin.--Sinking the Body.--Resting. CHAPTER VIII. Sleeping in the Woods.--Raising the Dead.--Exploring the Island.--Finding Jim.--Jim s Escape.--Signs.--Balum. CHAPTER IX. The Cave.--The Floating House. CHAPTER X. The Find.--Old Hank Bunker.--In Disguise. CHAPTER XI. Huck and the Woman.--The Search.--Prevarication.--Going to Goshen. CHAPTER XII. Slow Navigation.--Borrowing Things.--Boarding the Wreck.--The Plotters.--Hunting for the Boat. CHAPTER XIII. Escaping from the Wreck.--The Watchman.--Sinking. CHAPTER XIV. A General Good Time.--The Harem.--French. CHAPTER XV. Huck Loses the Raft.--In the Fog.--Huck Finds the Raft.--Trash. CHAPTER XVI. Expectation.--A White Lie.--Floating Currency.--Running by Cairo.--Swimming Ashore. CHAPTER XVII. An Evening Call.--The Farm in Arkansaw.--Interior Decorations.--Stephen Dowling Bots.--Poetical Effusions. CHAPTER XVIII. Col. Grangerford.--Aristocracy.--Feuds.--The Testament.--Recovering the Raft.--The Wood--pile.--Pork and Cabbage. CHAPTER XIX. Tying Up Day--times.--An Astronomical Theory.--Running a Temperance Revival.--The Duke of Bridgewater.--The Troubles of Royalty. CHAPTER XX. Huck Explains.--Laying Out a Campaign.--Working the Camp--meeting.--A Pirate at the Camp--meeting.--The Duke as a Printer. CHAPTER XXI. Sword Exercise.--Hamlet s Soliloquy.--They Loafed Around Town.--A Lazy Town.--Old Boggs.--Dead. CHAPTER XXII. Sherburn.--Attending the Circus.--Intoxication in the Ring.--The Thrilling Tragedy. CHAPTER XXIII. Sold.--Royal Comparisons.--Jim Gets Home-sick. CHAPTER XXIV. Jim in Royal Robes.--They Take a Passenger.--Getting Information.--Family Grief. CHAPTER XXV. Is It Them?--Singing the Doxologer. --Awful Square--Funeral Orgies.--A Bad Investment . CHAPTER XXVI. A Pious King.--The King s Clergy.--She Asked His Pardon.--Hiding in the Room.--Huck Takes the Money. CHAPTER XXVII. The Funeral.--Satisfying Curiosity.--Suspicious of Huck,--Quick Sales and Small. CHAPTER XXVIII. The Trip to England.-- The Brute! --Mary Jane Decides to Leave.--Huck Parting with Mary Jane.--Mumps.--The Opposition Line. CHAPTER XXIX. Contested Relationship.--The King Explains the Loss.--A Question of Handwriting.--Digging up the Corpse.--Huck Escapes. CHAPTER XXX. The King Went for Him.--A Royal Row.--Powerful Mellow. CHAPTER XXXI. Ominous Plans.--News from Jim.--Old Recollections.--A Sheep Story.--Valuable Information. CHAPTER XXXII. Still and Sunday--like.--Mistaken Identity.--Up a Stump.--In a Dilemma. CHAPTER XXXIII. A Nigger Stealer.--Southern Hospitality.--A Pretty Long Blessing.--Tar and Feathers. CHAPTER XXXIV. The Hut by the Ash Hopper.--Outrageous.--Climbing the Lightning Rod.--Troubled with Witches. CHAPTER XXXV. Escaping Properly.--Dark Schemes.--Discrimination in Stealing.--A Deep Hole. CHAPTER XXXVI. The Lightning Rod.--His Level Best.--A Bequest to Posterity.--A High Figure. CHAPTER XXXVII. The Last Shirt.--Mooning Around.--Sailing Orders.--The Witch Pie. CHAPTER XXXVIII. The Coat of Arms.--A Skilled Superintendent.--Unpleasant Glory.--A Tearful Subject. CHAPTER XXXIX. Rats.--Lively Bed--fellows.--The Straw Dummy. CHAPTER XL. Fishing.--The Vigilance Committee.--A Lively Run.--Jim Advises a Doctor. CHAPTER XLI. The Doctor.--Uncle Silas.--Sister Hotchkiss.--Aunt Sally in Trouble. CHAPTER XLII. Tom Sawyer Wounded.--The Doctor s Story.--Tom Confesses.--Aunt Polly Arrives.--Hand Out Them Letters . CHAPTER THE LAST. Out of Bondage.--Paying the Captive.--Yours Truly, Huck Finn. ILLUSTRATIONS. The Widows Moses and the Bulrushers Miss Watson Huck Stealing Away They Tip-toed Along Jim Tom Sawyer s Band of Robbers Huck Creeps into his Window Miss Watson s Lecture The Robbers Dispersed Rubbing the Lamp ! ! ! ! Judge Thatcher surprised Jim Listening Pap Huck and his Father Reforming the Drunkard Falling from Grace The Widows Moses and the Bulrushers Miss Watson Huck Stealing Away They Tip-toed Along Jim Tom Sawyer s Band of Robbers Huck Creeps into his Window Miss Watson s Lecture The Robbers Dispersed Rubbing the Lamp ! ! ! ! Judge Thatcher surprised Jim Listening Pap Huck and his Father Reforming the Drunkard Falling from Grace Getting out of the Way Solid Comfort Thinking it Over Raising a Howl Git Up The Shanty Shooting the Pig Taking a Rest In the Woods Watching the Boat Discovering the Camp Fire Jim and the Ghost Misto Bradish s Nigger Exploring the Cave In the Cave Jim sees a Dead Man They Found Eight Dollars Jim and the Snake Old Hank Bunker A Fair Fit Come In Him and another Man She puts up a Snack Hump Yourself On the Raft He sometimes Lifted a Chicken Please don t, Bill It ain t Good Morals Oh! Lordy, Lordy! In a Fix Hello, What s Up? The Wreck We turned in and Slept Turning over the Truck Solomon and his Million Wives The story of Sollermun We Would Sell the Raft Among the Snags Asleep on the Raft Something being Raftsman Boy, that s a Lie Here I is, Huck Climbing up the Bank Who s There? Buck It made Her look Spidery They got him out and emptied Him The House Col. Grangerford Young Harney Shepherdson Miss Charlotte And asked me if I Liked Her Behind the Wood-pile Hiding Day-times And Dogs a-Coming By rights I am a Duke! I am the Late Dauphin Tail Piece On the Raft The King as Juliet Courting on the Sly A Pirate for Thirty Years Another little Job Practizing Hamlet s Soliloquy Gimme a Chaw A Little Monthly Drunk The Death of Boggs Sherburn steps out A Dead Head He shed Seventeen Suits Tragedy Their Pockets Bulged Henry the Eighth in Boston Harbor Harmless Adolphus He fairly emptied that Young Fellow Alas, our Poor Brother You Bet it is Leaking Making up the Deffisit Going for him The Doctor The Bag of Money The Cubby Supper with the Hare-Lip Honest Injun The Duke looks under the Bed Huck takes the Money A Crack in the Dining-room Door The Undertaker He had a Rat! Was you in my Room? Jawing In Trouble Indignation How to Find Them He Wrote Hannah with the Mumps The Auction The True Brothers The Doctor leads Huck The Duke Wrote Gentlemen, Gentlemen! Jim Lit Out The King shakes Huck The Duke went for Him Spanish Moss Who Nailed Him? Thinking He gave him Ten Cents Striking for the Back Country Still and Sunday-like She hugged him tight Who do you reckon it is? It was Tom Sawyer Mr. Archibald Nichols, I presume? A pretty long Blessing Traveling By Rail Vittles A Simple Job Witches Getting Wood One of the Best Authorities The Breakfast-Horn Smouching the Knives Going down the Lightning-Rod Stealing spoons Tom advises a Witch Pie The Rubbage-Pile Missus, dey s a Sheet Gone In a Tearing Way One of his Ancestors Jim s Coat of Arms A Tough Job Buttons on their Tails Irrigation Keeping off Dull Times Sawdust Diet Trouble is Brewing Fishing Every one had a Gun Tom caught on a Splinter Jim advises a Doctor The Doctor Uncle Silas in Danger Old Mrs. Hotchkiss Aunt Sally talks to Huck Tom Sawyer wounded The Doctor speaks for Jim Tom rose square up in Bed Hand out them Letters Out of Bondage Tom s Liberality Yours Truly EXPLANATORY IN this book a number of dialects are used, to wit: the Missouri negro dialect  the extremest form of the backwoods Southwestern dialect  the ordinary Pike County dialect  and four modified varieties of this last. The shadings have not been done in a haphazard fashion, or by guesswork  but painstakingly, and with the trustworthy guidance and support of personal familiarity with these several forms of speech. I make this explanation for the reason that without it many readers would suppose that all these characters were trying to talk alike and not succeeding. THE AUTHOR. HUCKLEBERRY FINN Scene: The Mississippi Valley Time: Forty to fifty years ago CHAPTER I. YOU don t know about me without you have read a book by the name of The Adventures of Tom Sawyer  but that ain t no matter. That book was made by Mr. Mark Twain, and he told the truth, mainly. There was things which he stretched, but mainly he told the truth. That is nothing. I never seen anybody but lied one time or another, without it was Aunt Polly, or the widow, or maybe Mary. Aunt Polly--Tom s Aunt Polly, she is--and Mary, and the Widow Douglas is all told about in that book, which is mostly a true book, with some stretchers, as I said before. Now the way that the book winds up is this: Tom and me found the money that the robbers hid in the cave, and it made us rich. We got six thousand dollars apiece--all gold. It was an awful sight of money when it was piled up. Well, Judge Thatcher he took it and put it out at interest, and it fetched us a dollar a day apiece all the year round--more than a body could tell what to do with. The Widow Douglas she took me for her son, and allowed she would sivilize me  but it was rough living in the house all the time, considering how dismal regular and decent the widow was in all her ways  and so when I couldn t stand it no longer I lit out. I got into my old rags and my sugar-hogshead again, and was free and satisfied. But Tom Sawyer he hunted me up and said he was going to start a band of robbers, and I might join if I would go back to the widow and be respectable. So I went back. The widow she cried over me, and called me a poor lost lamb, and she called me a lot of other names, too, but she never meant no harm by it. She put me in them new clothes again, and I couldn t do nothing but sweat and sweat, and feel all cramped up. Well, then, the old thing commenced again. The widow rung a bell for supper, and you had to come to time. When you got to the table you couldn t go right to eating, but you had to wait for the widow to tuck down her head and grumble a little over the victuals, though there warn t really anything the matter with them,--that is, nothing only everything was cooked by itself. In a barrel of odds and ends it is different  things get mixed up, and the juice kind of swaps around, and the things go better. After supper she got out her book and learned me about Moses and the Bulrushers, and I was in a sweat to find out all about him  but by and by she let it out that Moses had been dead a considerable long time  so then I didn t care no more about him, because I don t take no stock in dead people. Pretty soon I wanted to smoke, and asked the widow to let me. But she wouldn t. She said it was a mean practice and wasn t clean, and I must try to not do it any more. That is just the way with some people. They get down on a thing when they don t know nothing about it. Here she was a-bothering about Moses, which was no kin to her, and no use to anybody, being gone, you see, yet finding a power of fault with me for doing a thing that had some good in it. And she took snuff, too  of course that was all right, because she done it herself. Her sister, Miss Watson, a tolerable slim old maid, with goggles on, had just come to live with her, and took a set at me now with a spelling-book. She worked me middling hard for about an hour, and then the widow made her ease up. I couldn t stood it much longer. Then for an hour it was deadly dull, and I was fidgety. Miss Watson would say, Don t put your feet up there, Huckleberry  and Don t scrunch up like that, Huckleberry--set up straight  and pretty soon she would say, Don t gap and stretch like that, Huckleberry--why don t you try to behave? Then she told me all about the bad place, and I said I wished I was there. She got mad then, but I didn t mean no harm. All I wanted was to go somewheres  all I wanted was a change, I warn t particular. She said it was wicked to say what I said  said she wouldn t say it for the whole world  she was going to live so as to go to the good place. Well, I couldn t see no advantage in going where she was going, so I made up my mind I wouldn t try for it. But I never said so, because it would only make trouble, and wouldn t do no good. Now she had got a start, and she went on and told me all about the good place. She said all a body would have to do there was to go around all day long with a harp and sing, forever and ever. So I didn t think much of it. But I never said so. I asked her if she reckoned Tom Sawyer would go there, and she said not by a considerable sight. I was glad about that, because I wanted him and me to be together. Miss Watson she kept pecking at me, and it got tiresome and lonesome. By and by they fetched the niggers in and had prayers, and then everybody was off to bed. I went up to my room with a piece of candle, and put it on the table. Then I set down in a chair by the window and tried to think of something cheerful, but it warn t no use. I felt so lonesome I most wished I was dead. The stars were shining, and the leaves rustled in the woods ever so mournful  and I heard an owl, away off, who-whooing about somebody that was dead, and a whippowill and a dog crying about somebody that was going to die  and the wind was trying to whisper something to me, and I couldn t make out what it was, and so it made the cold shivers run over me. Then away out in the woods I heard that kind of a sound that a ghost makes when it wants to tell about something that s on its mind and can t make itself understood, and so can t rest easy in its grave, and has to go about that way every night grieving. I got so down-hearted and scared I did wish I had some company. Pretty soon a spider went crawling up my shoulder, and I flipped it off and it lit in the candle  and before I could budge it was all shriveled up. I didn t need anybody to tell me that that was an awful bad sign and would fetch me some bad luck, so I was scared and most shook the clothes off of me. I got up and turned around in my tracks three times and crossed my breast every time  and then I tied up a little lock of my hair with a thread to keep witches away. But I hadn t no confidence. You do that when you ve lost a horseshoe that you ve found, instead of nailing it up over the door, but I hadn t ever heard anybody say it was any way to keep off bad luck when you d killed a spider. I set down again, a-shaking all over, and got out my pipe for a smoke  for the house was all as still as death now, and so the widow wouldn t know. Well, after a long time I heard the clock away off in the town go boom--boom--boom--twelve licks  and all still again--stiller than ever. Pretty soon I heard a twig snap down in the dark amongst the trees--something was a stirring. I set still and listened. Directly I could just barely hear a me-yow! me-yow! down there. That was good! Says I, me-yow! me-yow! as soft as I could, and then I put out the light and scrambled out of the window on to the shed. Then I slipped down to the ground and crawled in among the trees, and, sure enough, there was Tom Sawyer waiting for me. CHAPTER II. WE went tiptoeing along a path amongst the trees back towards the end of the widow s garden, stooping down so as the branches wouldn t scrape our heads. When we was passing by the kitchen I fell over a root and made a noise. We scrouched down and laid still. Miss Watson s big nigger, named Jim, was setting in the kitchen door  we could see him pretty clear, because there was a light behind him. He got up and stretched his neck out about a minute, listening. Then he says: Who dah? He listened some more  then he come tiptoeing down and stood right between us  we could a touched him, nearly. Well, likely it was minutes and minutes that there warn t a sound, and we all there so close together. There was a place on my ankle that got to itching, but I dasn t scratch it  and then my ear begun to itch  and next my back, right between my shoulders. Seemed like I d die if I couldn t scratch. Well, I ve noticed that thing plenty times since. If you are with the quality, or at a funeral, or trying to go to sleep when you ain t sleepy--if you are anywheres where it won t do for you to scratch, why you will itch all over in upwards of a thousand places. Pretty soon Jim says: Say, who is you? Whar is you? Dog my cats ef I didn hear sumf n. Well, I know what I s gwyne to do: I s gwyne to set down here and listen tell I hears it agin. So he set down on the ground betwixt me and Tom. He leaned his back up against a tree, and stretched his legs out till one of them most touched one of mine. My nose begun to itch. It itched till the tears come into my eyes. But I dasn t scratch. Then it begun to itch on the inside. Next I got to itching underneath. I didn t know how I was going to set still. This miserableness went on as much as six or seven minutes  but it seemed a sight longer than that. I was itching in eleven different places now. I reckoned I couldn t stand it more n a minute longer, but I set my teeth hard and got ready to try. Just then Jim begun to breathe heavy  next he begun to snore--and then I was pretty soon comfortable again. Tom he made a sign to me--kind of a little noise with his mouth--and we went creeping away on our hands and knees. When we was ten foot off Tom whispered to me, and wanted to tie Jim to the tree for fun. But I said no  he might wake and make a disturbance, and then they d find out I warn t in. Then Tom said he hadn t got candles enough, and he would slip in the kitchen and get some more. I didn t want him to try. I said Jim might wake up and come. But Tom wanted to resk it  so we slid in there and got three candles, and Tom laid five cents on the table for pay. Then we got out, and I was in a sweat to get away  but nothing would do Tom but he must crawl to where Jim was, on his hands and knees, and play something on him. I waited, and it seemed a good while, everything was so still and lonesome. As soon as Tom was back we cut along the path, around the garden fence, and by and by fetched up on the steep top of the hill the other side of the house. Tom said he slipped Jim s hat off of his head and hung it on a limb right over him, and Jim stirred a little, but he didn t wake. Afterwards Jim said the witches be witched him and put him in a trance, and rode him all over the State, and then set him under the trees again, and hung his hat on a limb to show who done it. And next time Jim told it he said they rode him down to New Orleans  and, after that, every time he told it he spread it more and more, till by and by he said they rode him all over the world, and tired him most to death, and his back was all over saddle-boils. Jim was monstrous proud about it, and he got so he wouldn t hardly notice the other niggers. Niggers would come miles to hear Jim tell about it, and he was more looked up to than any nigger in that country. Strange niggers would stand with their mouths open and look him all over, same as if he was a wonder. Niggers is always talking about witches in the dark by the kitchen fire  but whenever one was talking and letting on to know all about such things, Jim would happen in and say, Hm! What you know bout witches? and that nigger was corked up and had to take a back seat. Jim always kept that five-center piece round his neck with a string, and said it was a charm the devil give to him with his own hands, and told him he could cure anybody with it and fetch witches whenever he wanted to just by saying something to it  but he never told what it was he said to it. Niggers would come from all around there and give Jim anything they had, just for a sight of that five-center piece  but they wouldn t touch it, because the devil had had his hands on it. Jim was most ruined for a servant, because he got stuck up on account of having seen the devil and been rode by witches. Well, when Tom and me got to the edge of the hilltop we looked away down into the village and could see three or four lights twinkling, where there was sick folks, maybe  and the stars over us was sparkling ever so fine  and down by the village was the river, a whole mile broad, and awful still and grand. We went down the hill and found Jo Harper and Ben Rogers, and two or three more of the boys, hid in the old tanyard. So we unhitched a skiff and pulled down the river two mile and a half, to the big scar on the hillside, and went ashore. We went to a clump of bushes, and Tom made everybody swear to keep the secret, and then showed them a hole in the hill, right in the thickest part of the bushes. Then we lit the candles, and crawled in on our hands and knees. We went about two hundred yards, and then the cave opened up. Tom poked about amongst the passages, and pretty soon ducked under a wall where you wouldn t a noticed that there was a hole. We went along a narrow place and got into a kind of room, all damp and sweaty and cold, and there we stopped. Tom says: Now, we ll start this band of robbers and call it Tom Sawyer s Gang. Everybody that wants to join has got to take an oath, and write his name in blood. Everybody was willing. So Tom got out a sheet of paper that he had wrote the oath on, and read it. It swore every boy to stick to the band, and never tell any of the secrets  and if anybody done anything to any boy in the band, whichever boy was ordered to kill that person and his family must do it, and he mustn t eat and he mustn t sleep till he had killed them and hacked a cross in their breasts, which was the sign of the band. And nobody that didn t belong to the band could use that mark, and if he did he must be sued  and if he done it again he must be killed. And if anybody that belonged to the band told the secrets, he must have his throat cut, and then have his carcass burnt up and the ashes scattered all around, and his name blotted off of the list with blood and never mentioned again by the gang, but have a curse put on it and be forgot forever. Everybody said it was a real beautiful oath, and asked Tom if he got it out of his own head. He said, some of it, but the rest was out of pirate-books and robber-books, and every gang that was high-toned had it. Some thought it would be good to kill the _families_ of boys that told the secrets. Tom said it was a good idea, so he took a pencil and wrote it in. Then Ben Rogers says: Here s Huck Finn, he hain t got no family  what you going to do bout him? Well, hain t he got a father? says Tom Sawyer. Yes, he s got a father, but you can t never find him these days. He used to lay drunk with the hogs in the tanyard, but he hain t been seen in these parts for a year or more. They talked it over, and they was going to rule me out, because they said every boy must have a family or somebody to kill, or else it wouldn t be fair and square for the others. Well, nobody could think of anything to do--everybody was stumped, and set still. I was most ready to cry  but all at once I thought of a way, and so I offered them Miss Watson--they could kill her. Everybody said: Oh, she ll do. That s all right. Huck can come in. Then they all stuck a pin in their fingers to get blood to sign with, and I made my mark on the paper. Now, says Ben Rogers, what s the line of business of this Gang? Nothing only robbery and murder, Tom said. But who are we going to rob?--houses, or cattle, or-- Stuff! stealing cattle and such things ain t robbery  it s burglary, says Tom Sawyer. We ain t burglars. That ain t no sort of style. We are highwaymen. We stop stages and carriages on the road, with masks on, and kill the people and take their watches and money. Must we always kill the people? Oh, certainly. It s best. Some authorities think different, but mostly it s considered best to kill them--except some that you bring to the cave here, and keep them till they re ransomed. Ransomed? What s that? I don t know. But that s what they do. I ve seen it in books  and so of course that s what we ve got to do. But how can we do it if we don t know what it is? Why, blame it all, we ve _got_ to do it. Don t I tell you it s in the books? Do you want to go to doing different from what s in the books, and get things all muddled up? Oh, that s all very fine to _say_, Tom Sawyer, but how in the nation are these fellows going to be ransomed if we don t know how to do it to them?--that s the thing I want to get at. Now, what do you reckon it is? Well, I don t know. But per aps if we keep them till they re ransomed, it means that we keep them till they re dead. Now, that s something _like_. That ll answer. Why couldn t you said that before? We ll keep them till they re ransomed to death  and a bothersome lot they ll be, too--eating up everything, and always trying to get loose. How you talk, Ben Rogers. How can they get loose when there s a guard over them, ready to shoot them down if they move a peg? A guard! Well, that _is_ good. So somebody s got to set up all night and never get any sleep, just so as to watch them. I think that s foolishness. Why can t a body take a club and ransom them as soon as they get here? Because it ain t in the books so--that s why. Now, Ben Rogers, do you want to do things regular, or don t you?--that s the idea. Don t you reckon that the people that made the books knows what s the correct thing to do? Do you reckon _you_ can learn em anything? Not by a good deal. No, sir, we ll just go on and ransom them in the regular way. All right. I don t mind  but I say it s a fool way, anyhow. Say, do we kill the women, too? Well, Ben Rogers, if I was as ignorant as you I wouldn t let on. Kill the women? No  nobody ever saw anything in the books like that. You fetch them to the cave, and you re always as polite as pie to them  and by and by they fall in love with you, and never want to go home any more. Well, if that s the way I m agreed, but I don t take no stock in it. Mighty soon we ll have the cave so cluttered up with women, and fellows waiting to be ransomed, that there won t be no place for the robbers. But go ahead, I ain t got nothing to say. Little Tommy Barnes was asleep now, and when they waked him up he was scared, and cried, and said he wanted to go home to his ma, and didn t want to be a robber any more. So they all made fun of him, and called him cry-baby, and that made him mad, and he said he would go straight and tell all the secrets. But Tom give him five cents to keep quiet, and said we would all go home and meet next week, and rob somebody and kill some people. Ben Rogers said he couldn t get out much, only Sundays, and so he wanted to begin next Sunday  but all the boys said it would be wicked to do it on Sunday, and that settled the thing. They agreed to get together and fix a day as soon as they could, and then we elected Tom Sawyer first captain and Jo Harper second captain of the Gang, and so started home. I clumb up the shed and crept into my window just before day was breaking. My new clothes was all greased up and clayey, and I was dog-tired. CHAPTER III. WELL, I got a good going-over in the morning from old Miss Watson on account of my clothes  but the widow she didn t scold, but only cleaned off the grease and clay, and looked so sorry that I thought I would behave awhile if I could. Then Miss Watson she took me in the closet and prayed, but nothing come of it. She told me to pray every day, and whatever I asked for I would get it. But it warn t so. I tried it. Once I got a fish-line, but no hooks. It warn t any good to me without hooks. I tried for the hooks three or four times, but somehow I couldn t make it work. By and by, one day, I asked Miss Watson to try for me, but she said I was a fool. She never told me why, and I couldn t make it out no way. I set down one time back in the woods, and had a long think about it. I says to myself, if a body can get anything they pray for, why don t Deacon Winn get back the money he lost on pork? Why can t the widow get back her silver snuffbox that was stole? Why can t Miss Watson fat up? No, says I to my self, there ain t nothing in it. I went and told the widow about it, and she said the thing a body could get by praying for it was spiritual gifts. This was too many for me, but she told me what she meant--I must help other people, and do everything I could for other people, and look out for them all the time, and never think about myself. This was including Miss Watson, as I took it. I went out in the woods and turned it over in my mind a long time, but I couldn t see no advantage about it--except for the other people  so at last I reckoned I wouldn t worry about it any more, but just let it go. Sometimes the widow would take me one side and talk about Providence in a way to make a body s mouth water  but maybe next day Miss Watson would take hold and knock it all down again. I judged I could see that there was two Providences, and a poor chap would stand considerable show with the widow s Providence, but if Miss Watson s got him there warn t no help for him any more. I thought it all out, and reckoned I would belong to the widow s if he wanted me, though I couldn t make out how he was a-going to be any better off then than what he was before, seeing I was so ignorant, and so kind of low-down and ornery. Pap he hadn t been seen for more than a year, and that was comfortable for me  I didn t want to see him no more. He used to always whale me when he was sober and could get his hands on me  though I used to take to the woods most of the time when he was around. Well, about this time he was found in the river drownded, about twelve mile above town, so people said. They judged it was him, anyway  said this drownded man was just his size, and was ragged, and had uncommon long hair, which was all like pap  but they couldn t make nothing out of the face, because it had been in the water so long it warn t much like a face at all. They said he was floating on his back in the water. They took him and buried him on the bank. But I warn t comfortable long, because I happened to think of something. I knowed mighty well that a drownded man don t float on his back, but on his face. So I knowed, then, that this warn t pap, but a woman dressed up in a man s clothes. So I was uncomfortable again. I judged the old man would turn up again by and by, though I wished he wouldn t. We played robber now and then about a month, and then I resigned. All the boys did. We hadn t robbed nobody, hadn t killed any people, but only just pretended. We used to hop out of the woods and go charging down on hog-drivers and women in carts taking garden stuff to market, but we never hived any of them. Tom Sawyer called the hogs ingots, and he called the turnips and stuff julery, and we would go to the cave and powwow over what we had done, and how many people we had killed and marked. But I couldn t see no profit in it. One time Tom sent a boy to run about town with a blazing stick, which he called a slogan (which was the sign for the Gang to get together), and then he said he had got secret news by his spies that next day a whole parcel of Spanish merchants and rich A-rabs was going to camp in Cave Hollow with two hundred elephants, and six hundred camels, and over a thousand sumter mules, all loaded down with di monds, and they didn t have only a guard of four hundred soldiers, and so we would lay in ambuscade, as he called it, and kill the lot and scoop the things. He said we must slick up our swords and guns, and get ready. He never could go after even a turnip-cart but he must have the swords and guns all scoured up for it, though they was only lath and broomsticks, and you might scour at them till you rotted, and then they warn t worth a mouthful of ashes more than what they was before. I didn t believe we could lick such a crowd of Spaniards and A-rabs, but I wanted to see the camels and elephants, so I was on hand next day, Saturday, in the ambuscade  and when we got the word we rushed out of the woods and down the hill. But there warn t no Spaniards and A-rabs, and there warn t no camels nor no elephants. It warn t anything but a Sunday-school picnic, and only a primer-class at that. We busted it up, and chased the children up the hollow  but we never got anything but some doughnuts and jam, though Ben Rogers got a rag doll, and Jo Harper got a hymn-book and a tract  and then the teacher charged in, and made us drop everything and cut. I didn t see no di monds, and I told Tom Sawyer so. He said there was loads of them there, anyway  and he said there was A-rabs there, too, and elephants and things. I said, why couldn t we see them, then? He said if I warn t so ignorant, but had read a book called Don Quixote, I would know without asking. He said it was all done by enchantment. He said there was hundreds of soldiers there, and elephants and treasure, and so on, but we had enemies which he called magicians  and they had turned the whole thing into an infant Sunday-school, just out of spite. I said, all right  then the thing for us to do was to go for the magicians. Tom Sawyer said I was a numskull. Why, said he, a magician could call up a lot of genies, and they would hash you up like nothing before you could say Jack Robinson. They are as tall as a tree and as big around as a church. Well, I says, s pose we got some genies to help _us_--can t we lick the other crowd then? How you going to get them? I don t know. How do _they_ get them? Why, they rub an old tin lamp or an iron ring, and then the genies come tearing in, with the thunder and lightning a-ripping around and the smoke a-rolling, and everything they re told to do they up and do it. They don t think nothing of pulling a shot-tower up by the roots, and belting a Sunday-school superintendent over the head with it--or any other man. Who makes them tear around so? Why, whoever rubs the lamp or the ring. They belong to whoever rubs the lamp or the ring, and they ve got to do whatever he says. If he tells them to build a palace forty miles long out of di monds, and fill it full of chewing-gum, or whatever you want, and fetch an emperor s daughter from China for you to marry, they ve got to do it--and they ve got to do it before sun-up next morning, too. And more: they ve got to waltz that palace around over the country wherever you want it, you understand. Well, says I, I think they are a pack of flat-heads for not keeping the palace themselves stead of fooling them away like that. And what s more--if I was one of them I would see a man in Jericho before I would drop my business and come to him for the rubbing of an old tin lamp. How you talk, Huck Finn. Why, you d _have_ to come when he rubbed it, whether you wanted to or not. What! and I as high as a tree and as big as a church? All right, then  I _would_ come  but I lay I d make that man climb the highest tree there was in the country. Shucks, it ain t no use to talk to you, Huck Finn. You don t seem to know anything, somehow--perfect saphead. I thought all this over for two or three days, and then I reckoned I would see if there was anything in it. I got an old tin lamp and an iron ring, and went out in the woods and rubbed and rubbed till I sweat like an Injun, calculating to build a palace and sell it  but it warn t no use, none of the genies come. So then I judged that all that stuff was only just one of Tom Sawyer s lies. I reckoned he believed in the A-rabs and the elephants, but as for me I think different. It had all the marks of a Sunday-school. CHAPTER IV. WELL, three or four months run along, and it was well into the winter now. I had been to school most all the time and could spell and read and write just a little, and could say the multiplication table up to six times seven is thirty-five, and I don t reckon I could ever get any further than that if I was to live forever. I don t take no stock in mathematics, anyway. At first I hated the school, but by and by I got so I could stand it. Whenever I got uncommon tired I played hookey, and the hiding I got next day done me good and cheered me up. So the longer I went to school the easier it got to be. I was getting sort of used to the widow s ways, too, and they warn t so raspy on me. Living in a house and sleeping in a bed pulled on me pretty tight mostly, but before the cold weather I used to slide out and sleep in the woods sometimes, and so that was a rest to me. I liked the old ways best, but I was getting so I liked the new ones, too, a little bit. The widow said I was coming along slow but sure, and doing very satisfactory. She said she warn t ashamed of me. One morning I happened to turn over the salt-cellar at breakfast. I reached for some of it as quick as I could to throw over my left shoulder and keep off the bad luck, but Miss Watson was in ahead of me, and crossed me off. She says, Take your hands away, Huckleberry  what a mess you are always making! The widow put in a good word for me, but that warn t going to keep off the bad luck, I knowed that well enough. I started out, after breakfast, feeling worried and shaky, and wondering where it was going to fall on me, and what it was going to be. There is ways to keep off some kinds of bad luck, but this wasn t one of them kind  so I never tried to do anything, but just poked along low-spirited and on the watch-out.";
	char string2[] = "The Project Gutenberg EBook of Adventures of Huckleberry Finn, Complete by Mark Twain (Samuel Clemens) This eBook is for the use of anyone anywhere at no cost and with almost no restrictions whatsoever. You may copy it, give it away or re-use it under the terms of the Project Gutenberg License included with this eBook or online at www.gutenberg.net Title: Adventures of Huckleberry Finn, Complete Author: Mark Twain (Samuel Clemens) Release Date: August 20, 2006 [EBook #76] Last Updated: April 18, 2015] Language: English Character set encoding: ISO-8859-1 *** START OF THIS PROJECT GUTENBERG EBOOK HUCKLEBERRY FINN *** Produced by David Widger ADVENTURES OF HUCKLEBERRY FINN (Tom Sawyer s Comrade) By Mark Twain Complete CONTENTS. CHAPTER I. Civilizing Huck.--Miss Watson.--Tom Sawyer Waits. CHAPTER II. The Boys Escape Jim.--Torn Sawyer s Gang.--Deep-laid Plans. CHAPTER III. A Good Going-over.--Grace Triumphant.-- One of Tom Sawyers s Lies . CHAPTER IV. Huck and the Judge.--Superstition. CHAPTER V. Huck s Father.--The Fond Parent.--Reform. CHAPTER VI. He Went for Judge Thatcher.--Huck Decided to Leave.--Political Economy.--Thrashing Around. CHAPTER VII. Laying for Him.--Locked in the Cabin.--Sinking the Body.--Resting. CHAPTER VIII. Sleeping in the Woods.--Raising the Dead.--Exploring the Island.--Finding Jim.--Jim s Escape.--Signs.--Balum. CHAPTER IX. The Cave.--The Floating House. CHAPTER X. The Find.--Old Hank Bunker.--In Disguise. CHAPTER XI. Huck and the Woman.--The Search.--Prevarication.--Going to Goshen. CHAPTER XII. Slow Navigation.--Borrowing Things.--Boarding the Wreck.--The Plotters.--Hunting for the Boat. CHAPTER XIII. Escaping from the Wreck.--The Watchman.--Sinking. CHAPTER XIV. A General Good Time.--The Harem.--French. CHAPTER XV. Huck Loses the Raft.--In the Fog.--Huck Finds the Raft.--Trash. CHAPTER XVI. Expectation.--A White Lie.--Floating Currency.--Running by Cairo.--Swimming Ashore. CHAPTER XVII. An Evening Call.--The Farm in Arkansaw.--Interior Decorations.--Stephen Dowling Bots.--Poetical Effusions. CHAPTER XVIII. Col. Grangerford.--Aristocracy.--Feuds.--The Testament.--Recovering the Raft.--The Wood--pile.--Pork and Cabbage. CHAPTER XIX. Tying Up Day--times.--An Astronomical Theory.--Running a Temperance Revival.--The Duke of Bridgewater.--The Troubles of Royalty. CHAPTER XX. Huck Explains.--Laying Out a Campaign.--Working the Camp--meeting.--A Pirate at the Camp--meeting.--The Duke as a Printer. CHAPTER XXI. Sword Exercise.--Hamlet s Soliloquy.--They Loafed Around Town.--A Lazy Town.--Old Boggs.--Dead. CHAPTER XXII. Sherburn.--Attending the Circus.--Intoxication in the Ring.--The Thrilling Tragedy. CHAPTER XXIII. Sold.--Royal Comparisons.--Jim Gets Home-sick. CHAPTER XXIV. Jim in Royal Robes.--They Take a Passenger.--Getting Information.--Family Grief. CHAPTER XXV. Is It Them?--Singing the Doxologer. --Awful Square--Funeral Orgies.--A Bad Investment . CHAPTER XXVI. A Pious King.--The King s Clergy.--She Asked His Pardon.--Hiding in the Room.--Huck Takes the Money. CHAPTER XXVII. The Funeral.--Satisfying Curiosity.--Suspicious of Huck,--Quick Sales and Small. CHAPTER XXVIII. The Trip to England.-- The Brute! --Mary Jane Decides to Leave.--Huck Parting with Mary Jane.--Mumps.--The Opposition Line. CHAPTER XXIX. Contested Relationship.--The King Explains the Loss.--A Question of Handwriting.--Digging up the Corpse.--Huck Escapes. CHAPTER XXX. The King Went for Him.--A Royal Row.--Powerful Mellow. CHAPTER XXXI. Ominous Plans.--News from Jim.--Old Recollections.--A Sheep Story.--Valuable Information. CHAPTER XXXII. Still and Sunday--like.--Mistaken Identity.--Up a Stump.--In a Dilemma. CHAPTER XXXIII. A Nigger Stealer.--Southern Hospitality.--A Pretty Long Blessing.--Tar and Feathers. CHAPTER XXXIV. The Hut by the Ash Hopper.--Outrageous.--Climbing the Lightning Rod.--Troubled with Witches. CHAPTER XXXV. Escaping Properly.--Dark Schemes.--Discrimination in Stealing.--A Deep Hole. CHAPTER XXXVI. The Lightning Rod.--His Level Best.--A Bequest to Posterity.--A High Figure. CHAPTER XXXVII. The Last Shirt.--Mooning Around.--Sailing Orders.--The Witch Pie. CHAPTER XXXVIII. The Coat of Arms.--A Skilled Superintendent.--Unpleasant Glory.--A Tearful Subject. CHAPTER XXXIX. Rats.--Lively Bed--fellows.--The Straw Dummy. CHAPTER XL. Fishing.--The Vigilance Committee.--A Lively Run.--Jim Advises a Doctor. CHAPTER XLI. The Doctor.--Uncle Silas.--Sister Hotchkiss.--Aunt Sally in Trouble. CHAPTER XLII. Tom Sawyer Wounded.--The Doctor s Story.--Tom Confesses.--Aunt Polly Arrives.--Hand Out Them Letters . CHAPTER THE LAST. Out of Bondage.--Paying the Captive.--Yours Truly, Huck Finn. ILLUSTRATIONS. The Widows Moses and the Bulrushers Miss Watson Huck Stealing Away They Tip-toed Along Jim Tom Sawyer s Band of Robbers Huck Creeps into his Window Miss Watson s Lecture The Robbers Dispersed Rubbing the Lamp ! ! ! ! Judge Thatcher surprised Jim Listening Pap Huck and his Father Reforming the Drunkard Falling from Grace The Widows Moses and the Bulrushers Miss Watson Huck Stealing Away They Tip-toed Along Jim Tom Sawyer s Band of Robbers Huck Creeps into his Window Miss Watson s Lecture The Robbers Dispersed Rubbing the Lamp ! ! ! ! Judge Thatcher surprised Jim Listening Pap Huck and his Father Reforming the Drunkard Falling from Grace Getting out of the Way Solid Comfort Thinking it Over Raising a Howl Git Up The Shanty Shooting the Pig Taking a Rest In the Woods Watching the Boat Discovering the Camp Fire Jim and the Ghost Misto Bradish s Nigger Exploring the Cave In the Cave Jim sees a Dead Man They Found Eight Dollars Jim and the Snake Old Hank Bunker A Fair Fit Come In Him and another Man She puts up a Snack Hump Yourself On the Raft He sometimes Lifted a Chicken Please don t, Bill It ain t Good Morals Oh! Lordy, Lordy! In a Fix Hello, What s Up? The Wreck We turned in and Slept Turning over the Truck Solomon and his Million Wives The story of Sollermun We Would Sell the Raft Among the Snags Asleep on the Raft Something being Raftsman Boy, that s a Lie Here I is, Huck Climbing up the Bank Who s There? Buck It made Her look Spidery They got him out and emptied Him The House Col. Grangerford Young Harney Shepherdson Miss Charlotte And asked me if I Liked Her Behind the Wood-pile Hiding Day-times And Dogs a-Coming By rights I am a Duke! I am the Late Dauphin Tail Piece On the Raft The King";
	char string3[] = "Hello world! I'm testing this test!";

	int fildes[5];
	int length_long;

	// Test code
	printf("--- Mount file system, get list of files - should be empty\n");
	if(mount_fs("Test") == -1)
		return -1;
	char **files;
	if(fs_listfiles(&files) == -1)
		return -1;
	if(files[0] != NULL)
		return -1;

	printf("--- Create a file that is 2 blocks long, 10 blocks long, and another that fills up the system\n");
	if(fs_create(name1) == -1)
		return -1;
	if(fs_create(name2) == -1)
		return -1;
	if(fs_create(name3) == -1)
		return -1;
	fildes[0] = fs_open(name1);
	if(fildes[0] == -1)
		return -1;
	fildes[1] = fs_open(name2);
	if(fildes[1] == -1)
		return -1;
	fildes[2] = fs_open(name3);
	if(fildes[2] == -1)
		return -1;
	if(fs_write(fildes[0], string2, strlen(string2)) == -1)
		return -1;
	if(fs_get_filesize(fildes[0]) != strlen(string2))
		return -1;
	if(fs_write(fildes[1], string1, strlen(string1)) == -1)
		return -1;
	if(fs_get_filesize(fildes[1]) != strlen(string1))
		return -1;
	int ii;
	for(ii = 0; ii<450; ii++)
	{
		if(fs_write(fildes[2], string1, strlen(string1)) == -1)
			return -1;
	}
	length_long = fs_get_filesize(fildes[2]);
	if(length_long == -1)
		return -1;

	printf("--- Try creating another file\n");
	if(fs_create(name4) == -1)
		return -1;
	fildes[3] = fs_open(name4);
	if(fildes[3] == -1)
		return -1;
	if(fs_write(fildes[3], string3, strlen(string3)) != 0)
		return -1;

	printf("--- Unmount and Remount then try to create a file and check file size\n");
	if(umount_fs("Test") == -1)
		return -1;
	if(mount_fs("Test_fdkjalg;ksfdja") != -1)
		return -1;
	if(mount_fs("Test") == -1)
		return -1;
	if(mount_fs("Test") != -1)
		return -1;

	if(fs_get_filesize(fildes[0]) != -1)
		return -1;
	if(fs_get_filesize(fildes[1]) != -1)
		return -1;
	if(fs_get_filesize(fildes[2]) != -1)
		return -1;
	if(fs_get_filesize(fildes[3]) != -1)
		return -1;

	fildes[0] = fs_open(name1);
	if(fildes[0] == -1)
		return -1;
	fildes[1] = fs_open(name2);
	if(fildes[1] == -1)
		return -1;
	fildes[2] = fs_open(name3);
	if(fildes[2] == -1)
		return -1;
	fildes[3] = fs_open(name4);
	if(fildes[3] == -1)
		return -1;

	if(fs_get_filesize(fildes[0]) != strlen(string2))
		return -1;
	if(fs_get_filesize(fildes[1]) != strlen(string1))
		return -1;
	if(fs_get_filesize(fildes[2]) != length_long)
		return -1;

	printf("--- Try reading the file after the remount\n");
	char temp[100000];
	if(fs_read(fildes[0], temp, strlen(string2) + 10) != strlen(string2))
		return -1;
	if(memcmp(temp, string2, strlen(string2)) != 0)
		return -1;
	if(fs_read(fildes[1], temp, strlen(string1) + 10) != strlen(string1))
		return -1;
	if(memcmp(temp, string1, strlen(string1)) != 0)
		return -1;
	if(fs_read(fildes[2], temp, 100000) != 100000)
		return -1;
	if(memcmp(temp, string1, 35000) != 0)
		return -1;

	printf("--- Truncate the long file and write a new one to fill up the filesystem\n");
	if(fs_truncate(fildes[2], length_long-15000) == -1)
			return -1;
	if(fs_lseek(fildes[3], 0) == -1)
		return -1;
	if(fs_write(fildes[3], string1, length_long) == -1)
		return -1;
	if(fs_lseek(fildes[3], 3200) == -1)
		return -1;
	memset(temp, 0, 100000);
	if(fs_read(fildes[3], temp, 6000) != 6000)
		return -1;
	if(memcmp(temp, string1+3200, 6000) != 0)
		return -1;

	printf("--- Try writing to another file\n");
	if(fs_create(name5) == -1)
		return -1;
	fildes[4] = fs_open(name5);
	if(fildes[4] == -1)
		return -1;
	if(fs_write(fildes[4], string3, 10) != 0)
		return -1;

	printf("--- Read the newest file again\n");
	if(fs_lseek(fildes[3], 3200) == -1)
		return -1;
	memset(temp, 0, 100000);
	if(fs_read(fildes[3], temp, 6000) != 6000)
		return -1;
	if(memcmp(temp, string1+3200, 6000) != 0)
		return -1;


	printf("--- Truncate the long file again and try reading from untruncated to truncated\n");
	int length_2 = fs_get_filesize(fildes[2]);
	if(length_2 == -1)
		return -1;
	if(fs_truncate(fildes[2], length_2 - 15000) == -1)
		return -1;
	if(fs_lseek(fildes[2], length_long-35000) == -1)
		return -1;
	memset(temp, 0, 100000);
	if(fs_read(fildes[2], temp, 10000) != 5000)
		return -1;
	if(fs_read(fildes[2], temp, 10000) != 0)
		return -1;

	printf("--- Unmount and remount the filesystem again\n");
	if(umount_fs("Test") == -1)
		return -1;
	if(umount_fs("Test") != -1)
		return -1;
	if(mount_fs("Test") == -1)
		return -1;
	if(mount_fs("Test") != -1)
		return -1;

	printf("--- Read all files and verify length\n");
	
	fildes[0] = fs_open(name1);
	if(fildes[0] == -1)
		return -1;
	fildes[1] = fs_open(name2);
	if(fildes[1] == -1)
		return -1;
	fildes[2] = fs_open(name3);
	if(fildes[2] == -1)
		return -1;
	fildes[3] = fs_open(name4);
	if(fildes[3] == -1)
		return -1;
	fildes[4] = fs_open(name5);
	if(fildes[4] == -1)
		return -1;

	if(fs_get_filesize(fildes[0]) != strlen(string2))
		return -1;
	if(fs_get_filesize(fildes[1]) != strlen(string1))
		return -1;
	if(fs_get_filesize(fildes[2]) != length_long - 30000)
		return -1;

	if(fs_read(fildes[0], temp, strlen(string2) + 10) != strlen(string2))
		return -1;
	if(memcmp(temp, string2, strlen(string2)) != 0)
		return -1;
	if(fs_read(fildes[1], temp, strlen(string1) + 10) != strlen(string1))
		return -1;
	if(memcmp(temp, string1, strlen(string1)) != 0)
		return -1;
	if(fs_read(fildes[2], temp, 100000) != 100000)
		return -1;
	if(memcmp(temp, string1, 35000) != 0)
		return -1;
	if(fs_lseek(fildes[3], 3200) == -1)
		return -1;
	memset(temp, 0, 100000);
	if(fs_read(fildes[3], temp, 6000) != 6000)
		return -1;
	if(memcmp(temp, string1+3200, 6000) != 0)
		return -1;

	printf("--- List all files\n");
	char **files2;
	if(fs_listfiles(&files2) == -1)
		return -1;
	int num = 0;
	while(files2[num++] != NULL);
	num--;
	char *array[5];
	array[0] = name1;
	array[1] = name2;
	array[2] = name3;
	array[3] = name4;
	array[4] = name5;
	for(ii = 0; ii<num; ii++)
	{
		if(strcmp(array[ii], files2[ii]) != 0)
			return -1;
		free(files2[ii]);
	}
	free(files2);

	printf("--- Delete all 5 files\n");
	if(fs_close(fildes[0]) == -1)
		return -1;
	if(fs_close(fildes[1]) == -1)
		return -1;
	if(fs_close(fildes[2]) == -1)
		return -1;
	if(fs_close(fildes[3]) == -1)
		return -1;
	if(fs_close(fildes[4]) == -1)
		return -1;

	if(fs_delete(name1) == -1)
		return -1;
	if(fs_delete(name2) == -1)
		return -1;
	if(fs_delete(name3) == -1)
		return -1;
	if(fs_delete(name4) == -1)
		return -1;
	if(fs_delete(name5) == -1)
		return -1;


	printf("--- Unmount and remount again\n");
	if(umount_fs("Test") == -1)
		return -1;
	if(umount_fs("Test") != -1)
		return -1;
	if(mount_fs("Test") == -1)
		return -1;
	if(mount_fs("Test") != -1)
		return -1;

	printf("--- Try to open and read all files\n");
	if(fs_open(name1) != -1)
		return -1;
	if(fs_open(name2) != -1)
		return -1;
	if(fs_open(name3) != -1)
		return -1;
	if(fs_open(name4) != -1)
		return -1;
	if(fs_open(name5) != -1)
		return -1;

	if(fs_read(fildes[0], temp, 2) != -1)
		return -1;
	if(fs_read(fildes[1], temp, 2) != -1)
		return -1;
	if(fs_read(fildes[2], temp, 2) != -1)
		return -1;
	if(fs_read(fildes[3], temp, 2) != -1)
		return -1;
	if(fs_read(fildes[4], temp, 2) != -1)
		return -1;

	printf("--- Create one big file to fill up filesystem\n");
	if(fs_create(name1) == -1)
		return -1;
	fildes[0] = fs_open(name1);
	if(fildes[0] == -1)
		return -1;
	for(ii = 0; ii<500; ii++)
	{
		if(fs_write(fildes[0], string1, strlen(string1)) == -1)
			return -1;
	}

	printf("--- Unmount and remount and open the file\n");
	if(umount_fs("Test") == -1)
		return -1;
	if(umount_fs("Test") != -1)
		return -1;
	if(mount_fs("Test") == -1)
		return -1;
	if(mount_fs("Test") != -1)
		return -1;
	fildes[0] = fs_open(name1);
	if(fildes[0] == -1)
		return -1;

	printf("--- Get file size and truncate\n");
	int size = fs_get_filesize(fildes[0]);
	if(size == -1)
		return -1;
	size = size/2;
	if(fs_truncate(fildes[0], size) == -1)
		return -1;

	printf("--- Write to fill up filesystem again\n");
	if(fs_create(name2) == -1)
		return -1;
	fildes[1] = fs_open(name2);
	if(fildes[1] == -1)
		return -1;
	for(ii = 0; ii<500; ii++)
	{
		if(fs_write(fildes[0], string1, strlen(string1)) == -1)
			return -1;
	}
	if(fs_create(name3) == -1)
		return -1;
	fildes[2] = fs_open(name3);
	if(fildes[2] == -1)
		return -1;
	if(fs_write(fildes[2], string1, strlen(string1)) != 0)
			return -1;

	printf("--- List all files\n");
	if(fs_listfiles(&files2) == -1)
		return -1;
	num = 0;
	while(files2[num++] != NULL);
	num--;
	array[0] = name1;
	array[1] = name2;
	array[2] = name3;

	for(ii = 0; ii<num; ii++)
	{
		if(strcmp(array[ii], files2[ii]) != 0)
			return -1;
		free(files2[ii]);
	}
	free(files2);

	printf("--- Delete file and create a new one, write something to it and verify\n");
	
	if(fs_close(fildes[0]) == -1)
		return -1;
	if(fs_delete(name1) == -1)
		return -1;
	if(fs_create(name4) == -1)
		return -1;
	fildes[3] = fs_open(name4);
	if(fildes[3] == -1)
		return -1;
	if(fs_write(fildes[3], string1, strlen(string1)) != strlen(string1))
		return -1;
	if(fs_lseek(fildes[3], 0) == -1)
		return -1;
	if(fs_read(fildes[3], temp, strlen(string1)) != strlen(string1))
		return -1;
	if(memcmp(temp, string1, strlen(string1)) != 0)
		return -1;

	printf("--- Finishing test and cleaning up\n");
	if(fs_close(fildes[1]) == -1)
		return -1;
	if(fs_close(fildes[2]) == -1)
		return -1;
	if(fs_close(fildes[3]) == -1)
		return -1;
	if(fs_delete(name1) != -1)
		return -1;
	if(fs_delete(name2) == -1)
		return -1;
	if(fs_delete(name3) == -1)
		return -1;
	if(fs_delete(name4) == -1)
		return -1;
	if(fs_listfiles(&files2) == -1)
		return -1;
	if(files2[0] != NULL)
		return -1;
	if(umount_fs("Test") == -1)
		return -1;

	return 0;
} 