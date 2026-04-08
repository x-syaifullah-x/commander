#pragma once

#include "driver/motor_driver_bts7960.h"
#include "packet/packet_rx.h"
#include "packet/packet_tx.h"
#include "types/motor_cmd.h"
#include "types/motor_direction.h"
#include "types/motor_power.h"
#include "utils/pwm/pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

motor_power_t motor_power = MOTOR_POWER_OFF;
motor_direction_t motor_direction = MOTOR_DIRECTION_STOP;
uint8_t motor_speed = MOTOR_DIRECTION_STOP;

static uint slice_num;
static uint channel;

static inline packet_tx_t handle_motor_power(packet_rx_t rx) {
    const motor_power_t on = rx.args[1];
    switch (on) {
        case MOTOR_POWER_OFF:
            motor_power = motor_driver_bts7960_deinit(motor_driver_bts7960_pin);
            return (packet_tx_t){.id = rx.id, .cmd = rx.cmd, .status = STATUS_OK, .data = {motor_power, motor_direction, motor_speed, 0, 0}};
        case MOTOR_POWER_ON:
            const motor_driver_bts7960_pin_t pin = {.EN = rx.args[2], .R_PWM = rx.args[3], .L_PWM = rx.args[4]};
            motor_power = motor_driver_bts7960_init(pin);
            return (packet_tx_t){.id = rx.id, .cmd = rx.cmd, .status = STATUS_OK, .data = {motor_power, motor_direction, motor_speed, 0, 0}};
        default:
            return (packet_tx_t){.id = rx.id, .cmd = rx.cmd, .status = STATUS_ERR, .data = {STATUS_ERR_CMD_ARG_INVALID}};
    }
}

static inline packet_tx_t handle_motor_dir(packet_rx_t rx) {
    const motor_direction_t dir = rx.args[1];
    switch (dir) {
        case MOTOR_DIRECTION_STOP:
            slice_num = pwm_gpio_to_slice_num(motor_driver_bts7960_pin.L_PWM);
            channel = pwm_gpio_to_channel(motor_driver_bts7960_pin.L_PWM);
            pwm_set_chan_level(slice_num, channel, 0);

            slice_num = pwm_gpio_to_slice_num(motor_driver_bts7960_pin.R_PWM);
            channel = pwm_gpio_to_channel(motor_driver_bts7960_pin.R_PWM);
            pwm_set_chan_level(slice_num, channel, 0);

            slice_num = -1;
            channel = -1;
            break;
        case MOTOR_DIRECTION_FORWARD:
            slice_num = pwm_gpio_to_slice_num(motor_driver_bts7960_pin.L_PWM);
            channel = pwm_gpio_to_channel(motor_driver_bts7960_pin.L_PWM);
            pwm_set_chan_level(slice_num, channel, 0);

            slice_num = pwm_gpio_to_slice_num(motor_driver_bts7960_pin.R_PWM);
            channel = pwm_gpio_to_channel(motor_driver_bts7960_pin.R_PWM);
            pwm_set_chan_level(slice_num, channel, PWM_DUTY(motor_speed));

            break;
        case MOTOR_DIRECTION_REVERSE:
            slice_num = pwm_gpio_to_slice_num(motor_driver_bts7960_pin.R_PWM);
            channel = pwm_gpio_to_channel(motor_driver_bts7960_pin.R_PWM);
            pwm_set_chan_level(slice_num, channel, 0);

            slice_num = pwm_gpio_to_slice_num(motor_driver_bts7960_pin.L_PWM);
            channel = pwm_gpio_to_channel(motor_driver_bts7960_pin.L_PWM);
            pwm_set_chan_level(slice_num, channel, PWM_DUTY(motor_speed));

            break;
        case MOTOR_DIRECTION_BRAKE:
            pwm_set_chan_level(pwm_gpio_to_slice_num(motor_driver_bts7960_pin.R_PWM), pwm_gpio_to_channel(motor_driver_bts7960_pin.R_PWM), PWM_DUTY(100));
            pwm_set_chan_level(pwm_gpio_to_slice_num(motor_driver_bts7960_pin.L_PWM), pwm_gpio_to_channel(motor_driver_bts7960_pin.L_PWM), PWM_DUTY(100));

            break;
        default:
            return (packet_tx_t){.id = rx.id, .cmd = rx.cmd, .status = STATUS_ERR, .data = {STATUS_ERR_CMD_ARG_INVALID}};
    }

    motor_direction = dir;
    return (packet_tx_t){.id = rx.id, .cmd = rx.cmd, .status = STATUS_OK, .data = {motor_power, motor_direction, motor_speed, 0, 0}};
}

static inline packet_tx_t handle_motor_speed(packet_rx_t rx) {
    const uint8_t speed = rx.args[1];

    motor_speed = speed > 100 ? 100 : speed;

    if (motor_direction == MOTOR_DIRECTION_BRAKE)
        return (packet_tx_t){.id = rx.id, .cmd = rx.cmd, .status = STATUS_OK, .data = {motor_power, motor_direction, motor_speed, 0, 0}};

    pwm_set_chan_level(slice_num, channel, PWM_DUTY(motor_speed));

    return (packet_tx_t){.id = rx.id, .cmd = rx.cmd, .status = STATUS_OK, .data = {motor_power, motor_direction, motor_speed, 0, 0}};
}

// ret: [id_t, status_t, cmd_t, motor_power_t, motor_direction_t, uint8_t(speed), uint8_t(0), uint8_t(0)]
static inline packet_tx_t handle_motor(packet_rx_t rx) {
    switch ((motor_cmd_t)rx.args[0]) {
        // args: [MOTOR_CMD(80), MOTOR_CMD_POWER(0), OFF(0) | ON(1), EN_PIN, R_PWM, L_PWM]
        case MOTOR_CMD_POWER:
            return handle_motor_power(rx);
        // args: [MOTOR_CMD(80), MOTOR_CMD_DIR(1), STOP(0) | FORWARD(1) | REVERSE(2) | BRAKE(3), 0, 0, 0]
        case MOTOR_CMD_DIR:
            return handle_motor_dir(rx);
        // args: [MOTOR_CMD(80), MOTOR_CMD_SPEED(2), SPEED(0-100), 0, 0, 0]
        case MOTOR_CMD_SPEED:
            return handle_motor_speed(rx);
        // args: [MOTOR_CMD(80), MOTOR_CMD_STATE(255), SPEED(0-100), 0, 0, 0]
        case MOTOR_CMD_STATE:
            return (packet_tx_t){.id = rx.id, .status = STATUS_OK, .cmd = rx.cmd, .data = {motor_power, motor_direction, motor_speed, 0, 0}};
        default:
            return (packet_tx_t){.id = rx.id, .status = STATUS_ERR, .cmd = rx.cmd, .data = {STATUS_ERR_CMD_ARG_INVALID}};
    }
}

#ifdef __cplusplus
}
#endif