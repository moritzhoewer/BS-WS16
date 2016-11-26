/** ****************************************************************
 * @file    vmaccess.c 
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    26.11.2016
 * @brief   Implementation of Virtual Memory access
 ******************************************************************
 */

#include "vmaccess.h"
#include "vmem.h"

static struct vmem_struct *vmem = NULL;

/* Connect to shared memory (key from vmem.h) */
void vmem_init(void){
    // connect to shared memory
    int shm_fd = shm_open(SHMNAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("Error connecting to vmem");
        exit(EXIT_FAILURE);
    } else {
        PDEBUG("vmem successfully opened\n")
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
}

/* Read from "virtual" address */
int vmem_read(int address){
    if(vmem == NULL){
        vmem_init();
    }

    // increase access counter
    vmem->adm.g_count++;

    int page = address / VMEM_PAGESIZE;
    if(page < 0 || page > VMEM_NPAGES){
        perror("Index out of bounds!");
        vmem_cleanup();
        exit(EXIT_FAILURE);
    }
    if((vmem->pt.entries[page].flags & PTF_PRESENT) == 0){ /* page is not present */
        // pagefault
        vmem->adm.req_pageno = page;
        kill(vmem->adm.mmanage_pid, SIGUSR1);
        sem_wait(&(vmem->adm.sema));
    }

    int data_offset = address - page * VMEM_PAGESIZE;
    int frame_offset = vmem->pt.entries[page].frame * VMEM_PAGESIZE;

    // update last used on page
    vmem->pt.entries[page].last_used = vmem->adm.g_count;

    return vmem->data[frame_offset + data_offset];
}

/* Write data to "virtual" address */
void vmem_write(int address, int data){
    if(vmem == NULL){
        vmem_init();
    }

    // increase access counter
    vmem->adm.g_count++;

    int page = address / VMEM_PAGESIZE;
    if(page < 0 || page > VMEM_NPAGES){
        perror("Index out of bounds!");
        vmem_cleanup();
        exit(EXIT_FAILURE);
    }
    if((vmem->pt.entries[page].flags & PTF_PRESENT) == 0){ /* page is not present */
        // pagefault
        vmem->adm.req_pageno = page;
        kill(vmem->adm.mmanage_pid, SIGUSR1);
        sem_wait(&(vmem->adm.sema));
    }

    int data_offset = address - page * VMEM_PAGESIZE;
    int frame_offset = vmem->pt.entries[page].frame * VMEM_PAGESIZE;

    vmem->data[frame_offset + data_offset] = data;

    // update last used on page and flag as dirty
    vmem->pt.entries[page].last_used = vmem->adm.g_count;
    vmem->pt.entries[page].flags |= PTF_DIRTY;
}

void vmem_cleanup(void){
    // DEBUG!
    kill(vmem->adm.mmanage_pid, SIGUSR2);
    // DEBUG!

    munmap(vmem, SHMSIZE);
    vmem = NULL;
    shm_unlink(SHMNAME);
    PDEBUG("cleaned up shared memory\n");

}
