/*
 * This file is in the public domain.
 * lpc_syscall.h
 */

static inline void syscall(int id, int arg0, int arg1, int arg2)
{
	__asm__ __volatile__("swi 0"::"r" (id), "r" (arg0), "r" (arg1), "r" (arg2));
	return;
}

static inline void enableInterrupt(void)
{
	__asm__ __volatile__("swi 1");
	return;
}

static inline void disableInterrupt(void)
{
	__asm__ __volatile__("swi 2");
	return;
}

