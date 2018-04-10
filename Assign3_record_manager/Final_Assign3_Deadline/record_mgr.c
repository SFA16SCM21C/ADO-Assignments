      #include <stdio.h>
      #include <stdlib.h>
      #include <string.h>
      #include "record_mgr.h"
      #include "buffer_mgr.h"
      #include "storage_mgr.h"
      
      typedef struct RecordManager
      {
      	// pagehandle for buffer manager
      	BM_PageHandle pageHandle;  //buffer manager handler	
      	BM_BufferPool bufferPool;  //buffer pool for files
      	RID recordID;			//to catch hold of recordid used
      	Expr *condition;			//condition to be tested
      	int tuplesCount;			//hold for tuple count
      	int freePage;			//if the page would be free/not
      	int scanCount;			//count for scans done
      } RecordManager;
      	
      const int MAX_NUMBER_OF_PAGES = 100, ATTRIBUTE_SIZE = 15;
      RecordManager *recordManager;
      
      int findFreeSlot(char *data, int recordSize)
      {
      	int i, totalSlots = PAGE_SIZE / recordSize; 
      	//to find if there is any free slot
      	for (i = 0; i < totalSlots; i++)
      		if (data[i * recordSize] != '+')
      			return i;
      	return -1;
      }
      
      extern RC initRecordManager (void *mgmtData)
      {
      	initStorageManager();	//initiate record manager
      	return RC_OK;
      }
      
      extern RC shutdownRecordManager ()
      {
      	recordManager = NULL;
      	//to shut down the record manager
      	free(recordManager);
      	return RC_OK;
      }
      
      extern RC createTable (char *name, Schema *schema)
      {
      	recordManager = (RecordManager*) malloc(sizeof(RecordManager));	//allocating space for record manager
      
      	initBufferPool(&recordManager->bufferPool, name, MAX_NUMBER_OF_PAGES, RS_LRU, NULL);	//initiate bufer manager
      
      	char data[PAGE_SIZE];
      	char *pageHandle = data;
      	int result, k;
      	*(int*)pageHandle = 0; 
      
      	pageHandle = pageHandle + sizeof(int);
      
      	*(int*)pageHandle = 1;
      
      	pageHandle = pageHandle + sizeof(int);
      
      	*(int*)pageHandle = schema->numAttr;	//set attributes
      
      	pageHandle = pageHandle + sizeof(int); 
      
      	*(int*)pageHandle = schema->keySize;	//set size of attributes
      
      	pageHandle = pageHandle + sizeof(int);
      	
      	for(k = 0; k < schema->numAttr; k++)
          	{
             		strncpy(pageHandle, schema->attrNames[k], ATTRIBUTE_SIZE);
      	       	pageHandle = pageHandle + ATTRIBUTE_SIZE;
      	       	*(int*)pageHandle = (int)schema->dataTypes[k];
      	       	pageHandle = pageHandle + sizeof(int);
      	       	*(int*)pageHandle = (int) schema->typeLength[k];
      	       	pageHandle = pageHandle + sizeof(int);
          	}
      
      	SM_FileHandle fileHandle;
      	if((result = createPageFile(name)) != RC_OK)
      		return result;
      	if((result = openPageFile(name, &fileHandle)) != RC_OK)
      		return result;
      	if((result = writeBlock(0, &fileHandle, data)) != RC_OK)
      		return result;
      	if((result = closePageFile(&fileHandle)) != RC_OK)
      		return result;
      	return RC_OK;
      }
      
      extern RC openTable (RM_TableData *rel, char *name)
      {
      	SM_PageHandle pageHandle;    
      	int k;
      	int attributeCount;
      	rel->mgmtData = recordManager;
      	rel->name = name;
      
      	pinPage(&recordManager->bufferPool, &recordManager->pageHandle, 0);
      
      	pageHandle = (char*) recordManager->pageHandle.data;
      	recordManager->tuplesCount= *(int*)pageHandle;
      	pageHandle = pageHandle + sizeof(int);
      	recordManager->freePage= *(int*) pageHandle;
          	pageHandle = pageHandle + sizeof(int);
          	attributeCount = *(int*)pageHandle;
      	pageHandle = pageHandle + sizeof(int);
       	
      	Schema *schema;	//allocate space based on teh schema
      	schema = (Schema*) malloc(sizeof(Schema));
      	schema->numAttr = attributeCount;
      	schema->attrNames = (char**) malloc(sizeof(char*) *attributeCount);
      	schema->dataTypes = (DataType*) malloc(sizeof(DataType) *attributeCount);
      	schema->typeLength = (int*) malloc(sizeof(int) *attributeCount);
      
      	for(k = 0; k < attributeCount; k++)
      		schema->attrNames[k]= (char*) malloc(ATTRIBUTE_SIZE); 
      	for(k = 0; k < schema->numAttr; k++)
          	{
      		strncpy(schema->attrNames[k], pageHandle, ATTRIBUTE_SIZE);		//set attribute name
      		pageHandle = pageHandle + ATTRIBUTE_SIZE;
      		schema->dataTypes[k]= *(int*) pageHandle;
      		pageHandle = pageHandle + sizeof(int);
      		schema->typeLength[k]= *(int*)pageHandle;
      		pageHandle = pageHandle + sizeof(int);
      	}
      	rel->schema = schema;	
      	unpinPage(&recordManager->bufferPool, &recordManager->pageHandle);
      	//write page back to disk from buffer pool
   
      	forcePage(&recordManager->bufferPool, &recordManager->pageHandle);
      	return RC_OK;
      }   
        
      extern RC closeTable (RM_TableData *rel)
      {
      	RecordManager *recordManager = rel->mgmtData;	
      	shutdownBufferPool(&recordManager->bufferPool);
      	//rel->mgmtData = NULL;
      	//helps closing the table being used
      	return RC_OK;
      }
      
      extern RC deleteTable (char *name)
      {
      	destroyPageFile(name);	//remove page file using storage manager from memory
      	return RC_OK;
      }
      
      extern int getNumTuples (RM_TableData *rel)
      {
      	RecordManager *recordManager = rel->mgmtData;

      	//helps etting the tuple count
      	return recordManager->tuplesCount;
      }
      
      extern RC insertRecord (RM_TableData *rel, Record *record)
      {
      	RecordManager *recordManager = rel->mgmtData;	
      	//etrieve meta data of the table
      	// Setting the Record ID for this record
      	RID *recordID = &record->id;
      	//change done here 
      	char *data;
      	char *slotPointer;
      	int recordSize = getRecordSize(rel->schema);
      	recordID->page = recordManager->freePage;
      
      	pinPage(&recordManager->bufferPool, &recordManager->pageHandle, recordID->page);	
      	//free page being used by the record manaager to dump the data
      	data = recordManager->pageHandle.data;
      	//use it to store data of table into the page used
      	// Getting a free slot using our custom function
      	recordID->slot = findFreeSlot(data, recordSize);
      
      	while(recordID->slot == -1)
      	{
      		// If the pinned page doesn't have a free slot then 			//unpin that page and incrment the page
      		unpinPage(&recordManager->bufferPool, &recordManager->pageHandle);	
      		recordID->page++;
      		
      		// Bring the new page into the BUffer Pool using 			//Buffer Manager untill free page found and set the 
      		//data to sarting positon of the record data
      		pinPage(&recordManager->bufferPool, &recordManager->pageHandle, recordID->page);	
      		data = recordManager->pageHandle.data;
      		recordID->slot = findFreeSlot(data, recordSize);
      	}
      	
      	slotPointer = data;
      	markDirty(&recordManager->bufferPool, &recordManager->pageHandle);	//mark it dirty if modified
      	slotPointer = slotPointer + (recordID->slot * recordSize);
      	*slotPointer = '+';
      	memcpy(++slotPointer, record->data + 1, recordSize - 1);
      	unpinPage(&recordManager->bufferPool, &recordManager->pageHandle);
      	recordManager->tuplesCount++;
      	pinPage(&recordManager->bufferPool, &recordManager->pageHandle, 0);
      	return RC_OK;
      }
      
      extern RC deleteRecord (RM_TableData *rel, RID id)
      {
      	RecordManager *recordManager = rel->mgmtData;	//retrieve meta data from table
      	pinPage(&recordManager->bufferPool, &recordManager->pageHandle, id.page); 
      	recordManager->freePage = id.page;
      	//update the page as free after the page has been pinned
      	char *data = recordManager->pageHandle.data;
      
      	// Getting the size of the record
      	int recordSize = getRecordSize(rel->schema);
      	data = data + (id.slot * recordSize);
      	//get the size of the record and increment the pinter to 
      	//next position
      	// - is used for Tombstone mechanism denoting the record is deleted
      	*data = '-';
      	markDirty(&recordManager->bufferPool, &recordManager->pageHandle);
      	//mark page dirty since modified, nd unpin page as no 	//longer it is needed in memory
      	unpinPage(&recordManager->bufferPool, &recordManager->pageHandle);
      	return RC_OK;
      }
      
      extern RC updateRecord (RM_TableData *rel, Record *record)
      {	
      	RecordManager *recordManager = rel->mgmtData;
      	pinPage(&recordManager->bufferPool, &recordManager->pageHandle, record->id.page);
      	char *data;
      	int recordSize = getRecordSize(rel->schema);
      	RID id = record->id;
      	data = recordManager->pageHandle.data;
      	data = data + (id.slot * recordSize);
      	
      	// + is used for Tombstone mechanism denoting the record is not empty
      	*data = '+';
      	memcpy(++data, record->data + 1, recordSize - 1 );
      	// copy new recrd in existing records and mark the page 	//dirty because it has been modified
      	markDirty(&recordManager->bufferPool, &recordManager->pageHandle);
      	unpinPage(&recordManager->bufferPool, &recordManager->pageHandle);	
      	return RC_OK;	
      }
      
      extern RC getRecord (RM_TableData *rel, RID id, Record *record)
      {
      	RecordManager *recordManager = rel->mgmtData;
      	pinPage(&recordManager->bufferPool, &recordManager->pageHandle, id.page);
      	int recordSize = getRecordSize(rel->schema);
      	char *dataPointer = recordManager->pageHandle.data;
      	dataPointer = dataPointer + (id.slot * recordSize);
      	
      	if(*dataPointer != '+')
      	{
      		return RC_RM_NO_TUPLE_WITH_GIVEN_RID;
      	}
      	else
      	{
      		record->id = id;
      		char *data = record->data;
      		memcpy(++data, dataPointer + 1, recordSize - 1);
      	}
      	unpinPage(&recordManager->bufferPool, &recordManager->pageHandle);
      	return RC_OK;
      }
      
      extern RC startScan (RM_TableData *rel, RM_ScanHandle *scan, Expr *cond)
      {
      	if (cond == NULL)
      	{
      		return RC_SCAN_CONDITION_NOT_FOUND;
      	}
      	openTable(rel, "ScanTable");
          	RecordManager *scanManager;
      	RecordManager *tableManager;
          	scanManager = (RecordManager*) malloc(sizeof(RecordManager));
          	scan->mgmtData = scanManager;
          	scanManager->recordID.page = 1;	
      	scanManager->recordID.slot = 0;    	
      	scanManager->scanCount = 0;
          	scanManager->condition = cond;
          	tableManager = rel->mgmtData;
          	tableManager->tuplesCount = ATTRIBUTE_SIZE;
          	scan->rel= rel;
      	return RC_OK;
      }
      
      extern RC next (RM_ScanHandle *scan, Record *record)
      {
      	RecordManager *scanManager = scan->mgmtData;
      	RecordManager *tableManager = scan->rel->mgmtData;
          	Schema *schema = scan->rel->schema;
      	if (scanManager->condition == NULL)
      	{
      		return RC_SCAN_CONDITION_NOT_FOUND;
      	}
      
      	Value *result = (Value *) malloc(sizeof(Value));
      	char *data;
      	int recordSize = getRecordSize(schema);
      	int totalSlots = PAGE_SIZE / recordSize;
      	int scanCount = scanManager->scanCount;
      	int tuplesCount = tableManager->tuplesCount;
      	if (tuplesCount == 0)
      		return RC_RM_NO_MORE_TUPLES;
      	while(scanCount <= tuplesCount)
      	{  
      				if (scanCount <= 0)
      		{
      			scanManager->recordID.page = 1;
      			scanManager->recordID.slot = 0;
      		}
      		else
      		{
      			scanManager->recordID.slot++;
      			if(scanManager->recordID.slot >= totalSlots)
      			{
      				scanManager->recordID.slot = 0;
   
      				scanManager->recordID.page++;
      			}
      		}
      		pinPage(&tableManager->bufferPool, &scanManager->pageHandle, scanManager->recordID.page);			
      		data = scanManager->pageHandle.data;
      		data = data + (scanManager->recordID.slot * recordSize);
      		record->id.page = scanManager->recordID.page;
      		record->id.slot = scanManager->recordID.slot;
      		char *dataPointer = record->data;
      		*dataPointer = '-';
      		
      		memcpy(++dataPointer, data + 1, recordSize - 1);
      		scanManager->scanCount++;
      		scanCount++;
      		evalExpr(record, schema, scanManager->condition, &result); 
      
      		if(result->v.boolV == TRUE)
      		{
      			unpinPage(&tableManager->bufferPool, &scanManager->pageHandle);			
      			return RC_OK;
      		}
      	}
      	unpinPage(&tableManager->bufferPool, &scanManager->pageHandle);
      
      	scanManager->recordID.page = 1;
      	scanManager->recordID.slot = 0;
      	scanManager->scanCount = 0;
      	return RC_RM_NO_MORE_TUPLES;
      }
      
      extern RC closeScan (RM_ScanHandle *scan)
      {
      	RecordManager *scanManager = scan->mgmtData;
      	RecordManager *recordManager = scan->rel->mgmtData;
      	if(scanManager->scanCount > 0)
      	{
      		unpinPage(&recordManager->bufferPool, &scanManager->pageHandle);
      		scanManager->scanCount = 0;
      		scanManager->recordID.page = 1;
      		scanManager->recordID.slot = 0;
      	}
          	scan->mgmtData = NULL;
          	free(scan->mgmtData);  
      	return RC_OK;
      }
      
      extern int getRecordSize (Schema *schema)
      {
      	int size = 0; 
      	int i=0; // offset set to zero
      	//change made here
      	// iterating through all attributes in schema
      	for( i= 0; i < schema->numAttr; i++)
      	{
      		switch(schema->dataTypes[i])
      		{
      			case DT_STRING:
      				size = size + schema->typeLength[i];
      				break;
      			case DT_INT:
      				size = size + sizeof(int);
      				break;
      			case DT_FLOAT:
      				size = size + sizeof(float);
      				break;
      			case DT_BOOL:
      				size = size + sizeof(bool);
      				break;
      		}
      	}
      	return ++size;
      }
      
      extern Schema *createSchema (int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys)
      {
      	Schema *schema = (Schema *) malloc(sizeof(Schema));
      	schema->numAttr = numAttr;
      	schema->attrNames = attrNames;
      	schema->dataTypes = dataTypes;
      	schema->typeLength = typeLength;
      	schema->keySize = keySize;
      	schema->keyAttrs = keys;
      	return schema; 
      }
      
      extern RC freeSchema (Schema *schema)
   
      {
      	free(schema);
      	return RC_OK;
      }
      
      extern RC createRecord (Record **record, Schema *schema)
      {
      	Record *newRecord = (Record*) malloc(sizeof(Record));
      	int recordSize = getRecordSize(schema); 
      	newRecord->data= (char*) malloc(recordSize);
      	newRecord->id.page = newRecord->id.slot = -1;
      	char *dataPointer = newRecord->data;
      	*dataPointer = '-';
      	*(++dataPointer) = '\0';
      	*record = newRecord;
      	return RC_OK;
      }
      
      RC attrOffset (Schema *schema, int attrNum, int *result)
      {
      	int i;
      	*result = 1;
   
      
      	for(i = 0; i < attrNum; i++)
      	{
      		switch (schema->dataTypes[i])
      		{
      			case DT_STRING:
      				*result = *result + schema->typeLength[i];
      				break;
      			case DT_INT:
      				*result = *result + sizeof(int);
      				break;
      			case DT_FLOAT:
      				*result = *result + sizeof(float);
      				break;
      			case DT_BOOL:
      				*result = *result + sizeof(bool);
      				break;
      		}
      	}
      	return RC_OK;
      }
      
      extern RC freeRecord (Record *record)
      {
      	free(record);
      	return RC_OK;
      }
      
      extern RC getAttr (Record *record, Schema *schema, int attrNum, Value **value)
      {
      	int offset = 0;
      	attrOffset(schema, attrNum, &offset);
      	Value *attribute = (Value*) malloc(sizeof(Value));
      	char *dataPointer = record->data;
      	dataPointer = dataPointer + offset;
      	schema->dataTypes[attrNum] = (attrNum == 1) ? 1 : schema->dataTypes[attrNum];
      
      	switch(schema->dataTypes[attrNum])
      	{
      		case DT_STRING:
      		{
      			int length = schema->typeLength[attrNum];
      			attribute->v.stringV = (char *) malloc(length + 1);
      
      			strncpy(attribute->v.stringV, dataPointer, length);
      			attribute->v.stringV[length] = '\0';
      			attribute->dt = DT_STRING;
            			break;
      		}
      
      		case DT_INT:
      		{
      			int value = 0;
      			memcpy(&value, dataPointer, sizeof(int));
      			attribute->v.intV = value;
      			attribute->dt = DT_INT;
            			break;
      		}
          
      		case DT_FLOAT:
      		{
      	  		float value;
      	  		memcpy(&value, dataPointer, sizeof(float));
      	  		attribute->v.floatV = value;
      			attribute->dt = DT_FLOAT;
      			break;
      		}
      
      		case DT_BOOL:
      		{
      			bool value;
      			memcpy(&value,dataPointer, sizeof(bool));
      			attribute->v.boolV = value;
      			attribute->dt = DT_BOOL;
            			break;
      		}
      
      		default:
      			printf("Serializer is not defined for the above given datatype. \n");
      			break;
      	}
      	*value = attribute;
      	return RC_OK;
      }
      
      extern RC setAttr (Record *record, Schema *schema, int attrNum, Value *value)
      {
      	int offset = 0;
      	attrOffset(schema, attrNum, &offset);
      	char *dataPointer = record->data;
      	dataPointer = dataPointer + offset;
      		
      	switch(schema->dataTypes[attrNum])
      	{
      		case DT_STRING:
      		{
      			int length = schema->typeLength[attrNum];
      			strncpy(dataPointer, value->v.stringV, length);
      			dataPointer = dataPointer + schema->typeLength[attrNum];
      		  	break;
      		}
      
      		case DT_INT:
      		{
      			*(int *) dataPointer = value->v.intV;	  
      			dataPointer = dataPointer + sizeof(int);
      		  	break;
      		}
      		
      		case DT_FLOAT:
      		{
      			*(float *) dataPointer = value->v.floatV;
      			dataPointer = dataPointer + sizeof(float);
      			break;
      		}
      		
      		case DT_BOOL:
      		{
      			*(bool *) dataPointer = value->v.boolV;
      			dataPointer = dataPointer + sizeof(bool);
      			break;
      		}
      
      		default:
      			printf("Serializer is not defined for the above given datatype. \n");
      			break;
      	}			
      	return RC_OK;
      }
