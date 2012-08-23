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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <config.h>
#include <ugpio.h>

#define GPIO_ROOT "/sys/class/gpio"
#define GPIO_EXPORT    GPIO_ROOT "/export"
#define GPIO_UNEXPORT  GPIO_ROOT "/unexport"
#define GPIO_DIRECTION GPIO_ROOT "/gpio%d/direction"
#define GPIO_ACTIVELOW GPIO_ROOT "/gpio%d/active_low"
#define GPIO_VALUE     GPIO_ROOT "/gpio%d/value"

static int gpio_write_value(const char *pathname, const char *buf, size_t count)
{
    int fd;

    if ((fd = open(pathname, O_WRONLY)) == -1)
        return -1;

    if (write(fd, buf, count) != count) {
        close(fd);
        return -1;
    }

    return close(fd);
}

int gpio_is_requested(unsigned int gpio)
{
    int rv;
    char pathname[255];
    snprintf(pathname, sizeof(pathname), GPIO_VALUE, gpio);

    if ((rv = access(pathname, R_OK)) == -1)
        return -1;

    return (rv == 0);
}

int gpio_request(unsigned int gpio, const char *label)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d\n", gpio);
    return gpio_write_value(GPIO_EXPORT, buffer, strlen(buffer));
}

int gpio_free(unsigned int gpio)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d\n", gpio);
    return gpio_write_value(GPIO_UNEXPORT, buffer, strlen(buffer));
}

int gpio_direction_input(unsigned int gpio)
{
    char pathname[255];
    snprintf(pathname, sizeof(pathname), GPIO_DIRECTION, gpio);
    return gpio_write_value(pathname, "in", 3);
}

int gpio_direction_output(unsigned int gpio, int value)
{
    char pathname[255];
    char *val;
    snprintf(pathname, sizeof(pathname), GPIO_DIRECTION, gpio);
    val = value ? "high" : "low";
    return gpio_write_value(pathname, val, strlen(val) + 1);
}

int gpio_get_value(unsigned int gpio)
{
    int fd;
    char pathname[255];
    char buffer;

    snprintf(pathname, sizeof(pathname), GPIO_VALUE, gpio);

    if ((fd = open(pathname, O_RDONLY)) == -1)
        return -1;

    if (read(fd, &buffer, sizeof(buffer)) != sizeof(buffer)) {
        close(fd);
        return -1;
    }

    if (close(fd) == -1)
        return -1;

    return buffer - '0';
}

int gpio_set_value(unsigned int gpio, int value)
{
    char pathname[255];
    snprintf(pathname, sizeof(pathname), GPIO_VALUE, gpio);
    return gpio_write_value(pathname, value ? "1" : "0", 2);
}

int gpio_request_one(unsigned int gpio, unsigned long flags, const char *label)
{
    int err;

    err = gpio_request(gpio, label);
    if (err)
        return err;

    if (flags & GPIOF_DIR_IN)
        err = gpio_direction_input(gpio);
    else
        err = gpio_direction_output(gpio, (flags & GPIOF_INIT_HIGH) ? 1 : 0);

    if (err)
        gpio_free(gpio);

    return err;
}

int gpio_request_array(const struct gpio *array, size_t num)
{
    int i, err;

    for (i = 0; i < num; i++, array++) {
        err = gpio_request_one(array->gpio, array->flags, array->label);
        if (err)
            goto err_free;
    }

    return 0;

  err_free:
    while (i--)
        gpio_free((--array)->gpio);

    return err;
}

void gpio_free_array(const struct gpio *array, size_t num)
{
    while (num--)
        gpio_free((array++)->gpio);
}
