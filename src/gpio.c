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

int gpio_fd_open(const char *format, unsigned int gpio, int flags)
{
    char pathname[255];
    int rv;

    rv = snprintf(pathname, sizeof(pathname), format, gpio);
    if (rv < 0 || rv >= sizeof(pathname)) {
        errno = ENOMEM;
        return -1;
    }

    return open(pathname, flags | O_NONBLOCK);
}

ssize_t gpio_fd_read(int fd, void *buf, size_t count)
{
    ssize_t ret;
    ssize_t n = 0;

    if (lseek(fd, 0, SEEK_SET) < 0)
        return -1;

    do {
        ret = read(fd, (char *)buf + n, count - n);
        if (ret < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                continue; /* try again */
            return -1;
        }
        n += ret;
    } while (n < count && ret);

    return n;
}

ssize_t gpio_fd_write(int fd, const void *buf, size_t count)
{
    ssize_t ret;
    ssize_t n = 0;

    do {
        ret = write(fd, (char *)buf + n, count - n);
        if (ret < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                continue; /* try again */
            return -1;
        }
        n += ret;
    } while (n < count);

    return n;
}

ssize_t gpio_read(unsigned int gpio, const char *key, char *buf, size_t count)
{
    char pathname[255];
    ssize_t c;
    int fd;

    fd = snprintf(pathname, sizeof(pathname), key, gpio);
    if (fd < 0 || fd >= sizeof(pathname)) {
        errno = ENOMEM;
        return -1;
    }

    if ((fd = open(pathname, O_RDONLY | O_CLOEXEC)) == -1)
        return -1;

    if ((c = read(fd, buf, count)) == -1) {
        close(fd);
        return -1;
    }

    if (close(fd) == -1)
        return -1;

    return c;
}

int gpio_write(unsigned int gpio, const char *key, const char *buf, size_t count)
{
    char pathname[255];
    int fd;

    fd = snprintf(pathname, sizeof(pathname), key, gpio);
    if (fd < 0 || fd >= sizeof(pathname)) {
        errno = ENOMEM;
        return -1;
    }

    if ((fd = open(pathname, O_WRONLY)) == -1)
        return -1;

    if (gpio_fd_write(fd, buf, count) != count) {
        close(fd);
        return -1;
    }

    return close(fd);
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

    return gpio_write(-1, GPIO_EXPORT, buffer, strlen(buffer));
}

int gpio_free(unsigned int gpio)
{
    char buffer[16];

    snprintf(buffer, sizeof(buffer), "%d\n", gpio);

    return gpio_write(-1, GPIO_UNEXPORT, buffer, strlen(buffer));
}

int gpio_alterable_direction(unsigned int gpio)
{
    return gpio_check(gpio, GPIO_DIRECTION);
}

int gpio_get_direction(unsigned int gpio)
{
    char buffer;

    if (gpio_read(gpio, GPIO_DIRECTION, &buffer, sizeof(buffer)) < 1)
        return -1;

    return (buffer == 'i') ? GPIOF_DIR_IN : GPIOF_DIR_OUT;
}

int gpio_direction_input(unsigned int gpio)
{
    return gpio_write(gpio, GPIO_DIRECTION, "in", 3);
}

int gpio_direction_output(unsigned int gpio, int value)
{
    char *val = value ? "high" : "low";

    return gpio_write(gpio, GPIO_DIRECTION, val, strlen(val) + 1);
}

int gpio_get_activelow(unsigned int gpio)
{
    char buffer;

    if (gpio_read(gpio, GPIO_ACTIVELOW, &buffer, sizeof(buffer)) != sizeof(buffer))
        return -1;

    return buffer - '0';
}

int gpio_set_activelow(unsigned int gpio, int value)
{
    return gpio_write(gpio, GPIO_ACTIVELOW, value ? "1" : "0", 2);
}

int gpio_get_value(unsigned int gpio)
{
    char buffer;

    if (gpio_read(gpio, GPIO_VALUE, &buffer, sizeof(buffer)) != sizeof(buffer))
        return -1;

    return buffer - '0';
}

int gpio_set_value(unsigned int gpio, int value)
{
    return gpio_write(gpio, GPIO_VALUE, value ? "1" : "0", 2);
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

    if (i >= ARRAY_SIZE(trigger_types)) {
        errno = EINVAL;
        return -1;
    }

    return gpio_write(gpio, GPIO_EDGE, trigger_types[i].name,
                      strlen(trigger_types[i].name) + 1);
}

int gpio_get_edge(unsigned int gpio)
{
    char buffer[16];
    int i;

    if (gpio_read(gpio, GPIO_EDGE, buffer, sizeof(buffer)) == -1)
        return -1;

    for (i = 0; i < ARRAY_SIZE(trigger_types); i++)
        if (strncmp(buffer, trigger_types[i].name, strlen(trigger_types[i].name)) == 0)
             break;

    if (i >= ARRAY_SIZE(trigger_types)) {
        errno = EFAULT;
        return -1;
    }

    return trigger_types[i].flags;
}
