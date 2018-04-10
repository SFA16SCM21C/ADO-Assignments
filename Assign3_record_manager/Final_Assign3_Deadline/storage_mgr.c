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
	pageFile = NULL;		//initiate storage manager
}

extern RC createPageFile (char *fileName) {
	pageFile = fopen(fileName, "w+");
	//open file in read & wrtie mode if not found crate a new 	//file
	// Check if file is successfully opened
	if(pageFile == NULL) {
		return RC_FILE_NOT_FOUND;
	} else 
	{
		SM_PageHandle emptyPage = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
		
		// creating space in meory for creating new file 			//writing empty page to file.
		if(fwrite(emptyPage, sizeof(char), PAGE_SIZE,pageFile) < PAGE_SIZE)
			{
			printf("write failed \n");
			}
		else
			{
			printf("write succeeded \n");
			}		
		fclose(pageFile);
		free(emptyPage);
		return RC_OK;
		//return success
	}
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
	pageFile = fopen(fileName, "r");
	//create an empty file for read only
	// Checking if file was successfully opened.
	if(pageFile == NULL) 
		{
			return RC_FILE_NOT_FOUND;
		} 
	else 
		{ 
		fHandle->fileName = fileName;
		fHandle->curPagePos = 0;
		//calculate no of pages using methods and file size
		struct stat fileInfo;
		if(fstat(fileno(pageFile), &fileInfo) < 0)    
			return RC_ERROR;
		fHandle->totalNumPages = fileInfo.st_size/ PAGE_SIZE;
		fclose(pageFile);
		return RC_OK;
		//return success
	}
}

extern RC closePageFile (SM_FileHandle *fHandle) {
	if(pageFile != NULL)
		{
			pageFile = NULL;		//to check if page is 				//closed
		}
	return RC_OK; 
	//return success
}


extern RC destroyPageFile (char *fileName) {	
	pageFile = fopen(fileName, "r");
	if(pageFile == NULL)
		{
			return RC_FILE_NOT_FOUND; 
		}		
	remove(fileName);
	return RC_OK;
	//return success
}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
	{
        	return RC_READ_NON_EXISTING_PAGE;
	}
	//check done to see if this function works
	//pageFile = fopen(fHandle->fileName, "w");
	pageFile = fopen(fHandle->fileName, "r");
	//open the file in read mode to read only
	// Checking if file was successfully opened.
	if(pageFile == NULL)
	{
		return RC_FILE_NOT_FOUND;
	}
	int isSeekSuccess = fseek(pageFile, (pageNum * PAGE_SIZE), SEEK_SET);
	if(isSeekSuccess == 0) 
	{
		if(fread(memPage, sizeof(char), PAGE_SIZE, pageFile) < PAGE_SIZE)	
		{
			return RC_ERROR;
		}
	} 
	else 
	{
		return RC_READ_NON_EXISTING_PAGE; 
	}
	fHandle->curPagePos = ftell(pageFile);     	
	fclose(pageFile);
    	return RC_OK;
	//return success
}

extern int getBlockPos (SM_FileHandle *fHandle) {	
	return fHandle->curPagePos;
	//to get the current position of the block being used in 	//file handler
}

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {	
	pageFile = fopen(fHandle->fileName, "r");
	//open the file in read mode to read only
	// Checking if file was successfully opened.
	if(pageFile == NULL)
	{
		return RC_FILE_NOT_FOUND;
	}
	int i = 0;
	for(i = 0; i < PAGE_SIZE; i++) {
		char c = fgetc(pageFile);
		if(feof(pageFile))
		{
			break;
		}
		else
		{
			memPage[i] = c;
		}
	}
	fHandle->curPagePos = ftell(pageFile); 
	fclose(pageFile);
	return RC_OK;
	//return success
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	//check to put a printf statement to see which page 		//position it was
	//printf("CURRENT PAGE POSITION = %d \n", fHandle->curPagePos);
	//check to put a printf statement to see the number of 	//total pages from the file handler
	//printf("TOTAL PAGES = %d \n", fHandle->totalNumPages)
	if(fHandle->curPagePos <= PAGE_SIZE) 
	{
		printf("\n First block: Previous block not present.");
		return RC_READ_NON_EXISTING_PAGE;	
	} 
	else 
	{	
		int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
		int startPosition = (PAGE_SIZE * (currentPageNumber - 2));
		pageFile = fopen(fHandle->fileName, "r");
		if(pageFile == NULL)
		{
			return RC_FILE_NOT_FOUND;
		}
		fseek(pageFile, startPosition, SEEK_SET);
		int i = 0;
		for(i = 0; i < PAGE_SIZE; i++) 
		{
			memPage[i] = fgetc(pageFile);
		}

		// Setting the current page position to the cursor(pointer) position of the file stream
		fHandle->curPagePos = ftell(pageFile); 
		//set page position to cursr position of file 
		// Closing file stream so that all the buffers are flushed.
		fclose(pageFile);
		return RC_OK;
		//return success
	}
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {	
	int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
	int startPosition = (PAGE_SIZE * (currentPageNumber - 2));
	//calculate current page and read its content
	//open the file in read only mode to read the block 	//caluclate above	
	pageFile = fopen(fHandle->fileName, "r");
	if(pageFile == NULL)
	{
		return RC_FILE_NOT_FOUND;
	}
	fseek(pageFile, startPosition, SEEK_SET);
	int i = 0;
	for(i = 0; i < PAGE_SIZE; i++) {
		char c = fgetc(pageFile);		
		if(feof(pageFile))
			break;
		memPage[i] = c;
	}
	fHandle->curPagePos = ftell(pageFile); 
	//set file handler to currnt page position
	// Closing file stream so that all the buffers are 	//flushed
	fclose(pageFile);
	return RC_OK;
	//return success		
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	if(fHandle->curPagePos == PAGE_SIZE) 
	{
		//check if file is at last page
		printf("\n This is Last block. Next block not present.");
		return RC_READ_NON_EXISTING_PAGE;	
	} 
	else 
	{	
		int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
		int startPosition = (PAGE_SIZE * (currentPageNumber - 2));

		//calcualte current page number and starting position			//and open the file in read only mode to read file
		pageFile = fopen(fHandle->fileName, "r");
		if(pageFile == NULL)
		{
			return RC_FILE_NOT_FOUND;
		}
		fseek(pageFile, startPosition, SEEK_SET);
		int i = 0;
		for(i = 0; i < PAGE_SIZE; i++) {
			char c = fgetc(pageFile);		
			if(feof(pageFile))
				break;
			memPage[i] = c;
		}
		fHandle->curPagePos = ftell(pageFile); 
		//set current pge position to cursor position of file
		//close file to close all buffers opened
		fclose(pageFile);
		return RC_OK;
		//return success
	}
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){	
	pageFile = fopen(fHandle->fileName, "r");
	//open the file in read only mode to read contents
	//check if file is opened succesfully
	if(pageFile == NULL)
	{
		return RC_FILE_NOT_FOUND;
	}
	//find the start position of the file
	int startPosition = (fHandle->totalNumPages - 1) * PAGE_SIZE;
	// Initializing file pointer position.
	fseek(pageFile, startPosition, SEEK_SET);
	int i = 0;
	for(i = 0; i < PAGE_SIZE; i++) {
		char c = fgetc(pageFile);		
		if(feof(pageFile))
			break;
		memPage[i] = c;
	}
	fHandle->curPagePos = ftell(pageFile); 
	//set the current page position that to cursor position
	//close file to eliminate the buffers holding data
	fclose(pageFile);
	return RC_OK;	
	//return success
}

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
	{
        	return RC_WRITE_FAILED;
	}	
	pageFile = fopen(fHandle->fileName, "r+");
	if(pageFile == NULL)
	{
		return RC_FILE_NOT_FOUND;
	}
	int startPosition = pageNum * PAGE_SIZE;
	if(pageNum == 0) 
	{ 
		fseek(pageFile, startPosition, SEEK_SET);
		int i = 0;	
		for(i = 0; i < PAGE_SIZE; i++) 
		{
			if(feof(pageFile)) // check file is ending in between writing
				 appendEmptyBlock(fHandle);
			fputc(memPage[i], pageFile);
		}
		fHandle->curPagePos = ftell(pageFile); 
		fclose(pageFile);	
	} else 
	{	
		// Writing data to the first page.
		fHandle->curPagePos = startPosition;
		fclose(pageFile);
		writeCurrentBlock(fHandle, memPage);
	}
	return RC_OK;
	//return success
}

extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {	
	pageFile = fopen(fHandle->fileName, "r+");
	//open the file in red and write mode
	//check if file is opened successfully
	if(pageFile == NULL)
	{
		return RC_FILE_NOT_FOUND;
	}
	appendEmptyBlock(fHandle);
	//appnd the empty block to make space for new content
	// initialize file pointer
	//write mempage contents to file
	fseek(pageFile, fHandle->curPagePos, SEEK_SET);
	fwrite(memPage, sizeof(char), strlen(memPage), pageFile);
	fHandle->curPagePos = ftell(pageFile);    	
	fclose(pageFile);
	return RC_OK;
	//return success
}


extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
	SM_PageHandle emptyBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
	int isSeekSuccess = fseek(pageFile, 0, SEEK_END);
	if( isSeekSuccess == 0 ) 
	{
		fwrite(emptyBlock, sizeof(char), PAGE_SIZE, pageFile);
	} 
	else 
	{
		free(emptyBlock);
		return RC_WRITE_FAILED;
	}
	free(emptyBlock);
	fHandle->totalNumPages++;
	return RC_OK;
	//return success
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
	pageFile = fopen(fHandle->fileName, "a");
	if(pageFile == NULL)
	{
		return RC_FILE_NOT_FOUND;
	}
	while(numberOfPages > fHandle->totalNumPages)
		appendEmptyBlock(fHandle); 
	fclose(pageFile);
	return RC_OK;
	//return success
}