/*
 * Copyright © 2012 Michael Heimpold <mhei@heimpold.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _UGPIO_H_
#define _UGPIO_H_

#include "ugpio-version.h"

#ifdef  __cplusplus
# define UGPIO_BEGIN_DECLS  extern "C" {
# define UGPIO_END_DECLS    }
#else
# define UGPIO_BEGIN_DECLS
# define UGPIO_END_DECLS
#endif

UGPIO_BEGIN_DECLS

/* taken from linux/gpio.h */
#define GPIOF_DIR_OUT   (0 << 0)
#define GPIOF_DIR_IN    (1 << 0)

#define GPIOF_INIT_LOW  (0 << 1)
#define GPIOF_INIT_HIGH (1 << 1)

#define GPIOF_IN                (GPIOF_DIR_IN)
#define GPIOF_OUT_INIT_LOW      (GPIOF_DIR_OUT | GPIOF_INIT_LOW)
#define GPIOF_OUT_INIT_HIGH     (GPIOF_DIR_OUT | GPIOF_INIT_HIGH)

/**
 * struct gpio - a structure describing a GPIO with configuration
 * @gpio:       the GPIO number
 * @flags:      GPIO configuration as specified by GPIOF_*
 * @label:      a literal description string of this GPIO
 */
struct gpio {
	unsigned int  gpio;
	unsigned long flags;
	const char    *label;
};

inline int gpio_is_valid(unsigned int gpio)
{
	return 1;
}

extern int gpio_request(unsigned int gpio, const char *label);
extern int gpio_request_one(unsigned int gpio, unsigned long flags, const char *label);
extern int gpio_request_array(const struct gpio *array, size_t num);
extern int gpio_free(unsigned int gpio);
extern void gpio_free_array(const struct gpio *array, size_t num);
extern int gpio_direction_input(unsigned int gpio);
extern int gpio_direction_output(unsigned int gpio, int value);
extern int gpio_get_value(unsigned int gpio);
extern int gpio_set_value(unsigned int gpio, int value);

inline int gpio_cansleep(unsigned int gpio)
{
	return 1;
}

#define gpio_get_value_cansleep gpio_get_value
#define gpio_set_value_cansleep gpio_set_value

UGPIO_END_DECLS

#endif  /* _UGPIO_H_ */
