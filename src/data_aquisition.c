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
#include "voltage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>

// Constants
#define FNAME_BUFFER_SIZE 50

// Signal to stop logging
static volatile int8_t stop = 0;
void end_handler(int s) {
    stop = 1;
}

void print_chars(FILE*, char*);

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
    //fp_voltages = fopen("voltage_pre_vaccuum.csv", "w");
    //fp_voltages = fopen("voltage_sat_test.csv", "w");
    //fp_voltages = fopen(fname_voltages, "w");
    fp_voltages = fopen("logs/voltage_pre_vaccuum.csv", "w");
    fp_voltages_slow = fopen("logs/voltages_slow.csv", "w");

    // setup file for data to be read to
    FILE *data;
    FILE *fp_thermo_slow;
    char filename[FNAME_BUFFER_SIZE];
    printf("\n Enter filename to record temperature data to (include .csv): ");
    fgets(filename, FNAME_BUFFER_SIZE, stdin);
    //data = fopen(filename, "w");
    //data = fopen("temperature_sat_test.csv", "w");
    data = fopen("logs/temperature_pre_vaccuum.csv", "w");
    fp_thermo_slow = fopen("logs/thermo_slow.csv", "w");
    
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
