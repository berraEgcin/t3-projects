AMAÇ: 
GNSS (Küresel Uydu Seyrüsefer Sistemi), sabit veya hareketli nesnelerin dünya üzerindeki konum ve zaman bilgisini belirlemesini sağlayan sistemdir. Bu projede ATGM336H-6N-74 GNSS modülü kullanılarak elde edilen konum verileri, ESP32-S3-MINI-1-N4R2 üzerinden işlenerek VS Code terminalinde görüntülenmektedir. Geliştirme ortamı olarak VS Code üzerinde PlatformIO framework (Espressif32 platformu) kullanılmaktadır.



KURULUM:
Board: ESP32-S3

Platform: Espressif32

Framework: PlatformIO (ESP-IDF tabanlı)



CLASSLAR: 
gnss_esp.cpp: GNSS modülünün UART üzerinden başlatılması sağlanır. İsteğe bağlı olarak uyku (sleep) ve düşük güç modları yönetilir.
nmea_parser.c: GNSS’ten gelen NMEA formatındaki ham veriyi parse ederek anlamlı konum verisine (enlem, boylam, zaman vb.) dönüştüren kütüphanedir.
Task.cpp: FreeRTOS üzerinde GPS verisini okumak ve işlemek için ayrı bir task oluşturulur.
main.cpp: Tanımlanan GNSS task’i başlatılır ve sistem çalıştırılır.


FONKSİYONLAR
gnss_esp.cpp:

gnss_init()
UART konfigürasyonu yapılır (baudrate, TX/RX pinleri).

GNSS modülü (ATGM336H-6N-74) ile haberleşme başlatılır.

UART driver yüklenir ve buffer oluşturulur.

GNSS veri akışı hazır hale getirilir.



gnss_sleep() (Opsiyonel)
GNSS modülüne sleep komutu gönderilir.

Güç tüketimi azaltılır.

UART veri akışı durdurulabilir.



gnss_wake() (Opsiyonel)
GNSS modülü aktif moda alınır.

Veri üretimi tekrar başlatılır.



nmea_parser.c

nmea_parser_init()
Parser yapısı oluşturulur.

Veri buffer’ı ve callback mekanizması ayarlanır.

NMEA cümlelerini ayrıştırmaya hazır hale gelir.



nmea_parse()
UART’tan gelen ham NMEA string verisini alır.

$GPGGA, $GPRMC gibi mesajları ayrıştırır.

Enlem, boylam, zaman, uydu sayısı gibi bilgileri struct içine işler.



Task.cpp

gnss_task(void *pvParameters)
FreeRTOS üzerinde sürekli çalışan bir task oluşturur.

UART’tan veri okur.

Gelen veriyi parser’a gönderir.

Parse edilen konum bilgisini terminale yazdırır.

Belirli aralıklarla vTaskDelay() ile zamanlama yapılır.



main.cpp

app_main()
GNSS başlatma fonksiyonu çağrılır.

GNSS task’i xTaskCreate() ile oluşturulur.

Sistem çalışmaya başlar.
