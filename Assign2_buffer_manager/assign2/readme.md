
Team
----------------------------
1)Abhinav Pimpalgaonkar(A0387324)
2)Raj Sekhar Dutta(A20380630)
3)Saloni Chaudhary(A20377221)
4)Shubham Bhardwaj(A20379553)


Implementation
----------------

This code implements a buffer manager.

Reading and writing files from the file consumes lot of time, buffer manager helps us to manage this. It keeps the buffer in memory for fast access. With the help of buffer manager program makes changes inside the memory and writes the file once the task is completed.

In this code we implement buffer manager, where a buffer pool manages several buffer pages and each pool keeps a list of page handles. Buffer page contains the priority as value, when the program pins a page and buffer pool is full, it will find a page with low priority value to replace it. With the help of priority we implement LRU and FIFO and it helps us implementing our code.

We have used two strucutures to manage the buffer page. One stores the total number of read and write I/O and priority count and the other one store the statistics about the buffer page for example read, write count for itself, if the page is dirty, the priority etc.


In order to execute the buffer manager code we run our makefile, which is responsible for creating the test and binary file linking.

Procedure to Execute the buffer manager:-
-----------------------------------------

1)Copy all the files to one folder
2)In Unix Terminal navigate to folder where files are stored.
3)Execute "make" command
4)Execute "./buffer_mgr" command


Here the short description of the structure and organization of the code:
__________________________________________________________________________


buffer_mgr.h --> It defines the various structures and functions to be used by buffer_mgr.c. 

buffer_mgr.c --> This file contains the implementaion of the buffer manager function, and it initializes the program. It is used to initialize buffer as per the number of PageFrames and is used to copy file in pages from disk to PageFrame. It then checks for fixedcount of Pages and Dirty bits to write the edited page back to disk.

buffer_mgr_stat.c --> This contains the statistics functions for the bufferpool.

buffer_mgr_stat.h --> It contains the BufferPool statistic functions definitions.

dberror.c --> It contains the error message codes and error message functions.

dberror.h --> It is the header file for dberror.h and it defines page_size constant as 4096 and definition for RC(return codes) for various File and Block functions in storage_mgr.
 
dt.h --> This file contains constant variables for True and False. 
 
storage_mgr.h--> This file is the header file for storage manager and it is responsible for containing files and block functions which are called from test file and it contains the read and write functions from file on disk. It contains all the file handling functions and it implements the dberror and test helper files.

test_assign2_1.c --> It contains the test cases which checks each and every function within buffer_mgr.c

test_helper.h -> It contains the validations which is called by test file. It helps printing the correct error code and message based on the function's execution.

Functions implemented in Buffermanager file:

1. initBufferPool: 
This function initializes the buffer buffer pool manager and allocates memory to variables pointed by bufferpool. It also creates head node and subsequent frames in Bufferpool.

2. *getFixCounts:
It returns the page frame fixed count for each frame in the buffer pool.
 
3. getNumReadIO:
It returns the number of pages read from disk after buffer pool being initialized.
 
4. getNumWriteIO:
It returns the number of pages that has been written to the disk after buffer pool being initialized.
 
5. getDirtyFlags:
This function checks for the frame which is dirty in the buffer pool.
 
6. markDirty:
Checks all the page frames in the pool and checks if the page in the frame is same as page to be modified, if so then it marks the page as dirty.

7. unpinPage:
It checks all the pages in the pool if it finds the page to be unpinned and reduces the fixedcount by 1.

8. pinPage:
It pins the page with pageNum in page fram in the pool. It also leds the page from the disk to the page frame. Its the function which calls the replacement algorithm on request if there are no empty page frames.

9. shutdownBufferPool:
It first calls the forceFlushPool function, then it to shuts down a BufferPool while iterating through all the PageFrames in the Pool to check if a page is marked dirty, if so then they have to be written back to file on disk. It also frees up all the memory allocated to PageFrames.

10. forceFlushPool:
It iterates through all the page  frames in the pool and checks for the dirty page, then it checks the fixed count if the fixedcount for the page is 0 then it writes the page content to the file on disk.

11. forcePage:
It writes the content of current page having dirtybit, to file on disk.

12. *getFrameContents:
This function returns the pagenumber of page stored in page frame.

13. locateNode:
It locates the page requested in the page frame in pool as per the pageNum value passed in the function and returns the page pointer if found or else it returns NULL.

14. locateNodeinMemory:
It locates the requested page frame in the pool, if the page is found then fixedcount of the page is incremented and page value gets copied to the page in the page frame. If the page is not found it returns NULL.

15. updatePageHead:
This function iterates through the page frame in the pool which needs to be replaced and updates the pointers within the page frames accordingly.

16. updatePage:
It calls the various storage manager functions implemented in storage_mgr.c file to handle the files and page frames.

17. fifo:
It implements the FIFO algorithm by calling the locateNodeinMemory function to find if the page frame has already been requested by the client. If it is not requested then it checks for an empty page frame and loads the page into empty page frame.

18. LRU:
It implements the LRU algorithm by calling the locateNodeinMemory function to find if the page frame has already been requested by the client. If it is not requested then it checks for an empty page frame and loads the page into empty page frame.
