#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __attribute__((packed)) {
    MOTOR_DIRECTION_STOP = 0x00,
    MOTOR_DIRECTION_FORWARD,
    MOTOR_DIRECTION_REVERSE,
    MOTOR_DIRECTION_BRAKE,
} motor_direction_t;

#ifdef __cplusplus
}
#endif