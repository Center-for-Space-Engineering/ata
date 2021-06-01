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
#include "thermo.h"
#include "rtd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>

// Constants
#define DELAY_TIME 500
#define FNAME_BUFFER_SIZE 50

// Signal to stop logging
static volatile int8_t stop = 0;
void end_handler(int s) {
    stop = 1;
}

void print_chars(FILE*, char*);

static const char THERMO_HEADER[] = "Sample,Timestamp(dd:hh:mm:ss),Channel 0,SS,Channel 1,SS,Channel 2,SS,Channel 3,SS,Channel 4,SS,Channel 5,SS,Channel 6,SS,Channel 7,SS,Channel 8,SS,Channel 9,SS,Channel 10,SS,Channel 11,SS\n";
static const char VOLTAGE_HEADER[] = "Sample,Timestamp(dd:hh:mm:ss),Channel 0,Channel 1,Channel 2,Channel 3,Channel 4,Channel 5,Channel 6,Channel 7,Channel 8,Channel 9,Channel 10,Channel 11,Channel 12,Channel 13,Channel 14,Channel 15,RPM,Pressure\n";
static const char RTD_HEADER[] = "Sample,Timestamp(dd:hh:mm:ss),RTD 0,RTD 1,RTD 2,RTD 3\n";

int main()
{
    // Setup MCC variables
    int result = RESULT_SUCCESS;
    int samples_per_channel = 0;
    
    // Delay and loop constants
    const int delay_between_reads = 1;  // ms
    // Set up how many loops before sampling for easier modification
    //const int one_hertz = 1000 / delay_between_reads;
    //const int sixty_hertz = 60000 / delay_between_reads;
    const int one_hertz = 2000 / delay_between_reads;
    const int sixty_hertz = 120000 / delay_between_reads;
    //const int one_hertz = 4000 / delay_between_reads;
    //const int sixty_hertz = 240000 / delay_between_reads;
    
    // Set up handler to catch Ctrl+C
    signal(SIGINT, end_handler);

    // Start busPirate things to read from RTD
    //rtd_setup();

    // TODO update these file pointers so the extension/suffix is added
    // File data for Voltages
    FILE *fp_voltages;
    FILE *fp_voltages_slow;
    //char fname_voltages[FNAME_BUFFER_SIZE];
    //printf("\n Enter filename to record voltage data to (include .csv): ");
    //fgets(fname_voltages, FNAME_BUFFER_SIZE, stdin);
    fp_voltages = fopen("logs/voltages.csv", "a");
    fp_voltages_slow = fopen("logs/voltages_slow.csv", "a");

    // setup file for data to be read to
    FILE *fp_thermo;
    FILE *fp_thermo_slow;
    //char filename[FNAME_BUFFER_SIZE];
    //printf("\n Enter filename to record temperature data to (include .csv): ");
    //fgets(filename, FNAME_BUFFER_SIZE, stdin);
    fp_thermo = fopen("logs/thermo.csv", "a");
    fp_thermo_slow = fopen("logs/thermo_slow.csv", "a");

    // setup file for RPM logging
    FILE *fp_rpm;
    fp_rpm = fopen("logs/rpm.csv", "a");
    fprintf(fp_rpm, "Value\n");
    
    // setup files for RTD logging
    //FILE *fp_rtd      = fopen("logs/rtd.csv", "w");
    //FILE *fp_rtd_slow = fopen("logs/rtd_slow.csv", "w");

    // setup for timestamp 
    time_t seconds;
    
    printf("Acquiring data ... Press 'Ctrl+C' to abort\n\n");

    // Write header row for temperature data
    fprintf(fp_thermo,      THERMO_HEADER);
    fprintf(fp_thermo_slow, THERMO_HEADER);
    
    // Write header row for voltage data
    fprintf(fp_voltages,      VOLTAGE_HEADER);
    fprintf(fp_voltages_slow, VOLTAGE_HEADER);

    // Write header row for rtd data
    //fprintf(fp_rtd,      RTD_HEADER);
    //fprintf(fp_rtd_slow, RTD_HEADER);

    // Setup MCC 134 (thermo) boards
    setup_thermo_daq();
    
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
            printf("\n");
            printf("    Time     |                                          Voltage Channel                                                          |\n");
            printf(" ");
            print_chars(NULL, ctime(&seconds));
            printf(" |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   |  10   |  11   |  12   |  13   |  14   |  15   |  16   |\n");
            printf("             |");
            // MCC 118 boards (voltage)
            fprintf(fp_voltages, "%d,", samples_per_channel);
            print_chars(fp_voltages, ctime(&seconds));
            
            result = get_voltages(fp_voltages, 1);
            printf("\n");
            STOP_ON_ERROR(result);
            
            printf("             -------------------------------------------------------------------------------------------------\n");
            printf("             |                                       Thermo Channel                                          |\n");
            printf("             |   1   |   2   |   3   |   4   |   5   |   6   |   7   |   8   |   9   |  10   |  11   |  12   |\n");
            printf("             |");
            // MCC 134 boards (thermocouple)
            fprintf(fp_thermo, "%d,", samples_per_channel);
            print_chars(fp_thermo, ctime(&seconds));
            
            result = get_thermo(fp_thermo, 1);
            printf("\n");
            STOP_ON_ERROR(result);

            printf("             -------------------------------------------------------------------------------------------------\n");
            printf("             |   RPM   |   Pressure      |\n");
            printf("             |");

            // RPM calculation
            get_rpm(fp_voltages, 1);
            // Pressure calculation
            get_pressure(fp_voltages,1,3, 1);
            printf("             -----------------------------\n");

            // RTD data
            //printf("             ---------------------------------\n");
            //printf("             |              RTD              |\n");
            //printf("             |   0   |   1   |   2   |   3   |\n");
            //printf("             |");
            //rtd_read(fp_rtd, 1);
            //printf("             ---------------------------------\n");
        }
        
        ////////////////////////////////////////////
        // Read values at 1/60 Hz from all boards //
        ////////////////////////////////////////////
        if (samples_per_channel % sixty_hertz == 0) {
            // MCC 118 boards (voltage)
            fprintf(fp_voltages_slow, "%d,", samples_per_channel);
            print_chars(fp_voltages_slow, ctime(&seconds));
            
            result = get_voltages(fp_voltages_slow, 0);
            STOP_ON_ERROR(result);
            
            // MCC 134 boards (thermocouple)
            fprintf(fp_thermo_slow, "%d,", samples_per_channel);
            print_chars(fp_thermo_slow, ctime(&seconds));
            
            result = get_thermo(fp_thermo_slow, 0);
            STOP_ON_ERROR(result);

            // RPM calculation
            get_rpm(fp_voltages_slow, 0);
            // Pressure calculation
            get_pressure(fp_voltages_slow,1,3, 0);

            // RTD data
            //rtd_read(fp_rtd_slow, 0);
        }

        ////////////////////////
        // Get sample for RPM //
        ////////////////////////
        result = read_sample(fp_rpm, 1,2);
        STOP_ON_ERROR(result);
        
        // Sleep for a second
        usleep(delay_between_reads * DELAY_TIME);
    }

stop:
    printf("Cleaning up\n");
    print_error(result);

    fclose(fp_voltages);
    fclose(fp_voltages_slow);
    fclose(fp_thermo);
    fclose(fp_thermo_slow);

    return 0;
}

void print_chars(FILE* fp, char* time_seconds)
{
    for(int i = 8; i < 19; i++)
    {
        // If there's a space, print a ":" instead
        if((int)time_seconds[i] == 32) {
            // Only print the colon if we're not at index 8,
            // because 8 will be a space if hours is less
            // than 10
            if (i != 8) {
                if (fp != NULL)
                    fprintf(fp, ":");
                else
                    printf(":");
            } else {
                if (fp != NULL)
                    fprintf(fp, "0");
                else
                    printf("0");
            }
        }
        else
        {
            if (fp != NULL)
                fprintf(fp, "%c", time_seconds[i]);
            else
                printf("%c", time_seconds[i]);
        }
    }
    if (fp != NULL)
        fprintf(fp, ",");
}
