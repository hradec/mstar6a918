void __weak arch_set_wdt(unsigned int sec);
int kernel_level(unsigned int line, const char *function);

static inline void arch_enable_wdt(unsigned int sec)
{
	arch_set_wdt(sec);
}

static inline void arch_disable_wdt(void)
{
	arch_set_wdt(0);
}


