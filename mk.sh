#!/usr/bin

PATH=/usr/bin:$PATH


make uImage
cp -f ./arch/arm/boot/uImage /tftpboot/uImage_k27_src
