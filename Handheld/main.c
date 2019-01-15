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
#include "communication/lora_d7.h"

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

#define TEST_FLANK      GPIO_RISING
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

#ifdef BTN0_PIN /* assuming that first button is always BTN0 */
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
#endif

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * Main loop
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */
int main(void)
{
    //----------Program initialization----------//
    if(lsm303agr_init(&dev, &lsm303agr_params[0]) != 0)
    {
        puts("Init not completed");
    }
    if(lsm303agr_enable_interrupt(&dev) != 0)
    {
        puts("Interrupt not enabled");
    }
    
    // Init interrupt handlers
    gpio_init_int(INT_1, GPIO_IN , GPIO_RISING, acc_callback, (void*) 0);
    gpio_irq_enable(INT_1);

    #ifdef BTN0_PIN
    int cnt = 0;
    if (gpio_init_int(BTN0_PIN, BTN0_MODE , TEST_FLANK, BTN_callback, (void *)cnt) < 0) {
        puts("[FAILED] init BTN0!");
        return 1;
    }
    #endif

    LoRa_D7_init();

    //xtimer_ticks32_t last_wakeup = xtimer_now();
    last_measured_sleep_time = xtimer_now();
    last_measured_send_time = xtimer_now();
    last_measured_interrupt_time = xtimer_now();
        
    mode = 1;
    send_mode = 0;

    uint8_t payload[] = {0x00,0x01};
    
    //---------------Program loop---------------//
    while(1) {
        //xtimer_periodic_wakeup(&last_wakeup, WAKEUP_INTERVAL);
        //printf("slept until %" PRIu32 "\n", xtimer_usec_from_ticks(xtimer_now()));
        //acc_measurement();

        if ( !( xtimer_less( xtimer_diff(xtimer_now(), last_measured_sleep_time), xtimer_ticks_from_usec(TIME_TO_SLEEP) ) ) )
        {
            // Go to sleep if: (time - last_measured_sleep_time) > TIME_TO_SLEEP
            mode = 0;
        }
        else
        {
            mode = 1;
        }

        if ( !( xtimer_less( xtimer_diff(xtimer_now(), last_measured_send_time), xtimer_ticks_from_usec(TIME_TO_SEND) ) ) )
        {    
            last_measured_send_time = xtimer_now();
            if (mode)
            {
                // Send via LoRa
                    // ---------------------
                    LoRa_send(payload);
                    // ---------------------

                // Send via D7
                    // ---------------------
                    //D7_send(uint8_t payload)
                    // ---------------------
            }
        }
    }
    return 0;
}

