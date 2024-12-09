#include <WebServer.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

// Network credentials
const char* ssid = "";
const char* password = "";

int MQ = 33;


// UDP setup
WiFiUDP Udp;
unsigned int localUdpPort = 4210; // Port to listen on
char incomingPacket[255];         // Buffer for incoming packets

// I2C setup
const int arduino_addr = 1;

// Variables to store the remote IP and port
IPAddress remoteIP;
unsigned int remotePort = 4220;

// Functions to handle robot commands
void Forward() {
  Wire.beginTransmission(arduino_addr);
  Wire.write("F");
  Wire.endTransmission();
  Serial.println("FORWARD");
}

void Backward() {
  Wire.beginTransmission(arduino_addr);
  Wire.write("B");
  Wire.endTransmission();
  Serial.println("BACKWARD");
}

void Left() {
  Wire.beginTransmission(arduino_addr);
  Wire.write("L");
  Wire.endTransmission();
  Serial.println("LEFT");
}

void Right() {
  Wire.beginTransmission(arduino_addr);
  Wire.write("R");
  Wire.endTransmission();
  Serial.println("RIGHT");
}

void Brake() {
  Wire.beginTransmission(arduino_addr);
  Wire.write("P");
  Wire.endTransmission();
  Serial.println("BRAKE");
}

String Poluare() {
  return String(map(analogRead(MQ), 0, 4095, 0, 100));
}

void setup() {
  Serial.begin(9600);
  Wire.begin(SDA, SCL);

  pinMode(MQ, INPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());

  // Start UDP
  Udp.begin(localUdpPort);
  Serial.printf("Listening for UDP packets on port %d\n", localUdpPort);
}

void loop() {
  // Check for incoming packets
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // Store the remote IP and port for sending data back
    remoteIP = Udp.remoteIP();
    remotePort = Udp.remotePort();

    int len = Udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;  // Null-terminate the string
    }

    char comm = incomingPacket[0];
    Serial.printf("Received command: %c\n", comm);

    // Execute commands based on received character
    if (comm == 'W') Forward();
    else if (comm == 'S') Backward();
    else if (comm == 'A') Left();
    else if (comm == 'D') Right();
    else if (comm == 'B') Brake();
  }

  // Send the "contor" value to the Python program
  if (remotePort != 0) {  // Check if the remote port has been set
    Udp.beginPacket(remoteIP, remotePort);
    Serial.println(Poluare());
    Udp.printf("%d", Poluare());
    Udp.endPacket();
  }
}
