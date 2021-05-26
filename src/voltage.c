#include "voltage.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>


double sample_values[SAMPLE_COUNT];
int8_t sample_index = 0;

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
int8_t get_voltages(FILE *fp, uint8_t print) {
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
            fprintf(fp, "%4.2f,", value);

            //printf("Voltage, Channel %d: T\n", address*8 + channel);
            //printf("Voltage, Channel %d: %4.2f\n", address*8 + channel, value);
            if (print)
                printf(" %2.2f |", value);
        }
    }
    //fprintf(fp, "\n");

    // Flush the buffer to keep it up to date
    fflush(fp);

    return RESULT_SUCCESS;
}

/*
 * Function: read_sample
 * 
 * Function gets sample from sensor to allow calculation of frequency.
 *
 * address: Address of board to read from
 * channel: Channel on board to read
 * 
 * returns: error condition (0 is no error)
 */
int8_t read_sample(uint8_t address, uint8_t channel) {
    uint32_t options = OPTS_DEFAULT; // Options for voltage boards

    double value;
    uint8_t result;
    
    result = mcc118_open(address);
    // Check for error
    if (result != RESULT_SUCCESS) {
        return result;
    }

    // Read voltage from board on channel
    result = mcc118_a_in_read(address, channel, options, &value);

    // Check for error
    if (result != RESULT_SUCCESS) {
        return result;
    }

    // Put value into sample array
    sample_values[sample_index++] = value;
    if (sample_index >= SAMPLE_COUNT) {
        sample_index = 0;
    }
    
    return 0;
}

/*
 * Function: calculate_rpm
 * 
 * Function determines RPM of system and prints to file
 * 
 * returns: RPM
 */
int16_t get_rpm(FILE *fp, uint8_t print) {
    double threshold = 1.0;
    uint16_t count = 0;

    uint16_t i = 0;
    // Find start of high pulse
    while (sample_values[i] < threshold) {
        i++;
    }

    // Determine how long signal is high
    while (sample_values[i++] > threshold) {
        count++;
    }

    // Calculate frequency
    int16_t freq = 1000 / count;
    // RPM = freq * 10
    int16_t rpm = freq * 10;

    // Write data to CSV file
    fprintf(fp, "%d,", rpm);
    // Flush the buffer to keep it up to date
    fflush(fp);

    if (print)
        printf(" %4d |", rpm);

    return rpm;
}

/*
 * Function: get_pressure
 * 
 * Function determines pressure from channel and prints to file
 * 
 * returns: pressure
 */
double get_pressure(FILE *fp, uint8_t address, uint8_t channel, uint8_t print) {
    uint32_t options = OPTS_DEFAULT; // Options for voltage boards

    double value;
    uint8_t result;

    result = mcc118_open(address);
    // Check for error
    if (result != RESULT_SUCCESS) {
        return -1;
    }

    // Read voltage from board on channel
    result = mcc118_a_in_read(address, channel, options, &value);

    // Check for error
    if (result != RESULT_SUCCESS) {
        return -1;
    }

    // Calculate pressure
    double pressure = result * 10.0 / 6;

    // Write to file
    fprintf(fp, "%12.2f\n", pressure);
    fflush(fp);

    if (print)
        printf("%12.2f|\n", pressure);
    
    return pressure;
}
