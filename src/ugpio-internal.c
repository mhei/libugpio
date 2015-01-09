/*
 * Copyright © 2015 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <config.h>
#include <ugpio.h>
#include <ugpio-internal.h>

int gpio_fd_open(unsigned int gpio, const char *key, int flags)
{
    char pathname[255];
    int rv;

    rv = snprintf(pathname, sizeof(pathname), key, gpio);
    if (rv < 0 || rv >= sizeof(pathname)) {
        errno = ENOMEM;
        return -1;
    }

    return open(pathname, flags | O_NONBLOCK);
}

int gpio_fd_close(int fd)
{
    return close(fd);
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

static const struct {
    const char *name;
    unsigned int flags;
} ugpio_triggers[] = {
    { "none",    0 },
    { "falling", GPIOF_TRIG_FALL },
    { "rising",  GPIOF_TRIG_RISE },
    { "both",    GPIOF_TRIG_FALL | GPIOF_TRIG_RISE },
};

int gpio_fd_get_edge(int fd)
{
    char buffer[16];
    int i;

    if (gpio_fd_read(fd, buffer, sizeof(buffer)) == -1)
        return -1;

    for (i = 0; i < ARRAY_SIZE(ugpio_triggers); i++)
        if (strncmp(buffer, ugpio_triggers[i].name, strlen(ugpio_triggers[i].name)) == 0)
             break;

    if (i >= ARRAY_SIZE(ugpio_triggers)) {
        errno = EFAULT;
        return -1;
    }

    return ugpio_triggers[i].flags;
}

int gpio_fd_set_edge(int fd, unsigned int flags)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(ugpio_triggers); i++)
        if ((flags & GPIOF_TRIGGER_MASK) == ugpio_triggers[i].flags)
            break;

    if (i >= ARRAY_SIZE(ugpio_triggers)) {
        errno = EINVAL;
        return -1;
    }

    return gpio_fd_write(fd, ugpio_triggers[i].name,
                         strlen(ugpio_triggers[i].name) + 1);
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

    if ((c = gpio_fd_read(fd, buf, count)) == -1) {
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

int gpio_check(unsigned int gpio, const char *key)
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
