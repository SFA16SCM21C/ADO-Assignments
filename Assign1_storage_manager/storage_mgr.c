#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<math.h>

#include "storage_mgr.h"

FILE *pageFile;

extern void initStorageManager (void) {
	// Initiating storage manager along with file pointer.
	pageFile = NULL;
}

extern RC createPageFile (char *fileName) {
	// Open file in read and write mode.
	pageFile = fopen(fileName, "w+");

	// Check file if succesfully opened.
	if(pageFile != NULL) {

		// allocate memory space equivalent to one page and write it into the file.
		SM_PageHandle emptyPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
	        fwrite(emptyPage, sizeof(char), PAGE_SIZE, pageFile);
		
		//de-allocate the memory for the emptypage for proper memory management and release space.
		free(emptyPage);	
		
		// Closing the open file after writing into it. 
		fclose(pageFile);

		return RC_OK;
	} else {
		 return RC_FILE_NOT_FOUND;			
	}
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
	// Openfile in read only mode with the name specified in parameters.
	pageFile = fopen(fileName, "r");

	// Check if file is successfully opened.
	if(pageFile != NULL) {
		// Update file handle's to filename and set current page position to the start of the page.
		// Calculating the total number of pages using file size.
		// Total number of pages in the filename specified.
		struct stat fileSize;
		if(fstat(fileno(pageFile), &fileSize) < 0)    
			return RC_ERROR;
		fHandle->fileName = fileName;
		fHandle->curPagePos = 0;
		fHandle->totalNumPages = fileSize.st_size/ PAGE_SIZE;
		//fHandle->mgmtInfo = pageFile;

		// Close the file to erase the storage in buffer. 
		fclose(pageFile);
		return RC_OK;
	} else { 
		return RC_FILE_NOT_FOUND;
	}
}

extern RC closePageFile (SM_FileHandle *fHandle) {
	// Check for the file if intialised. If it is initialised, then close the file.
	if(pageFile != NULL)
		pageFile = NULL;	
	return RC_OK; 
}


extern RC destroyPageFile (char *fileName) {
	// Open file in reading mode.	
	pageFile = fopen(fileName, "r");
	
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;
	
	// Delete the filename given.	
	remove(fileName);
	return RC_OK;
}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Check if the pageNumber is less than Total number of pages in the file or less than 0, then error out.
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
        	return RC_READ_NON_EXISTING_PAGE;

	// Open the file after the above check regarding the page in the file.	
	pageFile = fopen(fHandle->fileName, "r");

	// Check if file is successfully opened.
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;
	
	// set the position to read the data as per the block the data is in.
	int seekSuccessfull = fseek(pageFile, (pageNum * PAGE_SIZE), SEEK_SET);

	if(seekSuccessfull == 0) {
		fread(memPage, sizeof(char), PAGE_SIZE, pageFile);
	} else {
		return RC_READ_NON_EXISTING_PAGE; 
	}
    	
	// Set the current page position to the cursor position mentioned in the file structure information.
	fHandle->curPagePos = ftell(pageFile); 
	
	// Close the file and clear the buffer space.     	
	fclose(pageFile);
	
    	return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle) {
	// Return the current positon of the block from the file handle pointer.	
	return fHandle->curPagePos;
}

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Point the cursor using readblock to direct to the first page in the file.	
	return readBlock(0, fHandle, memPage);
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Calculate the current page number of the file.	
	int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;

	// Point the cursor using readblock to direct to the previous page from the current page number.
	return readBlock(currentPageNumber - 1, fHandle, memPage);
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Calculate the current page number of the file.	
	int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
	
	// Point the cursor using readblock to direct to the current page from the current page number.
	return readBlock(currentPageNumber, fHandle, memPage);
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	// Calculate the current page number of the file.	
	int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
	
	// Point the cursor using readblock to direct to the next page from the current page number.
	return readBlock(currentPageNumber + 1, fHandle, memPage);
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	// POint the cursor using readblock to point to the last page in the file using totalNumPages.
	return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Check if the pageNumber is less than Total number of pages in the file or less than 0, then error out.
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
        	return RC_WRITE_FAILED;
	
	// Opening the file in read and write mode.	
	pageFile = fopen(fHandle->fileName, "r+");
	
	// Check if file is successfully opened.
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;

	// set the position to read the data as per the block the data is in.
	int seekSuccess = fseek(pageFile, (pageNum * PAGE_SIZE), SEEK_SET);
	if(seekSuccess == 0) {
		
		// Write content from memPage to pageFile stream.
		fwrite(memPage, sizeof(char), strlen(memPage), pageFile);

		// Set the current page position to the cursor position of the file stream.
		fHandle->curPagePos = ftell(pageFile);
		
		// Close the file and clear the buffer.
		fclose(pageFile);
	} else {
		return RC_WRITE_FAILED;
	}	
	
	return RC_OK;
}

extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Calculate the current page number.	
	int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
	
	// Increment the total number of pages as we will add the content to a new page in the file. Use writeblock method to write into the 		file.
	fHandle->totalNumPages++;
	return writeBlock(currentPageNumber, fHandle, memPage);
}


extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
	// Generate an empty page/block.
	SM_PageHandle emptyBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
	
	// Move the cursor position to the starting position of the file.
	int seekSuccess = fseek(pageFile, 0, SEEK_END);
	
	if( seekSuccess == 0 ) {
		// Write the empty page/bloack created to the file.
		fwrite(emptyBlock, sizeof(char), PAGE_SIZE, pageFile);
	} else {
		free(emptyBlock);
		return RC_WRITE_FAILED;
	}
	
	// Empty the space occupied by the emptyPage.
	free(emptyBlock);
	
	// Increment the total number of pages in the file since another page is added here.
	fHandle->totalNumPages++;
	return RC_OK;
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
	// Open the file in append(addition) mode.
	pageFile = fopen(fHandle->fileName, "a");
	
	if(pageFile == NULL)
		return RC_FILE_NOT_FOUND;
	
	// append pages to the file if numberofpags is greater than the total number of pages present already in the file.
	while(numberOfPages > fHandle->totalNumPages)
		appendEmptyBlock(fHandle);
	
	// Close the file and clear the buffer storage. 
	fclose(pageFile);
	return RC_OK;
}

