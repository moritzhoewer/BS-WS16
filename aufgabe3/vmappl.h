/** ****************************************************************
 * @file    aufgabe3/vmappl.h
 * @author  Prof. Dr. Wolfgang Fohl, HAW Hamburg
 * @version 1.0
 * @date    15.11.2016
 * @brief   Header file for vmappl.c
 ******************************************************************
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

#define NDISPLAYCOLS 8
void display_data(int length);
#endif
