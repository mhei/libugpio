/*
 * Copyright © 2012-2015 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: GPL-3.0+
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include <config.h>
#include <ugpio.h>

void print_usage(void)
{
	printf("gpioctl dirin|dirout|dirout-low|dirout-high|get|set|clear gpio\n");
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	unsigned int gpio_pin;
	int rq, al, rv = -1;

	if (argc != 3)
	{
		print_usage();
	}

	gpio_pin = atoi(argv[2]);
	printf("Using gpio pin %u.\n", gpio_pin);

	if ((rq = gpio_is_requested(gpio_pin)) < 0)
	{
		perror("gpio_is_requested");
		return EXIT_FAILURE;
	}

	if (!rq) {
		if ((rv = gpio_request(gpio_pin, NULL)) < 0)
		{
			perror("gpio_request");
			return EXIT_FAILURE;
		}
	}

	if ((al = gpio_get_activelow(gpio_pin)) < 0)
	{
		perror("gpio_get_activelow");
		goto err_out;
	}

	if (!strcmp(argv[1], "dirin"))
	{
		if ((rv = gpio_direction_input(gpio_pin)) < 0)
		{
			perror("gpio_direction_input");
		}
	} else if (!strcmp(argv[1], "dirout") || !strcmp(argv[1], "dirout-low"))
	{
		if ((rv = gpio_direction_output(gpio_pin, GPIOF_INIT_LOW)) < 0)
		{
			perror("gpio_direction_output");
		}
	} else if (!strcmp(argv[1], "dirout-high"))
	{
		if ((rv = gpio_direction_output(gpio_pin, GPIOF_INIT_HIGH)) < 0)
		{
			perror("gpio_direction_output");
		}
	} else if (!strcmp(argv[1], "get"))
	{
		if ((rv = gpio_get_value(gpio_pin)) < 0)
		{
			perror("gpio_get_value");
		} else {
			printf("Pin %u is %s\n", gpio_pin, (al != rv) ? "HIGH" : "LOW");
		}
	} else if (!strcmp(argv[1], "set"))
	{
		if ((rv = gpio_set_value(gpio_pin, al ? 0 : 1)) < 0)
		{
			perror("gpio_set_value");
		}
	} else if (!strcmp(argv[1], "clear"))
	{
		if ((rv = gpio_set_value(gpio_pin, al ? 1 : 0)) < 0)
		{
			perror("gpio_set_value");
		}
	} else print_usage();

err_out:
	if (!rq) {
		if (gpio_free(gpio_pin) < 0)
		{
			perror("gpio_free");
		}
	}

	return (rv < 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
