#pragma once

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pin.h"
#include "utils/pwm/pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

enum __attribute__((packed)) {
    MOTOR_DRV8833_EN_PIN_ALWAYS_ON = (uint8_t)-1,
    MOTOR_DRV8833_CHANNEL = 2,
};

typedef struct __attribute__((packed)) {
    pin_t in_1;
    pin_t in_2;
} motor_driver_drv8833_pin_in_t;

typedef struct __attribute__((packed)) {
    uint8_t EN;
    motor_driver_drv8833_pin_in_t in[MOTOR_DRV8833_CHANNEL];
} motor_driver_drv8833_pin_t;

static motor_driver_drv8833_pin_t motor_driver_drv8833_pin;

static uint32_t motor_driver_drv8833_init(motor_driver_drv8833_pin_t pin) {
    if (pin.EN != (uint8_t)MOTOR_DRV8833_EN_PIN_ALWAYS_ON) {
        gpio_init(pin.EN);
        if (gpio_get_dir(pin.EN) != GPIO_OUT)
            gpio_set_dir(pin.EN, GPIO_OUT);
        gpio_put(pin.EN, true);
    }

    for (size_t i = 0; i < MOTOR_DRV8833_CHANNEL; i++) {
        const motor_driver_drv8833_pin_in_t in = pin.in[i];
        gpio_init(in.in_1);
        if (gpio_get_dir(in.in_1) != GPIO_OUT)
            gpio_set_dir(in.in_1, GPIO_OUT);

        gpio_init(in.in_2);
        if (gpio_get_dir(in.in_2) != GPIO_OUT)
            gpio_set_dir(in.in_2, GPIO_OUT);
    }

    motor_driver_drv8833_pin = pin;
    return 1;
}

static bool motor_driver_drv8833_deinit() {
    if (motor_driver_drv8833_pin.EN != MOTOR_DRV8833_EN_PIN_ALWAYS_ON) {
        gpio_deinit(motor_driver_drv8833_pin.EN);
        return 0;
    }

    for (size_t i = 0; i < MOTOR_DRV8833_CHANNEL; i++) {
        gpio_deinit(motor_driver_drv8833_pin.in[i].in_1);
        gpio_deinit(motor_driver_drv8833_pin.in[i].in_2);
    }

    return 1;
}

#ifdef __cplusplus
}
#endif