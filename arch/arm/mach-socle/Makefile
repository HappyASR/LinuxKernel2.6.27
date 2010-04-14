#
# Makefile for the linux kernel.
#

# Object file lists.

#obj-y                          := core.o clock.o
obj-y                           := irq.o clock.o devices.o time.o gpio.o dma.o socle_rfkill.o

obj-$(CONFIG_SOCLE_DMA_A2A)		+= socle-a2a.o
obj-$(CONFIG_SOCLE_DMA_HDMA)		+= socle-hdma.o
obj-$(CONFIG_SOCLE_DMA_PANTHER7_HDMA)	+= panther7-hdma.o

obj-$(CONFIG_ARCH_LDK3V21)	+= ldk3v21.o
obj-$(CONFIG_ARCH_LDK5)		+= ldk5.o

obj-$(CONFIG_ARCH_CDK)          += cheetah.o mp-gpio.o cheetah-scu.o socle-pwmt.o
obj-$(CONFIG_ARCH_SCDK)         += cheetah.o mp-gpio.o cheetah-scu.o socle-pwmt.o
obj-$(CONFIG_ARCH_PDK_PC9002)   += cheetah.o mp-gpio.o cheetah-scu.o socle-pwmt.o

obj-$(CONFIG_ARCH_MSMV)   	+= msmv.o mp-gpio.o msmv-scu.o socle-pwmt.o
obj-$(CONFIG_ARCH_PDK_PC9220)   += pc9220.o pc9220-scu.o socle-pwmt.o

obj-$(CONFIG_ARCH_P7DK)         += panther7.o pc7210-scu.o socle-cache.o socle-pwmt.o
obj-$(CONFIG_ARCH_PDK_PC7210)   += panther7.o pc7210-scu.o socle-cache.o pc7210-pm.o socle-pwmt.o 

obj-$(CONFIG_PCI)		+= pci.o
obj-$(CONFIG_PM)		+= pm.o sleep.o
