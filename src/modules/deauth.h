#define MAX_WIFI_DEVICE 100
#include <Arduino.h>


struct WifiRecord{
    String SSID;
    uint8_t* BSSID;
    String BSSID_STR;
    int32_t RSSI;
};
extern WifiRecord WifiDevice[MAX_WIFI_DEVICE];
void wsl_bypasser_send_raw_frame(const uint8_t *frame_buffer, int size);
void getAvailableWifi();
void sendDeAuth(const char* bssid_str);
void sendDeAuth(const String bssid_str);
void sendDeAuth(uint8_t* bssid);

