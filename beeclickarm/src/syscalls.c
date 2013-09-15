#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include "stm32f4xx.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Variables */
//#undef errno
extern int errno;

register char * stack_ptr asm("sp");

char *__env[1] = { 0 };
char **environ = __env;


/* Functions */
void initialise_monitor_handles() {
}

int _getpid(void) {
	return 1;
}

int _kill(int pid, int sig) {
	errno = EINVAL;
	return -1;
}

void _exit (int status) {
	_kill(status, -1);
	while (1) {}		/* Make sure we hang here */
}

int _read (int file, char *ptr, int len) {
	/*
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		*ptr++ = syscallReadHandler();
	}

	return len;
	*/

	return 0;
}

int _write(int file, char *ptr, int len) {
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)	{
		//		USART_SendData(USART2, (uint8_t)*ptr);
		ITM_SendChar((uint8_t)*ptr); // Sends it to ST-Link SWO as well so that it can be observed in ST-Link Utility

		//		while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET) {}

		ptr++;
	}

	//	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) {}

	return len;
}

caddr_t _sbrk(int incr) {
	extern char end asm("end");
	static char *heap_end;
	char *prev_heap_end;

	if (heap_end == 0)
		heap_end = &end;

	prev_heap_end = heap_end;
	if (heap_end + incr > stack_ptr)
	{
//		write(1, "Heap and stack collision\n", 25);
//		abort();
		errno = ENOMEM;
		return (caddr_t) -1;
	}

	heap_end += incr;

	return (caddr_t) prev_heap_end;
}

int _close(int file) {
	return -1;
}


int _fstat(int file, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file) {
	return 1;
}

int _lseek(int file, int ptr, int dir) {
	return 0;
}

int _open(char *path, int flags, ...) {
	/* Pretend like we always fail */
	return -1;
}

int _wait(int *status) {
	errno = ECHILD;
	return -1;
}

int _unlink(char *name) {
	errno = ENOENT;
	return -1;
}

int _times(struct tms *buf) {
	return -1;
}

int _stat(char *file, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

int _link(char *o, char *n) {
	errno = EMLINK;
	return -1;
}

int _fork(void) {
	errno = EAGAIN;
	return -1;
}

int _execve(char *name, char **argv, char **env) {
	errno = ENOMEM;
	return -1;
}


#ifdef __cplusplus
}
#endif
