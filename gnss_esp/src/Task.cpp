#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "gnss_esp.h"
#include "Task.h"

static const char *TAG = "GNSS_TASK";

void gnss_task(void *pvParameters)
{
    ESP_LOGI(TAG, "GNSS Task started");
    
    gnss_config_t config = {
        .uart_num = 1,
        .rx_pin = 7,
        .tx_pin = 8,
        .wk_pin = 9,
        .baud_rate = 115200  
    };
    
    ESP_LOGI(TAG, "Initializing GNSS module...");
    esp_err_t ret = gnss_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GNSS initialization failed. Error: 0x%x", ret);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "GNSS initialized successfully");
    ESP_LOGI(TAG, "Waiting for GPS fix...");
    
    gnss_data_t data;
    int attempt = 0;
    int success_count = 0;
    
    while (1) {
        attempt++;
        ret = gnss_get_data(&data, 10000);

        if (ret == ESP_OK) {
            success_count++;
            ESP_LOGI(TAG, "=== GPS FIX [%d] ===", success_count);
            ESP_LOGI(TAG, "  Lat: %.6f | Lon: %.6f", data.latitude, data.longitude);
            ESP_LOGI(TAG, "  Satellites: %d locked", data.satellites_locked);
        
            if (data.altitude_valid) {
                ESP_LOGI(TAG, "  Altitude: %.2f m", data.altitude);
            }
            
            if (data.time_valid) {
                ESP_LOGI(TAG, "  UTC: %02d:%02d:%02d | Date: %02d/%02d/%04d",
                        data.hour, data.minute, data.second,
                        data.day, data.month, data.year);
            }
            
            if (data.hdop_valid) {
                ESP_LOGI(TAG, "  HDOP: %.2f", data.hdop);
            }
        } 
        else if (ret == ESP_ERR_TIMEOUT) {
            if (attempt % 6 == 1) {
                ESP_LOGW(TAG, "Waiting for GPS fix... [Attempt %d]", attempt);
            }
        } 
        else {
            ESP_LOGE(TAG, "GPS read error: 0x%x", ret);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}