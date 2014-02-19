/*
 * Copyright © 2012-2014 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <config.h>
#include <ugpio.h>
#include <ugpio-internal.h>

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

static ssize_t gpio_read_value(const char *pathname, char *buf, size_t count)
{
    int fd;
    ssize_t c;

    if ((fd = open(pathname, O_RDONLY)) == -1)
        return -1;

    if ((c = read(fd, buf, count)) == -1) {
        close(fd);
        return -1;
    }

    if (close(fd) == -1)
        return -1;

    return c;
}

static int gpio_check(unsigned int gpio, const char *key)
{
    int fd;
    char pathname[255];
    snprintf(pathname, sizeof(pathname), key, gpio);

    fd = open(pathname, O_RDONLY | O_CLOEXEC);

    /* file does not exist */
    if (fd == -1 && errno == ENOENT)
        return 0;

    /* an unexpected error occured */
    if (fd == -1)
        return -1;

    /* file exists, so cleanup */
    close(fd);

    return 1;
}

int gpio_is_requested(unsigned int gpio)
{
    return gpio_check(gpio, GPIO_VALUE);
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

int gpio_alterable_direction(unsigned int gpio)
{
    return gpio_check(gpio, GPIO_DIRECTION);
}

int gpio_get_direction(unsigned int gpio)
{
    char pathname[255];
    char buffer;

    snprintf(pathname, sizeof(pathname), GPIO_DIRECTION, gpio);

    if (gpio_read_value(pathname, &buffer, sizeof(buffer)) == -1)
        return -1;

    return (buffer == 'i') ? GPIOF_DIR_IN : GPIOF_DIR_OUT;
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

int gpio_get_activelow(unsigned int gpio)
{
    char pathname[255];
    char buffer;

    snprintf(pathname, sizeof(pathname), GPIO_ACTIVELOW, gpio);

    if (gpio_read_value(pathname, &buffer, sizeof(buffer)) == -1)
        return -1;

    return buffer - '0';
}

int gpio_set_activelow(unsigned int gpio, int value)
{
    char pathname[255];
    snprintf(pathname, sizeof(pathname), GPIO_ACTIVELOW, gpio);
    return gpio_write_value(pathname, value ? "1" : "0", 2);
}

int gpio_get_value(unsigned int gpio)
{
    char pathname[255];
    char buffer;

    snprintf(pathname, sizeof(pathname), GPIO_VALUE, gpio);

    if (gpio_read_value(pathname, &buffer, sizeof(buffer)) == -1)
        return -1;

    return buffer - '0';
}

int gpio_set_value(unsigned int gpio, int value)
{
    char pathname[255];
    snprintf(pathname, sizeof(pathname), GPIO_VALUE, gpio);
    return gpio_write_value(pathname, value ? "1" : "0", 2);
}

int gpio_request_one(unsigned int gpio, unsigned int flags, const char *label)
{
    int rv;

    rv = gpio_request(gpio, label);
    if (rv)
        return rv;

    if (flags & GPIOF_DIR_IN)
        rv = gpio_direction_input(gpio);
    else
        rv = gpio_direction_output(gpio, (flags & GPIOF_INIT_HIGH) ? 1 : 0);

    if (rv)
        goto err_free;

    if ((rv = gpio_alterable_edge(gpio)) < 0)
        goto err_free;

    if (rv)
        rv = gpio_set_edge(gpio, flags);

  err_free:
    if (rv)
        gpio_free(gpio);

    return rv;
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

int gpio_alterable_edge(unsigned int gpio)
{
    return gpio_check(gpio, GPIO_EDGE);
}

int gpio_set_edge_str(unsigned int gpio, const char *edge)
{
    char pathname[255];
    snprintf(pathname, sizeof(pathname), GPIO_EDGE, gpio);
    return gpio_write_value(pathname, edge, strlen(edge) + 1);
}

static const struct {
    const char *name;
    unsigned int flags;
} trigger_types[] = {
    { "none",    0 },
    { "falling", GPIOF_TRIG_FALL },
    { "rising",  GPIOF_TRIG_RISE },
    { "both",    GPIOF_TRIG_FALL | GPIOF_TRIG_RISE },
};

int gpio_set_edge(unsigned int gpio, unsigned int flags)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(trigger_types); i++)
        if ((flags & GPIOF_TRIGGER_MASK) == trigger_types[i].flags)
            break;

    return gpio_set_edge_str(gpio, trigger_types[i].name);
}

int gpio_get_edge(unsigned int gpio)
{
    char pathname[255];
    char buffer[16];
    int i;

    snprintf(pathname, sizeof(pathname), GPIO_EDGE, gpio);

    if (gpio_read_value(pathname, buffer, sizeof(buffer)) == -1)
        return -1;

    for (i = 0; i < ARRAY_SIZE(trigger_types); i++)
        if (strncmp(buffer, trigger_types[i].name, strlen(trigger_types[i].name)) == 0)
             break;

    return trigger_types[i].flags;
}
