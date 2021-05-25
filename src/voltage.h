/*
 * Header for reading voltage information from MCC 118 boards
 */

#ifndef VOLTAGE_H_
#define VOLTAGE_H_

#include "daqhats_utils.h"

// Constants
#define VOLTAGE_MIN_ADDR 0
#define VOLTAGE_MAX_ADDR 1
#define VOLTAGE_LOW_CHANNEL 0
#define VOLTAGE_HIGH_CHANNEL 7

int8_t get_voltages(FILE *fp);

#endif
