#include <EtherCard.h>
#include <net.h>
#include <enc28j60.h>
#include <VS1053.h>

uint8_t Ethernet::buffer[1400]; // configure buffer size to 700 octets
static uint8_t mymac[] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 }; // define (unique on LAN) hardware (MAC) address

const int   stationPort1 = 8080;
const int   stationPort2 = 80;
const int   stationPort3 = 80;

static uint32_t timer;

//VS1053( uint8_t _cs_pin, uint8_t _dcs_pin, uint8_t _dreq_pin, uint8_t _reset_pin);
VS1053  player(9, 6, 7, 8);

const char website1[] PROGMEM = "stream.morow.com";
const char website2[] PROGMEM = "swr-mp3-m-swr3.akacast.akamaistream.net";
const char website3[] PROGMEM = "stream-eu1.radioparadise.com";

//the length of the buffer between the meta info in stream
//aka the value of 'icy-metaint'
int metaLen = -1;

boolean headerParsed = false;

char * getHeaderValue(char *line, char *key) {

  char* line_copy = malloc(strlen(line));
  strcpy(line_copy, line);

  int compare = strncmp(line, key, strlen(key));

  if (compare != 0) {
    return NULL;
  }

  char* s = strtok(line_copy, ":");
  s = strtok(NULL, "\n");

  free(line_copy);

  return s;
}

int parseHTTPHeader(int *position, int maxLength) {

  char *ptr;
  char *startPtr;

  //position in the buffer
  int pos = position;

  //length of a single line
  int lineLength = -1;

  while (lineLength != 0) {

    startPtr = &Ethernet::buffer[pos];

    for (ptr = startPtr ; *ptr != '\n'; ptr++) {
      lineLength = ptr - startPtr;
    }

    char line[lineLength];
    memcpy(line, startPtr, lineLength);

    line[lineLength] = '\0';

    Serial.println(*position);
    Serial.println(maxLength);
    Serial.print(line);
    Serial.println("|");
    Serial.println(lineLength);

    if (lineLength != 0 && metaLen == -1) {

      char *value = getHeaderValue (line, "icy-metaint");
      if (value != NULL) {
        metaLen = atoi(value);
      }
    }

    pos += (lineLength + 2);

    if (pos >= (position + maxLength)) {
      Serial.println("length check problem");
      break;
    }

  }

  headerParsed = true;
  return pos;
}

int count = 0;

static void my_callback (byte status, word off, word len)
{
  if (count > 3) {
    uint8_t* data = (uint8_t *) Ethernet::buffer + off; //Get the data stream from ENC28J60 and...
    //Serial.println("play chunk");
    player.playChunk(data, len);
    count = 4;
  }

  count++;
}

void loop() {

  ether.packetLoop(ether.packetReceive());

  if (millis() > timer) {
    timer = millis() + 10000;
    Serial.println();
    Serial.print("<<< REQ ");

  }

}

void setup() {
  Serial.begin(57600);

  for (byte i = 0; i < 6; ++i) {
    Serial.print(mymac[i], HEX);
    if (i < 5)
      Serial.print(':');
  }

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println(F("Failed to access Ethernet controller"));

  Serial.println(F("Setting up DHCP"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));

  ether.persistTcpConnection(true);

  ether.printIp("My IP: ", ether.myip);
  ether.printIp("Netmask: ", ether.netmask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);

  if (!ether.dnsLookup(website1))
    Serial.println("DNS failed");

  ether.hisport = stationPort1;
  //  ether.copyIp(ether.hisip, stationIP);

  ether.printIp("HIS  IP: ", ether.hisip);

  ether.browseUrl(PSTR("/morow_med.mp3"), "", website1, "", my_callback);
  //ether.browseUrl(PSTR("/7/720/137136/v1/gnl.akacast.akamaistream.net/swr-mp3-m-swr3"), "", website2, "", my_callback);
  //ether.browseUrl(PSTR("/mp3-128"), "", website3, "", my_callback);

  player.begin();
  player.modeSwitch();
  player.setVolume(0);

}
