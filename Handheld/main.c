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

#define WAKEUP_INTERVAL     (1U * US_PER_SEC)                                           // 1 second (in µs)
#define TIME_TO_SLEEP       (16U * US_PER_SEC)                                            // 16 seconds (in µs)
#define TIME_TO_SEND        (5U * US_PER_SEC)                                              // 5 seconds (in µs)
#define MOTION_INTERVAL     (1u * US_PER_SEC)
#define SLEEP               0
#define ACTIVE              1
#define SEND_LORA           1
#define SEND_D7             0

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * Global variables
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */

static lsm303agr_t dev;                                                             // static = only initialised once
static lsm303agr_3d_data_t acc_value;
static xtimer_ticks32_t last_measured_sleep_time;
static xtimer_ticks32_t last_measured_send_time;
static xtimer_ticks32_t last_measured_interrupt_time;
static int mode;                                                                    
static int send_mode;
static uint32_t time_now;
static kernel_pid_t pid;
static char thread_stack[THREAD_STACKSIZE_MAIN];
i2c_t DEV = I2C_DEV(0);

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * Callbacks
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */

/**
 * @name    Accelerometer callback
 * 
 * @note    Switch between ACTIVE mode and SLEEP mode
 */
static void acc_callback(void *arg)
{
    (void) arg;

    puts("Motion detected");
  
    // Only reset the sleeptimer is two sequent movements are inside an interval of MOTION_INTERVAL
    if (xtimer_less( xtimer_diff(xtimer_now(), last_measured_interrupt_time), xtimer_ticks_from_usec(MOTION_INTERVAL)))
    {
        puts("Switch to active mode");
        last_measured_sleep_time = xtimer_now();
        
    }

    last_measured_interrupt_time = xtimer_now();
}

/**
 * @name    Button callback
 * 
 * @note    Switch between LoRa send_mode and D7 send_mode
 */
static void BTN_callback(void *arg)
{
    (void) arg;

    uint32_t time_start = xtimer_now_usec();

    if (time_start-time_now > 500000)         //ignore bounce of push button for 500ms
    {	
        send_mode ^= 1;    
        
        if (send_mode == SEND_D7)
        {
            puts("Send mode: D7");
        }
        else
        {
            puts("Send mode: LoRa");
        }
    }

    time_now = xtimer_now_usec();

}

/**
 * @name    Thread handler 
 * 
 * @note    Sends the LoRa or D7 message
 */
static void *thread_handler(void *arg)
{
    (void) arg;

    puts(arg);
    
    // Send via LoRa
    if(send_mode == SEND_LORA)
    {
        // ---------------------
        puts("LoRa msg");
        LoRa_send(arg);
        // ---------------------
    }
    // Send via D7
    if(send_mode == SEND_D7)
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

/**
 * @name    Accelerometer measurement
 * 
 * @note    Take a single measurement with the accelerometer
 */
int acc_measurement(void)
{
    lsm303agr_read_acc(&dev, &acc_value);
    printf("Accelerometer x: %i y: %i z: %i\n", acc_value.x_axis,
                                                acc_value.y_axis,
                                                acc_value.z_axis);
    return 0;
}

/**
 * @name    Configure lsm303agr 
 * 
 * @note    Configure the lsm303agr sensor (accelerometer) 
 */
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

/**
 * @name    Configure the button interrupt
 *
 */
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

/**
 * @name    Configure the lsm303agr interrupt
 *
 */
int configuration_lsm303agr_interrupt(void)
{
    // GPIO_IN: input, no pull
    if (gpio_init_int(GPIO_PIN(PORT_B, 13), GPIO_IN , GPIO_RISING, acc_callback, (void*) 0) < 0) 
    {
        puts("[FAILED] init lsm303agr!");
        return 0;
    }

    gpio_irq_enable(GPIO_PIN(PORT_B, 13));

    return 1;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * Main loop
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */
 
int main(void)
{
    //----------Program initialization----------//
    
    mode = ACTIVE;
    send_mode = SEND_D7;
    configuration_lsm303agr();

    // Init interrupt handlers
    configuration_button_interrupt();
    configuration_lsm303agr_interrupt();

    // Modules initializations
    LoRa_D7_init();
    // i2c_init(DEV);

    // Timers
    last_measured_sleep_time = xtimer_now();
    last_measured_send_time = xtimer_now();
    last_measured_interrupt_time = xtimer_now();

    // GPS vars
    // struct XM1110_output_buffer gpsOutputBuffer;
    // float gps_latitude = 51.177327;
    // float gps_longitude = 4.416928;

    // LoRa vars
    // uint8_t lora_payload_length = 8;
    // uint8_t lora_payload[lora_payload_length];

    //D7 vars
    // uint8_t d7_payload_length = 8;
    // uint8_t d7_payload[d7_payload_length];
    
    //---------------Program loop---------------//
    while(1) 
    {
        // Check mode and sleep if (time - last_measured_sleep_time) > TIME_TO_SLEEP
        if ( xtimer_less( xtimer_diff(xtimer_now(), last_measured_sleep_time), xtimer_ticks_from_usec(TIME_TO_SLEEP) ) )
        {
            mode = ACTIVE;

            // Send data if (time - last_measured_send_time) > TIME_TO_SEND
            if ( !( xtimer_less( xtimer_diff(xtimer_now(), last_measured_send_time), xtimer_ticks_from_usec(TIME_TO_SEND) ) ) )
            {    
                // Reset send_time
                last_measured_send_time = xtimer_now();

                if (send_mode == SEND_LORA)
                {
            //     read_sensor(DEV, &gpsOutputBuffer);
            //     if (!gpsOutputBuffer.latitude && !gpsOutputBuffer.longitude) 
            //     {
            //         printf("[GPS] - Coords %f N, %f E\n", gpsOutputBuffer.latitude, gpsOutputBuffer.longitude);
            //         gps_latitude = gpsOutputBuffer.latitude;
            //         gps_longitude = gpsOutputBuffer.longitude;
            //     } 
            //     else
            //     {
            //         printf("[GPS] - No fix - no coord\n");
            //         gps_latitude = 51.177327;
            //         gps_longitude = 4.416928;
            //     }

            //     // Payload formation
            //     int32_t gps_latitude_payload = round(gps_latitude * 1000000);
            //     lora_payload[0] = (gps_latitude_payload & 0xFF000000) >> 24;
            //     lora_payload[1] = (gps_latitude_payload & 0x00FF0000) >> 16;
            //     lora_payload[2] = (gps_latitude_payload & 0x0000FF00) >> 8;
            //     lora_payload[3] = (gps_latitude_payload & 0X000000FF);

            //     int32_t gps_longitude_payload = round(gps_longitude * 1000000);
            //     lora_payload[4] = (gps_longitude_payload & 0xFF000000) >> 24;
            //     lora_payload[5] = (gps_longitude_payload & 0x00FF0000) >> 16;
            //     lora_payload[6] = (gps_longitude_payload & 0x0000FF00) >> 8;
            //     lora_payload[7] = (gps_longitude_payload & 0X000000FF);

                    // thread_create (char *stack, int stacksize, char priority, int flags, thread_task_func_t task_func, void *arg, const char *name)
                    pid = thread_create(thread_stack, sizeof(thread_stack),
                                        THREAD_PRIORITY_MAIN - 1,
                                        0,
                                        thread_handler,
                                        0x00, "send thread");
                }
                else
                {
                    pid = thread_create(thread_stack, sizeof(thread_stack),
                                        THREAD_PRIORITY_MAIN - 1,
                                        0,
                                        thread_handler,
                                        NULL, "send thread");
                }
            }

        }
        else
        {
            if (mode == ACTIVE)
            {
                mode = SLEEP;
                puts("Sleeping");
                //pm_set_lowest();
            }
        }
    }
    return 0;
}

