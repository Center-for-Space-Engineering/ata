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

int8_t get_voltages(FILE *fp, uint8_t print);

// PWM related functions
#define SAMPLE_COUNT 2000

int8_t read_sample(FILE *fp, uint8_t address, uint8_t channel);

// RPM calculation and logging
int16_t get_rpm(FILE *fp, uint8_t print);

// Pressure calculation and logging
double get_pressure(FILE *fp, uint8_t address, uint8_t channel, uint8_t print);

#endif
