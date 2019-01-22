#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "XM1110_I2C.h"
#include "periph/i2c.h"
#include "xtimer.h"
#include "modem.h"
#include "thread.h"
#include "shell_commands.h"
#include "errors.h"

i2c_t DEV = I2C_DEV(1);

#define INTERVAL (20U * US_PER_SEC)

#define LORAWAN_NETW_SESSION_KEY  { 0xF3, 0x16, 0xF6, 0xC0, 0xEE, 0xA3, 0xA2, 0xC5, 0x76, 0x0B, 0x2D, 0x40, 0xEB, 0x4E, 0x3E, 0x8B }
#define LORAWAN_APP_SESSION_KEY  { 0xEB, 0xFD, 0x83, 0x12, 0x1F, 0x62, 0x09, 0x95, 0x0C, 0x2D, 0x2F, 0xD3, 0xD9, 0x92, 0x8D, 0xF0 }
#define LORAWAN_DEV_ADDR 0x2601192D
#define LORAWAN_NETW_ID 0x000000

void on_modem_command_completed_callback(bool with_error)
{
    printf("[LoRa] - modem command completed (success = %i)\n", !with_error);
}

void on_modem_return_file_data_callback(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* output_buffer)
{
    printf("[LoRa] - modem return file data file %i offset %li size %li buffer %p\n", file_id, offset, size, output_buffer);
}

void on_modem_write_file_data_callback(uint8_t file_id, uint32_t offset, uint32_t size, uint8_t* output_buffer)
{
    printf("[LoRa] - modem write file data file %i offset %li size %li buffer %p\n", file_id, offset, size, output_buffer);
}

static lorawan_session_config_abp_t lorawan_session_config = {
        .appSKey = LORAWAN_APP_SESSION_KEY,
        .nwkSKey = LORAWAN_NETW_SESSION_KEY,
        .devAddr = LORAWAN_DEV_ADDR,
        .request_ack = false,
        .network_id = LORAWAN_NETW_ID,
        .application_port = 1
};

int main(void)
{
    /////////////////////
    // Init
    /////////////////////
    puts("Welcome to RIOT!");
    i2c_init(DEV);

    modem_callbacks_t modem_callbacks = {
            .command_completed_callback = &on_modem_command_completed_callback,
            .return_file_data_callback = &on_modem_return_file_data_callback,
            .write_file_data_callback = &on_modem_write_file_data_callback,
    };

    modem_init(UART_DEV(1), &modem_callbacks);

    xtimer_ticks32_t last_wakeup = xtimer_now();
    alp_itf_id_t current_interface_id = ALP_ITF_ID_LORAWAN_ABP;
    void* current_interface_config = (void*)&lorawan_session_config;

    struct XM1110_output_buffer gpsOutputBuffer;
    uint8_t counter = 0;

    uint8_t payload_length = 8;
    uint8_t payload[payload_length];
    float gps_latitude;
    float gps_longitude;


    while(1) {
//        printf("[LoRa] - Sending msg with counter %i\n", counter);
//        uint32_t start = xtimer_now_usec();

        /////////////////////
        // Read the GPS sensor
        /////////////////////
        read_sensor(DEV, &gpsOutputBuffer);
        if (!gpsOutputBuffer.latitude && !gpsOutputBuffer.longitude) {
            printf("[GPS] - Coords %f N, %f E\n", gpsOutputBuffer.latitude, gpsOutputBuffer.longitude);
            gps_latitude = gpsOutputBuffer.latitude;
            gps_longitude = gpsOutputBuffer.longitude;
        } else {
            printf("[GPS] - No fix - no coord --- %f N, %f E\n", gpsOutputBuffer.latitude, gpsOutputBuffer.longitude);
            gps_latitude = 51.177327;
            gps_longitude = -4.416928;
        }

        /////////////////////
        // Payload formation
        /////////////////////
        int32_t gps_latitude_payload = round(gps_latitude * 1000000);
        payload[0] = (gps_latitude_payload & 0xFF000000) >> 24;
        payload[1] = (gps_latitude_payload & 0x00FF0000) >> 16;
        payload[2] = (gps_latitude_payload & 0x0000FF00) >> 8;
        payload[3] = (gps_latitude_payload & 0X000000FF);

        int32_t gps_longitude_payload = round(gps_longitude * 1000000);
        payload[4] = (gps_longitude_payload & 0xFF000000) >> 24;
        payload[5] = (gps_longitude_payload & 0x00FF0000) >> 16;
        payload[6] = (gps_longitude_payload & 0x0000FF00) >> 8;
        payload[7] = (gps_longitude_payload & 0X000000FF);

        /////////////////////
        // Data transmission
        /////////////////////
//        modem_status_t status =
        modem_send_unsolicited_response(0x40, 0, payload_length, payload, current_interface_id, current_interface_config);
//        uint32_t duration_usec = xtimer_now_usec() - start;
//        printf("[LoRa] - Command completed in %li ms\n", duration_usec / 1000);
//        if(status == MODEM_STATUS_COMMAND_COMPLETED_SUCCESS) {
//            printf("[LoRa] - Command completed successfully\n");
//        } else if(status == MODEM_STATUS_COMMAND_COMPLETED_ERROR) {
//            printf("[LoRa] - Command completed with error\n");
//        } else if(status == MODEM_STATUS_COMMAND_TIMEOUT) {
//            printf("[LoRa] - Command timed out\n");
//        }

        counter++;
        xtimer_periodic_wakeup(&last_wakeup, INTERVAL);
        printf("[Sys] - slept until %" PRIu32 "\n", xtimer_usec_from_ticks(xtimer_now()));
    }

    return 0;
}