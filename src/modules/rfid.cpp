#include "rfid.h"
#include "web.h"

int MAX_COL = 512;

MFRC522::MIFARE_Key key;
MFRC522 mfrc522(SS_PIN, RST_PIN);
// Known keys, see: https://code.google.com/p/mfcuk/wiki/MifareClassicDefaultKeys
const byte knownKeys[NR_KNOWN_KEYS][MFRC522::MF_KEY_SIZE] = {
    {0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF},
    {0XD3, 0XF7, 0XD3, 0XF7, 0XD3, 0XF7},
    {0X00, 0X00, 0X00, 0X00, 0X00, 0X00},
    {0XA0, 0XA1, 0XA2, 0XA3, 0XA4, 0XA5},
    {0XB0, 0XB1, 0XB2, 0XB3, 0XB4, 0XB5},
    {0X00, 0X00, 0X00, 0X00, 0X00, 0X00},
    {0X4D, 0X3A, 0X99, 0XC3, 0X51, 0XDD},
    {0X1A, 0X98, 0X2C, 0X7E, 0X45, 0X9A},
    {0XAA, 0XBB, 0XCC, 0XDD, 0XEE, 0XFF},
};
byte newuidBuffer[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
byte newuidBufferSize = 4;
// byte waarde[256][16];
byte **waarde;
int maxBlocks = 256;
int blockSize = 16;
void ReadRFID(MFRC522 &mfrc522)
{

  // Dynamic Allocate Waarde
  waarde = (byte **)malloc(maxBlocks * sizeof(byte *));
  for (int i = 0; i < maxBlocks; i++)
  {
    waarde[i] = (byte *)malloc(blockSize * sizeof(byte));
  }

  initializeModule(1);
  bool result = mfrc522.PCD_PerformSelfTest();
  mfrc522.PCD_DumpVersionToSerial();
  displayStringCentre(display, "Place Your RFID Tag");
  Serial.println("Read RFID");
  Serial.println("...........");
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
  {
    display.clearDisplay();
    displayStringCentre(display, "Scanning ...");
    // mfrc522.PICC_DumpToSerial(&mfrc522.uid);
    String uid = "";
    newuidBufferSize = mfrc522.uid.size;
    for (byte i = 0; i < newuidBufferSize; i++)
    {
      newuidBuffer[i] = mfrc522.uid.uidByte[i];
      uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    int blockSize = 0;
    // Read the card's type
    String cardType = "";
    switch (piccType)
    {
    case MFRC522::PICC_TYPE_MIFARE_1K:
    {
      cardType = "MIFARE Classic Protocol, 1KB";
      blockSize = 64;
      break;
    }
    case MFRC522::PICC_TYPE_MIFARE_DESFIRE:
      cardType = "PICC_TYPE_MIFARE_DESFIRE";
      break;
    case MFRC522::PICC_TYPE_MIFARE_PLUS:
      cardType = "MIFARE Plus";
      break;
    case MFRC522::PICC_TYPE_MIFARE_UL:
      cardType = "MIFARE Ultralight or Ultralight C";
      break;
    case MFRC522::PICC_TYPE_MIFARE_4K:
    {
      cardType = "MIFARE Classic protocol, 4KB";
      blockSize = 256;
      break;
    }

    case MFRC522::PICC_TYPE_MIFARE_MINI:
    {
      cardType = "MIFARE Classic protocol, 320 bytes";
      blockSize = 20;
      break;
    }

    case MFRC522::PICC_TYPE_ISO_18092:
      cardType = "ISO/IEC 18092";
      break;
    case MFRC522::PICC_TYPE_ISO_14443_4:
      cardType = "PICC Type 4";
      break;
    default:
      cardType = "Unknown";
      break;
    }
    if (piccType == MFRC522::PICC_TYPE_MIFARE_1K || piccType == MFRC522::PICC_TYPE_MIFARE_4K || piccType == MFRC522::PICC_TYPE_MIFARE_MINI)
    {
      for (byte k = 0; k < NR_KNOWN_KEYS; k++)
      {
        for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++)
        {
          key.keyByte[i] = knownKeys[k][i];
        }
        // Try the key
        if (try_keyR(&key, blockSize))
        {
          break;
        }
        else
        {
          // mfrc522.PICC_HaltA(); // Halt PICC
          Serial.println("Error: RFID Doesn't use default keys, Unable to decrypt data blocks !");
        }
      }

      send2DataToClient(waarde, blockSize, 16, 1);
      // Display the UID on the OLED display
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("RFID Tag Info");
      display.println("..............");
      display.println("UID: " + uid);
      display.println("Type: " + cardType);
      display.setTextSize(1);
      display.display();
      delay(7000);
      display.clearDisplay();
      displayLogo();
    }
  }

  // Free Up Waarde
  for (int i = 0; i < maxBlocks; i++)
  {
    free(waarde[i]);
  }
  free(waarde);
}

void WriteRFID(MFRC522 &mfrc522, int bSize, byte w[][16])
{
  initializeModule(1);
  displayStringCentre(display, "Place Tag to Write");
  delay(3000);
  Serial.println("Write RFID");
  Serial.println("...........");
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
  {
    // Show some details of the PICC (that is: the tag/card)
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));

    int blockSize = 0;
    String cardType = "";
    switch (piccType)
    {
    case MFRC522::PICC_TYPE_MIFARE_1K:
    {
      cardType = "MIFARE Classic Protocol, 1KB";
      blockSize = 64;
      break;
    }
    case MFRC522::PICC_TYPE_MIFARE_DESFIRE:
      cardType = "PICC_TYPE_MIFARE_DESFIRE";
      break;
    case MFRC522::PICC_TYPE_MIFARE_PLUS:
      cardType = "MIFARE Plus";
      break;
    case MFRC522::PICC_TYPE_MIFARE_UL:
      cardType = "MIFARE Ultralight or Ultralight C";
      break;
    case MFRC522::PICC_TYPE_MIFARE_4K:
    {
      cardType = "MIFARE Classic protocol, 4KB";
      blockSize = 256;
      break;
    }

    case MFRC522::PICC_TYPE_MIFARE_MINI:
    {
      cardType = "MIFARE Classic protocol, 320 bytes";
      blockSize = 20;
      break;
    }

    case MFRC522::PICC_TYPE_ISO_18092:
      cardType = "ISO/IEC 18092";
      break;
    case MFRC522::PICC_TYPE_ISO_14443_4:
      cardType = "PICC Type 4";
      break;
    default:
      cardType = "Unknown";
      break;
    }
    if (piccType == MFRC522::PICC_TYPE_MIFARE_1K || piccType == MFRC522::PICC_TYPE_MIFARE_4K || piccType == MFRC522::PICC_TYPE_MIFARE_MINI)
    {
      displayStringCentre(display, "Writing to RFID Tag ...");
      Serial.println(bSize);
      Serial.println(blockSize);
      if (bSize != blockSize)
      {
        Serial.println("There's some error with the data received");
        displayStringCentre(display, "Error: RFID Write Failed ...");
        delay(5000);
        displayLogo();
        return;
      }
      MFRC522::MIFARE_Key key;
      for (int k = 0; k < NR_KNOWN_KEYS; k++)
      {
        // Copy the known key into the MIFARE_Key structure
        for (int i = 0; i < MFRC522::MF_KEY_SIZE; i++)
        {
          key.keyByte[i] = knownKeys[k][i];
        }
        try_keyW(&key, blockSize, w);
      }
    }
    Serial.println();
    mfrc522.PICC_HaltA();      // Halt PICC
    mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD
  }
  delay(5000);
  displayLogo();
}

bool try_keyR(MFRC522::MIFARE_Key *key, int blockSize)
{
  bool result = false;
  byte buffer[18];
  byte block = 0;
  MFRC522::StatusCode status, status1;
  for (int i = 0; i < blockSize; i++)
  {
    block = i;
    // Serial.println(F("Authenticating using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, i, key, &(mfrc522.uid));
    status1 = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, i, key, &(mfrc522.uid));
    // mfrc522.PICC_DumpMifareClassicSectorToSerial(&mfrc522.uid,key,i);
    if (status != MFRC522::STATUS_OK && status1 != MFRC522::STATUS_OK)
    {
      // Serial.print(F("PCD_Authenticate() failed: "));
      // Serial.println(mfrc522.GetStatusCodeName(status));
      return false;
    }
    // Read block
    byte byteCount = sizeof(buffer);
    status = mfrc522.MIFARE_Read(i, buffer, &byteCount);
    if (status != MFRC522::STATUS_OK)
    {
      for (int p = 0; p < 16; p++) // De 16 bits uit de block uitlezen
      {
        waarde[block][p] = '-';
      }
      dump_byte_array(waarde[block], 16);
      Serial.println();
      // Serial.print(F("MIFARE_Read() failed: "));
      // Serial.println(mfrc522.GetStatusCodeName(status));
    }
    else
    {
      // Successful read
      result = true;
      Serial.println("Success with key:");
      dump_byte_array((*key).keyByte, MFRC522::MF_KEY_SIZE);
      Serial.println();
      // Dump block data
      Serial.print(F("Block "));
      Serial.print(i);
      Serial.print(F(":"));
      for (int p = 0; p < 16; p++) // De 16 bits uit de block uitlezen
      {
        waarde[block][p] = buffer[p];
        // Serial.print(waarde[block][p]);
      }
      // dump_byte_array(waarde[block], 16);
      Serial.println();
    }
    // Serial.println();
  }
  mfrc522.PICC_HaltA();      // Halt PICC
  mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD
  return result;
}

bool try_keyW(MFRC522::MIFARE_Key *key, int blockSize, byte w[][16])
{
  bool result = true;
  byte buffer[18];
  int block = 0;
  MFRC522::StatusCode status, status1;
  for (int i = 4; i < blockSize; i++)
  {
    block = i;
    if (i % 4 == 3)
    {
      continue;
    }

    // Serial.println();
    //  Serial.println(F("Authenticating using key A..."));
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, key, &(mfrc522.uid));
    status1 = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, block, key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK && status1 != MFRC522::STATUS_OK)
    {
      // Serial.print(F("PCD_Authenticate() failed: "));
      // Serial.println(mfrc522.GetStatusCodeName(status));
      return false;
    }
    // dump_byte_array(waarde[block], 16);
    status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(block, w[block], 16);
    if (status != MFRC522::STATUS_OK)
    {
      Serial.println(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      result = false;
    }
    else
    {
      // Serial.println("RFID Write Success");
    }
  }
  if (result)
  {
    displayStringCentre(display, "RFID Write Complete");
    delay(5000);
  }
  else
  {
    displayStringCentre(display, "RFID Write Failed");
    delay(5000);
  }

  mfrc522.PICC_HaltA();      // Halt PICC
  mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD
  return result;
}

void unBrickTag(MFRC522 &mfrc522)
{
  displayString(display, "Place Tag to UnBrick");
  if (mfrc522.MIFARE_UnbrickUidSector(true))
  {
    displayString(display, "Tag Unbricked Successfully !");
    Serial.println("Tag Unbricked Successfully !");
  }
  else
  {
    Serial.println("Tag Unbrick Failed !");
    displayString(display, "Tag Unbrick Failed !");
  }
  delay(10000);
  displayLogo();
}
void copyUID(MFRC522 &mfrc522)
{
  displayString(display, "Place Tag to COPY UID to");
  if (mfrc522.MIFARE_SetUid(newuidBuffer, newuidBufferSize, true))
  {
    Serial.println("UID Written Successfully");
    displayString(display, "UID Written Successfully");
  }
  else
  {
    Serial.println("Error: Couldn't Write UID");
    displayString(display, "Error: Couldn't Write UID");
  }
  delay(5000);
  displayLogo();
}
