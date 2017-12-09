#include "src/ethercard/EtherCard.h"
#include "src/ethercard/net.h"
#include "src/ethercard/enc28j60.h"
#include "src/VS1053/VS1053.h"

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
//  player.modeSwitch();
  player.setVolume(0);

}
