/** ****************************************************************
 * @file    aufgabe3/mmanage.c
 *
 * This is the memory manager process that works together with the vmaccess
 * process to mimic virtual memory management.
 *
 * The memory manager process will be invoked via a SIGUSR1 signal. It
 * maintains the page table and provides the data pages in shared memory
 *
 * This process is initiating the shared memory, so it has to be started prior
 * to the vmaccess process.
 *
 * @author  Prof. Dr. Wolfgang Fohl, HAW Hamburg (original author)
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    28.11.2016
 * @brief   Memory Manager BSP3
 ******************************************************************
 */

#include <stdlib.h>
#include <stdio.h>
#include "mmanage.h"

/**
 * @brief root structure for virtual memory
 */
static struct vmem_struct *vmem = NULL;

/**
 * @brief the pagefile
 */
static FILE *pagefile = NULL;

/**
 * @brief the logfile
 */
static FILE *logfile = NULL;

/**
 * @brief the number of the last signal to have been processed
 */
static int signal_number = 0;

/**
 * @brief function pointer for switching page replacement algorithm.
 */
static int (*get_frame_to_replace)(void);

/**
 * @brief program entry point for mmanage
 *
 * @param argc command line argument count
 * @param argv command line arguments
 *
 * @return exit code
 */
int main(int argc, char** argv) {
	struct sigaction sigact;

	/* set algorithm for replacement */
	if(argc < 2){
		printf("Please specify algorithm (FIFO, LRU, CLOCK)!\n");
		return EXIT_FAILURE;
	}

	if(strcmp(argv[1], "LRU") == 0){
		get_frame_to_replace = get_frame_lru;
	} else if(strcmp(argv[1], "CLOCK") == 0){
		get_frame_to_replace = get_frame_clock;
	} else {
		get_frame_to_replace = get_frame_fifo;
	}

	/* Init pagefile */
	init_pagefile(MMANAGE_PFNAME);

	/* Open logfile */
	logfile = fopen(MMANAGE_LOGFNAME, "w");
	if (!logfile) {
		perror("Error creating logfile");
		exit(EXIT_FAILURE);
	}

	/* Create shared memory and init vmem structure */
	vmem_init();

	/* Setup signal handler */
	/* Handler for USR1 */
	sigact.sa_handler = sighandler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	if (sigaction(SIGUSR1, &sigact, NULL) == -1) {
		perror("Error installing signal handler for USR1");
		exit(EXIT_FAILURE);
	} else {
		PDEBUG("USR1 handler successfully installed\n")
	}

	if (sigaction(SIGUSR2, &sigact, NULL) == -1) {
		perror("Error installing signal handler for USR2");
		exit(EXIT_FAILURE);
	} else {
		PDEBUG("USR2 handler successfully installed\n");
	}

	if (sigaction(SIGINT, &sigact, NULL) == -1) {
		perror("Error installing signal handler for INT");
		exit(EXIT_FAILURE);
	} else {
		PDEBUG("INT handler successfully installed\n");
	}

	/* Signal processing loop */
	signal_number = 0;
	while (signal_number != SIGINT) {
		signal_number = 0;
		pause();
		if (signal_number == SIGUSR1) { /* Page fault */
			PDEBUG("Processed SIGUSR1\n");
			signal_number = 0;
		} else if (signal_number == SIGUSR2) { /* PT dump */
			PDEBUG("Processed SIGUSR2\n");
			signal_number = 0;
		} else if (signal_number == SIGINT) {
			PDEBUG("Processed SIGINT\n");
		}
	}

	/* Cleanup */
	fclose(pagefile);
	fclose(logfile);
	vmem_cleanup();
	return 0;
}
/*
 * Initializes the pagefile for swapping pages out of memory
 *
 * Precondition:
 * pfname must be a valid filename
 *
 * Postcondition:
 * the File described by pfname will be overwritten.
 */
void init_pagefile(const char *pfname) {
	// create / overwrite file
	pagefile = fopen(pfname, "w+b");
	if (pagefile == NULL) {
		perror("Error creating pagefile");
		exit(EXIT_FAILURE);
	}

	// initialize pagefile with random junk
	for (int i = 0; i < VMEM_VIRTMEMSIZE; i++) {
		fwrite(&i, sizeof(int), 1, pagefile);
	}
}

/*
 * Initialize virtual memory.
 *
 * Will request and truncate shared memory and then map it to a vmem_struct.
 * The administration struct (with the semaphore) and page table in vmem_struct
 * will then be initialized.
 */
void vmem_init(void) {
	// request shared memory
	int shm_fd = shm_open(SHMNAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
	if (shm_fd == -1) {
		perror("Error initialising vmem");
		exit(EXIT_FAILURE);
	} else {
		PDEBUG("vmem successfully opened\n")
	}

	// resize
	int res = ftruncate(shm_fd, SHMSIZE);
	if (res == -1) {
		perror("Error resizing vmem");
		exit(EXIT_FAILURE);
	} else {
		PDEBUG("vmem successfully resized\n")
	}

	// map
	vmem = (struct vmem_struct*) mmap(NULL, SHMSIZE, PROT_READ | PROT_WRITE,
			MAP_SHARED, shm_fd, 0);
	if (!vmem) {
		perror("Error mapping vmem");
		exit(EXIT_FAILURE);
	} else {
		PDEBUG("vmem successfully mapped\n")
	}

	// init administration structure
	res = sem_init(&(vmem->adm.sema), 1, 0);
	if (res == -1) {
		perror("Error initialising semaphore");
		exit(EXIT_FAILURE);
	} else {
		PDEBUG("semaphore successfully initialized\n")
	}
	vmem->adm.mmanage_pid = getpid();
	vmem->adm.pf_count = 0;
	vmem->adm.req_pageno = 0;

	PDEBUG("Administration initialized\n");

	// init pagetable
	for (int i = 0; i < VMEM_NPAGES; i++) {
		vmem->pt.entries[i].last_used = 0;
		vmem->pt.entries[i].flags = 0;
		vmem->pt.entries[i].frame = VOID_IDX;
	}
	for (int i = 0; i < VMEM_NFRAMES; i++) {
		vmem->pt.framepage[i] = VOID_IDX;
	}

	PDEBUG("Pagetable initialized\n");
	dump();

}
/*
 * Cleanup virtual memory.
 *
 * Will destroy the shared semaphore first.
 */
void vmem_cleanup(void) {
	sem_destroy(&(vmem->adm.sema));
	munmap(vmem, SHMSIZE);
	vmem = NULL;
	shm_unlink(SHMNAME);
	PDEBUG("cleaned up shared memory\n");
}

/*
 * performs the necessary actions to handle a pagefault
 */
void pagefault() {
	PDEBUG("Pagefault\n");
	vmem->adm.pf_count++;
	struct logevent le;

	int frame_to_replace = get_free_frame();

	// no free frame ==> replace one
	if(frame_to_replace == VOID_IDX){
		frame_to_replace = get_frame_to_replace();
		int page_to_replace = vmem->pt.framepage[frame_to_replace];

		if(page_to_replace != -1 && (vmem->pt.entries[page_to_replace].flags & PTF_DIRTY)){
			store_page(page_to_replace, frame_to_replace);
		}
		vmem->pt.entries[page_to_replace].flags = 0; /* not present, not dirty, not used */
		le.replaced_page = page_to_replace;
	}

	int page_to_load = vmem->adm.req_pageno;
	load_page(page_to_load, frame_to_replace);

	vmem->pt.entries[page_to_load].flags = PTF_PRESENT; /* present, not dirty, not used */
	vmem->pt.entries[page_to_load].frame = frame_to_replace;
	vmem->pt.framepage[frame_to_replace] = page_to_load;

	/* logging */
	le.alloc_frame = frame_to_replace;
	le.g_count = vmem->adm.g_count;
	le.pf_count = vmem->adm.pf_count;

	le.req_pageno = page_to_load;
	logger(le);

	// wakeup vmappl
	sem_post(&(vmem->adm.sema));
}

/*
 * Stores a page to disk.
 *
 * Precondition:
 * page is between 0 and VMEM_NPAGES
 * frame is between 0 and VMEM_NFRAMES
 *
 * Postcondition:
 * data stored in frame will be written to disk
 */
void store_page(int page, int frame){
	// TODO: Range checking?
	int *pagedata = vmem->data + frame * VMEM_PAGESIZE;
	int offset = page * VMEM_PAGESIZE * sizeof(int);

	// go to page in pagefile
	int res = fseek(pagefile, offset, SEEK_SET);
	if(res != 0){
		perror("Failed to seek to file while storing page\n");
		vmem_cleanup();
		exit(EXIT_FAILURE);
	}

	// write data
	int size = fwrite(pagedata, sizeof(int), VMEM_PAGESIZE, pagefile);
	if(size != VMEM_PAGESIZE){
		perror("Failed to write file while storing page\n");
		vmem_cleanup();
		exit(EXIT_FAILURE);
	}
}

/*
 * Loads a page from disk.
 *
 * SIDEEFFECT:
 * Will change (overwrite) the data associated with frame.
 *
 * Precondition:
 * page is between 0 and VMEM_NPAGES
 * frame is between 0 and VMEM_NFRAMES
 *
 * Postcondition:
 * data stored in frame will be overwritten
 */
void load_page(int page, int frame){
	// TODO: Range check?
	int *pagedata = vmem->data + frame * VMEM_PAGESIZE;
	int offset = page * VMEM_PAGESIZE * sizeof(int);

	// go to page in pagefile
	int res = fseek(pagefile, offset, SEEK_SET);
	if(res != 0){
		perror("Failed to seek to file while fetching page\n");
		vmem_cleanup();
		exit(EXIT_FAILURE);
	}

	// read data
	int size = fread(pagedata, sizeof(int), VMEM_PAGESIZE, pagefile);
	if(size != VMEM_PAGESIZE){
		perror("Failed to read while fetching page\n");
		vmem_cleanup();
		exit(EXIT_FAILURE);
	}
}

/*
 * prints out the contents of the administration section and the page table.
 */
void dump() {
	PDEBUG("Dump\n");
	printf("====================================\n");
	printf("     Administrative Information\n");
	printf("====================================\n");
	printf("PID = %d\n", vmem->adm.mmanage_pid);
	printf("Pagefaults = %d\n", vmem->adm.pf_count);
	printf("Requested Page = %d\n", vmem->adm.req_pageno);

	printf("====================================\n");
	printf("            Pagetable\n");
	printf("====================================\n");
	int i;
	for (i = 0; i < VMEM_NPAGES / 4; i++) {
		int index = i;
		printf("%3d --> %3d (%d) | ", index, vmem->pt.entries[index].frame, vmem->pt.entries[index].flags);
		index += VMEM_NPAGES / 4;
		printf("%3d --> %3d (%d) | ", index, vmem->pt.entries[index].frame, vmem->pt.entries[index].flags);
		index += VMEM_NPAGES / 4;
		printf("%3d --> %3d (%d) | ", index, vmem->pt.entries[index].frame, vmem->pt.entries[index].flags);
		index += VMEM_NPAGES / 4;
		printf("%3d --> %3d (%d)\n", index, vmem->pt.entries[index].frame, vmem->pt.entries[index].flags);
	}

	printf("\n");

	for (i = 0; i < VMEM_NFRAMES / 4; i++) {
		int index = i;
		printf("%2d --> %3d | ", index, vmem->pt.framepage[index]);
		index += VMEM_NFRAMES / 4;
		printf("%2d --> %3d | ", index, vmem->pt.framepage[index]);
		index += VMEM_NFRAMES / 4;
		printf("%2d --> %3d | ", index, vmem->pt.framepage[index]);
		index += VMEM_NFRAMES / 4;
		printf("%2d --> %3d\n", index, vmem->pt.framepage[index]);
	}
}

/*
 * custom signal handler
 */
void sighandler(int signo) {
	signal_number = signo;
	switch (signo) {
	case SIGUSR1:
		pagefault();
		break;
	case SIGUSR2:
		dump();
		break;
	case SIGINT:
		break;
	}
}

/*
 * finds a free frame
 */
int get_free_frame(){
	for(int i = 0; i < VMEM_NFRAMES; i++){
		if(vmem->pt.framepage[i] == VOID_IDX){
			return i;
		}
	}
	return VOID_IDX;
}

/*
 * Gets a frame to be replaced according to FIFO principle.
 */
int get_frame_fifo(){ /* 557 */
	static int next = -1;
	next = (next + 1) % VMEM_NFRAMES;
	return next;
}

/*
 * Gets a frame to be replaced according to LRU principle.
 */
int get_frame_lru(){ /* 531 */
	int frame = 0;
	int min = vmem->pt.entries[vmem->pt.framepage[0]].last_used;

	for(int i = 1; i < VMEM_NFRAMES; i++){
		int current = vmem->pt.entries[vmem->pt.framepage[i]].last_used;
		if(current < min){
			min = current;
			frame = i;
		}
	}

	return frame;
}

/*
 * Gets a frame to be replaced according to CLOCK algorithm.
 */
int get_frame_clock(){ /* 543 */
	static int current = 0;
	while(vmem->pt.entries[vmem->pt.framepage[current]].flags & PTF_USED){
		vmem->pt.entries[vmem->pt.framepage[current]].flags &= ~PTF_USED;
		current = (current + 1) % VMEM_NFRAMES;
	}
	return current;
}

/* Do not change!  */
void logger(struct logevent le) {
	fprintf(logfile, "Page fault %10d, Global count %10d:\n"
			"Removed: %10d, Allocated: %10d, Frame: %10d\n", le.pf_count, le.g_count,
			le.replaced_page, le.req_pageno, le.alloc_frame);
	fflush(logfile);
}
