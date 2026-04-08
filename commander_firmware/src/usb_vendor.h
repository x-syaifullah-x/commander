#ifndef PICO_RP2040_USB_VENDOR_H
#define PICO_RP2040_USB_VENDOR_H

#include "dispatcher.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(LIB_PICO_STDIO_USB) & (CFG_TUD_CDC == 1)
inline void packet_dump(packet_rx_t *packet_rx, packet_tx_t *packet_tx) {
    printf("========== PACKET DUMP ==========\n");
    printf("[RX]\n");
    printf("  ID   : %u\n", packet_rx->id);
    printf("  CMD  : %d\n", packet_rx->cmd);
    printf("  ARGS : ");
    for (int i = 0; i < packet_rx->args_size; i++) {
        printf("%02X ", packet_rx->args[i]);
    }
    printf("\n\n");

    printf("[TX]\n");
    printf("  TYPE    : %02X %02X\n", packet_tx->type[0], packet_tx->type[1]);
    printf("  ID      : %u\n", packet_tx->id);
    printf("  PAYLOAD : ");
    for (int i = 0; i < 5; i++) {
        printf("%02X ", packet_tx->payload[i]);
    }
    printf("\n\n");
}
#endif

void tud_vendor_rx_cb(uint8_t idx, const uint8_t *buffer, uint32_t bufsize) {
    (void)buffer;
    (void)bufsize;

    packet_rx_t packet_rx;
    packet_rx.args_size = tud_vendor_n_read(idx, &packet_rx, SIZE_PACKET_RX_T) - sizeof(packet_rx.id) - sizeof(packet_rx.cmd);

    packet_tx_t packet_tx = dispatch(packet_rx);

    tud_vendor_n_write(idx, &packet_tx, SIZE_PACKET_TX_T);
    tud_vendor_n_write_flush(idx);

    // clang-format off
    #if defined(LIB_PICO_STDIO_USB) & (CFG_TUD_CDC == 1)
        packet_dump(&packet_rx, &packet_tx);
    #endif
    // clang-format on

    // clang-format off
    // #if (CFG_TUD_CDC == 1)
    //     if(tud_cdc_ready()) {
    //         tud_cdc_write(&packet_tx, sizeof(packet_tx));
    //         tud_cdc_write_flush();
    //     }
    // #endif

    // #if (CFG_TUD_HID == 1)
    //     if (tud_hid_ready())
    //         tud_hid_n_report(0x00, 0x00, &packet_tx, sizeof(packet_tx));
    // #endif
    // clang-format on
}

#ifdef __cplusplus
}
#endif

#endif