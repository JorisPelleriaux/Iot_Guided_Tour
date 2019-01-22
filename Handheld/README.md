This project needs following files and libraries:

[ ] Board
    Place the octa-directory in the RIOT/boards-directory
    Now you can flash the project with the following command:
    make BOARD=octa -C RIOT/<path_to_project> flash
    

[ ] riot-oss7-modem
    Via GitHub https://github.com/MOSAIC-LoPoW/riot-oss7-modem.git
    The map has to be placed next to the RIOT-students folder
    In the Makefile add following lines:
        EXTERNAL_MODULE_DIRS += $(RIOTPROJECT)/../../riot-oss7-modem/drivers/oss7_modem
        USEMODULE += oss7_modem
        INCLUDES += -I$(RIOTPROJECT)/../../riot-oss7-modem/drivers/oss7_modem/include

[ ] lsm303agr
    This drivermap can be found under Drivers and has to be placed in the RIOT/drivers directory. 
    REMARK: place lms303agr.h in RIOT/drivers/include directory
    Find the following files in RIOT/drivers and add a new section for the lsm303agr driver
        - Makefile.dep
	    - Makefile.include
    As last but not least 
        - add a section  for the driver in sys/auto_init/auto_init.c
        - make a new file in sys/auto_init/saul/auto_init_<driver>.c

[ ] GPS
    In the following file boards\octa\include\periph_conf.h, add the following code in the right section
        static const i2c_conf_t i2c_config[] = {
        {
            .dev            = I2C2,
            .speed          = I2C_SPEED_NORMAL,
            .scl_pin        = GPIO_PIN(PORT_F, 0),
            .sda_pin        = GPIO_PIN(PORT_F, 1),
            .scl_af         = GPIO_AF4,
            .sda_af         = GPIO_AF4,
            .bus            = APB1,
            .rcc_mask       = RCC_APB1ENR1_I2C2EN,
            .irqn           = I2C2_EV_IRQn
        }
        };
    Go to the Things Network and under 'payload formats' in the application tab, add the following code
        function Decoder(bytes, port) {
        var gps_latitude = ((bytes[4]) << 24) + ((bytes[5]) << 16) + ((bytes[6]) << :sunglasses: + ((bytes[7]));
        var gps_longitude = ((bytes[8]) << 24) + ((bytes[9]) << 16) + ((bytes[10]) << :sunglasses: + ((bytes[11]));

        return{
        gps_latitude: gps_latitude/1000000,
        gps_longitude: gps_longitude/1000000
        };
        }