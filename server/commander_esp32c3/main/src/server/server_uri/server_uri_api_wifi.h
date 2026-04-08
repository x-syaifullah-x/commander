#pragma once

#include "cJSON.h"
#include "esp_http_server.h"
#include "esp_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

static esp_err_t server_api_wifi_handler(httpd_req_t* req);

static const httpd_uri_t server_api_uri_wifi_option = {
    .uri = "/api/wifi/*",
    .method = HTTP_OPTIONS,
    .handler = server_api_wifi_handler,
};

static const httpd_uri_t server_api_uri_wifi_scan = {
    .uri = "/api/wifi/scan",
    .method = HTTP_POST,
    .handler = server_api_wifi_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t server_api_uri_wifi_connect = {
    .uri = "/api/wifi/connect",
    .method = HTTP_POST,
    .handler = server_api_wifi_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t server_api_uri_wifi_disconnect = {
    .uri = "/api/wifi/disconnect",
    .method = HTTP_POST,
    .handler = server_api_wifi_handler,
    .user_ctx = NULL,
};

static esp_err_t server_api_wifi_handler(httpd_req_t* req) {
    if (req->method == HTTP_OPTIONS) {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");

        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    }

    if (strcmp(req->uri, server_api_uri_wifi_scan.uri) == 0) {
        wifi_scan_config_t scan_config = {
            .ssid = NULL,
            .bssid = NULL,
            .channel = 0,
            .show_hidden = false,
        };

        ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

        uint16_t ap_count = 0;
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

        wifi_ap_record_t* ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);

        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_records));

        cJSON* root = cJSON_CreateArray();

        for (int i = 0; i < ap_count; i++) {
            cJSON* ap = cJSON_CreateObject();
            cJSON_AddStringToObject(ap, "ssid", (char*)ap_records[i].ssid);
            cJSON_AddNumberToObject(ap, "authmode", ap_records[i].authmode);
            cJSON_AddNumberToObject(ap, "rssi", ap_records[i].rssi);
            cJSON_AddItemToArray(root, ap);
        }

        free(ap_records);

        char* json = cJSON_PrintUnformatted(root);

        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);

        cJSON_free(json);
        cJSON_Delete(root);

        return ESP_OK;
    }

    if (strcmp(req->uri, server_api_uri_wifi_connect.uri) == 0) {
        char buf[256];
        if (req->content_len >= sizeof(buf)) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Request too large");
            return ESP_FAIL;
        }

        int received = httpd_req_recv(req, buf, req->content_len);
        if (received <= 0)
            return ESP_FAIL;

        cJSON* root = cJSON_Parse(buf);

        const cJSON* ssid = cJSON_GetObjectItem(root, "ssid");
        const cJSON* password = cJSON_GetObjectItem(root, "password");
        const cJSON* authmode = cJSON_GetObjectItem(root, "authmode");

        esp_wifi_disconnect();
        wifi_config_t wifi_sta_config = {0};
        strncpy((char*)wifi_sta_config.sta.ssid, ssid->valuestring, sizeof(wifi_sta_config.sta.ssid) - 1);
        strncpy((char*)wifi_sta_config.sta.password, password->valuestring, sizeof(wifi_sta_config.sta.password) - 1);
        wifi_sta_config.sta.threshold.authmode = authmode->valueint;
        if (authmode->valueint == WIFI_AUTH_WPA3_PSK || authmode->valueint == WIFI_AUTH_WPA2_WPA3_PSK) {
            wifi_sta_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
        }
        // wifi_sta_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
        esp_wifi_connect();

        cJSON_Delete(root);

        return ESP_OK;
    }

    if (strcmp(req->uri, server_api_uri_wifi_disconnect.uri) == 0) {
        esp_wifi_disconnect();
        return ESP_OK;
    }

    return ESP_FAIL;
}

static const httpd_uri_t server_api_wifi_uris[] = {
    server_api_uri_wifi_option,
    server_api_uri_wifi_scan,
    server_api_uri_wifi_connect,
    server_api_uri_wifi_disconnect,
};

#ifdef __cplusplus
}
#endif