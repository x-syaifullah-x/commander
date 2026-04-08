#pragma once

#include "server_uri/server_uri_api_wifi.h"
#include "server_uri/server_uri_ws.h"

#ifdef __cplusplus
extern "C" {
#endif

static httpd_handle_t server_start(const uint16_t port) {
    s_ws_mutex = xSemaphoreCreateMutex();
    configASSERT(s_ws_mutex);
    memset(s_ws_fds, -1, sizeof(s_ws_fds));

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.max_open_sockets = CONFIG_WS_MAX_CLIENTS + 2; /* +2 untuk margin HTTP biasa */
    config.uri_match_fn = httpd_uri_match_wildcard;

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG_WS, "Failed to start httpd");
        return NULL;
    }

    httpd_register_uri_handler(server, &server_uri_ws);
    ESP_LOGI(TAG_WS, "WebSocket server started → ws://[ip]%s", server_uri_ws.uri);

    for (size_t i = 0; i < sizeof(server_api_wifi_uris) / sizeof(server_api_wifi_uris[0]); i++) {
        httpd_register_uri_handler(server, &server_api_wifi_uris[i]);
    }

    return server;
}

#ifdef __cplusplus
}
#endif