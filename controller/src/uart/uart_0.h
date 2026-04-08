#pragma once

#include "handlers/boot.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "packet/packet_rx.h"
#include "pin.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UART_0_ENABLE 1

#define UART_0_ID uart0
#define UART_0_IRQ UART0_IRQ
#define UART_0_RX_PIN PIN_22
#define UART_0_TX_PIN PIN_21

void uart_0_irq_handler(void) {
    static uint32_t rx_len = 0;

    uint32_t rsr = uart_get_hw(UART_0_ID)->rsr;
    if (rsr) {
        uart_get_hw(UART_0_ID)->rsr = rsr;
        rx_len = 0;
        return;
    }

    static packet_rx_t rx;
    uint8_t* rx_ptr = (uint8_t*)&rx;

    while (uart_is_readable(UART_0_ID)) {
        rx_ptr[rx_len++] = uart_getc(UART_0_ID);
        if (rx_len == sizeof(packet_rx_t)) {
            rx_len = 0;
            if (rx.cmd == CMD_SYSTEM_REBOOT)
                handle_boot_rom(rx);
            packet_rx_rb_push(&rx);
        }
    }
}

void uart_0_init(uint32_t baudrate) {
#if UART_0_ENABLE
    if (uart_is_enabled(UART_0_ID)) return;

    uart_init(UART_0_ID, baudrate);

    gpio_set_function(UART_0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_0_RX_PIN, GPIO_FUNC_UART);

    uart_set_fifo_enabled(UART_0_ID, true);

    irq_set_exclusive_handler(UART_0_IRQ, uart_0_irq_handler);
    irq_set_enabled(UART_0_IRQ, true);

    uart_set_irq_enables(UART_0_ID, true, false);
#else
    (void)baudrate;
#endif
}

#ifdef __cplusplus
}
#endif