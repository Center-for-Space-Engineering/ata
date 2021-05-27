#include "rtd.h"
#include "thermo.h"

#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>

static const char MQ_REQUEST[] = "/mqRequest";
static const char MQ_VALUE[]   = "/mqValue";
mqd_t requestQueue;
mqd_t valueQueue;

void rtd_setup() {
    // Set up message queue for bus pirates
    mq_unlink(MQ_REQUEST);
    mq_unlink(MQ_VALUE);
    requestQueue = mq_open(MQ_REQUEST, O_CREAT | O_RDWR, 0666, 0);
    valueQueue   = mq_open(MQ_VALUE  , O_CREAT | O_RDWR, 0666, 0);
    
    // Spawn process for Python to interface with rtd
    system("python3 bus_pirate.py &");
}

void rtd_read(FILE* fp, uint8_t print) {
    // Static Variable declaration
    static double   bins        [RTD_COUNT][MAX_BIN_SIZE];
    static uint32_t bin_index   [RTD_COUNT];
    static uint32_t can_deviate [RTD_COUNT];

    // Send message to Python to request next sample from RTD
    mq_send(requestQueue, "1", 1, 0);

    // Get responses
    char buffer [5];
    uint16_t value;
    for (uint8_t i = 0; i < 4; ++i) {
        mq_receive(valueQueue, buffer, 5, 0);
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
    mq_send(requestQueue, "0", 1, 0);
}
