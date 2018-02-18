#include <SoftwareSerial.h>

/**
 * Don't forget to:
 * 1. Set correct GPRS APN
 * 2. Set correct GPRS username
 * 3. Set correct GPRS password
 * 4. Set SIM800L TX/RX pins
 * 5. Set your server URL
 */

#define DEBUG 1
#define SIM800_TX_PIN D1 // TX pin of SIM800L
#define SIM800_RX_PIN D2 // RX pin of SIM800L

SoftwareSerial serialSIM800(SIM800_TX_PIN,SIM800_RX_PIN);
char responseBuffer[256];
 
char *initSim[]       = {
                          (char *)"AT\r\n", // Ping module
                        };
char *gprsStart[]     = {
                          (char *)"AT+SAPBR=3,1,\"Contype\", \"GPRS\"\r\n", // Configure bearer profile
                          (char *)"AT+SAPBR=3,1,\"APN\",\"internet.beeline.ru\"\r\n",
                          (char *)"AT+SAPBR=3,1,\"USER\",\"beeline\"\r\n",
                          (char *)"AT+SAPBR=3,1,\"PWD\",\"beeline\"\r\n",
                          (char *)"AT+SAPBR=1,1\r\n", // Open a GPRS context
                          (char *)"AT+SAPBR=2,1\r\n" // Query the GPRS context
                        };
char *gprsEnd[]       = { (char *)"AT+SAPBR=0,1\r\n" }; // Close a GPRS context

char *httpStart[]     = {
                          (char *)"AT+HTTPINIT\r\n", // Init HTTP service
                          (char *)"AT+HTTPPARA=\"CID\",1\r\n", // Set parameters for HTTP session
                          (char *)"AT+HTTPPARA=\"URL\",\"104.131.152.226:80/api/test/requestlog/index.php\"\r\n",
                          (char *)"AT+HTTPPARA=\"CONTENT\",\"image\"\r\n",
                          (char *)"AT+HTTPDATA=5,10000\r\n", // POST the data, params (size as bytes, input latency time ms)
                          (char *)"DEBUG", // POST Data
                          (char *)"AT+HTTPACTION=1\r\n", // POST Session start
                          (char *)"AT+HTTPREAD\r\n" // Read the data
                        };
char *httpEnd[]       = { (char *)"AT+HTTPTERM\r\n" }; // Terminate HTTP Service

void setup() {
  Serial.begin(115200);
  while(!Serial);
  serialSIM800.begin(115200);

  if(DEBUG) Serial.println("SETUP");
  delay(3000);
  initSimModule();
  setGprs();
  postData();
  terminateHttp();
  endGprs();
}

void loop() {
  delay(60000);
}

void initSimModule() {
  writeToSim(initSim, sizeof(initSim)/sizeof(*initSim));
}

void setGprs() {
  if(DEBUG) Serial.println("setGprs");
  writeToSim(gprsStart, sizeof(gprsStart)/sizeof(*gprsStart));
}

void endGprs() {
  if(DEBUG) Serial.println("endGprs");
  writeToSim(gprsEnd, sizeof(gprsEnd)/sizeof(*gprsEnd));
}

void postData() { // TODO Insert payload
  if(DEBUG) Serial.println("postData");
  writeToSim(httpStart, sizeof(httpStart)/sizeof(*httpStart));
}

void terminateHttp() {
  if(DEBUG) Serial.println("terminateHttp");
  writeToSim(httpEnd, sizeof(httpEnd)/sizeof(*httpEnd));
}

void writeToSim(char *commands[], int size) {
  for(int i = 0; i < size; i++) {
    serialSIM800.println(commands[i]);
    if(DEBUG) { Serial.print("Writing: "); Serial.println(commands[i]); }
    delay(1000);
    while(serialSIM800.available()) Serial.print(serialSIM800.read()); // Ignore written commands
  }
}

// Read Sim response until timeout
char* readSim(char* buffer, int timeout) {
  unsigned long timeStart = millis();
  int charCount = 0;

  while(1) {
    if(serialSIM800.available()) buffer[charCount++] = serialSIM800.read();

    if(millis() - timeStart > timeout) break;
  }
  buffer[charCount++] = 0;
  return buffer;
}
