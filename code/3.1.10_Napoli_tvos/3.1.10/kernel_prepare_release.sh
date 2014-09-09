rm -rf arch/arm/Kconfig.kdebug 
rm -rf arch/mips/Kconfig.kdebug 
rm -rf kernel/kdebugd/
cp arch/arm/Kconfig_no_kdebug arch/arm/Kconfig
cp arch/mips/Kconfig_no_kdebug arch/mips/Kconfig
cp kernel/Makefile_no_kedubug kernel/Makefile
