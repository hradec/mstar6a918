rm -rf release_bsp
mkdir release_bsp
cp arch/arm/boot/zImage release_bsp
cp vmlinux release_bsp
cp `find -L -name "*.ko"` release_bsp