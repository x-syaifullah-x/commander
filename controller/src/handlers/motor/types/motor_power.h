#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __attribute__((packed)) {
    MOTOR_POWER_OFF = 0x00,
    MOTOR_POWER_ON,
} motor_power_t;

#ifdef __cplusplus
}
#endif