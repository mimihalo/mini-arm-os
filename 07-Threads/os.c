#include <stddef.h>
#include <stdint.h>
#include "reg.h"
#include "threads.h"
#include "mstring.h"

/* USART TXE Flag
 * This flag is cleared when data is written to USARTx_DR and
 * set when that data is transferred to the TDR
 */
#define USART_FLAG_TXE	((uint16_t) 0x0080)

/* when RXNE is set, data can be read */
#define USART_FLAG_RXNE ((uint16_t) 0x0020)

int fib(int num)
{
	if (num == 0)
		return 0;

	int res;
	asm volatile("push {r3, r4, r5, r6}");
	asm volatile("mov r6, %0" :: "r"(num) :);
	asm volatile("mov r3,#0\n"
	             "mov r4,#1\n"
	             "mov r5,#0\n"
	             ".forloop:\n"
	             "mov r3,r4\n"
	             "mov r4,r5\n"
	             "add r5,r3,r4\n"
	             "subs r6,r6,#1\n"
	             "bgt .forloop\n");
	asm volatile("mov %0, r5" : "=r"(res) ::);
	asm volatile("pop {r3, r4, r5, r6}");
	return res;
}

int atoi(char s[])
{
	int i, n, sign;
	for (i = 0; s[i] == ' ' ; i++)
		sign = (s[i] == '-') ? -1 : 1;
	if (s[i] == '+' || s[i] == '-')
		i++;
	for (n = 0; (s[i] >= '0' && s[i] <= '9'); i++)
		n = 10 * n + (s[i] - '0');
	return sign * n;
}

void itoa(int n, char *dst, int base)
{
	char buf[33] = {0};
	char *p = &buf[32];

	if (n == 0)
		*--p = '0';
	else {
		unsigned int num = (base == 10 && num < 0) ? -n : n;

		for (; num; num /= base)
			* --p = "0123456789ABCDEF" [num % base];
		if (base == 10 && n < 0)
			*--p = '-';
	}

	strcpy(dst, p);
}

void usart_init(void)
{
	*(RCC_APB2ENR) |= (uint32_t)(0x00000001 | 0x00000004);
	*(RCC_APB1ENR) |= (uint32_t)(0x00020000);

	/* USART2 Configuration, Rx->PA3, Tx->PA2 */
	*(GPIOA_CRL) = 0x00004B00;
	*(GPIOA_CRH) = 0x44444444;
	*(GPIOA_ODR) = 0x00000000;
	*(GPIOA_BSRR) = 0x00000000;
	*(GPIOA_BRR) = 0x00000000;

	*(USART2_CR1) = 0x0000000C;
	*(USART2_CR2) = 0x00000000;
	*(USART2_CR3) = 0x00000000;
	*(USART2_CR1) |= 0x2000;
}

void print_str(const char *str)
{
	while (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
		str++;
	}
}

char recv_char(void)
{
	while (1) {
		if ((*USART2_SR) & (USART_FLAG_RXNE)) {
			return (*USART2_DR) & 0xff;
		}
	}
}

void print_char(const char *str)
{
	if (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
	}
}

void cmd_help()
{
	print_str("supported command:\n");
	print_str("help\n");
	print_str("fib\n");
}

void cmd_fib(void *num)
{
	char res[16];
	itoa(fib((int)num), res, 10);
	print_str("fib(10)=");
	print_str(res);
	print_str("\n");
}

void cmd(char str[])
{
	char *sstr = strtok(str, " ");
	if (strncmp("help", sstr, 64) == 0) {
		cmd_help();
	} else if (strncmp("fib", sstr, 64) == 0) {
		//sstr = strtok(NULL, " ");
		if (thread_create(cmd_fib, (void *) 10) == -1)
			print_str("fib thread creation failed\r\n");
	}
}

void shell(void *userdata)
{
	char buf[64];
	int ii = 0, enter = 0;
	while (1) {
		print_str(userdata);
		print_str("@mimi $ ");
		while (enter == 0) {
			buf[ii] = recv_char();
			if (buf[ii] >= 32 && buf[ii] <= 126) {
				print_char(&buf[ii]);
				ii++;
			} else {
				switch (buf[ii]) {
				case 13:
				case 10:
					buf[ii] = '\0';
					print_str("\n");
					ii = 0;
					enter++;
					cmd(buf);
					break;
				case 8:
					break;
				case 127:
					break;
				}
			}
		}
		enter = 0;
	}
}

/* 72MHz */
#define CPU_CLOCK_HZ 72000000

/* 100 ms per tick. */
#define TICK_RATE_HZ 10

int main(void)
{
	const char *str1 = "mimihalo";

	usart_init();

	if (thread_create(shell, (void *) str1) == -1)
		print_str("shell thread creation failed\r\n");

	/* SysTick configuration */
	*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTRL = 0x07;

	thread_start();

	return 0;
}
