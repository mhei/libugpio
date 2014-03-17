/*
 * Copyright © 2012-2014 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include <ugpio.h>
#include <ugpio-internal.h>

ugpio_t *ugpio_request_one(unsigned int gpio, unsigned int flags, const char *label)
{
    ugpio_t *ctx;
    int is_requested;

    if ((ctx = malloc(sizeof(*ctx))) == NULL)
        return NULL;

    ctx->gpio = gpio;
    ctx->flags = flags;
    ctx->label = label;
    ctx->fd_value = -1;

    if ((is_requested = gpio_is_requested(ctx->gpio)) < 0)
        goto error_free;

    if (!is_requested) {
        if (gpio_request_one(ctx->gpio, ctx->flags, ctx->label) < 0)
            goto error_free;

        ctx->flags |= GPIOF_REQUESTED;
    }

    return ctx;

error_free:
    free(ctx);
    return NULL;
}

void ugpio_free(ugpio_t *ctx)
{
    if (ctx == NULL)
        return;

    if (ctx->flags & GPIOF_REQUESTED)
        gpio_free(ctx->gpio);

    free(ctx);
}

int ugpio_open(ugpio_t *ctx)
{
    ctx->fd_value = ugpio_fd_open(ctx->gpio, ctx->flags & GPIOF_DIR_IN);
    return ctx->fd_value;
}

void ugpio_close(ugpio_t *ctx)
{
    if (ctx == NULL)
        return;

    if (ctx->fd_value == -1)
        return;

    ugpio_fd_close(ctx->fd_value);
    ctx->fd_value = -1;
}

int ugpio_fd(ugpio_t *ctx)
{
    return ctx->fd_value;
}

int ugpio_get_value(ugpio_t *ctx)
{
    char buffer;
    ssize_t c;
    off_t o;

    if ((o = lseek(ctx->fd_value, 0, SEEK_SET)) == -1)
        return -1;

    if ((c = read(ctx->fd_value, &buffer, sizeof(buffer))) == -1)
        return -1;

    return buffer - '0';
}

int ugpio_set_value(ugpio_t *ctx, int value)
{
    ssize_t c;

    c = write(ctx->fd_value, value ? "1" : "0", 2);

    return (c != 2) ? -1 : 0;
}
