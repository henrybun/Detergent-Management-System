#include <SPI.h>
#include <WiFi101.h>
#include <HttpClient.h>
#include <math.h>
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
//IPAddress server(74,125,232,128);  // numeric IP for Google (no DNS)
char server[] = "detergent-checker.herokuapp.com";    // name address for Google (using DNS)
int port = 80;

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

int fsrPin[3] = { 0, 1, 2 };
//int buttonPin[3] = { 6, 7, 8 };
int LEDPin[3] = { 3, 4, 5 };
double fsrReading[3];
int weight[3];

void setup(void) {
  Serial.begin(9600);      // initialize serial communication
  for (int i = 0; i < 3; i++)
    pinMode(LEDPin[i], OUTPUT);
  WifiSetup();
}

void loop(void) {
  if (client.connect(server, port)) {
    Serial.println("connected to server");
    for (int i = 0; i < 3; i++)
      analogWrite(LEDPin[i], 0);
    for (int i = 0; i < 3; i++) {
      fsrReading[i] = analogRead(fsrPin[i]);
      weight[i] = calibration(fsrReading[i], i);
      analogWrite(LEDPin[i], 255 - weight[i]);
      Serial.println((String)(i + 1) + ". reading: " + fsrReading[i] + ", weight: " + weight[i]);
    }
    client.println((String)"GET /input.php?reading1=" + weight[0] + "&reading2=" + weight[1] + "&reading3=" + weight[2] + " HTTP/1.1");
    client.println("Host: detergent-checker.herokuapp.com");
    client.println("Connection: close");
    client.println();
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
  }
  delay(5000);//wait for respose, can be change
  client.stop();
  delay(10000);
}

void WifiSetup() { //used in setup()
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 5 seconds for connection:
    delay(5000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
  // you're connected now, so print out the status
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

int calibration(double reading, int index) {
  int weight;
  switch (index) {
    case 0: return 0;
    case 1:
      weight = 101384 * pow((1023 - reading), -1.028);
      break;
    case 2:
      weight = exp((reading + 629.93) / 230.96);
      break;
  }
  return weight;
}
