# ADS8681-Interface-Beaglebone-Black-in-C


You need to enable the pins with uBoot overlays:
You need to edit /boot/uEnv.txt:

Change these two lines: (Change as per SPI that you are used)

#uboot_overlay_addr5=<file5>.dtbo
to
uboot_overlay_addr5=/lib/firmware/BB-SPIDEV1-00A0.dtbo

and then reboot.
