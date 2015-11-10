#include <stddef.h>
#include <stdint.h>
#include "reg.h"
#include "threads.h"

/* USART TXE Flag
 * This flag is cleared when data is written to USARTx_DR and
 * set when that data is transferred to the TDR
 */
#define USART_FLAG_TXE	((uint16_t) 0x0080)

int fib(int num)
{
	if(num==0)
		return 0;
	
	int res;
	asm volatile("push {r3, r4, r5, r6}");
	asm volatile("mov r6, %0" :: "r" (num) :);
	asm volatile("mov r3,#0\n"
				"mov r4,#1\n"
				"mov r5,#0\n"
				".forloop:\n"
				"mov r3,r4\n"
				"mov r4,r5\n"
				"add r5,r3,r4\n"
				"subs r6,r6,#1\n"
				"bgt .forloop\n");
	asm volatile("mov %0, r5" : "=r" (res) ::);
	asm volatile("pop {r3, r4, r5, r6}");
	return res;
}

void usart_init(void)
{
	*(RCC_APB2ENR) |= (uint32_t) (0x00000001 | 0x00000004);
	*(RCC_APB1ENR) |= (uint32_t) (0x00020000);

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

static void delay(volatile int count)
{
	count *= 50000;
	while (count--);
}

static void busy_loop(void *str)
{
	while (1) {
		print_str(str);
		print_str(": Running...\n");
		delay(1000);
	}
}

void test1(void *userdata)
{
	busy_loop(userdata);
}

void test2(void *userdata)
{
	busy_loop(userdata);
}

void test3(void *userdata)
{
	busy_loop(userdata);
}

/* 72MHz */
#define CPU_CLOCK_HZ 72000000

/* 100 ms per tick. */
#define TICK_RATE_HZ 10

int main(void)
{
	const char *str1 = "Task1", *str2 = "Task2", *str3 = "Task3";

	usart_init();

	if (thread_create(test1, (void *) str1) == -1)
		print_str("Thread 1 creation failed\r\n");

	if (thread_create(test2, (void *) str2) == -1)
		print_str("Thread 2 creation failed\r\n");

	if (thread_create(test3, (void *) str3) == -1)
		print_str("Thread 3 creation failed\r\n");

	/* SysTick configuration */
	*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTRL = 0x07;

	thread_start();

	return 0;
}
