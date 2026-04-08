#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __attribute__((packed)) {
    PICO_ID_DEVICE = 0,
    PICO_ID_HOST = 1
} pico_id_t;

typedef enum __attribute__((packed)) {
    PICO_CMD_LED_DEFAULT = 21,
    PICO_CMD_ADC_DMA = 41,
    PICO_CMD_ADC_CH0 = 42,
    PICO_CMD_ADC_CH1 = 43,
    PICO_CMD_ADC_CH2 = 44,
    PICO_CMD_ADC_CH4 = 46,
    PICO_CMD_TB6612FNG_POWER = 61,
    PICO_CMD_TB6612FNG_MOTOR_1 = 62,
    PICO_CMD_TB6612FNG_MOTOR_2 = 63,
    PICO_CMD_MOTOR = 81,
    PICO_CMD_MOTOR_DRV8833,
    PICO_CMD_END
} pico_cmd_t;

typedef uint8_t pico_data_t[5];

typedef enum __attribute__((packed)) {
    PICO_STATUS_OK = 0,
    PICO_STATUS_ERR = 1,
} pico_status_t;

#ifdef __cplusplus
}
#endif