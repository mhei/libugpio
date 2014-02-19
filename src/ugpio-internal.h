/*
 * Copyright © 2012-2014 Michael Heimpold <mhei@heimpold.de>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef _UGPIO_INTERNAL_H_
#define _UGPIO_INTERNAL_H_

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#define GPIO_ROOT "/sys/class/gpio"
#define GPIO_EXPORT    GPIO_ROOT "/export"
#define GPIO_UNEXPORT  GPIO_ROOT "/unexport"
#define GPIO_DIRECTION GPIO_ROOT "/gpio%d/direction"
#define GPIO_ACTIVELOW GPIO_ROOT "/gpio%d/active_low"
#define GPIO_VALUE     GPIO_ROOT "/gpio%d/value"
#define GPIO_EDGE      GPIO_ROOT "/gpio%d/edge"

#endif  /* _UGPIO_INTERNAL_H_ */
