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
	/* file descriptor of /sys/class/gpio/gpioXY/active_low */
	int fd_active_low;
	/* file descriptor of /sys/class/gpio/gpioXY/direction */
	int fd_direction;
	/* file descriptor of /sys/class/gpio/gpioXY/edge */
	int fd_edge;
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
 *
 * Each GPIO is handled within a GPIO context object. Such an object encapsulates
 * all functions which can be run on a GPIO. First, you have to create such an
 * object using ugpio_request_one. Before you actually can use it, you have to
 * call ugpio_open. After usage, close the object with ugpio_close and free up
 * the used ressources with ugpio_free.
 */

/**
 * Request a GPIO context.
 *
 * @param gpio the GPIO number to request
 * @param flags a combination of or-ed GPIOF_* constants
 * @param label an optional label for this GPIO
 * @return returns a ugpio_t object on success, NULL otherwise
 */
ugpio_t *ugpio_request_one(unsigned int gpio, unsigned int flags, const char *label);

/**
 * Release/free a GPIO context.
 *
 * @param ctx a GPIO context
 */
void ugpio_free(ugpio_t *ctx);

/**
 * Open the GPIO.
 *
 * This opens /sys/class/gpio/gpioXY/value file, but not the other file handles.
 *
 * @param ctx a GPIO context
 */
int ugpio_open(ugpio_t *ctx);

/**
 * Close the GPIO.
 *
 * This closes all open file handles of the GPIO context.
 *
 * @param ctx a GPIO context
 */
void ugpio_close(ugpio_t *ctx);

/**
 * Return the GPIO context's value file descriptor.
 *
 * Using this function the application can feed the file descriptor into it's
 * select/poll... loop to monitor it for events.
 *
 * @param ctx a GPIO context
 * @return the file descriptor of the opened value file of the GPIO
 */
int ugpio_fd(ugpio_t *ctx);

/**
 * Get the GPIO context's current value.
 *
 * Read the current GPIO pin level which is 0 or 1. Usually this corresponds
 * to physical LOW and HIGH levels, but depends on 'active low' configuration
 * of the GPIO.
 *
 * @param ctx a GPIO context
 * @return 0 or 1 on success, -1 on error with errno set appropriately
 */
int ugpio_get_value(ugpio_t *ctx);

/**
 * Set the GPIO context's current value.
 *
 * Giving a value of 0 normally forces a LOW on the GPIO pin, whereas a value
 * of 1 sets the GPIO to HIGH. But note, that if the GPIO is configured as
 * 'active low' then the logic is reversed.
 *
 * @param ctx a GPIO context
 * @param value the value to set
 * @return 0 on success, -1 on error with errno set appropriately
 */
int ugpio_set_value(ugpio_t *ctx, int value);

UGPIO_END_DECLS

#endif  /* _UGPIO_H_ */
