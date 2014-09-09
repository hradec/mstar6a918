#include <linux/cpu.h>

static unsigned int kernel_debug = 0;

void __weak arch_set_wdt(unsigned int sec)
{
	unsigned int freq = sec * 12 * 10000 * 1000; /* 12M clock */

	/* reg_wdt_max high bytes*/
	*(volatile unsigned short *)(0xFD000000 + 0x003000 * 2 + 0x05 * 4) 
		= (freq >> 16) & 0xFF;
	/* reg_wdt_max low bytes*/
	*(volatile unsigned short *)(0xFD000000 + 0x003000 * 2 + 0x04 * 4) 
		= freq & 0xFF;
}

static int __init kdebug_setup(char *str)
{
	printk("KDebug = %s\n", str);
	if (str)
		kernel_debug = simple_strtol(str, NULL, 16);
	else
		printk("\nKDebug not set\n");

	return 0;
}
early_param("KDebug", kdebug_setup);

int kernel_level(unsigned int line, const char *function)
{
	printk("\n Kernel_Level test :%s  %d\n", function, line);

	return kernel_debug;
}
