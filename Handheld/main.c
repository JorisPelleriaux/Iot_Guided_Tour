/* -------------------------------------------------------------------------------------------------------------------------------------------
 * Includes
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "xtimer.h"
#include "timex.h"
#include "thread.h"
#include "periph/gpio.h"
#include "periph_conf.h"
#include "periph/pm.h"
#include "lora_d7.h"
#include "thread.h"
#include "XM1110_I2C.h"

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * Drivers
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */

#include "lsm303agr.h"
#include "lsm303agr_params.h"

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * Definers
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */

#define WAKEUP_INTERVAL (1U * US_PER_SEC)                                           // 1 second (in µs)
#define TIME_TO_SLEEP (16U * US_PER_SEC)                                            // 16 seconds (in µs)
#define TIME_TO_SEND (5U * US_PER_SEC)                                              // 5 seconds (in µs)
#define MOTION_INTERVAL (1u * US_PER_SEC)

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * Global variables
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */
static lsm303agr_t dev;                                                             // static = only initialised once
static lsm303agr_3d_data_t acc_value;
static xtimer_ticks32_t last_measured_sleep_time;
static xtimer_ticks32_t last_measured_send_time;
static xtimer_ticks32_t last_measured_interrupt_time;
static int mode;                                                                    // 0 = sleep, 1 = normal mode
static int send_mode;                                                               // 0 = D7, 1 = LoRa
static uint32_t time_now;
static kernel_pid_t pid;
static char thread_stack[THREAD_STACKSIZE_MAIN];
i2c_t DEV = I2C_DEV(0);

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * Callbacks
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */

static void acc_callback(void *arg)
{
    //int8_t res;
    if (arg == NULL)
    {

    }
    puts("Motion detected");
    //lsm303agr_clear_int(&dev, &res);

    if (xtimer_less( xtimer_diff(xtimer_now(), last_measured_interrupt_time), xtimer_ticks_from_usec(MOTION_INTERVAL)))
    {
        puts("Switch to active mode");
        // Only reset the sleeptimer is two sequent movementq are inside an interval of MOTION_INTERVAL
        last_measured_sleep_time = xtimer_now();
        
    }

    last_measured_interrupt_time = xtimer_now();
}

static void BTN_callback(void *arg)
{
    uint32_t time_start = xtimer_now_usec();

    if (time_start-time_now > 500000)         //ignore bounce of push button for 500ms
    {	
    	printf("Pressed BTN%d\n", (int)arg);
        send_mode ^= 1;
    }
    time_now = xtimer_now_usec();
}

static void *thread_handler(void *arg)
{
    (void) arg;
//    uint8_t payload[] = {0x00,0x01};
    // Send via LoRa
    if(send_mode)
    {
        // ---------------------
        puts("LoRa msg");
        LoRa_send(arg);
        // ---------------------
    }
    // Send via D7
    if(!send_mode)
    {
        // ---------------------
        puts("D7 msg");
        D7_send(arg);
        // ---------------------
    }

    return NULL;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * Functions
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */

int acc_measurement(void)
{
    lsm303agr_read_acc(&dev, &acc_value);
    printf("Accelerometer x: %i y: %i z: %i\n", acc_value.x_axis,
                                                acc_value.y_axis,
                                                acc_value.z_axis);
    return 0;
}

void configuration_lsm303agr(void)
{
    if(lsm303agr_init(&dev, &lsm303agr_params[0]) != 0)
    {
        puts("Init not completed");
    }
    if(lsm303agr_enable_interrupt_1(&dev) != 0)
    {
        puts("Interrupt not enabled");
    }

}

int configuration_button_interrupt(void)
{
    // Check param boards\octa\include\board.h

    // BTN1_MODE: input, pull-down
    if (gpio_init_int(GPIO_PIN(PORT_G, 0), GPIO_IN , GPIO_RISING, BTN_callback, (void *) 0) < 0) 
    {
        puts("[FAILED] init BTN1!");
        return 0;
    }
    gpio_irq_enable(GPIO_PIN(PORT_G, 0));
    return 1;

}

void configuration_lsm303agr_interrupt(void)
{
    // GPIO_IN: input, no pull
    gpio_init_int(GPIO_PIN(PORT_B, 13), GPIO_IN , GPIO_RISING, acc_callback, (void*) 0);
    gpio_irq_enable(GPIO_PIN(PORT_B, 13));

}

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * Main loop
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */
 
int main(void)
{
    //----------Program initialization----------//
    
    configuration_lsm303agr();

    // Init interrupt handlers
    configuration_button_interrupt();
    configuration_lsm303agr_interrupt();

    LoRa_D7_init();
    i2c_init(DEV);

    //xtimer_ticks32_t last_wakeup = xtimer_now();
    last_measured_sleep_time = xtimer_now();
    last_measured_send_time = xtimer_now();
    last_measured_interrupt_time = xtimer_now();
        
    mode = 1;
    send_mode = 0;
    // GPS vars
    struct XM1110_output_buffer gpsOutputBuffer;
    uint8_t lora_payload_length = 8;
    uint8_t lora_payload[lora_payload_length];
    float gps_latitude = 51.177327;
    float gps_longitude = 4.416928;

    //D7 vars
    //todo change vars as needed
    uint8_t d7_payload_length = 8;
    uint8_t d7_payload[d7_payload_length];
    
    //---------------Program loop---------------//
    while(1) 
    {
        /////////////////////
        // Read the GPS sensor
        /////////////////////
        read_sensor(DEV, &gpsOutputBuffer);
        if (!gpsOutputBuffer.latitude && !gpsOutputBuffer.longitude) {
            printf("[GPS] - Coords %f N, %f E\n", gpsOutputBuffer.latitude, gpsOutputBuffer.longitude);
            gps_latitude = gpsOutputBuffer.latitude;
            gps_longitude = gpsOutputBuffer.longitude;
        } else {
            printf("[GPS] - No fix - no coord\n");
            gps_latitude = 51.177327;
            gps_longitude = 4.416928;
        }

        /////////////////////
        // Payload formation
        /////////////////////
        uint32_t gps_latitude_payload = round(gps_latitude * 1000000);
        lora_payload[0] = (gps_latitude_payload & 0xFF000000) >> 24;
        lora_payload[1] = (gps_latitude_payload & 0x00FF0000) >> 16;
        lora_payload[2] = (gps_latitude_payload & 0x0000FF00) >> 8;
        lora_payload[3] = (gps_latitude_payload & 0X000000FF);

        uint32_t gps_longitude_payload = round(gps_longitude * 1000000);
        lora_payload[4] = (gps_longitude_payload & 0xFF000000) >> 24;
        lora_payload[5] = (gps_longitude_payload & 0x00FF0000) >> 16;
        lora_payload[6] = (gps_longitude_payload & 0x0000FF00) >> 8;
        lora_payload[7] = (gps_longitude_payload & 0X000000FF);

        // Go to sleep if: (time - last_measured_sleep_time) > TIME_TO_SLEEP
        if ( !( xtimer_less( xtimer_diff(xtimer_now(), last_measured_sleep_time), xtimer_ticks_from_usec(TIME_TO_SLEEP) ) ) )
        {
            mode = 0;
        }
        else
        {
            mode = 1;
        }

        // Send if: (time - last_measured_send_time) > TIME_TO_SEND
        if ( !( xtimer_less( xtimer_diff(xtimer_now(), last_measured_send_time), xtimer_ticks_from_usec(TIME_TO_SEND) ) ) )
        {    
            last_measured_send_time = xtimer_now();
            if (mode)
            {
                // thread_create (char *stack, int stacksize, char priority, int flags, thread_task_func_t task_func, void *arg, const char *name)
                if (send_mode == 1){
                    pid = thread_create(thread_stack, sizeof(thread_stack),
                                    THREAD_PRIORITY_MAIN - 1,
                                    0,
                                    thread_handler,
                                    lora_payload, "send thread");
                }else{
                    pid = thread_create(thread_stack, sizeof(thread_stack),
                                    THREAD_PRIORITY_MAIN - 1,
                                    0,
                                    thread_handler,
                                    d7_payload, "send thread");
                }

            }
            if (!mode)
            {
                puts("sleeping");
            }
        }
    }
    return 0;
}

