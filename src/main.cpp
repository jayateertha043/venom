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
    ;
  initializeModule(3);
  initializeWifi();
  
  //initializeModule(2); 
  //initializeModule(1); 
}

void loop()
{
  webSocket.loop();
}
