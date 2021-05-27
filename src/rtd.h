#ifndef RTD_H_
#define RTD_H_

#include <stdio.h>
#include <stdint.h>

#define RTD_COUNT 4

void rtd_setup();

void rtd_read(FILE* fp, uint8_t print);

void rtd_close();

#endif
