CS 525 Assignment 3

Team
----------------------------

1)Abhinav Pimpalgaonkar(A0387324)

2)Raj Sekhar Dutta(A20380630)

3)Saloni Chaudhary(A20377221)

4)Shubham Bhardwaj(A20379553)




Implementation
----------------


This code implements a record manager to store and scan the records.


Steps to execute the code:
---------------------------


1) make --> Compile the source code to execute files.

2) ./recordmgr --> Unit test 

3) ./test_expr --> Unit test

4) make clean --> remove all the output files from the directory

Here the short description of the structure and organization of the code:
__________________________________________________________________________


record_mgr.h --> It defines the various structures and functions to be used by record_mgr.c. 

record_mgr.c --> This file contains basic methods for record to create, get, insert, update, delete the records.

initRecordManager --> This function initializes the record manager, set its status to open.

shutdownRecordManager --> This function shuts down the record manager and sets its status to close.

createTable --> This function creates a page file for a table.

openTable --> It opens a page file and initializes its instance.

closeTable --> This function closes the table and destroys the handle.

deleteTable --> This function deletes the table file.

getNumTuples --> It returns the number of tuples of the table.

insertRecord --> It inserts a new record into the table file. When there is no space for new record, then creates a new page, and insert record into the first available slot.

deleteRecord --> This function deletes a record from the table file and sets its data to NULL.

updateRecord --> It updates the data of the record based on RID.

getRecord --> This function gets the data of the record based on RID.

startScan --> It initializes the RM_scanHandle, such as start position and scans the condition.

next -->  This function gets the next record that satisfy scan condition. Traverses all the pages, if the record matches condition, then output, until there are no more tuples.

closeScan --> This function closes the RM_scanHandle.

getRecordSize --> It returns the size of the one record or the schema.

createSchema --> It creates a schema and set its all attributes.

freeSchema --> This function releases the schema data.

createRecord --> This function initializes the record such as malloc space for pointers.

freeRecord --> This function frees a record data.

getAttr --> It gets the value of i-th attribute.

setAttr --> It sets the value of i-th attribute.
