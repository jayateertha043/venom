#include "web.h"
#include "rf.h"
#include "rfid.h"
#include "deauth.h"
#include "sourapple.h"

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const char *ssid = "VENOM";        // Enter SSID here
const char *password = "00000000"; // Enter Password here
int client1 = -1;
bool isFirstClient = false;
WebSocketsServer webSocket(81);
AsyncWebServer server(80);
DynamicJsonDocument doc(40000);

int getFirstClientID()
{
  return client1;
}

void processCommands(char *command, uint8_t num)
{
  Serial.println("Command Recieved: ");
  /*int clen=0;
  clen = sizeof(command)/sizeof(char*);
  //This might slow down process
  if(clen<200){
  Serial.println(command);
  }*/
  deserializeJson(doc, command);
  String action = doc["action"];
  Serial.println("Action: ");
  Serial.println(action);
  if (action == "READRFID")
  {
    Serial.println("READING RFID");
    ReadRFID(mfrc522);
  }
  else if (action == "WRITERFID")
  {
    Serial.println("Writing RFID");
    if (!doc["data"].isNull())
    {
      JsonArray data = doc["data"];
      const uint8_t numRows = data.size();
      const uint8_t numCols = data[0].size();
      if (numCols != 16)
      {
        Serial.println("Data Column Length Mismatch");
        return;
      }
      byte localArray[numRows][16];
      for (int i = 0; i < numRows; i++)
      {
        for (int j = 0; j < numCols; j++)
        {
          localArray[i][j] = data[i][j];
        }
      }
      Serial.println();
      WriteRFID(mfrc522, numRows, localArray);
    }
  }
  else if (action == "COPYUID")
  {
    Serial.println("COPYING RFID");
    copyUID(mfrc522);
  }
  else if (action == "UNBRICK")
  {
    Serial.println("UNBRICK Tag");
    unBrickTag(mfrc522);
  }
  else if (action == "RECIEVERF")
  {
    float freq = doc["config"]["frequency"];
    float mod = doc["config"]["modulation"];
    float drate = doc["config"]["datarate"];
    float dev = doc["config"]["deviation"];
    float bw = doc["config"]["bandwidth"];
    recieveRFRAW(freq, mod, dev, drate, bw, true);
  }
  else if (action == "TRANSMITRF")
  {
  }
  else if (action == "TRANSMITRFRAW")
  {
    Serial.println("Transmitting RAW RF");
    float freq = doc["config"]["frequency"];
    float mod = doc["config"]["modulation"];
    float drate = doc["config"]["datarate"];
    float dev = doc["config"]["deviation"];
    float bw = doc["config"]["bandwidth"];
    String preset = doc["config"]["Preset"];
    int raw_size = doc["config"]["rawSize"];
    Serial.println(raw_size);
    if (raw_size > 0)
    {
      initializeRFConfig(freq, mod, dev, drate, bw, 0, true, 1);
      delay(25);

      if (!doc["payload"].isNull())
      {
        JsonArray data = doc["payload"];
        const uint8_t numRows = data.size();
        long localArray[1000];

        for (int i = 0; i < numRows; i++)
        {
          const uint8_t numCols = data[i].size();
          Serial.println(numCols);
          for (int j = 0; j < numCols; j++)
          {
            localArray[j] = data[i][j];
          }
          sendRAW(localArray, numCols);
        }
        delay(3000);
        displayLogo();
      }
      else
      {
        Serial.println("No Payload");
      }
    }
  }
  else if (action == "JAMRF")
  {
    float freq = doc["config"]["frequency"];
    float mod = doc["config"]["modulation"];
    float drate = doc["config"]["datarate"];
    float dev = doc["config"]["deviation"];
    float bw = doc["config"]["bandwidth"];
    int ch = doc["config"]["channel"];
    int duration = doc["config"]["duration"];
    Serial.printf("\nChannel: %d\n", ch);
    Serial.printf("\nDuration: %d\n", duration);
    Serial.println();
    initializeRFConfig(freq, mod, dev, drate, bw, duration, true, 1);
    jamRF(ch, duration);
  }
  else if (action == "DETECTRF")
  {
    initializeRFConfig(433, 2, 47, 5, 270, true, 0);
    detectRF();
  }
  else if (action == "DEAUTH")
  {
    if (doc.containsKey("BSSID"))
    {
      String BSSID_STR = doc["BSSID"];
      const char *bssid_p = BSSID_STR.c_str();
      Serial.println("Sending Deauth Frame for BSSID:\t");
      Serial.print(BSSID_STR);
      Serial.println();
      sendDeAuth(BSSID_STR);
    }
    else
    {
      getAvailableWifi();
    }
  }
  else if (action == "SOURAPPLE")
  {
    startSourApple();
  }
  else
  {
    // webSocket.sendTXT(num, "Unknown action");
  }
  doc.clear();
}

void initializeWifi()
{
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length = 16)
{

  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\n", num);
    break;
  case WStype_CONNECTED:
  {
    IPAddress ip = webSocket.remoteIP(num);
    if (!isFirstClient && client1 != num)
    {
      client1 = num;
      isFirstClient = true;
    }

    Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s clientid: %d\n", num, ip[0], ip[1], ip[2], ip[3], payload, client1);
    // send message to client
    webSocket.sendTXT(num, "Connected");
  }
  break;
  case WStype_TEXT:
  {
    if (num == client1)
    {
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String payload_str = String((char *)payload);
      processCommands((char *)payload, num);
    }

    break;
  }
  case WStype_BIN:
  {
    if (num == client1)
    {
      Serial.printf("[%u] get binary length: %u\n", num, length);
      hexdump(payload, length, 16);
      // send message to client
      // webSocket.sendBIN(num, payload, length);
    }

    break;
  }

  case WStype_ERROR:
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
    break;
  }
}

void sendDataToClient(byte data[], size_t dataSize, uint8_t clientID)
{
  // Create a dynamic JSON document
  // DynamicJsonDocument doc(20000);
  doc.clear();
  Serial.println(dataSize);
  // Copy the byte array to the JSON document
  for (size_t i = 0; i < dataSize; i++)
  {
    doc.add(data[i]);
  }
  // Serialize the JSON document to a string
  String jsonStr;
  serializeJson(doc, jsonStr);
  // Serial.println(jsonStr);
  // Serial.println(doc.size());
  //  Send the JSON string to the client
  webSocket.sendTXT(client1, jsonStr);
  doc.clear();
}

void send2DataToClient(byte **data, size_t rows, size_t cols, uint8_t clientID)
{
  // Create a dynamic JSON document
  // DynamicJsonDocument doc(40000);
  doc.clear();
  // Add each element of the 2D array to the JSON array
  for (size_t i = 0; i < rows; i++)
  {
    Serial.println();
    // Create a JSON subarray for each row
    JsonArray subArray = doc.createNestedArray();
    for (size_t j = 0; j < cols; j++)
    {
      // Add each element of the row to the subarray
      subArray.add(data[i][j]);
      Serial.print(data[i][j]);
      Serial.print(" ");
    }
  }
  Serial.println();
  // Serialize the JSON document to a string
  String jsonStr;
  serializeJson(doc, jsonStr);
  // Serial.println("Sending 2D Array");
  // Serial.println(doc.size());
  //  Send the JSON string to the client
  webSocket.sendTXT(client1, jsonStr);
  doc.clear();
}

void sendStringToClient(String message)
{
  webSocket.sendTXT(client1, message);
}