/** ****************************************************************
 * @file    aufgabe3/vmaccess.h
 * @author  Prof. Dr. Wolfgang Fohl, HAW Hamburg (original author)
 * @author  Moritz Hoewer (Moritz.Hoewer@haw-hamburg.de)
 * @author  Jesko Treffler (Jesko.Treffler@haw-hamburg.de)
 * @version 1.0
 * @date    28.11.2016
 * @brief   Header file for vmaccess.c
 ******************************************************************
*/

#ifndef VMACCESS_H
#define VMACCESS_H

/**
 * @brief Read from "virtual" address
 *
 * Precondition:
 * address must be in process address space (between 0 and VMEM_VIRTMEMSIZE)
 *
 * Postcondition:
 * will return the value stored at address
 *
 * @param[in] address the address to read from
 *
 * @return value stored at that address
 */
int vmem_read(int address);

/**
 * @brief Write data to "virtual" address
 *
 * Precondition:
 * address must be in process address space (between 0 and VMEM_VIRTMEMSIZE)
 *
 * Postcondition:
 * will write the value stored in data to address
 *
 * @param[in] address the address to write to
 * @param[in] data    the data to write
 */
void vmem_write(int address, int data);

/**
 * @brief Connect to virtual memory.
 *
 * Will request shared memory and then map it to a vmem_struct.
 * Assumes that the shared memory already exists and has been initialized.
 */
void vmem_init(void);

/**
 * @brief Unmap and unlink shared memory
 */
void vmem_cleanup(void);

#endif
