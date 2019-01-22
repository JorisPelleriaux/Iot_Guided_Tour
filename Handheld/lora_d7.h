

/**
 * @brief   Initialize the loar and d7 protocol
 *
 * @param[in] dev           device descriptor of an LSM303agr device
 * @param[in] params        initialization parameters
 *
 * @return                  0 on success
 * @return                  -1 on error
 */
int LoRa_D7_init(void);

/**
 * @brief   Send D7 message
 *
 * @param[in] dev           device descriptor of an LSM303agr device
 * @param[in] params        initialization parameters
 *
 * @return                  0 on success
 * @return                  -1 on error
 */
int D7_send(uint8_t* payload, uint32_t payloadLength);

/**
 * @brief   Send LoRa message
 *
 * @param[in] dev           device descriptor of an LSM303agr device
 * @param[in] params        initialization parameters
 *
 * @return                  0 on success
 * @return                  -1 on error
 */
int LoRa_send(uint8_t* payload, uint32_t payloadLength);
