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

USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
# include and auto-initialize all available sensors
USEMODULE += saul_default
USEMODULE += xtimer
USEMODULE += periph_gpio_irq
USEMODULE += printf_float

EXTERNAL_MODULE_DIRS += $(RIOTBASE)/../riot-oss7-modem/drivers/oss7_modem
USEMODULE += oss7_modem

INCLUDES += -I$(RIOTBASE)/../riot-oss7-modem/drivers/oss7_modem/include

USEPKG += minmea

include $(RIOTBASE)/Makefile.include

CFLAGS += -DDEBUG_ASSERT_VERBOSE