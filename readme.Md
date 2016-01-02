cp arch/arm/configs/godarm_defconfig .config

make ARCH=arm CROSS_COMPILE=arm-hisiv200-linux-gnueabi- uImage -j8

make ARCH=arm CROSS_COMPILE=arm-hisiv200-linux-gnueabi- menuconfig

makeuImage:
gzip -9 zImage
mkimage  -A arm -O linux -T kernel -C gzip -a 0x80008000 -e 0x80008000 -n "linux kernel image" -d zImage.gz uImage

install module:
make ARCH=arm CROSS_COMPILE=arm-hisiv200-linux-gnueabi- modules_install INSTALL_MOD_PATH=/home/xy/works/test/


install debian:
add CGroup 
