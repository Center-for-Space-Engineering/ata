/*****************************************************************************

    MCC 134 Functions Demonstrated:
        mcc134_t_in_read
        mcc134_tc_type_write
        mcc134_info

    Purpose:
        Read a single data value for each channel in a loop.

    Description:
        This example demonstrates acquiring data using a software timed loop
        to read a single value from each selected channel on each iteration
        of the loop.

*****************************************************************************/
#include "daqhats_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>

// Constants
#define VOLTAGE_MIN_ADDR 0
#define VOLTAGE_MAX_ADDR 1
#define VOLTAGE_LOW_CHANNEL 0
#define VOLTAGE_HIGH_CHANNEL 7

#define THERMO_MIN_ADDR 2
#define THERMO_MAX_ADDR 4
#define THERMO_ADDRS    3
#define THERMO_LOW_CHANNEL  0
#define THERMO_HIGH_CHANNEL 3
#define THERMO_CHANNELS     4

#define MAX_BIN_SIZE 450

#define FNAME_BUFFER_SIZE 50

// Signal to stop logging
static volatile int8_t stop = 0;
void end_handler(int s) {
    stop = 1;
}

void print_chars(FILE*, char*);
double calc_deviation(double bin[MAX_BIN_SIZE]);

int8_t get_voltages(FILE *fp);
int8_t get_thermo(FILE *fp);

int main()
{
    uint8_t address;
    uint8_t channel;
    uint8_t low_chan;
    uint8_t high_chan;
    uint8_t tc_type = TC_TYPE_T;    // change this to desired thermocouple typehow to print the last 10 characters of a string in C
    
    int result = RESULT_SUCCESS;
    int samples_per_channel = 0;
    int num_channels = mcc134_info()->NUM_AI_CHANNELS;
    
    const int delay_between_reads = 1;  // ms
    // Set up how many loops before sampling for easier modification
    const int one_hertz = 1000 / delay_between_reads;
    const int sixty_hertz = 60000 / delay_between_reads;
    
    signal(SIGINT, end_handler);

    // TODO update these file pointers so the extension/suffix is added
    // File data for Voltages
    FILE *fp_voltages;
    FILE *fp_voltages_slow;
    char fname_voltages[FNAME_BUFFER_SIZE];
    printf("\n Enter filename to record voltage data to (include .csv): ");
    fgets(fname_voltages, FNAME_BUFFER_SIZE, stdin);
    fp_voltages = fopen("voltage_pre_vaccuum.csv", "w");
    //fp_voltages = fopen("voltage_sat_test.csv", "w");
    //fp_voltages = fopen(fname_voltages, "w");
    fp_voltages_slow = fopen("voltages_slow.csv", "w");

    // setup file for data to be read to
    FILE *data;
    FILE *fp_thermo_slow;
    char filename[FNAME_BUFFER_SIZE];
    printf("\n Enter filename to record temperature data to (include .csv): ");
    fgets(filename, FNAME_BUFFER_SIZE, stdin);
    //data = fopen(filename, "w");
    //data = fopen("temperature_sat_test.csv", "w");
    data = fopen("temperature_pre_vaccuum.csv", "w");
    fp_thermo_slow = fopen("thermo_slow.csv", "w");
    
    // setup for timestamp 
    time_t seconds;
    
    low_chan = 0;
    high_chan = num_channels - 1;
    
    uint8_t temp;
    for(int i = 2; i < 5; i++)
    {
        temp = (uint8_t)i;
        address = temp;
        result  = mcc134_open(address);
        STOP_ON_ERROR(result);
        for (channel = low_chan; channel <= high_chan; channel++)
        {
            result = mcc134_tc_type_write(address, channel, tc_type);
            STOP_ON_ERROR(result);
        } 
    }

    printf("Acquiring data ... Press 'Ctrl+C' to abort\n\n");

    // Display the header row for the data table
    fprintf(data, "Sample,Timestamp(dd:hh:mm:ss),Channel 0,SS,Channel 1,SS,Channel 2,SS,Channel 3,SS,Channel 4,SS,Channel 5,SS,Channel 6,SS,Channel 7,SS,Channel 8,SS,Channel 9,SS,Channel 10,SS,Channel 11,SS\n");
    fprintf(fp_thermo_slow, "Sample,Timestamp(dd:hh:mm:ss),Channel 0,SS,Channel 1,SS,Channel 2,SS,Channel 3,SS,Channel 4,SS,Channel 5,SS,Channel 6,SS,Channel 7,SS,Channel 8,SS,Channel 9,SS,Channel 10,SS,Channel 11,SS\n");
    
    // Header row for voltage table
    fprintf(fp_voltages, "Sample,Timestamp(dd:hh:mm:ss),Channel 0,Channel 1,Channel 2,Channel 3,Channel 4,Channel 5,Channel 6,Channel 7,Channel 8,Channel 9,Channel 10,Channel 11,Channel 12,Channel 13,Channel 14,Channel 15\n");
    fprintf(fp_voltages_slow, "Sample,Timestamp(dd:hh:mm:ss),Channel 0,Channel 1,Channel 2,Channel 3,Channel 4,Channel 5,Channel 6,Channel 7,Channel 8,Channel 9,Channel 10,Channel 11,Channel 12,Channel 13,Channel 14,Channel 15\n");
    
    //while (!enter_press())
    while (!stop)
    {
        // Display the updated samples per channel
        seconds = time(NULL);
        samples_per_channel++;
        
        /////////////////////////////////////////
        // Read values at 1 Hz from all boards //
        /////////////////////////////////////////
        if (samples_per_channel % one_hertz == 0) {
            system("clear");
            // MCC 118 boards (voltage)
            fprintf(fp_voltages, "%d,", samples_per_channel);
            print_chars(fp_voltages, ctime(&seconds));
            
            result = get_voltages(fp_voltages);
            STOP_ON_ERROR(result);
            
            // MCC 134 boards (thermocouple)
            fprintf(data, "%d,", samples_per_channel);
            print_chars(data, ctime(&seconds));
            
            result = get_thermo(data);
            STOP_ON_ERROR(result);
        }
        
        ////////////////////////////////////////////
        // Read values at 1/60 Hz from all boards //
        ////////////////////////////////////////////
        if (samples_per_channel % sixty_hertz == 0) {
            // MCC 118 boards (voltage)
            fprintf(fp_voltages_slow, "%d,", samples_per_channel);
            print_chars(fp_voltages_slow, ctime(&seconds));
            
            result = get_voltages(fp_voltages_slow);
            STOP_ON_ERROR(result);
            
            // MCC 134 boards (thermocouple)
            fprintf(fp_thermo_slow, "%d,", samples_per_channel);
            print_chars(fp_thermo_slow, ctime(&seconds));
            
            result = get_thermo(fp_thermo_slow);
            STOP_ON_ERROR(result);
        }
        
        // Sleep for a second
        usleep(delay_between_reads * 1000);
    }

stop:
    printf("Cleaning up\n");
    result = mcc134_close(address);
    print_error(result);

    return 0;
}

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

/*
 * Function: get_thermo
 * 
 * Function which iterates over all MCC 134 boards (thermo)
 * and writes the line to a file.
 * 
 * fp: file pointer for logging data
 * 
 * returns: error condition (0 is no error)
 */
int8_t get_thermo(FILE *fp) {
    // Static Variable declaration
    static double   bins        [THERMO_ADDRS][THERMO_CHANNELS][MAX_BIN_SIZE];
    static uint32_t bin_index   [THERMO_ADDRS][THERMO_CHANNELS];
    static uint32_t can_deviate [THERMO_ADDRS][THERMO_CHANNELS];
    
    // Variables
    uint8_t address;
    uint8_t channel;
    double value;
    double valueF;
    
    uint8_t result;
    double deviation;
    
    // Iterate over the boards
    for (address = 0; address < THERMO_ADDRS; ++address) {
        result = mcc134_open(address + THERMO_MIN_ADDR);
        // Check for error
        if (result != RESULT_SUCCESS) {
            return result;
        }
        
        // Iterate over each channel
        for (channel = THERMO_LOW_CHANNEL; channel <= THERMO_HIGH_CHANNEL; channel++) {
            result = mcc134_t_in_read(address + THERMO_MIN_ADDR, channel, &value);
            if (result != RESULT_SUCCESS) {
                return result;
            }
            
            valueF = value*1.8 + 32.0; // Convert to Fahrenheit
            if (value == OPEN_TC_VALUE) {
                fprintf(fp,"Open,");
            } else if (value == OVERRANGE_TC_VALUE) {
                fprintf(fp,"OverRange,");
            } else if (value == COMMON_MODE_TC_VALUE) {
                fprintf(fp,"Common Mode,");
            } else {
                fprintf(fp, "%12.2f,", valueF);
            }
            
            // Check for Steady State or Transient value
            
            // Get current index in circular buffer
            uint32_t j = bin_index[address][channel];
            // Update circular buffer
            bins[address][channel][j] = valueF;
            bin_index[address][channel]++;
            
            // If we have filled the buffer, it can then "deviate"
            if (bin_index[address][channel] == MAX_BIN_SIZE) {
                bin_index  [address][channel] = 0;
                can_deviate[address][channel] = 1;
            }
            
            // Determine whether Steady State (SS) or Transient (T)
            
            if (can_deviate[address][channel]) {
                deviation = calc_deviation(bins[address][channel]);
                // If deviation is less than the threshold, then SS
                if (deviation < 0.25) {
                    fprintf(fp, "Y,");
                    printf("Thermo, Channel %d: SS   %12.2f\n", address*THERMO_CHANNELS + channel, valueF);
                } else {
                    fprintf(fp, "N,");
                    printf("Thermo, Channel %d: T    %12.2f\n", address*THERMO_CHANNELS + channel, valueF);
                }
            }
        }
    }
    
    fprintf(fp, "\n");
    
    // Flush the buffer to keep it up to date
    fflush(fp);
    
    return result;
}

void print_chars(FILE* data, char* time_seconds)
{
    for(int i = 8; i < 19; i++)
    {
        // If there's a space, print a ":" instead
        if((int)time_seconds[i] == 32) {
            // Only print the colon if we're not at index 8,
            // because 8 will be a space if hours is less
            // than 10
            if (i != 8) {
                fprintf(data, ":");
            } else {
                fprintf(data, "0");
            }
        }
        else
        {
            fprintf(data, "%c", time_seconds[i]);
        }
    }
    fprintf(data, ",");
}

double calc_deviation(double bin[MAX_BIN_SIZE])
{
    double deviation = 0, sum = 0, mean = 0, temp = 0;
    for(int i = 0; i < MAX_BIN_SIZE; i++)
    {
        sum += bin[i];
    }
    mean = sum/(double)MAX_BIN_SIZE;
    for(int i = 0; i < MAX_BIN_SIZE; i++)
    {
        temp += pow(bin[i]-mean, 2);
    }
    deviation = sqrt(temp/(double)MAX_BIN_SIZE);
    return deviation;
}
