APPLICATION=shell

BOARD ?= octa

RIOTBASE ?= $(CURDIR)/../../../RIOT

# Add required modules here
USEMODULE += ps
USEMODULE += periph_gpio_irq
USEMODULE += lsm303agr
USEMODULE += xtimer
USEMODULE += saul_default

EXTERNAL_MODULE_DIRS += $(RIOTPROJECT)/../../../riot-oss7-modem/drivers/oss7_modem
USEMODULE += oss7_modem

INCLUDES += -I$(RIOTPROJECT)/../../../riot-oss7-modem/drivers/oss7_modem/include

USEMODULE += communication
INCLUDES += -I$(CURDIR)/communication
DIRS =+ $(CURDIR)/communication

include $(RIOTBASE)/Makefile.include
