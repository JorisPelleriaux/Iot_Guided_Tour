#include <stdio.h>
#include <stdint.h>

#include "board.h"
#include "periph/gpio.h"
#include "periph_conf.h"
#include "thread.h"
#include "shell.h"
#include "shell_commands.h"
#include "xtimer.h"
#include "errors.h"
#include "thread.h"
#include "modem.h"

#define TEST_FLANK      GPIO_RISING
#ifdef BTN0_PIN /* assuming that first button is always BTN0 */

//#define LED_RED            GPIO_PIN(PORT_D, 14)
//#define LED_GREEN          GPIO_PIN(PORT_B, 0)

static kernel_pid_t pid;
static char stack[THREAD_STACKSIZE_MAIN];
uint8_t counter = 0;
uint32_t now;

void on_modem_command_completed_callback(bool with_error)
{
    printf("modem command completed (success = %i)\n", !with_error);
}

void on_modem_return_file_data_callback(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* output_buffer)
{
    printf("modem return file data file %i offset %li size %li buffer %p\n", file_id, offset, size, output_buffer);
}

void on_modem_write_file_data_callback(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* output_buffer)
{
    printf("modem write file data file %i offset %li size %li buffer %p\n", file_id, offset, size, output_buffer);
}

static d7ap_session_config_t d7_session_config = {
    .qos = {
        .qos_resp_mode = SESSION_RESP_MODE_PREFERRED,
        .qos_retry_mode = SESSION_RETRY_MODE_NO
    },
    .dormant_timeout = 0,
    .addressee = {
        .ctrl = {
            .nls_method = AES_NONE,
            .id_type = ID_TYPE_NOID
        },
        .access_class = 0x01,
        .id = {0},
    },
};

static void *thread_handler(void *arg)
{    
	(void) arg;
	counter++;
    modem_status_t status = modem_send_unsolicited_response(0x33, 0, 1, &counter, ALP_ITF_ID_D7ASP, &d7_session_config);

    if(status == MODEM_STATUS_COMMAND_COMPLETED_SUCCESS) {
        printf("Command completed successfully\n");
		LED1_TOGGLE;
    } else if(status == MODEM_STATUS_COMMAND_COMPLETED_ERROR) {
        printf("Command completed with error\n");
		counter--;
		LED0_TOGGLE;
    } else if(status == MODEM_STATUS_COMMAND_TIMEOUT) {
        printf("Command timed out\n");
		counter--;
		LED0_TOGGLE;
    }

    return NULL;
}

//Callback of the button
static void cb(void *arg)
{
    uint32_t start = xtimer_now_usec();

    if (start-now > 500000){	//ignore bounce of push button for 500ms
		printf("Pressed BTN%d\n", (int)arg);

		//turn leds off
		LED0_OFF;
	    LED1_OFF;
		
        pid = thread_create(stack, sizeof(stack),
                            THREAD_PRIORITY_MAIN - 1,
                            0,
                            thread_handler,
                            NULL, "send thread");
    }
    now = xtimer_now_usec();
}
#endif

int main(void)
{
    modem_callbacks_t modem_callbacks = {
        .command_completed_callback = &on_modem_command_completed_callback,
        .return_file_data_callback = &on_modem_return_file_data_callback,
        .write_file_data_callback = &on_modem_write_file_data_callback,
    };

    modem_init(UART_DEV(1), &modem_callbacks);

    uint8_t uid[D7A_FILE_UID_SIZE];
    modem_read_file(D7A_FILE_UID_FILE_ID, 0, D7A_FILE_UID_SIZE, uid);
    
    /* init interrupt handler */
    #ifdef BTN0_PIN
    if (gpio_init_int(BTN0_PIN, BTN0_MODE , TEST_FLANK, cb, NULL) < 0) {
        puts("[FAILED] init BTN0!");
        return 1;
    }
    #endif
    
    puts("On-board button test\n");
    /* cppcheck-suppress knownConditionTrueFalse
     * rationale: board-dependent ifdefs */
    puts(" -- Try pressing buttons to test.\n");
    puts("[SUCCESS]");
    return 0;
}
