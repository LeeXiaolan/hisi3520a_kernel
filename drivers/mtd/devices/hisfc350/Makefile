#
# drivers/devices/hisfc350/Makefile
#

obj-$(CONFIG_MTD_HISFC350)     += hisfc350_spi_ids.o hisfc350.o


ifdef CONFIG_ARCH_PHOENIX

obj-$(CONFIG_MTD_HISFC350)     += hisfc350_phoenix.o

endif

ifdef CONFIG_ARCH_HI3518

obj-$(CONFIG_MTD_HISFC350)     += hisfc350_3518b.o

endif
