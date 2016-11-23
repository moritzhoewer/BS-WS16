/* Header file for vmappl.c
 * File: vmappl.h
 * Prof. Dr. Wolfgang Fohl, HAW Hamburg
 * Winter 2016
 */

#ifndef VMAPPL_H
#define VMAPPL_H

#include <stdio.h>
#include <stdlib.h>
#include "vmaccess.h"

#define SEED 161114
#define LENGTH 550
#define RNDMOD 1000

void init_data(int length);

void quicksort(int l, int r);

void sort(int length);

void swap(int addr1, int addr2);

void vmem_cleanup(void);

#define NDISPLAYCOLS 8
void display_data(int length);
#endif
