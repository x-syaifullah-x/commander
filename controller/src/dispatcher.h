#pragma once

#include "handlers/adc.h"
#include "handlers/led/led.h"
#include "handlers/motor/motor.h"
#include "handlers/motor/motor_drv8833.h"
#include "handlers/tb_6612_fng.h"
#include "packet/packet_rx.h"
#include "packet/packet_tx.h"

#ifdef __cplusplus
extern "C" {
#endif

// typedef packet_tx_t (*cmd_handler_t)(const packet_rx_t* rx);

// typedef struct {
//     cmd_handler_t handler;
// } cmd_entry_t;

// static const cmd_entry_t cmd_table[255] = {
//     [CMD_LED_DEFAULT] = {
//         .handler = handle_led_default,
//     },
//     [CMD_LED_WS2812_1BIT] = {
//         .handler = handle_led_WS2812_1BIT,
//     },
//     [CMD_ADC_DMA] = {
//         .handler = handle_led_WS2812_1BIT,
//     },
// };

static inline packet_tx_t dispatch(packet_rx_t rx) {
    // const cmd_entry_t* entry = &cmd_table[rx.cmd];
    // if (!entry->handler)
    //     return (packet_tx_t){.id = rx.id, .status = STATUS_ERR, .cmd = rx.cmd, .data = {STATUS_ERR_CMD_UNKNOWN}};
    // return entry->handler(&rx);

    switch (rx.cmd) {
        case CMD_LED_BEGIN ... CMD_LED_END:
            return handle_led(rx);
        case CMD_ADC_BEGIN ... CMD_ADC_END:
            return handle_adc(rx);
        case CMD_TB6612FNG_BEGIN ... CMD_TB6612FNG_END:
            return handle_tb6612fng(rx);
        case CMD_MOTOR:
            return handle_motor(rx);
        case CMD_MOTOR_DRV8833:
            return handle_motor_drv8833(rx);
        default:
            return (packet_tx_t){.id = rx.id, .status = STATUS_ERR, .cmd = rx.cmd, .data = {STATUS_ERR_CMD_UNKNOWN}};
    }
}

#ifdef __cplusplus
}
#endif