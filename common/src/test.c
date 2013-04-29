#include "dsb/test.h"
#include <stdio.h>

unsigned int check_count = 0;
unsigned int check_failed = 0;
unsigned int test_count = 0;
unsigned int test_failed = 0;
int test_pass = 1;
int test_pretty = 1;

void dsb_test_done(const char *function)
{
	if (test_pass == 1)
	{
		if (test_pretty == 1)
		{
			printf("%s \e[32;1mpassed\e[0m\n",function);
		}
		else
		{

		}
	}
	else
	{
		if (test_pretty == 1)
		{
			printf("%s \e[31;1failed\e[0m\n",function);
		}
		else
		{

		}
	}
}

void dsb_test_checkfailed(int line, const char *function, const char *file)
{
	if (test_pretty == 1)
	{
		printf("%s:%d \e[31;1mfailed!\e[0m\n",function,line);
	}
	else
	{

	}

	test_pass = 0;
	check_count++;
	check_failed++;
}

void dsb_test_checkpassed()
{
	check_count++;
}

void dsb_test(void (*test)(void))
{
	test_pass = 1;
	test_count++;
	test();
}

