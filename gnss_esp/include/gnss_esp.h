#ifndef GNSS_H
#define GNSS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <esp_err.h>

// GNSS Yapılandırma
typedef struct {
    int uart_num;        // UART port numarası (UART_NUM_1, UART_NUM_2, vb.)
    int rx_pin;          // RX pin numarası
    int tx_pin;          // TX pin numarası
    int wk_pin;          // Wake-up/Enable pin numarası
    uint32_t baud_rate;  // Baud rate (varsayılan: 115200)
} gnss_config_t;

// GNSS Veri Yapısı
typedef struct {
    // Konum bilgileri
    double latitude;       // Enlem (derece)
    double longitude;      // Boylam (derece)
    float altitude;        // Yükseklik (metre)
    
    // Zaman bilgileri (UTC)
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    
    // Kalite bilgileri
    uint8_t satellites_locked;  // Kilitlenmiş uydu sayısı
    float hdop;                 // Yatay Doğruluk Seyreltme (Horizontal Dilution of Precision)
    
    // Uydu sistem bilgileri (görünür uydular)
    uint8_t gps_satellites;     // GPS uydular
    uint8_t beidou_satellites;  // BeiDou uydular
    uint8_t glonass_satellites; // GLONASS uydular
    uint8_t galileo_satellites; // Galileo uydular
    
    // flags
    bool location_valid;
    bool time_valid;
    bool altitude_valid;
    bool hdop_valid;
} gnss_data_t;

// Fonksiyonlar
/**
 * @brief GNSS modülünü başlatır
 * @param config GNSS yapılandırma parametreleri
 * @return ESP_OK başarılı, ESP_FAIL hata
 */
esp_err_t gnss_init(const gnss_config_t *config);

/**
 * @brief GNSS modülünü uyku moduna alır (güç tasarrufu)
 * @return ESP_OK başarılı, ESP_FAIL hata
 */
esp_err_t gnss_sleep(void);

/**
 * @brief GNSS modülünü uyku modundan uyandırır
 * @return ESP_OK başarılı, ESP_FAIL hata
 */
esp_err_t gnss_wakeup(void);

/**
 * @brief GNSS verilerini okur ve günceller
 * @param data GNSS verisinin yazılacağı yapı
 * @param timeout_ms Maksimum bekleme süresi (ms)
 * @return ESP_OK veri alındı, ESP_ERR_TIMEOUT zaman aşımı, ESP_FAIL hata
 */
esp_err_t gnss_get_data(gnss_data_t *data, uint32_t timeout_ms);

/**
 * @brief GNSS modülünün durumunu kontrol eder
 * @return true modül çalışıyor, false modül uyuyor veya hata
 */
bool gnss_is_active(void);

/**
 * @brief GNSS sürücüsünü kapatır ve kaynakları serbest bırakır
 */
void gnss_deinit(void);

#ifdef __cplusplus
}
#endif

#endif //GNSS_H 