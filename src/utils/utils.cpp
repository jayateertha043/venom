#include "utils.h"


void initializeModule(int a)
{
  switch (a)
  {
  case 1:
  {
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522
    delay(25);          // Optional delay. Some board do need more time after init to be ready, see Readme
    mfrc522.PCD_AntennaOn();
    bool result = mfrc522.PCD_PerformSelfTest();
    if (!result)
    {
      Serial.println("Error: RFID Module Not Initialized");
      for (;;)
        ;
    }
    mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
    break;
  }
  case 2:
  {
    ELECHOUSE_cc1101.setSpiPin(18,19,23,5);
    //ELECHOUSE_cc1101.setSpiPin(14, 12, 13, 15);
    ELECHOUSE_cc1101.setGDO0(2); 
    ELECHOUSE_cc1101.Init();
    ELECHOUSE_cc1101.setRxBW(58);
    //ELECHOUSE_cc1101.setCCMode(1);     // Set config for internal transmission mode.
    ELECHOUSE_cc1101.setModulation(2); // Set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
    ELECHOUSE_cc1101.setDeviation(0);
    ELECHOUSE_cc1101.setDRate(5);
    ELECHOUSE_cc1101.setPktFormat(0);
    ELECHOUSE_cc1101.setMHZ(433);   // Set the basic frequency. The lib calculates the frequency automatically (default = 433.92 MHz). The CC1101 can operate in various frequency bands.
    ELECHOUSE_cc1101.setSyncMode(0);   // Set the sync-word qualifier mode.
    ELECHOUSE_cc1101.setCrc(0);
    //LECHOUSE_cc1101.setCRC_AF(true);
    //pinMode(26,INPUT);
    ELECHOUSE_cc1101.SetRx();
    delay(10);
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
    break;
  }
  case 3:
  {
  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println(F("Error: OLED Module Not Initialized"));
    
    for (;;)
      ; // Don't proceed, loop forever
  }
  delay(25);
  Serial.println("OLED Initialized...");
  displayLogo();
  break;
  }
  default:
  {
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522
    mfrc522.PCD_AntennaOn();
    delay(25); // Optional delay. Some board do need more time after init to be ready, see Readme
    bool result = mfrc522.PCD_PerformSelfTest();
    if (!result)
    {
      Serial.println("Error: RFID Module Not Initialized");
      for (;;)
        ;
    }
    mfrc522.PCD_DumpVersionToSerial();
  }
  }
}

// Via seriele monitor de bytes uitlezen in ASCI

void dump_byte_array1(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.write(buffer[i]);
  }
}

void dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void hexdump(const void *mem, uint32_t len, uint8_t cols)
{
  const uint8_t *src = (const uint8_t *)mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for (uint32_t i = 0; i < len; i++)
  {
    if (i % cols == 0)
    {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}