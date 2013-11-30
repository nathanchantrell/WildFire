/*
  WildFire Web Server
  A simple web server that shows the value of the analog input pins.
  
  Adapted from the Arduino ethernet example to work with the Wicked Device WildFire and
  Adafruit CC3000 library by Nathan Chantrell
 */

#include <Adafruit_CC3000.h>
#include <SPI.h>
#include <WildFire.h>
WildFire wf;

Adafruit_CC3000 cc3000 = Adafruit_CC3000(SPI_CLOCK_DIV2); // you can change this clock speed but DI

#define WLAN_SSID       "myNetwork"           // cannot be longer than 32 characters!
#define WLAN_PASS       "myPassword"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

// Initialize the CC3000 server library
// with the port you want to use 
// (port 80 is default for HTTP):
Adafruit_CC3000_Server server(80);

void setup() {

  wf.begin();

  Serial.begin(115200);
  Serial.println(F("Welcome to WildFire!\n")); 

  Serial.println(F("\nInitialising the CC3000 ..."));
  if (!cc3000.begin()) {
    Serial.println(F("Unable to initialise the CC3000! Check your wiring?"));
    for(;;);
  }
  
  Serial.println(F("\nDeleting old connection profiles"));
  if (!cc3000.deleteProfiles()) {
    Serial.println(F("Failed!"));
    while(1);
  }

  /* Attempt to connect to an access point */
  char *ssid = WLAN_SSID;             /* Max 32 chars */
  Serial.print(F("\nAttempting to connect to ")); Serial.println(ssid);
  
  /* NOTE: Secure connections are not available in 'Tiny' mode! */
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
   
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP()) {
    delay(100); // ToDo: Insert a DHCP timeout!
  }

  /* Display the IP address DNS, Gateway, etc. */  
  while (!displayConnectionDetails()) {
    delay(1000);
  }

  // Start listening for connections
  server.begin();
  
  Serial.println(F("Listening for connections..."));

 }


void loop() {
  // listen for incoming clients
  Adafruit_CC3000_ClientRef client = server.available();
  if (client) {
    Serial.println(F("new client"));
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you have got to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.fastrprintln("HTTP/1.1 200 OK");
          client.fastrprintln("Content-Type: text/html");
          client.fastrprintln("Connection: close");  // the connection will be closed after completion of the response
	  client.fastrprintln("Refresh: 5");  // refresh the page automatically every 5 sec
          client.fastrprintln("");
          client.fastrprintln("<!DOCTYPE HTML>");
          client.fastrprintln("<html>");
          // output the value of each analog input pin (note, 8 pins on WildFire not 6)
          for (int analogChannel = 0; analogChannel < 8; analogChannel++) {
            int sensorReading = analogRead(analogChannel);
            client.fastrprint("analog input ");
            char analogChannelC[1];
            client.fastrprint(itoa(analogChannel,analogChannelC,10));
            client.fastrprint(" is ");
            char sensorReadingC[1];
            client.fastrprint(itoa(sensorReading,sensorReadingC,10));
            client.fastrprintln("<br />");       
          }
          client.fastrprintln("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.close();
    Serial.println(F("client disonnected"));
  }
}

/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}
