/*
 * Header for reading thermo information from MCC 134 boards
 */

#ifndef THERMO_H_
#define THERMO_H_

#include "daqhats_utils.h"

// Constants
#define THERMO_MIN_ADDR 2
#define THERMO_MAX_ADDR 4
#define THERMO_ADDRS    3
#define THERMO_LOW_CHANNEL  0
#define THERMO_HIGH_CHANNEL 3
#define THERMO_CHANNELS     4

#define MAX_BIN_SIZE 450

int8_t get_thermo(FILE *fp);

double calc_deviation(double bin[MAX_BIN_SIZE]);

#endif
