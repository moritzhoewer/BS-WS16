/** ****************************************************************
 * @file    aufgabe3/mmanage.h
 * @author  Prof. Dr. Wolfgang Fohl, HAW Hamburg (original author)
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    28.11.2016
 * @brief   Definitions for virtual memory management model
 ******************************************************************
*/

#ifndef MMANAGE_H
#define MMANAGE_H
#include "vmem.h"
#include <limits.h>
#include <string.h>

/**
 * @brief Event struct for logging.
 */
struct logevent {
    int req_pageno;
    int replaced_page;
    int alloc_frame;
    int pf_count;
    int g_count;
};

/**
 * @brief Initialize virtual memory.
 *
 * Will request and truncate shared memory and then map it to a vmem_struct.
 * The administration struct (with the semaphore) and page table in vmem_struct
 * will then be initialized.
 */
void vmem_init(void);

/**
 * @brief Cleanup virtual memory.
 *
 * Will destroy the shared semaphore first.
 */
void vmem_cleanup(void);

/**
 * @brief Stores a page to disk.
 *
 * Precondition:
 * page is between 0 and VMEM_NPAGES
 * frame is between 0 and VMEM_NFRAMES
 *
 * Postcondition:
 * data stored in frame will be written to disk
 *
 * @param[in] page  the page number to store
 * @param[in] frame the frame number where page is currently mapped to
 */
void store_page(int page, int frame);

/**
 * @brief Loads a page from disk.
 *
 * <b>SIDEEFFECT:</b>
 * Will change (overwrite) the data associated with frame.
 *
 * Precondition:
 * page is between 0 and VMEM_NPAGES
 * frame is between 0 and VMEM_NFRAMES
 *
 * Postcondition:
 * data stored in frame will be overwritten
 *
 * @param[in] page  the page to load
 * @param[in] frame the frame to load into
 */
void load_page(int page, int frame);

/**
 * @brief Gets a frame to be replaced according to FIFO principle.
 *
 * @return index of the frame to be replaced
 */
int get_frame_fifo(void);

/**
 * @brief Gets a frame to be replaced according to LRU principle.
 *
 * @return index of the frame to be replaced
 */
int get_frame_lru(void);

/**
 * @brief Gets a frame to be replaced according to CLOCK algorithm.
 *
 * @return index of the frame to be replaced
 */
int get_frame_clock(void);

/**
 * @brief custom signal handler
 *
 * @param[in] signo the signal(number) to be handled
 */
void sighandler(int signo);

/**
 * @brief performs the necessary actions to handle a pagefault
 */
void pagefault(void);

/**
 * @brief prints out the contents of the administration section and the page
 *        table.
 */
void dump(void);

/**
 * @brief Initializes the pagefile for swapping pages out of memory
 *
 * Precondition:
 * pfname must be a valid filename
 *
 * Postcondition:
 * the File described by pfname will be overwritten.
 *
 * @param pfname the name of the file
 */
void init_pagefile(const char *pfname);

/**
 * @brief Logs a message to the logfile for later analysis
 *
 * @param le the logevent to log
 */
void logger(struct logevent le);

/* Misc */
/**
 * @brief pagefile name
 */
#define MMANAGE_PFNAME "./pagefile.bin"

/**
 * @brief logfile name
 */
#define MMANAGE_LOGFNAME "./logfile.txt"

/**
 * @brief indicates that page hasn't been initialized
 */
#define NO_FRAME -1

#endif /* MMANAGE_H */
