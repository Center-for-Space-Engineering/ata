/*
    This file contains functions used in the MCC DAQ HAT C examples
    to assist in displaying information, reading user inputs and
    handling errors.
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <daqhats/daqhats.h>

// Macros to handle error checking
#define STOP_ON_ERROR(result)\
{\
    if (result != RESULT_SUCCESS ){\
        print_error(result);\
        goto stop;\
    }\
}

// Channel definitions
#define CHAN0 0x01 << 0
#define CHAN1 0x01 << 1
#define CHAN2 0x01 << 2
#define CHAN3 0x01 << 3
#define CHAN4 0x01 << 4
#define CHAN5 0x01 << 5
#define CHAN6 0x01 << 6
#define CHAN7 0x01 << 7
#define MAX_CHAN_ARRAY_LENGTH 32

// Timeout definitions
#define WAIT_INDEFINITELY   -1
#define RETURN_IMMEDIATELY  0

// Read definitions
#define READ_ALL_AVAILABLE  -1

/****************************************************************************
 * Conversion functions
 ****************************************************************************/

/* This function converts the mask of options defined by the options parameter
   and sets the options_str parameter, which is passed by reference, to a
   comma separated string respresentation of the options. */
void convert_options_to_string(uint32_t options, char* options_str);

/* This function converts the trigger mode defined by the trigger_mode
   parameter to a string representation and returns the string
   respresentation of the trigger mode. */
void convert_trigger_mode_to_string(uint8_t trigger_mode, char* trigger_mode_str);

/* This function converts the thermocouple type defined by the tc_type
   parameter to a string representation and returns the string
   respresentation. */
void convert_tc_type_to_string(uint8_t tc_type, char* tc_type_str);


/* This function converts the mask of channels defined by the channel_mask
   parameter and sets the chans_str parameter, which is passed by reference,
   to a comma separated string respresentation of the channel numbers. */
void convert_chan_mask_to_string(uint32_t channel_mask, char* chans_str);

/* This function converts the mask of channels defined by the channel_mask
   parameter and sets the chans parameter, which is passed by reference,
   to an array of channel numbers.
   The return value is an integer representing the number of channels. */
int convert_chan_mask_to_array(uint32_t channel_mask, int chans[]);

/****************************************************************************
 * Display functions
 ****************************************************************************/
/* This function takes a result code as the result parameter and if the
   result code is not RESULT_SUCCESS, the error message is sent to stderr. */
void print_error(int result);

void resetCursor();
void clearEOL();
void cursorUp();

/****************************************************************************
 * User input functions
 ****************************************************************************/
void flush_stdin(void);

int enter_press();

/* This function displays the available DAQ HAT devices and allows the user
   to select a device to use with the associated example.  The address
   parameter, which is passed by reference, is set to the selected address.
   The return value is 0 for success and -1 for error.*/
int select_hat_device(uint16_t hat_filter_id, uint8_t* address);

#endif /* UTILITY_H_ */
