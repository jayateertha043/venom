#include "rf.h"
#include "utils/utils.h"
#include "web.h"

#define samplesize 1000

int RXPin = 4;
int Gdo0 = 2;
const int minsample = 30;
long *sample;
int samplecount;
static unsigned long lastTime = 0;
int raw_rx = 0;
int jam = 0;

struct RFConfig
{
    float freq;
    int mod;
    float dev;
    float drate;
    float bw;
    int duration;
    bool crc;
    int txrx;
    String protocol;
} rfconf;

void initializeRFConfig(float freq, int mod, float dev, float drate, float bw, int dur, bool crc, int rxtx)
{
    Serial.printf("\nFreq: %f", freq);
    Serial.printf("\nMod: %d", mod);
    Serial.printf("\nDev: %f", dev);
    Serial.printf("\nDRate: %f", drate);
    Serial.printf("\nBW: %f", bw);
    Serial.printf("\nCRC: %d\n", crc);
    Serial.printf("\nRXTX: %d\n", rxtx);
    rfconf.freq = freq;
    rfconf.mod = mod;
    rfconf.dev = dev;
    rfconf.drate = drate;
    rfconf.bw = bw;
    rfconf.crc = crc;
    rfconf.protocol = "RAW";
    // ELECHOUSE_cc1101.setSpiPin(18, 19, 23, 5);
    // ELECHOUSE_cc1101.setGDO(2, 4);
    if (rxtx > 0)
    {
        pinMode(Gdo0, OUTPUT);
    }
    else
    {
        pinMode(RXPin, INPUT);
    }

    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setDeviation(dev);
    ELECHOUSE_cc1101.setMHZ(freq);       // Set the basic frequency. The lib calculates the frequency automatically (default = 433.92 MHz). The CC1101 can operate in various frequency bands.
    ELECHOUSE_cc1101.setModulation(mod); // Set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
    // ELECHOUSE_cc1101.setCCMode(0); // Set config for internal transmission mode.

    if (!crc)
    {
        ELECHOUSE_cc1101.setCrc(0);
    }
    // ELECHOUSE_cc1101.setCRC_AF(true);
    if (rxtx > 0)
    {
        ELECHOUSE_cc1101.setPA(12);
        if (ELECHOUSE_cc1101.getMode() != 1)
        {
            ELECHOUSE_cc1101.SetTx();
        }
        Serial.println("transmitter on");
        delay(400);
    }
    else
    {
        rfconf.duration = dur;
        ELECHOUSE_cc1101.setSyncMode(0);  // Set the sync-word qualifier mode.
        ELECHOUSE_cc1101.setPktFormat(3); // Format of RX and TX data. 0 = Normal mode, use FIFOs for RX and TX. 1 = Synchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins. 2 = Random TX mode; sends random data using PN9 generator. Used for test. Works as normal mode, setting 0 (00), in RX. 3 = Asynchronous serial mode, Data in on GDO0 and data out on either of the GDOx pins.
        ELECHOUSE_cc1101.setRxBW(bw);
        ELECHOUSE_cc1101.setDRate(drate);
        if (raw_rx == 1)
        {
            Serial.println("Allocating sample");
            sample = (long *)malloc(samplesize * sizeof(long));
            enableReceive();
        }
        else
        {
            // if (ELECHOUSE_cc1101.getMode() != 2)
            ELECHOUSE_cc1101.SetRx();
        }
        delay(400);
    }

    if (ELECHOUSE_cc1101.getCC1101())
    { // Check the CC1101 SPI connection.
        Serial.println("CC1101: Connection OK");
    }
    else
    {
        Serial.println("CC1101: Connection Error");
        for (;;)
            ;
    }
    Serial.println("Transmission Mode: ");
    Serial.printf("%d", ELECHOUSE_cc1101.getMode());
    Serial.println();
}

void enableReceive()
{
    pinMode(RXPin, INPUT);
    RXPin = digitalPinToInterrupt(RXPin);
    ELECHOUSE_cc1101.SetRx();
    samplecount = 0;
    attachInterrupt(RXPin, receiver, CHANGE);
}
void receiver()
{
    const long time = micros();
    const unsigned int duration = time - lastTime;
    int rxinput = digitalRead(RXPin);
    if (duration > 100000)
    {
        samplecount = 0;
    }
    // Serial.print(rxinput);
    if (rxinput == 0)
    {
        sample[samplecount++] = duration;
    }
    else
    {
        sample[samplecount++] = -1 * duration;
    }
    // sample[samplecount++] = duration;

    if (samplecount >= samplesize)
    {
        detachInterrupt(RXPin);
        checkReceived();
    }
    lastTime = time;
}

bool checkReceived(void)
{
    delay(1);
    if (samplecount >= minsample && micros() - lastTime > 100000)
    {
        detachInterrupt(RXPin);
        return 1;
    }
    else
    {
        return 0;
    }
}

void recieveRFRAW(float freq, int mod, float dev, float drate, float bw, bool crc)
{
    raw_rx = 1;
    initializeRFConfig(freq, mod, dev, drate, bw, true, 0);
    if (sample == nullptr || sample == NULL)
    {
        Serial.println("Dynamic Allocation Failed");
        return;
    }
    unsigned long prMillis = millis();
    unsigned long maxScanDuration = 15000; // rfconf.duration * 1000; // Convert seconds to milliseconds
    samplecount = 0;
    lastTime = 0;
    displayStringCentre(display, "Scanning RF");
    // raw_rx = "1";
    do
    {

        if (checkReceived())
        {
            Serial.print("Count=");
            Serial.println(samplecount);
            Serial.println("");
            String f0 = "Filetype: Flipper SubGhz RAW File\nVersion: 1\n";
            f0 = f0 + "Frequency: " + String(long(rfconf.freq * 1000000)) + "\n";
            String preset = getF0Preset(rfconf.mod, rfconf.dev, rfconf.bw);
            f0 = f0 + "Preset: " + preset + "\n";
            if (preset == "Custom Preset")
            {
                f0 = f0 + "Modulation: " + String(rfconf.mod) + "\n";
                f0 = f0 + "Deviation: " + String(rfconf.dev) + "\n";
                f0 = f0 + "Bandwidth: " + String(rfconf.bw) + "\n";
            }
            f0 = f0 + "Protocol: " + rfconf.protocol + "\n";
            f0 = f0 + getf0RawDataString(sample, samplecount) + "\n";
            Serial.println(f0);
            displayStringCentre(display, "Captured RF");
            sendStringToClient(f0);
            delay(1500);
            displayStringCentre(display, "Scanning RF");
            enableReceive();
        }

    } while (millis() - prMillis < maxScanDuration);
    delay(500);
    raw_rx = 0;
    free(sample);
    displayLogo();
}

void sendRAW(long S[], int ssize)
{
    ELECHOUSE_cc1101.SetTx();
    delay(100);
    if (jam != 1)
        displayStringCentre(display, "Transmitting RAW RF");
    for (int i = 0; i < ssize - 1; i += 2)
    {
        if (S[i] < 0)
        {
            digitalWrite(Gdo0, LOW);
            delayMicroseconds(S[i] * -1);
        }
        else
        {
            digitalWrite(Gdo0, HIGH);
            delayMicroseconds(S[i]);
        }

        if (S[i + 1] < 0)
        {
            digitalWrite(Gdo0, LOW);
            delayMicroseconds(S[i + 1] * -1);
        }
        else
        {
            digitalWrite(Gdo0, HIGH);
            delayMicroseconds(S[i + 1]);
        }

        Serial.print(S[i]);
        Serial.print(" ");
    }
    Serial.println();
    if (jam != 1)
    {
        Serial.println("Transmitting RAW RF Ended");
        delay(500);
        displayLogo();
    }
}

void jamRF(int ch, int duration)
{
    jam = 1;
    displayStringCentre(display, "Jamming RF ...");
    delay(10);
    unsigned long dur = duration * 60000;
    unsigned long prMillis = millis();
    randomSeed(prMillis);
    long ccsendingbuffer[60] = {0};
    do
    {
        if (ch > 0)
        {
            for (int i = 0; i < ch; i++)
            {
                for (int j = 0; j < 60; j++)
                {
                    ccsendingbuffer[j] = (byte)random(1500);
                }
                ELECHOUSE_cc1101.setChannel(i);
                sendRAW(ccsendingbuffer, 60);
            }
        }

        else
        {

            for (int j = 0; j < 60; j++)
            {
                ccsendingbuffer[j] = (byte)random(1500);
            }
            ELECHOUSE_cc1101.setChannel(0);
            sendRAW(ccsendingbuffer, 60);
        }

    } while (millis() - prMillis < dur);
    jam = 0;
    digitalWrite(Gdo0, LOW);
    ELECHOUSE_cc1101.setChannel(0);
    displayLogo();
}

bool detectRF(int minRssi)
{
    displayStringCentre(display, "Detecting RF ...");
    const float hopperFrequencies[19] = {300.00, 303.87, 304.25, 310.00, 315.00, 318.00, 390.00, 418.00, 433.07, 433.92, 434.42, 434.77, 438.90, 868.3, 868.35, 868.865, 868.95, 915.00, 925.00};
    int rssi = -100;
    float frequency = 0.0;
    bool signalDetected = false;
    int modulation = 0;

    unsigned long prMillis = millis();
    do
    {
        for (int i = 4; i >= 0; i--)
        {
            if (i == 0 || i == 2)
            {
                // ELECHOUSE_cc1101.setModulation(i);
                for (float freq : hopperFrequencies)
                {
                    initializeRFConfig(freq, i, 47, 5, 270, 0, true, 0);
                    // ELECHOUSE_cc1101.setMHZ(freq);
                    int nrssi = ELECHOUSE_cc1101.getRssi();

                    if (nrssi >= rssi)
                    {
                        rssi = nrssi;
                        frequency = freq;
                        modulation = i;
                    }
                }
            }
            if (rssi >= minRssi)
            {
                signalDetected = true;
                break;
            }
        }

    } while (signalDetected == false && millis() - prMillis < 5000);
    if (signalDetected)
    {
        Serial.print("Frequency: ");
        Serial.print(frequency);
        Serial.print(" RSSI: ");
        Serial.println(rssi);
        String mod = "";
        switch (modulation)
        {
        case 0:
        {
            mod = "2-FSK";
            break;
        }
        case 1:
        {
            mod = "GFSK";
            break;
        }
        case 2:
        {
            mod = "ASK/OOK";
            break;
        }
        case 3:
        {
            mod = "4-FSK";
            break;
        }
        case 4:
        {
            mod = "MSK";
            break;
        }
        default:
            mod = "Unsupported Modulation";
            break;
        }
        String rf_disp = "\nRF Detected\n.....\nRSI: " + String(rssi) + "\n" + "Freq: " + String(frequency) + "\n" + "Mod: " + String(mod);
        displayString(display, rf_disp.c_str());
        sendStringToClient(rf_disp);
        delay(5000);
        displayLogo();
    }
    else
    {
        sendStringToClient("No Frequencies detected !");
        delay(3000);
        displayLogo();
    }
    return signalDetected;
}

String getF0Preset(float mod, float dev, float bw)
{
    if (mod == 2 && bw == 270 && dev == 2)
    {
        return "FuriHalSubGhzPresetOok270Async";
    }
    else if (mod == 2 && bw == 650 && dev == 2)
    {
        return "FuriHalSubGhzPresetOok650Async";
    }
    else if (mod == 0 && bw == 270 && dev == 2)
    {
        return "FuriHalSubGhzPreset2FSKDev238Async";
    }
    else if (mod == 0 && bw == 270 && dev == 47)
    {
        return "FuriHalSubGhzPreset2FSKDev476Async";
    }
    return "Custom Preset";
}
String getf0RawDataString(long S[], int ssize)
{
    String raw_data = "";
    if (ssize > 0)
    {
        raw_data = raw_data + "RAW_Data: ";
    }
    for (int i = 1; i < ssize; i++)
    {
        raw_data = raw_data + String(S[i]) + " ";
    }
    raw_data.trim();
    return raw_data;
}