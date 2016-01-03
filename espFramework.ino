#include <SoftwareSerial.h>
#define WIFI_SSID "burton2G"
#define WIFI_PWD "customflyingv"
#define rxPin 10
#define txPin 11
#define THINGSPEAK_IP "184.106.153.149"
#define THINGSPEAK_API "P9LWYC4RKOSD8XSI"
#define DEBUG

#ifdef DEBUG
#define DEBUGMSG(x) Serial.println(x)
#else
#define DEBUGMSG(x)
#endif

SoftwareSerial esp(rxPin,txPin);



bool espSend(String cmd, char* replyIfOK) {
  DEBUGMSG(cmd);
  esp.println(cmd);
  bool bRetVal = esp.find(replyIfOK);
  DEBUGMSG(bRetVal? "ok" : "err");
  return bRetVal;
}

bool espConnectWifi() {
  
}

bool espInit() {
  if (!espSend("AT", (char*)"OK")) return false;
  
  // the esp needs to be in wifi station mode (1). This setting is remembered
  // after a powercycle; so probably already in correct setting. Check this,
  // and set to wifi mode if not yet in it...
  if (!espSend("AT+CWMODE?", (char*)"CWMODE:1")) {
    if (!espSend("AT+CWMODE=1", (char*)"OK")) return false;;
  }

  // Check if the esp is already connected to an access point and join the
  // access point (connect to the wifi)
  if (espSend("AT+CWJAP?", (char*)"No AP")) {
    if (!espSend(String("AT+CWJAP=\"") + WIFI_SSID + String("\",\"") + WIFI_PWD + String("\""), (char*)"OK")) return false;
  }

  // Set transfer mode to normal (0) if not already set so.
  // The transfer mode is remembered after a powercycle, so mostly would be good
  if (!espSend("AT+CIPMODE?", (char*)"CIPMODE:0")) {
    if (!espSend("AT+CIPMODE=0", (char*)"OK")) return false;
  }

  // To accept multiple IP streams, CIPMUX must be set to 1.
  // This setting is not preserved after a powercycle so must be set each time.
  if (!espSend("AT+CIPMUX=1", (char*)"OK")) return false;

  // if we made it here, the initialization has been successful. 
  return true;
}

bool espSendToThingspeak(double temp, double humidity) {
  bool bRetVal = false;
  
  if (espSend(String("AT+CIPSTART=0,\"TCP\",\"") + THINGSPEAK_IP + String("\",80"), (char*)"0,CONNECT")) {

    String getStr = "GET /update?api_key=";
    getStr += THINGSPEAK_API;
    getStr +="&field1=";
    getStr += String(temp);
    getStr +="&field2=";
    getStr += String(humidity);
    getStr += "\r\n";

    String cmd = "AT+CIPSEND=0,";
    cmd += String(getStr.length());
    if (espSend(cmd, (char*)">")) {
      bRetVal = espSend(getStr, (char*)"SEND OK");
    }
  
    espSend("AT+CIPCLOSE=0", (char*)"0,CLOSED");
  }

  return bRetVal;
}

void setup() {
  Serial.println("ESP8266 framework code");
  
  // define pin modes for tx, rx:
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  
  Serial.begin(9600);
  esp.begin(9600); 

  if (!espInit()) return;
}

void loop() {
  double temp = random(-1500, 4000) / 100.0;
  double humidity = random(200, 1000) / 10.0;
  espSendToThingspeak(temp, humidity);
  delay(60000);

}
