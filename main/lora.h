#pragma once

#include <LoRa.h>

#define FREQ 915E6
#define CS 18
#define SCK 5
#define MOSI 27
#define MISO 19
#define RESET 14
#define DID0 26

void initLoRa();
void onReceive(int packetSize);
void sendLoRaTask(void *arg);
void receiveLoRaTask(void *arg);

struct LoRaPacket {
  uint8_t data[256];
  int length;
  int rssi;
  float snr;
};

struct SendLoRaData {
  float voltage;
} __attribute__((packed));
