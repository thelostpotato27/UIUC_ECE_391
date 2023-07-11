#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "filesys.h"
#include "terminal.h"
#include "rtc.h"
#include "syscalls.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	// tests if interrupt entries initialized correctly
	i = 0;
		if ((idt[i].reserved0) != 0 || (idt[i].reserved1 != 1) || 
		(idt[i].reserved2 != 1) || (idt[i].reserved3 != 1) ||
		(idt[i].dpl != 0)) {
			assertion_failure();
			result = FAIL;
		}
	
	i = 0x21; // tests if devices initialized correctly
	if((idt[i].reserved3 != 0) || idt[i].dpl != 3) {
		assertion_failure();
		result = FAIL;
	}

	return result;
}


/* Paging Test
 * 
 * Tests that values are initialized correctly
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging
 * Files: paging.h/c
 */
int paging_test(){
	TEST_HEADER;
	static uint32_t tester;
	int result = PASS;

	asm volatile ("mov %%cr4,%0" : "=r"(tester));   // read from cr4
	tester &= 0x10;		        					// check if fourth bit is 1
	if (tester == 0){
		return FAIL;
	}

	asm volatile ("mov %%cr0,%0" : "=r"(tester));   // read from cr4
	tester &= ENABLE_PG;		        			// check if 31st bit is 1
	if (tester == 0){
		return FAIL;
	}

	asm volatile ("mov %%cr3,%0" : "=r"(tester));	//read from cr3
	if (tester == NULL){							//check if its not null
		return FAIL;
	}
	
	if (((uint32_t*)tester)[0] == NULL){			//checks if address from cr3 can be dereferenced
		return FAIL;								//should not be null because cr3 points to directory and first directory entry
	}												//should point to initialized page table
	tester = ((uint32_t*)tester)[0]&0xFFFFF000; // bitmask for paging address
	if (((uint32_t*)tester)[VIDEO_INDEX] == NULL){
		return FAIL;
	}

	// checks if uninitialized entries in Directory return null for addresses
	asm volatile ("mov %%cr3,%0" : "=r"(tester));
    tester = ((uint32_t*)tester)[3]&0xFFFFF000;
    if (tester != NULL){
        printf("paging test dereferencing fail");
        return FAIL;
    }

	//tests to see if the page in directory 1 has the proper bit initialized to 
    //indicate it as a 4mb page
    asm volatile ("mov %%cr3,%0" : "=r"(tester));
    tester = ((uint32_t*)tester)[1];
    if ((tester&0x80) == 0){
        printf("4mb page not initialized properly");
        return FAIL;
    }
	//testing to see if there is a pagetable in directory[1]
    if (((uint32_t*)tester)[1] == NULL){
        printf("no page in directory[1]");
        return FAIL;
    }

	return result;
}

/* Divide by zero test
 * 
 * Attempts to divide by zero, it's supposed to blue screen
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Blue Screen, error message
 * Coverage: IDT
 * Files: idt.c/h, idt_wrap.S
 */
int divzero_test()
{
	TEST_HEADER;
	int a = 2;
	int b = 0;
	//should bluescreen
	int c = a / b;

	if(a == 0)
		return c; //to make gcc happy
	
	return FAIL;
}

/* NOT IMPLEMENTED OR USED YET
 * 
 * To test syscall later
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Bluescreen
 * Files: syscalls.c/h (not implemented yet)
 */

int syscall_test()
{
	TEST_HEADER;
	asm volatile("int $0x80"); //should bluescreen
	return FAIL;
}


/* Null Pointer Test
 * 
 * 
 * Inputs: None
 * Outputs: FAIL
 * Side Effects: Returns paging fault exception
 * Coverage: Tests interrupts and paging
 * Files: idt.c/h, idt_wrap.S, paging.c/h
 */
int nullptr_test()
{
	TEST_HEADER;
	int* null_pointer = (int*)0;
	int bluescreen = *null_pointer; //should bluescreen

	if(null_pointer != null_pointer)
		return bluescreen; //to make gcc happy

	return FAIL;
}


// add more tests here

/* Checkpoint 2 tests */

/*
 * File System Test 1
 *
 * DESCRIPTION: Tests ability of file system to list files.
 * 
 * INPUTS: none
 * 
 * OUTPUTS: PASS/FAIL
 */

int filesys_test1() {
	clear();
	TEST_HEADER;
	int result = PASS; 

	/*PSEUDOCODE */
	// get boot block


    int32_t i,j, fd;
    dentry_t * cur_dentry;
    uint8_t buf[MAX_NAME_LENGTH + 1]; // size 32 + null
    inode_t * cur_inode; // inode_ptr should point to 0th inode
    uint32_t entries = boot_block->dir_entries; // see Appendix A
    fd = 0;

	  printf("Number of entries is %d.\n", entries);
	 
	  // iterate through all directory entries
    for (i = 0; i < entries; i++) {
        cur_dentry = (dentry_t *) &(boot_block->d[i]);
        cur_inode = inode_start + cur_dentry->inode_num;
        printf("File name: ");
        dir_read(i, buf, MAX_NAME_LENGTH);
        

        for (j = 0; j < MAX_NAME_LENGTH; j++) {
            if (buf[j]) putc(buf[j], disp_terminal);
            else putc(' ', disp_terminal);
    }
    
        printf("\nFile Type: %d\n", cur_dentry->filetype);
        printf("File Size: %d\n", cur_inode->data_length);
		// printf("Inode Number: %d\n", cur_dentry->inode_num);
        
    }
	 
	

	return result;
}
/*
 * 
 *
 * DESCRIPTION: Tests terminal scrolling.
 * 
 * INPUTS: none
 * 
 * OUTPUTS: PASS/FAIL
 */

int terminal_scrolling_test()
{	
	TEST_HEADER;

	int i = 0;
	for(i = 0; i < 30; i++)
	{
		printf("Line %d\n", i);
	}
	
	return PASS;
}


/*
 * 
 *
 * DESCRIPTION: Tests RTC functionality. Reads and writes many frequencies.
 * 
 * INPUTS: none
 * 
 * OUTPUTS: PASS/FAIL
 */

int rtc_test() {
	TEST_HEADER;
	int i, j;
	
	clear();
	rtc_open(NULL);
	// print_freq = 1;

	for (i = 2; i <= 1024; i *= 2) { // 2 is min frequency, 1024 is max
		if(rtc_write(NULL, &i, sizeof(uint32_t))) return FAIL;
		for (j = 0; j < 10; j++) {
			if(rtc_read(NULL, &i, sizeof(uint32_t))) return FAIL;
		}
		printf("Frequency %d Hz successful.\n", i);
	}

	// print_freq = 0;
	rtc_close(NULL);
	
	return PASS;
}

/*
 * Opens a regular file.
 *
 * DESCRIPTION: Tests file functionality and properties.
 * 
 * INPUTS: none
 * 
 * OUTPUTS: PASS/FAIL
 */

int open_file_test() {
	clear();

	TEST_HEADER;


	unsigned char buf[6000] = {0};
	unsigned char buf_txt[256] = {0};
	int j;

	// long file
	if (file_open((unsigned char*)"verylargetextwithverylongname.txt")) {
		printf("Long file name open failed.\n");
		return FAIL;
	}
	
	printf("Inode Number is: %d\n", data_entry.inode_num);

	if(data_read(data_entry.inode_num, buf, 6000, 0)) {
		printf("File read at verylargetextwithverylongname.txt failed.\n");
		return FAIL;
	}

	file_close(data_entry.inode_num);

	for (j = 0; j < 6000; j++) {
            if (buf[j]) putc(buf[j], disp_terminal);
            // else putc(' ');
    }

	printf("\n");

	// we will be printing the contents of this file
	file_open((unsigned char*)"cat");
	printf("Inode Number is: %d\n", data_entry.inode_num);

	// tests when file does not end perfectly at end of block
	if(data_read(data_entry.inode_num, buf, 6000, 0)) {
		printf("File read at cat failed.\n");
		return FAIL;
	}

	file_close(data_entry.inode_num);

	for (j = 0; j < 6000; j++) {
            if (buf[j]) putc(buf[j], disp_terminal);
            else putc(' ', disp_terminal);
    }

	printf("\n");

	// short file
	if (file_open((unsigned char*)"frame0.txt")) {
		printf("frame0.txt open failed.\n");
		return FAIL;
	}
	
	printf("Inode Number is: %d\n", data_entry.inode_num);

	if(data_read(data_entry.inode_num, buf_txt, 256, 0)) {
		printf("File read at frame0.txt failed.\n");
		return FAIL;
	}

	file_close(data_entry.inode_num);

	for (j = 0; j < 256; j++) {
            if (buf_txt[j]) putc(buf_txt[j], disp_terminal);
            else putc(' ', disp_terminal);
    }

	printf("\n");

	if (!file_open((unsigned char*)"")) {
		printf("Empty file name open failed.\n");
		return FAIL;
	}

	file_close(data_entry.inode_num);

	if (!file_open((unsigned char*)"garbage")) {
		printf("Nonexistent file name open failed.\n");
		return FAIL;
	}

	file_close(data_entry.inode_num);

	return PASS;
}

/*
 *
 * DESCRIPTION: Tests reading and writing from terminal.
 * 
 * INPUTS: none
 * 
 * OUTPUTS: PASS/FAIL
 */

int terminal_test() {
	TEST_HEADER;

	int bytes;
	int result = PASS;
	char buf[256];
	int i;
	
	clear();

	private_terminal_open();

	for (i = 124; i <= 132; i+=2) {
		printf("Testing buffer Size: %d\n", i);
		private_terminal_write("Testing buffer\n", 15);
		bytes = private_terminal_read(buf, i);
		//if (bytes != i) return FAIL;
		printf("Read size %d output: ", bytes);
		private_terminal_write(buf, bytes);
		printf("\n");
	}

	private_terminal_close();

	return result;
}

/* Checkpoint 3 tests */

// Tests ability of system to execute program testprint
// Returns pass upon success.
int syscall_execute_test() {
	TEST_HEADER;

	char* command = "testprint";
	printf("Testing Program: %s\n", command);
	execute((uint8_t*)command);
	return PASS;
}
// Tests ability of system to open file
int syscall_open_test() {
	TEST_HEADER;

	char* command = "frame0.txt";
	printf("Opening File: %s\n", command);
	open((uint8_t*)command);

	if (curr_pcb->open_files[2].flags != FLAG_BUSY) return FAIL;

	return PASS;
}
// Tests ability of system to close file -- should be run after open without creating
// additional files 
int syscall_close_test() {
	TEST_HEADER;

	char* command = "frame0.txt";
	printf("Closing File: %s\n", command);
	close(2); // frame0.txt should be at 2
	if (curr_pcb->open_files[2].flags != FLAG_FREE) return FAIL;

	return PASS;
} 

// Tests ability of system to read files
int syscall_read_test() {
	TEST_HEADER;
	unsigned char buf[256] = {0};
	int i;
	char* command = "frame0.txt";
	printf("Reading from File: %s\n", command);

	i = read(2, buf, 256);

	if(i) return FAIL;

	for (i = 0; i < 256; i++) {
		if(buf[i]) {
			putc(buf[i], disp_terminal);
		}
	}


	return PASS;
}

// Tests ability of system to write
int syscall_write_test() {
	TEST_HEADER;

	unsigned char* buf[256] = {0};
	int ret = 0;
	printf("Attempting file write on open file.\n");
	ret = write(2, buf, 256); // should do nothing
	if (ret != -1) return FAIL;

	return PASS;
}


/* Checkpoint 4 tests */

// vidmap test -- checks for invalid values
int syscall_vidmap_test() {
	TEST_HEADER;
	uint32_t addr = 0x10;
	if (vidmap((uint8_t**)addr) == 0) return FAIL;

	addr = VIDEO_END + 10;

	if (vidmap((uint8_t**)addr) == 0) return FAIL;

	return PASS;
}

// getargs tests -- checks for invalid values
int syscall_getargs_test() {
	TEST_HEADER;

	uint8_t buf[10];

	curr_pcb->argv[0][0] = 'A';

	printf("Contents of curr_pcb : %c\n", curr_pcb->argv[0][0]);

	if (getargs(buf, -4) == 0) return FAIL;

	getargs(buf, 1);

	printf("Contents of buf : %c\n", buf[0]);

	if (buf[0] != 'A') return FAIL;


	return PASS;
}

int syscall_interrupt_test()
{
	TEST_HEADER;
	const char* filename = "frame0.txt";

	asm volatile(
		"mov %0, %%ebx \n \
		mov $5, %%eax \n \
		int $0x80":
		:"r"(filename):"cc"
	);

	return PASS;
}

/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// non-kernel crashers

	// test execute separately from rest of open/read/write/close
	// TEST_OUTPUT("syscall_execute_test", syscall_execute_test());

	// TEST_OUTPUT("syscall_interrupt_test", syscall_interrupt_test());
	//TEST_OUTPUT("syscall_vidmap_test", syscall_vidmap_test());
	//TEST_OUTPUT("syscall_getargs_test", syscall_getargs_test());

	// Note to tester: syscall_open_test and syscall_close_test 
	// must be run together
	// TEST_OUTPUT("syscall_open_test", syscall_open_test());
	// TEST_OUTPUT("syscall_read_test", syscall_read_test());
	// TEST_OUTPUT("syscall_write_test", syscall_write_test());
	// TEST_OUTPUT("syscall_close_test", syscall_close_test());
	// // printf("sanity check\n");
}
