#include "XM1110_I2C.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "board.h"

const uint16_t DEVICE_ADDR = 0x10;
const uint16_t FLAGS = 0;
const uint8_t REG_READ_ADDR = 0x21;

void printFailMsg(int failMsg) {
    if (failMsg == -EOPNOTSUPP) {
        printf("Error: EOPNOTSUPP [%d]\n", -failMsg);
        // return 1;
    } else if (failMsg == -EINVAL) {
        printf("Error: EINVAL [%d]\n", -failMsg);
        // return 1;
    } else if (failMsg == -EAGAIN) {
        printf("Error: EAGAIN [%d]\n", -failMsg);
        // return 1;
    } else if (failMsg == -ENXIO) {
        printf("Error: ENXIO [%d]\n", -failMsg);
        // return 1;
    } else if (failMsg == -EIO) {
        printf("Error: EIO [%d]\n", -failMsg);
        // return 1;
    } else if (failMsg == -ETIMEDOUT) {
        printf("Error: ETIMEDOUT [%d]\n", -failMsg);
        // return 1;
    } else {
        printf("Error: Unknown error [%d]\n", failMsg);
    }
}

/**
 * Read the sensor buffer and return the RMC data if found
 *
 * @param DEV i2c handle
 * @param *outputBuffer pointer to the output buffer
 */
void read_sensor(i2c_t DEV, struct XM1110_output_buffer *outputBuffer) {
    i2c_acquire(DEV);

    char i2cBuffer[255];
    for (int byteCount = 0; byteCount < 255; byteCount++) {
        int failMsg = i2c_read_reg(DEV, DEVICE_ADDR, REG_READ_ADDR, &i2cBuffer[byteCount], FLAGS);
        if (failMsg != 0) {
            printFailMsg(failMsg);
        }
    }
    i2c_release(DEV);

//    printf("[GPS] full buffer: %s\n", i2cBuffer);

    char *sentence = searchNMEAType(i2cBuffer, MINMEA_SENTENCE_RMC);
//    printf("[GPS] - Sentence=%s\n", sentence);
    struct minmea_sentence_rmc frame;
    if (strcmp(sentence, "-1") == 0) {
//        printf("GPS -- NO FIX");
        // No fix from the GPS module => send dummy data
        outputBuffer->latitude = NO_FIX_LAT;
        outputBuffer->longitude = NO_FIX_LONG;
        outputBuffer->isValid = false;
    } else {
//        printf("GPS -- FIX");
        // Fix from GPS module => send actual data
        minmea_parse_rmc(&frame, sentence);
        outputBuffer->latitude = minmea_tocoord(&frame.latitude);
        outputBuffer->longitude = minmea_tocoord(&frame.longitude);
        outputBuffer->isValid = true;
//        printf("$RMC floating point degree coordinates and speed: (%f N, %f E) %f\n",
//               minmea_tocoord(&frame.latitude),
//               minmea_tocoord(&frame.longitude),
//               minmea_tofloat(&frame.speed));
    }
}

/**
 * Decodes a single NMEA sentence and prints it to terminal [DEBUG FUNCTION]
 */
void decodeNMEA(char *sentence) {
    // Decode single NMEA senteces
    switch (minmea_sentence_id(sentence, false)) {
        case MINMEA_SENTENCE_RMC: {
            struct minmea_sentence_rmc frame;
            if (minmea_parse_rmc(&frame, sentence)) {
                printf("$RMC floating point degree coordinates and speed: (%f N, %f E) %f\n",
                       minmea_tocoord(&frame.latitude),
                       minmea_tocoord(&frame.longitude),
                       minmea_tofloat(&frame.speed));
            }
        }
            break;

        case MINMEA_SENTENCE_GGA: {
            struct minmea_sentence_gga frame;
            if (minmea_parse_gga(&frame, sentence)) {
                printf("$GGA: fix quality: %d\n", frame.fix_quality);
            }
        }
            break;

        case MINMEA_SENTENCE_GSV: {
            struct minmea_sentence_gsv frame;
            if (minmea_parse_gsv(&frame, sentence)) {
                printf("$GSV: message %d of %d\n", frame.msg_nr, frame.total_msgs);
                printf("$GSV: sattelites in view: %d\n", frame.total_sats);
                for (int i = 0; i < 4; i++)
                    printf("$GSV: sat nr %d, elevation: %d, azimuth: %d, snr: %d dbm\n",
                           frame.sats[i].nr,
                           frame.sats[i].elevation,
                           frame.sats[i].azimuth,
                           frame.sats[i].snr);
            }
        }
            break;

        case MINMEA_SENTENCE_GSA:
        case MINMEA_SENTENCE_GLL:
        case MINMEA_SENTENCE_GST:
        case MINMEA_SENTENCE_VTG:
        case MINMEA_SENTENCE_ZDA: {
//            printf("Sentence not implemented\n");
        }
            break;

        default: {
//            printf("/[][][][][][][][][][][]\n");
//            printf("Unvalid sentence\n");
//            printf("%d\n", minmea_sentence_id(sentence, false));
//            printf("Sentence=%s\n", sentence);
//            printf("[][][][][][][][][][][]/\n");
        }
    }
}

/**
 * Takes the i2c buffer as an input and returns the first valid NMEA sentence that fits the type specified
 *
 * @param bufferInput
 * @param minmea_sentence_id
 * @return the sentence, -1 if no valid sentence is found
 */
char *searchNMEAType(char *bufferInput, enum minmea_sentence_id sentence_id) {
//    printf("Started queue parser\n");
    const char DELIM[2] = "\n";                                     // The string to split each sentence on
    char *token;
    token = strtok(bufferInput, DELIM);                             // Split the message up in sentences (tokens)

    while (token != NULL) {                                         // Keep processing as long as there are tokens left
        char *sentence = (char *) calloc(strlen(token), sizeof(char));
        strncpy(sentence, token, strlen(token) - 1);
//        printf("[GPS] token lengths %d vs %d\n", strlen(token), strlen(sentence));
//        printf("[GPS] sentence %s\n", sentence);
//        printf("[GPS] sentence check %d\n", minmea_sentence_id(sentence, false));
        if (minmea_sentence_id(sentence, true) == sentence_id) {
            return sentence;                                        // Return the valid sentence
        } else {
            token = strtok(NULL, DELIM);                            // Continue processing
        }
    }

    return "-1";                                                    // Return -1 if nothing found
}