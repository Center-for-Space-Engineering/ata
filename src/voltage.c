#include "voltage.h"
#include <stdio.h>
#include <stdint.h>

/*
 * Function: get_voltages
 * 
 * Function which iterates over all MCC 118 boards (voltage)
 * and writes the line to a file.
 * 
 * fp: file pointer for logging data
 * 
 * returns: error condition (0 is no error)
 */
int8_t get_voltages(FILE *fp) {
    // Variable declarations
    uint8_t address;
    uint8_t channel;
    uint32_t options = OPTS_DEFAULT; // Options for voltage boards

    double value;
    uint8_t result;

    // Iterate over boards
    for (address = VOLTAGE_MIN_ADDR; address <= VOLTAGE_MAX_ADDR; ++address) {
        result = mcc118_open(address);
        // Check for error
        if (result != RESULT_SUCCESS) {
            return result;
        }

        // Iterate over all of the channels
        for (channel = VOLTAGE_LOW_CHANNEL; channel <= VOLTAGE_HIGH_CHANNEL; ++channel) {
            // Read voltage from board on channel
            result = mcc118_a_in_read(address, channel, options, &value);
            // Check for error
            if (result != RESULT_SUCCESS) {
                return result;
            }

            // Write data to CSV file
            fprintf(fp, "%12.2f,", value);

            //printf("Voltage, Channel %d: T\n", address*8 + channel);
            printf("Voltage, Chanel %d: %12.2f\n", address*8 + channel, value);
        }
    }
    fprintf(fp, "\n");

    // Flush the buffer to keep it up to date
    fflush(fp);

    return RESULT_SUCCESS;
}

