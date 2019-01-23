#include "modem.h"
#include "lora_d7.h"

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * Defines
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */

#define FILE_ID 0x40

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * LoRa and D7 configurations
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */
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
        .qos_resp_mode = SESSION_RESP_MODE_NO,
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
  
static lorawan_session_config_abp_t lorawan_session_config = {
    .appSKey = LORAWAN_APP_SESSION_KEY,
    .nwkSKey = LORAWAN_NETW_SESSION_KEY,
    .devAddr = LORAWAN_DEV_ADDR,
    .request_ack = false,
    .network_id = LORAWAN_NETW_ID,
    .application_port = 1
};

static void* current_interface_config;
static alp_itf_id_t current_interface_id;

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * Communication initialization
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */

int LoRa_D7_init(void)
{
    modem_callbacks_t modem_callbacks = {
        .command_completed_callback = &on_modem_command_completed_callback,
        .return_file_data_callback = &on_modem_return_file_data_callback,
        .write_file_data_callback = &on_modem_write_file_data_callback,
    };

    modem_init(UART_DEV(1), &modem_callbacks);

    uint8_t uid[D7A_FILE_UID_SIZE];
    modem_read_file(D7A_FILE_UID_FILE_ID, 0, D7A_FILE_UID_SIZE, uid);
    //printf("modem UID: %02X%02X%02X%02X%02X%02X%02X%02X\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6], uid[7]);

    current_interface_id = ALP_ITF_ID_D7ASP;
    current_interface_config = (void*)&d7_session_config;

    return 1;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * DASH7 send
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */

int D7_send(uint8_t* payload, uint32_t payloadLength)
{
    current_interface_id = ALP_ITF_ID_D7ASP;
    current_interface_config = &d7_session_config;

    modem_status_t status = modem_send_unsolicited_response(FILE_ID, 0, payloadLength, payload, current_interface_id, current_interface_config);

    if(status == MODEM_STATUS_COMMAND_COMPLETED_SUCCESS) {
        printf("Command completed successfully\n");
    } else if(status == MODEM_STATUS_COMMAND_COMPLETED_ERROR) {
        printf("Command completed with error\n");
    } else if(status == MODEM_STATUS_COMMAND_TIMEOUT) {
        printf("Command timed out\n");
    }

    return 1;
}

/* -------------------------------------------------------------------------------------------------------------------------------------------
 * LoRa send
 * -------------------------------------------------------------------------------------------------------------------------------------------
 */

int LoRa_send(uint8_t* payload, uint32_t payloadLength)
{
    current_interface_id = ALP_ITF_ID_LORAWAN_ABP;
    current_interface_config = &lorawan_session_config;
    
    // modem_status_t modem_send_unsolicited_response(uint8_t file_id, uint32_t offset, uint32_t length, uint8_t* data, alp_itf_id_t itf, void* interface_config);
    modem_status_t status = modem_send_unsolicited_response(FILE_ID, 0, payloadLength, payload, current_interface_id, current_interface_config);

    if(status == MODEM_STATUS_COMMAND_COMPLETED_SUCCESS) {
        printf("Command completed successfully\n");
    } else if(status == MODEM_STATUS_COMMAND_COMPLETED_ERROR) {
        printf("Command completed with error\n");
    } else if(status == MODEM_STATUS_COMMAND_TIMEOUT) {
        printf("Command timed out\n");
    }

    return 1;
}


