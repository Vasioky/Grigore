#include <Ethernet.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEB };
char server[] = "192.168.0.177";
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 178);
IPAddress myDns(192, 168, 0, 1);

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

int pumpStatus = 0;

void setup() {
   Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

   Ethernet.begin(mac, ip, myDns);
     // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
}

void callSlave(String action){
  Serial.print("connecting to ");
  Serial.print(server);
  Serial.println("...");
  if (client.connect(server, 80)) {
    Serial.print(F("connected to "));
    Serial.println(client.remoteIP());
    // Make a HTTP request:
    String method = "/relay1" + action;
    Serial.print("call ");
    Serial.println(method); 
    client.println("GET " + method + " HTTP/1.1");
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println(F("connection failed"));
  }
  delay(1);
  client.println("Host: 192.168.0.177");
  client.println(F("Connection: close"));
  client.println();
  delay(1);
  client.stop();
  Serial.println(F("Connection closed"));
  pumpStatus = action == "on" ? 1 : 0;
}

String readResponse(){
  String result = "";
  while (client.connected() || client.available()) { //connected or data available
    char c = client.read(); //gets byte from ethernet buffer
    if(c == '\n'){
      break;
    }
    result+=c; 
  }
  return result;
}
  
bool ping(){
  Serial.println(F("ping"));
  Serial.print(F("connecting to "));
  Serial.print(server);
  Serial.println("...");
  if (client.connect(server, 80)) {
    Serial.print(F("connected to "));
    Serial.println(client.remoteIP());
    // Make a HTTP request:
    client.println(F("GET /ping HTTP/1.1"));
    client.println();
  } else {
    // if you didn't get a connection to the server:
    Serial.println(F("connection failed"));
  }
  String resp = readResponse();
  bool result = false;
  if (resp.indexOf("OK") != -1){
    result = true;
  }
  delay(1);
  client.println("Host: 192.168.0.177");
  client.println(F("Connection: close"));
  client.println();
  delay(1);
  client.stop();
  Serial.println(F("Connection closed"));
  return result;
}

unsigned long lastChecked = 0;
int interval = 1000 * 60 * 60;
void loop() {
  int lowLevel = digitalRead(2);
  int highLevel = digitalRead(3);
  Serial.print(" lowLevel:");
  Serial.print(lowLevel);
  Serial.print(" highLevel:");
  Serial.print(highLevel);
  Serial.print(" pump:");
  Serial.println(pumpStatus);
  if(lowLevel == HIGH && highLevel == LOW){
    //this is should not happen any time, but in case it happens we need to go in error status otherwise we get in loop wich starts and stopes the slave
    Serial.println("ERROR");
  } else {
     if(lowLevel == HIGH && pumpStatus == 0){
        Serial.println("turn onn");
        callSlave("on");
      } else if(highLevel == LOW && pumpStatus == 1){
        Serial.println("turn off");
        callSlave("off");
      }
  }
  
  if(lastChecked == 0){
    lastChecked = millis(); 
  } else if((millis() - lastChecked) > interval){
    lastChecked = millis();
    if(ping()){
      Serial.println("Ping: OK");
    }else{
      Serial.println("Ping: ERROR");
    }
  }
  delay(500);
}
