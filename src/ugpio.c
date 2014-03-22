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
#include <string.h>

#include <config.h>
#include <ugpio.h>
#include <ugpio-internal.h>

ugpio_t *ugpio_request(unsigned int gpio, const char *label)
{
    ugpio_t *ctx;
    int is_requested, val;

    if ((ctx = malloc(sizeof(*ctx))) == NULL)
        return NULL;

    ctx->gpio = gpio;
    ctx->flags = GPIOF_CLOEXEC | GPIOF_DIRECTION_UNKNOWN;
    ctx->label = label;
    ctx->fd_value = -1;
    ctx->fd_active_low = -1;
    ctx->fd_direction = -1;
    ctx->fd_edge = -1;

    if ((is_requested = gpio_is_requested(ctx->gpio)) < 0)
        goto error_free;

    if (!is_requested) {
        if (gpio_request(ctx->gpio, ctx->label) < 0)
            goto error_free;

        ctx->flags |= GPIOF_REQUESTED;
    }

    if (gpio_alterable_direction(ctx->gpio)) {
        ctx->flags |= GPIOF_ALTERABLE_DIRECTION;

        if ((val = gpio_get_direction(ctx->gpio)) != -1) {
            ctx->flags &= ~GPIOF_DIRECTION_UNKNOWN;
            ctx->flags |= val;
        }
    }

    if (gpio_alterable_edge(ctx->gpio)) {
        ctx->flags |= GPIOF_ALTERABLE_EDGE;

        if ((val = gpio_get_edge(ctx->gpio)) != -1) {
            ctx->flags &= ~GPIOF_TRIGGER_MASK;
            ctx->flags |= val;
        }
    }

    return ctx;

error_free:
    free(ctx);
    return NULL;
}

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
    ctx->fd_active_low = -1;
    ctx->fd_direction = -1;
    ctx->fd_edge = -1;

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
    int flags;

    if (ctx->fd_value != -1)
        return ctx->fd_value;

    flags  = (ctx->flags & GPIOF_DIR_IN) ? O_RDONLY : O_RDWR;
    flags |= (ctx->flags & GPIOF_CLOEXEC) ? O_CLOEXEC : 0;

    ctx->fd_value = gpio_fd_open(ctx->gpio, GPIO_VALUE, flags);

    return ctx->fd_value;
}

int ugpio_full_open(ugpio_t *ctx)
{
    int flags;

    if (ugpio_open(ctx) == -1)
        return -1;

    flags  = O_RDWR;
    flags |= (ctx->flags & GPIOF_CLOEXEC) ? O_CLOEXEC : 0;

    if (ctx->fd_active_low == -1)
        ctx->fd_active_low = gpio_fd_open(ctx->gpio, GPIO_ACTIVELOW, flags);
    if (ctx->fd_active_low == -1)
        return -1;

    if (ctx->fd_direction == -1 && ctx->flags & GPIOF_ALTERABLE_DIRECTION)
        ctx->fd_direction = gpio_fd_open(ctx->gpio, GPIO_DIRECTION, flags);
    if (ctx->fd_direction == -1)
        return -1;

    if (ctx->fd_edge == -1 && ctx->flags & GPIOF_ALTERABLE_EDGE)
        ctx->fd_edge = gpio_fd_open(ctx->gpio, GPIO_EDGE, flags);
    if (ctx->fd_edge == -1)
        return -1;

    return 0;
}

void ugpio_close(ugpio_t *ctx)
{
    if (ctx == NULL)
        return;

    if (ctx->fd_value != -1) {
        gpio_fd_close(ctx->fd_value);
        ctx->fd_value = -1;
    }

    if (ctx->fd_active_low != -1) {
        gpio_fd_close(ctx->fd_active_low);
        ctx->fd_active_low = -1;
    }

    if (ctx->fd_direction != -1) {
        gpio_fd_close(ctx->fd_direction);
        ctx->fd_direction = -1;
    }

    if (ctx->fd_edge != -1) {
        gpio_fd_close(ctx->fd_edge);
        ctx->fd_edge = -1;
    }
}

int ugpio_fd(ugpio_t *ctx)
{
    return ctx->fd_value;
}

int ugpio_get_value(ugpio_t *ctx)
{
    char buffer;

    if (gpio_fd_read(ctx->fd_value, &buffer, sizeof(buffer)) < sizeof(buffer))
        return -1;

    return buffer - '0';
}

int ugpio_set_value(ugpio_t *ctx, int value)
{
    ssize_t c;

    c = gpio_fd_write(ctx->fd_value, value ? "1" : "0", 2);

    return (c != 2) ? -1 : 0;
}

int ugpio_get_activelow(ugpio_t *ctx)
{
    char buffer;

    if (gpio_fd_read(ctx->fd_active_low, &buffer, sizeof(buffer)) < sizeof(buffer))
        return -1;

    return buffer - '0';
}

int ugpio_set_activelow(ugpio_t *ctx, int value)
{
    ssize_t c;

    c = gpio_fd_write(ctx->fd_active_low, value ? "1" : "0", 2);

    return (c != 2) ? -1 : 0;
}

int ugpio_alterable_direction(ugpio_t *ctx)
{
    return !!(ctx->flags & GPIOF_ALTERABLE_DIRECTION);
}

int ugpio_get_direction(ugpio_t *ctx)
{
    char buffer;

    if (gpio_fd_read(ctx->fd_direction, &buffer, sizeof(buffer)) < sizeof(buffer))
        return -1;

    return (buffer == 'i') ? GPIOF_DIR_IN : GPIOF_DIR_OUT;
}

int ugpio_direction_input(ugpio_t *ctx)
{
    if (gpio_fd_write(ctx->fd_direction, "in", 3) < 0)
        return -1;

    if (fcntl(ctx->fd_value, F_SETFL, O_RDONLY) < 0)
        return -1;

    ctx->flags &= ~GPIOF_DIRECTION_UNKNOWN;
    ctx->flags |= GPIOF_DIR_IN;
    return 0;
}

int ugpio_direction_output(ugpio_t *ctx, int value)
{
    char *val = value ? "high" : "low";

    if (gpio_fd_write(ctx->fd_direction, val, strlen(val) + 1) < 0)
        return -1;

    if (fcntl(ctx->fd_value, F_SETFL, O_RDWR) < 0)
        return -1;

    ctx->flags &= ~GPIOF_DIR_IN;
    return 0;
}

int ugpio_alterable_edge(ugpio_t *ctx)
{
    return !!(ctx->flags & GPIOF_ALTERABLE_EDGE);
}

int ugpio_get_edge(ugpio_t *ctx)
{

    return gpio_fd_get_edge(ctx->fd_edge);
}

int ugpio_set_edge(ugpio_t *ctx, int flags)
{
    return gpio_fd_set_edge(ctx->fd_edge, flags);
}
