APPLICATION=shell

BOARD ?= octa

RIOTBASE ?= $(CURDIR)/../..

FEATURES_REQUIRED = periph_i2c

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Add required modules here
USEMODULE += ps
USEMODULE += periph_gpio
USEMODULE += periph_gpio_irq
USEMODULE += lsm303agr
USEMODULE += xtimer
USEMODULE += saul_default

EXTERNAL_MODULE_DIRS += $(RIOTPROJECT)/../../riot-oss7-modem/drivers/oss7_modem
USEMODULE += oss7_modem

INCLUDES += -I$(RIOTPROJECT)/../../riot-oss7-modem/drivers/oss7_modem/include

USEPKG += minmea

include $(RIOTBASE)/Makefile.include

CFLAGS += -DDEBUG_ASSERT_VERBOSE