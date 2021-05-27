#include "rtd.h"
#include "thermo.h"

#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

static const char MQ_NAME[] = "/mqATA";
mqd_t rtdQueue;

void rtd_setup() {
    // Set up message queue for bus pirates
    mq_unlink(MQ_NAME);
    //rtdQueue = mq_open(MQ_NAME, O_CREAT | O_RDRW, 0666, 0);
    
    // Spawn process for Python to interface with rtd
    system("python3 bus_pirate.py &");
}

void rtd_read(FILE* fp, uint8_t print) {
    // Static Variable declaration
    static double   bins        [RTD_COUNT][MAX_BIN_SIZE];
    static uint32_t bin_index   [RTD_COUNT];
    static uint32_t can_deviate [RTD_COUNT];

    // Send message to Python to request next sample from RTD
    mq_send(rtdQueue, "1", 1, 0);

    // Get responses
    char buffer [5];
    uint16_t value;
    for (uint8_t i = 0; i < 4; ++i) {
        mq_receive(rtdQueue, buffer, 5, 0);
        sscanf(buffer, "%d", &value);


        // Write to log file
        //fprintf(fp, "%3.2f,", value);
        fprintf(fp, "%d,", value);

        if (print) {
            //printf("%3.2f |",value);
            printf("%d,", value);
        }
    }
}

void rtd_close() {
    mq_send(rtdQueue, "0", 1, 0);
}
