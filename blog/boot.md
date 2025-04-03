https://stackoverflow.com/questions/31244862/what-is-the-use-of-spl-secondary-program-loader

https://github.com/u-boot/u-boot/blob/master/doc/board/emulation/qemu-riscv.rst

Booting from TPL/SPL:
https://docs.u-boot.org/en/latest/usage/spl_boot.html

SPL Kconfig
https://github.com/u-boot/u-boot/blob/c17f03a7e93dfbbe5d32f5738274187065d9493f/common/spl/Kconfig

xPL overview:
https://docs.u-boot.org/en/latest/develop/spl.html

FIT images howto
https://docs.u-boot.org/en/latest/usage/fit/howto.html
https://www.thegoodpenguin.co.uk/blog/u-boot-fit-image-overview/

Falcon mode
https://docs.u-boot.org/en/latest/develop/falcon.html

# Misc
`make qemu-riscv64_spl_defconfig && make`
`vim configs/qemu-riscv64_spl_defconfig`

Remember to edit spl file!

Copy jump_to_image_no_args -> jump_to_image_linux in u-boot/arch/riscv/lib/spl.c

Debug print common/spl/spl.c

Found out you have to load main uboot modules, even if you ONLY use the SPL, otherwise they are not included. (Saw this since SPL_LOAD_IMAGE_METHOD was not called when running the spl)


# Boot order
Simply a list of methods to boot: spl_boot_list
```
u32 spl_boot_list[] = {
		BOOT_DEVICE_MMC1,
		BOOT_DEVICE_MMC2,
		BOOT_DEVICE_MMC2_2,
		BOOT_DEVICE_NAND,
		BOOT_DEVICE_SATA,
	};
```

5 options. Full list in arch/arm/include/asm/spl.h

You can override boot by editing board/emulation/qemu-riscv/qemu-riscv.c
```
u32 spl_boot_device(void)
{
	/* RISC-V QEMU only supports RAM as SPL boot device */
	return BOOT_DEVICE_MMC1;
}
```




TODO:

To enable falcon boot, a hook function spl_start_uboot() returns 0 to indicate
booting U-Boot is not the first choice. The kernel FIT image needs to be put
at CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR. SPL mmc driver reads the header to
determine if this is a FIT image. If true, FIT image components are parsed and
copied or decompressed (if applicable) to their destinations. If FIT image is
not found, normal U-Boot flow will follow.

# Running
qemu-system-riscv64 -M virt -m 256M -nographic -bios ../u-boot/spl/u-boot-spl.bin -drive id=mysdcard,if=none,file=sdcard.img,format=raw,id=mydisk -device sdhci-pci -device sd-card,drive=mysdcard

qemu-system-riscv64 -M virt -m 256M -nographic -bios ../u-boot/spl/u-boot-spl.bin -device ich9-ahci,id=ahci -drive if=none,file=my-os.itb,format=raw,id=mydisk -device ide-hd,drive=mydisk,bus=ahci.0

# Test config added to default
CONFIG_RISCV_SMODE=n
CONFIG_OPENSBI=n
CONFIG_SPL_FS_SUPPORT=y
CONFIG_SPL_FS_EXT4=y
CONFIG_SPL_FS_LOADER=y
CONFIG_SPL_PAYLOAD=y
CONFIG_VIRTIO_BLK=y
CONFIG_SPL_VIRTIO=y
CONFIG_SPL_VIRTIO_BLK=y
CONFIG_SPL_OS_BOOT=y
CONFIG_SPL_PAYLOAD_ARGS_ADDR=0x80200000
CONFIG_SPL_PAYLOAD_NAME="kernel.elf"
CONFIG_SYS_SPL_ARGS_ADDR=0x80200000
CONFIG_SPL_FS_LOAD_KERNEL_NAME="kernel.elf"
#CONFIG_SPL_FS_LOAD_ARGS_NAME="boot.scr"  # Optional boot script
CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_DEVICE=0
CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION=1

#CONFIG_SPL_DISPLAY_PRINT=y
CONFIG_SPL_LIBDISK_SUPPORT=y
CONFIG_SPL_SYS_MALLOC_SIMPLE=y
CONFIG_SPL_LOG=y
#CONFIG_SPL_LOG_MAX_LEVEL=7  # Most verbose


CONFIG_SPL_FIT=y
CONFIG_SPL_FIT_SOURCE=""
CONFIG_SPL_LOAD_FIT=y
CONFIG_SPL_OF_CONTROL=y

CONFIG_SPL_LIBCOMMON_SUPPORT=y
CONFIG_SPL_LIBGENERIC_SUPPORT=y
CONFIG_SPL_SERIAL=y
CONFIG_SPL_PRINTF=y
CONFIG_LOG=y
#CONFIG_SPL_FS_LOAD_PAYLOAD_NAME="my-os.itb"

CONFIG_SPL_FS_LOAD_PAYLOAD_NAME="my-os.itb"
#CONFIG_SYS_UBOOT_START=n
CONFIG_SPL_RAM_DEVICE=n
CONFIG_BOOTM_LINUX=n
CONFIG_SPL_MMC=y
CONFIG_SILENT_CONSOLE=n
CONFIG_SHOW_ERRORS=y
CONFIG_SPL_PCI=y
CONFIG_SPL_PCI_PNP=y
CONFIG_MMC_PCI=y
CONFIG_MMC_SDHCI=y
CONFIG_MMC=y