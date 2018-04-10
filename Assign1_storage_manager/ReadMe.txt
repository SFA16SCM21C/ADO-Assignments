			              #Storage Manager

Structure of the project
------------------------------------------------------
dberror.c		storage_mgr.c		test_assign1_2.c
dberror.h		storage_mgr.h		test_helper.h
makefile		test_assign1_1.c

Team
----------------------------
1)Abhinav Pimpalgaonkar(A0387324)
2)Raj Sekhar Dutta(A20380630)
3)Saloni Chaudhary(A20377221)-Team Leader
4)Shubham Bhardwaj(A20379553)


Goal:
------------------------------------------------------------------------------------------------------------------------------------------------
We have created a storage manager in C programming language, that can read files from a file stored on disk into memory and can also write the blocks from memory to the file which is on the disk. It deals with the blocks of fixed size(PAGE_SIZE = 4096). The storage manager has functions to create, open and close files and read,
write,append pages in the file.

Instructions to run the code
----------------------------------------------------------------------------------------------------------------
 - Go to the path where souce files are present

 - Run the below commands in the locaton:
1. make -> This command will execute the makefile function and create all the object files
2. make run -> This command runs the test scenarios in the test_assign1_1.c
3. make clean -> To remove the object files run this command
4. ./test1 -> will run the test scenarios in the test_assign1_1.c
  
Functions:
------------
Here are the functions that are used to create the storage manager interface

***********************************
Manipulating page files:
***********************************

initStorageManager()
------------------------------
Initializes the program.

createPageFile()
--------------------------
Checks if the file already exists in the location, if the file does not exist it creates a new file in the location.


openPageFile()
--------------------
This function opens the file in read mode and if the file doesn't exists it returns the error code 'RC_FILE_NOT_FOUND'. It also initializes the filehandler variables.

closePageFile()
---------------
Closes the file and frees up the space in the memory. the the file doesn't exist it returns the error code RC_FILE_NOT_FOUND.


destroyPageFile()
-------------------
It removes the existing files, if the file is not present then returns RC_FILE_NOT_FOUND.

***********************************
Reading blocks from disc:
***********************************

readBlock()
----------------
Moves the pointer to the begining of the file and reads the data of 1 PAGE_SIZE(4096 bytes) and loads it into memory. Finds out the current page position using ftell and returns the error codes accordingly.

getBlockPos()
--------------------
Gets the position of the current blockm returns the error code if the file handler isn't initialized.

readFirstBlock ()
----------------------
Moves the pointer to the begining of the file and reads the first page. Returns the error code as defined in the funciton accordingly.

readPreviousBlock ()
------------------------
Moves the pointer to the previous position with respoec to the current position and reads the content of the previous page.  Returns the error code as defined in the funciton accordingly.

readCurrentBlock ()
----------------------
Sets the descriptor in the file handler to the current page and reads the page content. Returns the error code as defined in the funciton accordingly.

readNextBlock ()
--------------------
Sets the position of the pointer to the next page position and reads the page content. Returns the error code as defined in the funciton accordingly.

readLastBlock ()
-------------------
Sets the pointer to last page  and reads the page content to buffer specified. Returns the error code as defined in the funciton accordingly.

***********************************
Writing blocks to a page file:
***********************************

writeBlock()
-------------
Moves the pointer to the page from the begining of the file if the files is present, updated the file handler variables.  Returns the error code as defined in the funciton accordingly.

writeCurrentBlock()
------------------------
Checks for the file if the file is present moves the file pointer to the current page from the beging of the page and writes one page to it. Returns the error code as defined in the funciton accordingly.

appendEmptyBlock()
----------------------
Checks for the file in the location and if can be opened in the read mode. If these conditions are satisfied file pointer is set to the end of the file and one page is added with character '\0' in newly added empty block. File handle variable are updated. Returns the error code as defined in the funciton accordingly.

ensureCapacity()
--------------------
This function ensures that if file has capacity to to accomodate numberofpages. If the file does not sufficient pages, using 'appendEmptyBlock' function, add more pages. Returns the error code as defined in the funciton accordingly.