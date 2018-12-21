#include <stdio.h>
#include <stdlib.h>

#include "XM1110_I2C.h"
#include "periph/i2c.h"
#include "xtimer.h"


i2c_t DEV = I2C_DEV(0);

int main(void) {
    printf("Read the XM1110 positioning sensor via I2C\n");
    i2c_init(DEV);
    printf("Init completed\n");


    for (size_t readCount = 0; readCount < 3000; readCount++) {
        printf("----------------------------------------------\n");
        read_sensor(DEV);
        printf("----------------------------------------------\n");
        xtimer_usleep(2000000);
    }
}

