#ifndef RF_H
#define RF_H

#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include "display.h"
void initializeRFConfig(float freq = 433.92, int mod = 2, float dev = 0, float drate = 5, float bw = 58, int dur = 15, bool crc = true, int rxtx = 0);
void recieveRFRAW(float freq, int mod, float dev, float drate, float bw, bool crc);
void enableReceive();
void receiver();
bool checkReceived(void);
void sendRAW(long S[], int ssize);
void jamRF(int ch, int duration);
bool detectRF(int minRssi = -80);
String getf0RawDataString(long S[], int ssize);
String getF0Preset(float mod, float dev, float bw);
#endif