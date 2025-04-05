/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/util.h>

#include "pca9633.h"
#include "aip31068l.h"

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   200

static const struct gpio_dt_spec red_led = GPIO_DT_SPEC_GET(DT_ALIAS(rled), gpios);
static const struct gpio_dt_spec yellow_led = GPIO_DT_SPEC_GET(DT_ALIAS(yled), gpios);
static const struct gpio_dt_spec green_led = GPIO_DT_SPEC_GET(DT_ALIAS(gled), gpios);

#define DEFAULT_DT_SPEC  {0}

int putstr(const struct device *const charcontroller, char *str)
{
    int r = 0;
    for (int i = 0; str[i] != 0 && r == 0; i++) {
        r = aip31068l_putc(charcontroller, str[i]);
    }
    return r;
}

int main(void)
{
    printk("#### Hello from i2c test app ! ####\n");
    if (!gpio_is_ready_dt(&red_led)
            || !gpio_is_ready_dt(&green_led)
            || !gpio_is_ready_dt(&yellow_led)) {
        return 0;
    }

    const struct device *const backlight = DEVICE_DT_GET_ANY(mine_pca9633);
    if (!device_is_ready(backlight)) {
        printk("Device %s is not ready\n", backlight->name);
        return 0;
    }
    pca9633_configure(backlight);
    const struct device *const charcontroller = DEVICE_DT_GET_ANY(mine_aip31068l);
    if (!device_is_ready(charcontroller)) {
        printk("Device %s is not ready\n", charcontroller->name);
        return 0;
    }

    if (gpio_pin_configure_dt(&red_led, GPIO_OUTPUT_ACTIVE) < 0
            || gpio_pin_configure_dt(&yellow_led, GPIO_OUTPUT_ACTIVE) < 0
            || gpio_pin_configure_dt(&green_led, GPIO_OUTPUT_ACTIVE) < 0) {
        return 0;
    }

    char *str = "Hello World !";
    // Last 3 bit are fully ignored
    uint8_t custom_char[8] = {
        0b11100000,
        0b11110001,
        0b11110001,
        0b11100000,
        0b11100000,
        0b11110001,
        0b11101110,
        0b11100000,
    };
    uint8_t custom_char_v2[8] = {
        0b11100100,
        0b11101010,
        0b11101010,
        0b11101010,
        0b11101010,
        0b11101010,
        0b11110101,
        0b11101010,
    };

    putstr(charcontroller, str);
    pca9633_set_rgb(backlight, 255, 0, 0);
    k_msleep(1000);
    aip31068l_cursor_home(charcontroller);
    k_msleep(1000);
    aip31068l_cursor_set(charcontroller, 5, 0);
    aip31068l_putc(charcontroller, '4');
    aip31068l_putc(charcontroller, '2');
    k_msleep(1000);
    aip31068l_cursor_set(charcontroller, 5, 1);
    aip31068l_putc(charcontroller, '4');
    aip31068l_putc(charcontroller, '2');
    k_msleep(1000);
    aip31068l_set_cg(charcontroller, 0, custom_char);
    aip31068l_putc(charcontroller, 0);
    k_msleep(1000);
    // aip31068l_cursor_set(charcontroller, 5, 0);
    // aip31068l_putc(charcontroller, 0);
    // aip31068l_putc(charcontroller, 0);
    // k_msleep(1000);
    // aip31068l_clear_screen(charcontroller);
    // k_msleep(1000);
    // aip31068l_cursor_set(charcontroller, 10, 0);
    // aip31068l_putc(charcontroller, 0);
    // k_msleep(1000);
    aip31068l_set_cg(charcontroller, 1, custom_char_v2);
    aip31068l_cursor_home(charcontroller);
    // There is 30 char space, so this string leave 4 space.
    putstr(charcontroller, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    aip31068l_putc(charcontroller, 0);
    aip31068l_putc(charcontroller, 1);
    aip31068l_putc(charcontroller, 0);
    aip31068l_putc(charcontroller, 1);
    aip31068l_cursor_set(charcontroller, 5, 1);
    aip31068l_putc(charcontroller, 0);
    aip31068l_putc(charcontroller, 1);

    while (1) {
        gpio_pin_toggle_dt(&red_led);
        k_msleep(SLEEP_TIME_MS);
        gpio_pin_toggle_dt(&yellow_led);
        k_msleep(SLEEP_TIME_MS);
        gpio_pin_toggle_dt(&green_led);
        k_msleep(SLEEP_TIME_MS);
        aip31068l_display_shift(charcontroller, 1);
    }
    return 0;
}
