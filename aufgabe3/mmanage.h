/** Definitions for virtual memory management model
 * File: mmanage.h
 *
 * Prof. Dr. Wolfgang Fohl, HAW Hamburg
 * Winter 2016
 */
#ifndef MMANAGE_H
#define MMANAGE_H
#include "vmem.h"
#include <limits.h>
#include <getopt.h>
#include <string.h>

/** Event struct for logging */
struct logevent {
    int req_pageno;
    int replaced_page;
    int alloc_frame;
    int pf_count;
    int g_count;
};

/** Prototypes */
void usage(void);

void sighandler(int signo);

void vmem_init(void);

void allocate_page(void);

void fetch_page(int pt_idx);

void store_page(int pt_idx);

void update_pt(int frame);

int find_remove_frame(void);

int find_remove_fifo(void);

int find_remove_clock(void);

int find_remove_lru(void);

int search_freeframe(void);

void init_pagefile(const char *pfname);

void cleanup(void);

void logger(struct logevent le);

void dump_pt(void);

/** Misc */
#define MMANAGE_PFNAME "./pagefile.bin" /**< pagefile name */
#define MMANAGE_LOGFNAME "./logfile.txt"        /**< logfile name */

#define VMEM_ALGO_FIFO  0
#define VMEM_ALGO_LRU   1
#define VMEM_ALGO_CLOCK 2


#define SEED_PF 070514        /**< Get reproducable pseudo-random numbers for
                           init_pagefile */

#define VOID_IDX -1


/* Edit to modify algo, or remove line and provide
 * -DVMEM_ALGO ... compiler flag*/
/* #define VMEM_ALGO VMEM_ALGO_FIFO */

#endif /* MMANAGE_H */
