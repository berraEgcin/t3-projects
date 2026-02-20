#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <esp_err.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nmea_parser.h"
#include "gnss_esp.h"
#include "driver/rtc_io.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char *TAG = "GNSS";

#define UART_BUF_SIZE (1024)
#define RX_PIN GPIO_NUM_7
#define TX_PIN GPIO_NUM_8
#define WK_PIN GPIO_NUM_9
#define GPS_UART UART_NUM_1

static bool g_is_initialized = false;
static bool g_is_active = false;
static nmea_parser_handle_t nmea_hdl = NULL;

// Store the latest valid GPS data
static gps_t g_last_gps_data = {0};
static bool g_has_valid_data = false;

static void gps_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    gps_t *gps = NULL;
    switch (event_id) {
    case GPS_UPDATE:
        gps = (gps_t *)event_data;
        if (gps) {
            // Store the latest GPS data
            memcpy(&g_last_gps_data, gps, sizeof(gps_t));
            g_has_valid_data = true;
            ESP_LOGD(TAG, "GPS data updated: lat=%.6f, lon=%.6f, sats=%d", 
                     gps->latitude, gps->longitude, gps->sats_in_use);
        }
        break;
    case GPS_UNKNOWN:
        ESP_LOGD(TAG, "Unknown GPS sentence received");
        break;
    default:
        break;
    }
}

esp_err_t gnss_init(const gnss_config_t *config) {
    if (g_is_initialized) {
        ESP_LOGW(TAG, "GNSS already initialized");
        return ESP_OK;
    }
    
    if (!config) {
        ESP_LOGE(TAG, "Config is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    esp_err_t ret;
    
    // Configure wake pin as output
    gpio_config_t wake_pin_config = {
        .pin_bit_mask = (1ULL << WK_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ret = gpio_config(&wake_pin_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure wake pin");
        return ret;
    }
    
    // Set wake pin high (module on)
    gpio_set_level((gpio_num_t)WK_PIN, 1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    
    nmea_parser_config_t parser_config = {
    .uart = {
        .uart_port = (uart_port_t)config->uart_num,
        .rx_pin = (uint32_t)config->rx_pin,
        .baud_rate = config->baud_rate,   // Use baud_rate from config
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .event_queue_size = 32,           // Increased to handle GPS data bursts
        },
    };
    
    ESP_LOGI(TAG, "NMEA parser config: UART=%d, RX=%d, TX=%d, Baud=%d",
             config->uart_num, config->rx_pin, config->tx_pin, config->baud_rate);
    
    // Initialize NMEA parser
    nmea_hdl = nmea_parser_init(&parser_config);
    if (nmea_hdl == NULL) {
        ESP_LOGE(TAG, "Failed to initialize NMEA parser");
        return ESP_FAIL;
    }
    
    // Register event handler
    ret = nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add GPS event handler: %d", ret);
        nmea_parser_deinit(nmea_hdl);
        nmea_hdl = NULL;
        return ret;
    }

    g_is_initialized = true;
    g_is_active = true;
    
    ESP_LOGI(TAG, "GNSS initialized successfully");
    return ESP_OK;
}

esp_err_t gnss_sleep(void) {
    if (!g_is_initialized) {
        ESP_LOGW(TAG, "GNSS not initialized");
        return ESP_FAIL;
    }
    
    gpio_set_level((gpio_num_t)WK_PIN, 0);
    g_is_active = false;
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "GNSS sleep mode activated");
    return ESP_OK;
}

esp_err_t gnss_wakeup(void) {
    if (!g_is_initialized) {
        ESP_LOGW(TAG, "GNSS not initialized");
        return ESP_FAIL;
    }
    
    gpio_set_level((gpio_num_t)WK_PIN, 1);
    g_is_active = true;
    vTaskDelay(100 / portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "GNSS wakeup");
    return ESP_OK;
}

esp_err_t gnss_get_data(gnss_data_t *data, uint32_t timeout_ms) {
    if (!data) {
        ESP_LOGE(TAG, "Data pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    if (!g_is_initialized) {
        ESP_LOGE(TAG, "GNSS not initialized");
        return ESP_FAIL;
    }
    if (!g_is_active) {
        ESP_LOGE(TAG, "GNSS not active");
        return ESP_FAIL;
    }
    
    uint32_t start_time = xTaskGetTickCount();
    uint32_t timeout_ticks = (timeout_ms / portTICK_PERIOD_MS);
    
    // Zero out output structure
    memset(data, 0, sizeof(gnss_data_t));
    
    while ((xTaskGetTickCount() - start_time) < timeout_ticks) {
        if (g_has_valid_data && g_last_gps_data.valid) {
            // Copy GPS data to output structure
            data->location_valid = g_last_gps_data.valid;
            data->latitude = g_last_gps_data.latitude;
            data->longitude = g_last_gps_data.longitude;
            
            // Altitude
            data->altitude_valid = (g_last_gps_data.altitude != 0.0);
            data->altitude = g_last_gps_data.altitude;
            
            // HDOP (Horizontal Dilution of Precision)
            data->hdop_valid = (g_last_gps_data.dop_h > 0.0);
            data->hdop = g_last_gps_data.dop_h;
            
            // Satellites
            data->satellites_locked = g_last_gps_data.sats_in_use;
            data->gps_satellites = g_last_gps_data.sats_in_view;
            
            // Time and date
            data->time_valid = true;
            data->hour = g_last_gps_data.tim.hour;
            data->minute = g_last_gps_data.tim.minute;
            data->second = g_last_gps_data.tim.second;
            data->day = g_last_gps_data.date.day;
            data->month = g_last_gps_data.date.month;
            data->year = g_last_gps_data.date.year;
            
            ESP_LOGD(TAG, "GPS data retrieved successfully");
            return ESP_OK;
        }
        
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
    
    ESP_LOGW(TAG, "GNSS data timeout after %lu ms", timeout_ms);
    return ESP_ERR_TIMEOUT;
}

bool gnss_is_active(void) {
    return g_is_active && g_is_initialized;
}

void gnss_deinit(void) {
    if (g_is_initialized) {
        if (nmea_hdl) {
            ESP_LOGI(TAG, "Deinitializing NMEA parser...");
            nmea_parser_remove_handler(nmea_hdl, gps_event_handler);
            nmea_parser_deinit(nmea_hdl);
            nmea_hdl = NULL;
        }
        
        gpio_set_level((gpio_num_t)WK_PIN, 0);
        gpio_reset_pin((gpio_num_t)WK_PIN);
        
        g_is_initialized = false;
        g_is_active = false;
        g_has_valid_data = false;
        
        ESP_LOGI(TAG, "GNSS deinitialized");
    }
}

#ifdef __cplusplus
}
#endif