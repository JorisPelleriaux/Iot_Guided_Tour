RIOTBASE ?= $(CURDIR)/../../
RIOTPROJECT ?= $(CURDIR)/../../../riot-oss7-modem

APPLICATION = user_button
BOARD ?=octa

FEATURES_REQUIRED += periph_gpio_irq

# Modules to include:
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

# include and auto-initialize all available sensors
USEMODULE += saul_default
USEMODULE += xtimer

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

TEST_ON_CI_WHITELIST += all

EXTERNAL_MODULE_DIRS += $(RIOTPROJECT)/drivers/oss7_modem
USEMODULE += oss7_modem

INCLUDES += -I$(RIOTPROJECT)/drivers/oss7_modem/include

include $(RIOTBASE)/Makefile.include

CFLAGS += -DDEBUG_ASSERT_VERBOSE
