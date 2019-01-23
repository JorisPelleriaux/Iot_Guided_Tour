This project needs following files and libraries:

[ ] Board
    Place the octa-directory in the RIOT/boards-directory
    
[ ] riot-oss7-modem
    Via GitHub https://github.com/MOSAIC-LoPoW/riot-oss7-modem.git
    The map has to be placed NEXT to the RIOT-students folder

[ ] pyd7a
    Via GitHub https://github.com/MOSAIC-LoPoW/pyd7a.git
    The map has to be placed IN the RIOT-students folder

[ ] lsm303agr
    This drivermap can be found under Drivers and has to be placed in the RIOT/drivers directory. 
    REMARK: place lms303agr.h in RIOT/drivers/include directory
    Find the following files in RIOT/drivers and add a new section for the lsm303agr driver
        - Makefile.dep
	    - Makefile.include
    As last but not least 
        - add a section for the driver in sys/auto_init/auto_init.c
        - make a new file in sys/auto_init/saul/auto_init_<driver>.c

[ ] GPS
    Go to the Things Network and under 'payload formats' in the application tab, add the following code
        function Decoder(bytes, port) {
        var gps_latitude = ((bytes[4]) << 24) + ((bytes[5]) << 16) + ((bytes[6]) << :sunglasses: + ((bytes[7]));
        var gps_longitude = ((bytes[8]) << 24) + ((bytes[9]) << 16) + ((bytes[10]) << :sunglasses: + ((bytes[11]));

        return{
        gps_latitude: gps_latitude/1000000,
        gps_longitude: gps_longitude/1000000
        };
        }

[ ] Keys
    Before you can connenct with the things network, the keys.h-file needs to be filled with valid keys. You can find them on the things network.

[ ] Flash
    Now you can flash the project with the following command:
    make BOARD=octa -C RIOT/<path_to_project> flash