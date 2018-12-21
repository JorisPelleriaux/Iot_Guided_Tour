#include "periph/i2c.h"

#ifndef IOT_GUIDED_TOUR_XM1110_I2C_H
#define IOT_GUIDED_TOUR_XM1110_I2C_H

void printFailMsg(int failMsg);
void read_sensor(i2c_t DEV);
void parseQueue(char *bufferInput);
void decodeNMEA(char* sentence);

#endif //IOT_GUIDED_TOUR_XM1110_I2C_H
