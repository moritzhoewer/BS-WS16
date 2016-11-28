/** ****************************************************************
 * @file    aufgabe3/vmem.h
 * @author  Prof. Dr. Wolfgang Fohl, HAW Hamburg (original author)
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    28.11.2016
 * @brief   Model of virtual memory management
 ******************************************************************
 */

#ifndef VMEM_H
#define VMEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/shm.h>

/**
 * @brief common name for the shared memory
 */
#define SHMNAME "/shm-bsp3"

/* memory sizes */
/**
 * @brief process address space (items)
 */
#define VMEM_VIRTMEMSIZE 1024

/**
 * @brief physical memory (items)
 */
#define VMEM_PHYSMEMSIZE 128

/**
 * @brief items per page
 */
#define VMEM_PAGESIZE 8

/**
 * @brief total number of pages
 */
#define VMEM_NPAGES (VMEM_VIRTMEMSIZE / VMEM_PAGESIZE)

/**
 * @brief number of available frames
 */
#define VMEM_NFRAMES (VMEM_PHYSMEMSIZE / VMEM_PAGESIZE)


/* Page Table */

/**
 * @brief present flag
 */
#define PTF_PRESENT 1

/**
 * @brief dirty flag
 *
 * Indicates that page has changed and needs to be written to disk when removed
 * from memory.
 */
#define PTF_DIRTY 2

/**
 * @brief used flag
 *
 * necessary for CLOCK algorithm
 */
#define PTF_USED 4

/**
 * @brief structure for a page table entry
 */
struct pt_entry {
    int flags; /* see defines above */
    int frame; /* Frame idx */
    int last_used; /* Global counter as quasi-timestamp for LRU */
};

/**
 * @brief structure for administration of memory
 */
struct vmem_adm_struct {
    pid_t mmanage_pid;
    sem_t sema; /* Coordinate acces to shm */
    int req_pageno; /* Number of requested page */
    int pf_count; /* Page fault counter */
    int g_count; /* Global access counter as quasi-timestamp */
};

/**
 * @brief structure for the page table
 */
struct pt_struct {
    struct pt_entry entries[VMEM_NPAGES];
    int framepage[VMEM_NFRAMES]; /* pages on frame */
};

/**
 * @brief root structure for the shared memory
 *
 * Contains administration structure, page table and frames (data)
 */
struct vmem_struct {
    struct vmem_adm_struct adm;
    struct pt_struct pt;
    int data[VMEM_NFRAMES * VMEM_PAGESIZE];
};

/**
 * @brief size of root structure and therefore size of shared memory area
 */
#define SHMSIZE (sizeof(struct vmem_struct))

#undef PDEBUG             /* undef it, just in case */
#ifdef DEBUG_MESSAGES
#define PDEBUG(fmt, args...) fprintf(stderr,   fmt, ## args); fflush(stderr);
#else
#define PDEBUG(fmt, args...)
#endif

#endif /* VMEM_H */
