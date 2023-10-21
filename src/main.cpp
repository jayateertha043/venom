#include <SPI.h>
#include "./modules/rfid.h"
#include "./utils/utils.h"
#include "./modules/rf.h"
#include "./modules/display.h"
#include "./modules/web.h"

void setup()
{
  Serial.begin(9200); // Initialize serial communications with the PC
  while (!Serial)
  delay(25);
  initializeModule(3);
  delay(25);
  initializeWifi();
  delay(25);
  //initializeModule(2); 
  //initializeModule(1); 
}

void loop()
{
  webSocket.loop();
}
