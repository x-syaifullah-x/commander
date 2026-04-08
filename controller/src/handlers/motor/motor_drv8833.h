#pragma once

#include "driver/motor_driver_drv8833.h"
#include "packet/packet_rx.h"
#include "packet/packet_tx.h"
#include "types/motor_cmd.h"
#include "types/motor_direction.h"
#include "types/motor_power.h"
#include "utils/pwm/pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed)) {
    motor_direction_t direction;
    uint8_t speed;
} motor_drv8833_channel_t;

typedef struct __attribute__((packed)) {
    uint8_t power;
    motor_drv8833_channel_t channel[MOTOR_DRV8833_CHANNEL];
} motor_drv8833_state_t;

motor_drv8833_state_t motor_drv8833_state = {
    .power = MOTOR_POWER_OFF,
    .channel = {
        {.direction = MOTOR_DIRECTION_STOP, .speed = 0},
        {.direction = MOTOR_DIRECTION_STOP, .speed = 0},
    },
};

static inline packet_tx_t handle_motor_drv833_power(const packet_rx_t rx) {
    const motor_power_t on = rx.args[1];
    switch (on) {
        case MOTOR_POWER_OFF:
            if (motor_driver_drv8833_deinit()) {
                for (size_t i = 0; i < MOTOR_DRV8833_CHANNEL; i++) {
                    motor_drv8833_state.channel[i].direction = MOTOR_DIRECTION_STOP;
                    motor_drv8833_state.channel[i].speed = 0;
                }
            }
            return (packet_tx_t){.id = rx.id, .cmd = rx.cmd, .status = STATUS_OK, .data = {0, 0, 0, 0, 0}};
        case MOTOR_POWER_ON:
            const motor_driver_drv8833_pin_t pin = {
                .EN = (uint8_t)-1,
                .in = {
                    {.in_1 = rx.args[2], .in_2 = rx.args[3]},
                    {.in_1 = rx.args[4], .in_2 = rx.args[5]},
                },
            };
            motor_drv8833_state.power = motor_driver_drv8833_init(pin);
            return (packet_tx_t){
                .id = rx.id,
                .status = STATUS_OK,
                .cmd = rx.cmd,
                .data = {motor_drv8833_state.power, 0, 0, 0, 0},
            };
        default:
            return (packet_tx_t){.id = rx.id, .cmd = rx.cmd, .status = STATUS_ERR, .data = {STATUS_ERR_CMD_ARG_INVALID}};
    }
}

static inline packet_tx_t handle_motor_drv833_dir(packet_rx_t rx) {
    const uint8_t channel = rx.args[1];
    const motor_driver_drv8833_pin_in_t in = motor_driver_drv8833_pin.in[channel];
    const motor_direction_t dir = rx.args[2];
    switch (dir) {
        case MOTOR_DIRECTION_STOP:
            gpio_put(in.in_1, false);
            gpio_put(in.in_2, false);
            break;
        case MOTOR_DIRECTION_FORWARD:
            gpio_put(in.in_1, motor_drv8833_state.channel[channel].speed > 0);
            gpio_put(in.in_2, false);
            break;
        case MOTOR_DIRECTION_REVERSE:
            gpio_put(in.in_1, false);
            gpio_put(in.in_2, motor_drv8833_state.channel[channel].speed > 0);
            break;
        case MOTOR_DIRECTION_BRAKE:
            gpio_put(in.in_1, true);
            gpio_put(in.in_2, true);
            break;
        default:
            return (packet_tx_t){.id = rx.id, .status = STATUS_ERR, .cmd = rx.cmd, .data = {STATUS_ERR_CMD_ARG_INVALID}};
    }

    motor_drv8833_state.channel[channel].direction = dir;
    return (packet_tx_t){
        .id = rx.id,
        .status = STATUS_OK,
        .cmd = rx.cmd,
        .data = {
            motor_drv8833_state.power,
            motor_drv8833_state.channel[0].direction,
            motor_drv8833_state.channel[0].speed,
            motor_drv8833_state.channel[1].direction,
            motor_drv8833_state.channel[1].speed,
        },
    };
}

static inline packet_tx_t handle_motor_drv8833_speed(packet_rx_t rx) {
    const uint8_t channel = rx.args[1];
    motor_drv8833_state.channel[channel].speed = rx.args[2] > 0 ? 1 : 0;

    const motor_driver_drv8833_pin_in_t in = motor_driver_drv8833_pin.in[channel];
    switch (motor_drv8833_state.channel[channel].direction) {
        case MOTOR_DIRECTION_STOP:
            gpio_put(in.in_1, false);
            gpio_put(in.in_2, false);
            break;
        case MOTOR_DIRECTION_FORWARD:
            gpio_put(in.in_1, motor_drv8833_state.channel[channel].speed > 0);
            gpio_put(in.in_2, false);
            break;
        case MOTOR_DIRECTION_REVERSE:
            gpio_put(in.in_1, false);
            gpio_put(in.in_2, motor_drv8833_state.channel[channel].speed > 0);
            break;
        case MOTOR_DIRECTION_BRAKE:
            break;
        default:
            return (packet_tx_t){.id = rx.id, .status = STATUS_ERR, .cmd = rx.cmd, .data = {STATUS_ERR_CMD_ARG_INVALID}};
    }

    return (packet_tx_t){
        .id = rx.id,
        .status = STATUS_OK,
        .cmd = rx.cmd,
        .data = {
            motor_drv8833_state.power,
            motor_drv8833_state.channel[0].direction,
            motor_drv8833_state.channel[0].speed,
            motor_drv8833_state.channel[1].direction,
            motor_drv8833_state.channel[1].speed,
        },
    };
}

// ret: [id_t, status_t, cmd_t, motor_power_t, motor_direction_t, uint8_t(speed), uint8_t(0), uint8_t(0)]
static inline packet_tx_t handle_motor_drv8833(packet_rx_t rx) {
    switch ((motor_cmd_t)rx.args[0]) {
        // command: [ID, MOTOR_CMD(82), MOTOR_CMD_POWER(0), OFF(0) | ON(1), IN_A_1(GP14), IN_A_2(GP15), IN_B_1(GP13), IN_B_2(GP12)]
        case MOTOR_CMD_POWER:
            return handle_motor_drv833_power(rx);
        // command: [ID, MOTOR_CMD(82), MOTOR_CMD_DIR(1), CHANNEL, STOP(0) | FORWARD(1) | REVERSE(2) | BRAKE(3), 0, 0, 0]
        case MOTOR_CMD_DIR:
            return handle_motor_drv833_dir(rx);
        // command: [ID, MOTOR_CMD(82), MOTOR_CMD_SPEED(2), CHANNEL, SPEED(0-100), 0, 0, 0]
        case MOTOR_CMD_SPEED:
            return handle_motor_drv8833_speed(rx);
        // command: [ID, MOTOR_CMD(82), MOTOR_CMD_STATE(255), 0, 0, 0, 0, 0]
        // case MOTOR_CMD_STATE:
        //     return (packet_tx_t){.id = rx.id, .status = STATUS_OK, .cmd = rx.cmd, .data = {motor_drv833_power, motor_drv833_direction, motor_drv833_speed, 0, 0}};
        default:
            return (packet_tx_t){.id = rx.id, .status = STATUS_ERR, .cmd = rx.cmd, .data = {STATUS_ERR_CMD_ARG_INVALID}};
    }
}

#ifdef __cplusplus
}
#endif