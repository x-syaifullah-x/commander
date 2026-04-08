#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __attribute__((packed)) {
    MOTOR_CMD_POWER = 0x00,
    MOTOR_CMD_DIR,
    MOTOR_CMD_SPEED,
    MOTOR_CMD_BRAKE,
    MOTOR_CMD_STATE = 0xFF,
} motor_cmd_t;

#ifdef __cplusplus
}
#endif