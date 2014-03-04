/*
 * Copyright © 2012-2014 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef _UGPIO_H_
#define _UGPIO_H_

#include <stddef.h>
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

#define GPIOF_TRIG_FALL (1 << 2)
#define GPIOF_TRIG_RISE (1 << 3)
#define GPIOF_TRIGGER_MASK (GPIOF_TRIG_FALL | GPIOF_TRIG_RISE)

#define GPIOF_REQUESTED (1 << 4)

/**
 * A structure describing a GPIO with configuration.
 */
struct gpio {
	/* the GPIO number */
	unsigned int gpio;
	/* GPIO configuration as specified by GPIOF_* */
	unsigned int flags;
	/* file descriptor of /sys/class/gpio/gpioXY/value */
	int fd_value;
	/* a literal description string of this GPIO */
	const char *label;
};
typedef struct gpio ugpio_t;

/**
 * Low level API
 */
int gpio_is_requested(unsigned int gpio);
int gpio_request(unsigned int gpio, const char *label);
int gpio_request_one(unsigned int gpio, unsigned int flags, const char *label);
int gpio_request_array(const struct gpio *array, size_t num);
int gpio_free(unsigned int gpio);
void gpio_free_array(const struct gpio *array, size_t num);

int gpio_alterable_direction(unsigned int gpio);
int gpio_get_direction(unsigned int gpio);
int gpio_direction_input(unsigned int gpio);
int gpio_direction_output(unsigned int gpio, int value);

int gpio_get_activelow(unsigned int gpio);
int gpio_set_activelow(unsigned int gpio, int value);

int gpio_get_value(unsigned int gpio);
int gpio_set_value(unsigned int gpio, int value);

int gpio_alterable_edge(unsigned int gpio);
int gpio_set_edge_str(unsigned int gpio, const char *edge);
int gpio_set_edge(unsigned int gpio, unsigned int flags);
int gpio_get_edge(unsigned int gpio);

/**
 * Higher level API
 */
ugpio_t *ugpio_request_one(unsigned int gpio, unsigned int flags, const char *label);
void ugpio_free(ugpio_t *ctx);
int ugpio_open(ugpio_t *ctx);
void ugpio_close(ugpio_t *ctx);
int ugpio_fd(ugpio_t *ctx);
int ugpio_get_value(ugpio_t *ctx);
int ugpio_set_value(ugpio_t *ctx, int value);

UGPIO_END_DECLS

#endif  /* _UGPIO_H_ */
