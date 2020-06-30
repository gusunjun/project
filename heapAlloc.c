///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2019-2020 Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
//
// Main File:        heapAlloc.c
// This File:        heapAlloc.c
// Other Files:      None
// Semester:         CS 354 Spring 2020
//
// Author:           Sunjun Gu
// Email:            sgu59@wisc.edu
// CS Login:         sunjun
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of 
//                   of any information you find.
////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "heapAlloc.h"

/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 */
typedef struct blockHeader {
	int size_status;
	/*
	* Size of the block is always a multiple of 8.
	* Size is stored in all block headers and free block footers.
	*
	* Status is stored only in headers using the two least significant bits.
	*   Bit0 => least significant bit, last bit
	*   Bit0 == 0 => free block
	*   Bit0 == 1 => allocated block
	*
	*   Bit1 => second last bit
	*   Bit1 == 0 => previous block is free
	*   Bit1 == 1 => previous block is allocated
	*
	* End Mark:
	*  The end of the available memory is indicated using a size_status of 1.
	*
	* Examples:
	*
	* 1. Allocated block of size 24 bytes:
	*    Header:
	*      If the previous block is allocated, size_status should be 27
	*      If the previous block is free, size_status should be 25
	*
	* 2. Free block of size 24 bytes:
	*    Header:
	*      If the previous block is allocated, size_status should be 26
	*      If the previous block is free, size_status should be 24
	*    Footer:
	*      size_status should be 24
	*/
} blockHeader;

/* Global variable - DO NOT CHANGE. It should always point to the first block,
 * i.e., the block at the lowest address.
 */
blockHeader *heapStart = NULL;

/* Size of heap allocation padded to round to nearest page size.
 */
int allocsize;

/*
 * Additional global variables may be added as needed below
 */
blockHeader *cur_blk = NULL;//record the current block, find the right free block


 /*
  * Function for allocating 'size' bytes of heap memory.
  * Argument size: requested size for the payload
  * Returns address of allocated block on success.
  * Returns NULL on failure.
  * This function should:
  * - Check size - Return NULL if not positive or if larger than heap space.
  * - Determine block size rounding up to a multiple of 8 and possibly adding padding as a result.
  * - Use NEXT-FIT PLACEMENT POLICY to chose a free block
  * - Use SPLITTING to divide the chosen free block into two if it is too large.
  * - Update header(s) and footer as needed.
  * Tips: Be careful with pointer arithmetic and scale factors.
  */
void* allocHeap(int size) {
	//TODO: Your code goes in here.
	//Check size - Return NULL if not positive or if larger than heap space.
	if (size <= 0 || size > allocsize)
		return NULL;

	//Determine block size rounding up to a multiple of 8 and possibly adding padding as a result.
	int blockSize = 0; // the block size multiple of 8

	blockSize = size + sizeof(blockHeader); // add header size
	// add padding size
	if (blockSize % 8 != 0) {
		blockSize += (8 - blockSize % 8);
	}

	if (cur_blk == NULL) {
		cur_blk = heapStart;// first initial the cur_blk
	}
	//Use NEXT-FIT PLACEMENT POLICY to chose a free block
	int LSB; //least significant bit, last bit; currStatus
	int SLB; //second last bit; prevStatus
	int currBlockSize;// the most recent block size
	blockHeader *startBlock = cur_blk;// the starting block

	LSB = cur_blk->size_status & 1; //least significant bit, last bit
	SLB = cur_blk->size_status & 2; //second last bit
	currBlockSize = cur_blk->size_status - LSB - SLB; // remove the last three bits

	// when initheap, cur_blk must within the heap, only when allocate, 
	//cur_blk may out of bound then use while-loop to avoid this happen
	while (LSB || (blockSize > currBlockSize)) {
		//if current block is allocated or too small to allocate, skip to the next one 
		cur_blk = (blockHeader*)((char*)cur_blk + currBlockSize);
		LSB = cur_blk->size_status & 1; //least significant bit, last bit
		SLB = cur_blk->size_status & 2; //second last bit
		currBlockSize = cur_blk->size_status - LSB - SLB; // remove the last three bits

		// if until the end could not find righ free block then from the beginning to the starting block 
		if (cur_blk->size_status == 1) {
			cur_blk = heapStart;
			LSB = cur_blk->size_status & 1; //least significant bit, last bit
			SLB = cur_blk->size_status & 2; //second last bit
			currBlockSize = cur_blk->size_status - LSB - SLB; // remove the last three bits
		}
		else if (cur_blk == startBlock)// fail whrn return to the starting block
			return NULL;
	}


	cur_blk->size_status = (cur_blk->size_status & 2) + blockSize + 1; // update header both size and used status
	blockHeader *returnBlock = cur_blk + 1; // Returns address of allocated block on success.

	//Use SPLITTING to divide the chosen free block into two if it is too large.

	if (currBlockSize - blockSize >= 8) {
		blockHeader *splitBlock = (blockHeader*)((char*)cur_blk + blockSize); // the new splited block
		splitBlock->size_status = currBlockSize - blockSize + 2; // size and status of the new blcok
		blockHeader *splitBlockfooter = (blockHeader*)((char*)splitBlock + currBlockSize - blockSize - sizeof(blockHeader));
		// the new free block foooter
		splitBlockfooter->size_status = currBlockSize - blockSize; // footer contain the free block size
		cur_blk = splitBlock;// next from the split block
	}
	else {
		cur_blk = (blockHeader*)((char*)cur_blk + blockSize); // move the current block to the next one
		cur_blk->size_status += 2; // the next block previous block is allocated
	}
	// * -Update header(s) and footer as needed.

	return returnBlock; // Returns address of allocated block on success.
}

/*
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.                    //////!!!!!!ptr -4??
 * - Return -1 if ptr is not a multiple of 8.          ////// address 8?
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - USE IMMEDIATE COALESCING if one or both of the adjacent neighbors are free.
 * - Update header(s) and footer as needed.
 */
int freeHeap(void *ptr) {
	//TODO: Your code goes in here.
	if (ptr == NULL)
		return -1; // ptr is NULL
	blockHeader *freeBlock = (blockHeader*)((char*)ptr - sizeof(blockHeader)); // the header of the free block

	if ((unsigned int)ptr % 8 != 0)
		return -1; // ptr is not a multiple of 8
	if (ptr >= (void*)((char*)heapStart + allocsize) || ptr < (void*)heapStart)
		return -1;// ptr is outside of the heap space.
	if (!(freeBlock->size_status & 1))
		return -1; // ptr block is already freed.

	freeBlock->size_status -= 1;//the current block is free
	// USE IMMEDIATE COALESCING if one or both of the adjacent neighbors are free.
	// Update header(s) and footer as needed.
	int freeLSB = freeBlock->size_status & 1; //curr status
	int freeSLB = freeBlock->size_status & 2; //prev status
	int freeSize = freeBlock->size_status - freeLSB - freeSLB; // block's size to be freed

	blockHeader *footer;	// the footer of block
	blockHeader *nextHeader = (blockHeader*)((void*)freeBlock + freeSize);// next block's header
	blockHeader *preHeader; // free block's previous block's header
	// first check the next block
	if ((nextHeader->size_status & 1) == 0) { // next block is free
		int nextLSB = nextHeader->size_status & 1; //curr status a-bit
		int nextSLB = nextHeader->size_status & 2; //prev status p-bit
		int nextSize = nextHeader->size_status - nextLSB - nextSLB; // next block's size
		// next block's footer
		footer = (blockHeader*)((char*)nextHeader + nextSize - sizeof(blockHeader));

		freeBlock->size_status += nextSize; // coalesce free and next block
		freeSize = freeBlock->size_status - freeLSB - freeSLB;//update freeSize
		footer->size_status = freeSize;// add free and next size
		nextHeader = (blockHeader*)((void*)nextHeader + nextSize);// update next block
	}
	else { //next block is allocated and not endmark
		footer = (blockHeader*)((void*)freeBlock + freeSize - sizeof(blockHeader));
		footer->size_status = freeSize;// add free block size
	}
	if ((nextHeader->size_status != 1) && (nextHeader->size_status & 2) == 2) {
		nextHeader->size_status -= 2; // next block previous block is freed
	}

	//then check the previous block
	if (freeSLB == 0) { //if previous block is free
		blockHeader *preFooter = freeBlock - 1; // if previous block is free, it has footer 
		preHeader = freeBlock - preFooter->size_status / 4;// previous block header
		preHeader->size_status += freeSize; // coalesce pre and free block
		footer->size_status = preFooter->size_status + freeSize;// add pre and free block size
	}

	// update the cur_blk
	if (nextHeader->size_status != 1) {
		cur_blk = nextHeader; // next block not the end
	}
	else {
		cur_blk = heapStart; // if the whole heap free
	}
	return 0;
}

/*
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */
int initHeap(int sizeOfRegion) {

	static int allocated_once = 0; //prevent multiple initHeap calls

	int pagesize;  // page size
	int padsize;   // size of padding when heap size not a multiple of page size
	void* mmap_ptr; // pointer to memory mapped area
	int fd;

	blockHeader* endMark;

	if (0 != allocated_once) {
		fprintf(stderr,
			"Error:mem.c: InitHeap has allocated space during a previous call\n");
		return -1;
	}
	if (sizeOfRegion <= 0) {
		fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
		return -1;
	}

	// Get the pagesize
	pagesize = getpagesize();

	// Calculate padsize as the padding required to round up sizeOfRegion 
	// to a multiple of pagesize
	padsize = sizeOfRegion % pagesize;
	padsize = (pagesize - padsize) % pagesize;

	allocsize = sizeOfRegion + padsize;

	// Using mmap to allocate memory
	fd = open("/dev/zero", O_RDWR);
	if (-1 == fd) {
		fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
		return -1;
	}
	mmap_ptr = mmap(NULL, allocsize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (MAP_FAILED == mmap_ptr) {
		fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
		allocated_once = 0;
		return -1;
	}

	allocated_once = 1;

	// for double word alignment and end mark
	allocsize -= 8;

	// Initially there is only one big free block in the heap.
	// Skip first 4 bytes for double word alignment requirement.
	heapStart = (blockHeader*)mmap_ptr + 1;

	// Set the end mark
	endMark = (blockHeader*)((void*)heapStart + allocsize);
	endMark->size_status = 1;

	// Set size in header
	heapStart->size_status = allocsize;

	// Set p-bit as allocated in header
	// note a-bit left at 0 for free
	heapStart->size_status += 2;

	// Set the footer
	blockHeader *footer = (blockHeader*)((void*)heapStart + allocsize - 4);
	footer->size_status = allocsize;

	return 0;
}

/*
 * Function to be used for DEBUGGING to help you visualize your heap structure.
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts)
 * t_End    : address of the last byte in the block
 * t_Size   : size of the block as stored in the block header
 */
void dumpMem() {

	int counter;
	char status[5];
	char p_status[5];
	char *t_begin = NULL;
	char *t_end = NULL;
	int t_size;

	blockHeader *current = heapStart;
	counter = 1;

	int used_size = 0;
	int free_size = 0;
	int is_used = -1;

	fprintf(stdout, "************************************Block list***\
                    ********************************\n");
	fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
	fprintf(stdout, "-------------------------------------------------\
                    --------------------------------\n");

	while (current->size_status != 1) {
		t_begin = (char*)current;
		t_size = current->size_status;

		if (counter > 30) {
			return;
		}
		if (t_size & 1) {
			// LSB = 1 => used block
			strcpy(status, "used");
			is_used = 1;
			t_size = t_size - 1;
		}
		else {
			strcpy(status, "Free");
			is_used = 0;
		}

		if (t_size & 2) {
			strcpy(p_status, "used");
			t_size = t_size - 2;
		}
		else {
			strcpy(p_status, "Free");
		}

		if (is_used)
			used_size += t_size;
		else
			free_size += t_size;

		t_end = t_begin + t_size - 1;

		fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%d\n", counter, status,
			p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);

		current = (blockHeader*)((char*)current + t_size);
		counter = counter + 1;
	}

	fprintf(stdout, "---------------------------------------------------\
                    ------------------------------\n");
	fprintf(stdout, "***************************************************\
                    ******************************\n");
	fprintf(stdout, "Total used size = %d\n", used_size);
	fprintf(stdout, "Total free size = %d\n", free_size);
	fprintf(stdout, "Total size = %d\n", used_size + free_size);
	fprintf(stdout, "***************************************************\
                    ******************************\n");
	fflush(stdout);

	return;
}