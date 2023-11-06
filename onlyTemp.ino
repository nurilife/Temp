#include <EtherCard.h>
#include <DHT.h>

#define DHTPIN 2 // pino que estamos conectado
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// Ethernet interface ip address
static byte myip[] = { 192, 168, 0, 115 };
// Gateway ip address
static byte gwip[] = { 192, 168, 0, 1 };
// DNS ip address
static byte dnsip[] = { 210, 220, 163, 82 };
// Subnet mask
static byte mask[] = { 255, 255, 255, 0 };

// Ethernet mac address - must be unique on your network
static byte mymac[] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 };

// TCP/IP send and receive buffer
byte Ethernet::buffer[1000];

BufferFiller bfill;

const char okHeader[] PROGMEM =
  "HTTP/1.0 200 OK\r\n"
  "Content-Type: application/json\r\n"
  "Pragma: no-cache\r\n"
  "\r\n" ;

const char failHeader[] PROGMEM =
  "HTTP/1.0 404 Not Found\r\n"
  "Content-Type: text/html\r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "Pragma: no-cache\r\n"
  "\r\n" ;

const char responseHeader[] PROGMEM =
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: application/json\r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "\r\n" ;

int tipo_requisicao = 0;
unsigned long previousMillis = 0;
const long interval = 1000;

void setup() {
  // Open a serial connection for debugging
  Serial.begin(57600);
  Serial.println("\n[Debugger]");

  // Initialize the Ethernet interface
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println( "Failed to access Ethernet controller");

  // Setup IP statically
  ether.staticSetup(myip, gwip, dnsip, mask);

  // Print IP information out to serial line
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);

  dht.begin();
}

static word homePage() {
  bfill = ether.tcpOffset();

  float t = dht.readTemperature();
  char json_data[50];
  String json_str_data = "{ \"temperatura\": " + String(t) + "}";
  json_str_data.toCharArray(json_data, 50);
  bfill.emit_p(PSTR("$S"), json_data, responseHeader);
  return bfill.position();
}

void WebServer() {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if (pos) {
    tipo_requisicao = 1; // Both requests will display the same data
    ether.httpServerReply(homePage());
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    WebServer();
  }
}
