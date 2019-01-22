#include "periph/i2c.h"
#include "minmea.h"

#ifndef IOT_GUIDED_TOUR_XM1110_I2C_H
#define IOT_GUIDED_TOUR_XM1110_I2C_H

#define NO_FIX_LAT 51.177327;
#define NO_FIX_LONG 4.416928;

struct XM1110_output_buffer {
    float latitude;
    float longitude;
    bool isValid;
};

void printFailMsg(int failMsg);

void read_sensor(i2c_t DEV, struct XM1110_output_buffer *outputBuffer);

void parseQueue(char *bufferInput);

char *searchNMEAType(char *bufferInput, enum minmea_sentence_id minmea_sentence_id);

#endif //IOT_GUIDED_TOUR_XM1110_I2C_H
