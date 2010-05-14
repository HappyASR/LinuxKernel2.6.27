#!/usr/bin

PATH=/usr/bin:$PATH


make uImage
cp -f ./arch/arm/boot/uImage /tftpboot/uImage_k27_src
cp -f .config sq8000.config
cp -f ./arch/arm/boot/uImage ./uImage_k27_src
