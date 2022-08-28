/*
 * Rui Santos 
 * Complete Project Details http://randomnerdtutorials.com
*/

//#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0, 178);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

// Relay state and pin
String relay1State = "Off";
const int relay = 7;

String relay2State = "Off";
const int relay2 = 6;

// Client variables 
char linebuf[80];
int charcount=0;
const char head[] PROGMEM = "<!DOCTYPE HTML><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><body>";
const char end[] PROGMEM = "</body></html>";
String br = "<br/>";
void setup() { 
  // Relay module prepared 
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay2, HIGH);
  // Open serial communication at a baud rate of 9600
  Serial.begin(9600);
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  //Serial.println(Ethernet.getHostName());
}

// Display dashboard page with on/off button for relay
// It also print Temperature in C and F
void dashboardPage(EthernetClient &client) {
  client.println(head);                                                             
  client.println(F("<h3>Arduino Web Server - <a href=\"/\">Refresh</a></h3>"));
  // Generates buttons to control the relay
  client.println("<h4>Relay 1 - State: " + relay1State + "</h4>");
  // If relay is off, it shows the button to turn the output on          
  if(relay1State == "Off"){
    client.println(F("<a href=\"/relay1on\"><button>ON</button></a>"));
  }
  // If relay is on, it shows the button to turn the output off         
  else if(relay1State == "On"){
    client.println(F("<a href=\"/relay1off\"><button>OFF</button></a>"));                                                                    
  }
  //===============relay 2
  client.println("<h4>Relay 2 - State: " + relay2State + "</h4>");
  // If relay is off, it shows the button to turn the output on          
  if(relay2State == "Off"){
    client.println(F("<a href=\"/relay2on\"><button>ON</button></a>"));
  }
  // If relay is on, it shows the button to turn the output off         
  else if(relay2State == "On"){
    client.println(F("<a href=\"/relay2off\"><button>OFF</button></a>"));                                                                    
  }
  client.println(end); 
}

void handleConfig(EthernetClient &client) {
  client.println(head);
  // Generates buttons to control the relay
  client.println(F("<h4>Config</h4>"));
  client.println(F("<form action=\"\" method=\"GET\">"));
  client.println(F("<label for=\"iptxt\">IP  :</label>"));
  client.println("<input type=\"text\" id=\"iptxt\" name=\"ip\"/>");
  client.println(br);
  client.println(F("<label for=\"gttxt\">Gate:</label>"));
  client.println("<input type=\"text\" id=\"gttxt\" name=\"gate\"/>");
  client.println(br);
  client.println(F("<label for=\"mktxt\">Mask:</label>"));
  client.println("<input type=\"text\" id=\"mktxt\" name=\"mask\"/>");
  client.println(br);
  client.println(F("<label for=\"dntxt\">Dns :</label>"));
  client.println("<input type=\"text\" id=\"dntxt\" name=\"dns\"/>");
  client.println(br);
  client.println(F("<input type=\"submit\" id=\"btn\" value=\"Save\"/>"));
  client.println(F("</form>"));
  client.println(end); 
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    memset(linebuf,0,sizeof(linebuf));
    charcount=0;
    // an http request ends with a blank line
    while (client.connected()) {
      if (client.available()) {
       char c = client.read();
       //read char by char HTTP request
        linebuf[charcount]=c;
        if (charcount<sizeof(linebuf)-1) charcount++;
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n') {
          if(strstr(linebuf,"GET /config") > 0){
            Serial.println("config");
            handleConfig(client);
          }else{
            Serial.println("page");
            dashboardPage(client);
          } 
         
          if (strstr(linebuf,"GET /relay1off") > 0){
            digitalWrite(relay, HIGH);
            relay1State = "Off";
          }
          else if (strstr(linebuf,"GET /relay1on") > 0){
            digitalWrite(relay, LOW);
            relay1State = "On";
          } else if (strstr(linebuf,"GET /relay2off") > 0){
            digitalWrite(relay2, HIGH);
            relay2State = "Off";
          }
          else if (strstr(linebuf,"GET /relay2on") > 0){
            digitalWrite(relay2, LOW);
            relay2State = "On";
          } 
          memset(linebuf,0,sizeof(linebuf));
          charcount=0;   
          break;              
        } 
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}
