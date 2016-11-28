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

void vmem_init(void);

void vmem_cleanup(void);

void store_page(int page, int frame);

void load_page(int page, int frame);

int get_frame_fifo(void);

int get_frame_lru(void);

int get_frame_clock(void);

void sighandler(int signo);

void pagefault(void);

void dump(void);

void init_pagefile(const char *pfname);

void logger(struct logevent le);

/** Misc */
#define MMANAGE_PFNAME "./pagefile.bin" /**< pagefile name */
#define MMANAGE_LOGFNAME "./logfile.txt"        /**< logfile name */

#define NO_FRAME -1

#endif /* MMANAGE_H */
