#pragma once

#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pin.h"
#include "utils/pwm/pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

enum __attribute__((packed)) {
    EN_PIN_ALWAYS_ON = (uint8_t)-1,
};

typedef struct __attribute__((packed)) {
    uint8_t EN;
    pin_t R_PWM;
    pin_t L_PWM;
} motor_driver_bts7960_pin_t;

static motor_driver_bts7960_pin_t motor_driver_bts7960_pin;

static uint32_t motor_driver_bts7960_init(motor_driver_bts7960_pin_t pin) {
    if (pin.EN != (uint8_t)EN_PIN_ALWAYS_ON) {
        gpio_init(pin.EN);
        if (gpio_get_dir(pin.EN) != GPIO_OUT)
            gpio_set_dir(pin.EN, GPIO_OUT);
        gpio_put(pin.EN, true);
    }

    if (gpio_get_function(pin.R_PWM) == GPIO_FUNC_NULL)
        gpio_init(pin.R_PWM);
    if (gpio_get_dir(pin.R_PWM) != GPIO_OUT)
        gpio_set_dir(pin.R_PWM, GPIO_OUT);
    if (gpio_get_function(pin.R_PWM) != GPIO_FUNC_PWM) {
        gpio_set_function(pin.R_PWM, GPIO_FUNC_PWM);
        uint slice_num = pwm_gpio_to_slice_num(pin.R_PWM);
        pwm_set_wrap(slice_num, PWM_WRAP);
        pwm_set_clkdiv(slice_num, PWM_CLKDIV);
        pwm_set_enabled(slice_num, true);
    }

    if (gpio_get_function(pin.L_PWM) == GPIO_FUNC_NULL)
        gpio_init(pin.L_PWM);
    if (gpio_get_dir(pin.L_PWM) != GPIO_OUT)
        gpio_set_dir(pin.L_PWM, GPIO_OUT);
    if (gpio_get_function(pin.L_PWM) != GPIO_FUNC_PWM) {
        gpio_set_function(pin.L_PWM, GPIO_FUNC_PWM);
        uint slice_num = pwm_gpio_to_slice_num(pin.L_PWM);
        pwm_set_wrap(slice_num, PWM_WRAP);
        pwm_set_clkdiv(slice_num, PWM_CLKDIV);
        pwm_set_enabled(slice_num, true);
    }

    motor_driver_bts7960_pin = pin;
    return 1;
}

static bool motor_driver_bts7960_deinit() {
    if (motor_driver_bts7960_pin.EN != EN_PIN_ALWAYS_ON) {
        gpio_deinit(motor_driver_bts7960_pin.EN);
        return 0;
    }
    return 1;
}

#ifdef __cplusplus
}
#endif