#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "Task.h"
#include "gnss_esp.h"

static const char *TAG = "MAIN";

extern "C" void app_main()
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "     ESP32 GPS/GNSS Logger");
    ESP_LOGI(TAG, "========================================");
    
    // Initialize ESP event loop (required for NMEA parser)
    esp_event_loop_create_default();
    
    // Create GNSS Task
    ESP_LOGI(TAG, "Creating GNSS task...");
    xTaskCreate(gnss_task, "gnss_task", 4096, NULL, 4, NULL);
    
    ESP_LOGI(TAG, "System ready");
}