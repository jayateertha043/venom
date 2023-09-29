#ifndef RFID_H
#define RFID_H

#include <MFRC522.h>
#include "../utils/utils.h"
#include "display.h"

#define NR_KNOWN_KEYS 9
#define RST_PIN 0 // Configurable, see typical pin layout above
#define SS_PIN 5  // Configurable, see typical pin layout above

extern MFRC522 mfrc522; // Create MFRC522 instance

extern bool RFIDREAD;
extern bool RFIDWRITE;

void ReadRFID(MFRC522 &mfrc522);
void WriteRFID(MFRC522 &mfrc522, int bSize, byte w[][16]);
bool try_keyR(MFRC522::MIFARE_Key *key, int blockSize);
bool try_keyW(MFRC522::MIFARE_Key *key, int blockSize, byte w[][16]);
void unBrickTag(MFRC522 &mfrc522);
void copyUID(MFRC522 &mfrc522);

#endif // RFID_H