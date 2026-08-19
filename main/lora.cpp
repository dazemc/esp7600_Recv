#include "lora.h"
#include "events.h"

void initLoRa() {
  SPI.begin(SCK, MISO, MOSI, CS);
  LoRa.setPins(CS, RESET, DID0);

  if (!LoRa.begin(FREQ)) {
    Serial.println("LoRa failed");
    return;
  }
  Serial.println("LoRa started");
}

void onReceive(int packetSize) {
  if (packetSize) {
    Event event{};
    event.type = EVENT_LORA_RX;
    event.loraRecv.length = 0;
    event.loraRecv.rssi = LoRa.packetRssi();
    event.loraRecv.snr = LoRa.packetSnr();

    while (LoRa.available() &&
           event.loraRecv.length < sizeof(event.loraRecv.data)) {
      event.loraRecv.data[event.loraRecv.length++] = LoRa.read();
    }

    xQueueSend(loraRXQueue, &event, 0);
  }
}

void sendLoRaTask(void *arg) {

  Event event = {};
  // SendLoRaData *data = (SendLoRaData *)arg;
  while (true) {
    if (xQueueReceive(loraTXQueue, &event, portMAX_DELAY)) {
      switch (event.type) {
      case EVENT_VOLTAGE:
      case EVENT_LORA_SEND:
      case EVENT_LORA_RX:
      case EVENT_LORA_RECV:
        break;
      case EVENT_LORA_TX: {
        SendLoRaData data{
            .voltage = event.loraSend.voltage,
        };
        LoRa.beginPacket();
        LoRa.write((uint8_t *)&data, sizeof(data));
        LoRa.endPacket();
        // String str = String(event.voltageData.battery, 2);
        // snprintf(event.loraSend.voltage, sizeof(event.loraSend.voltage),
        // "%.2f", event.voltageData.battery);
        event.type = EVENT_LORA_SEND;
        event.loraSend.voltage = data.voltage;
        xQueueSend(displayQueue, &event, portMAX_DELAY);
        xQueueSend(serialQueue, &event, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));
        break;
      }
      case EVENT_WIFI:
        break;
      }
    }
  }
}

void receiveLoRaTask(void *arg) {
  Event event{};
  LoRa.onReceive(onReceive);
  LoRa.receive();
  while (true) {
    if (xQueueReceive(loraRXQueue, &event, portMAX_DELAY)) {
      switch (event.type) {
      case EVENT_WIFI:
      case EVENT_LORA_SEND:
      case EVENT_LORA_RECV:
      case EVENT_VOLTAGE:
      case EVENT_LORA_TX:
        break;
      case EVENT_LORA_RX: {
        if (event.loraRecv.length != sizeof(SendLoRaData)) {
          Serial.printf("Invalid packet size: %d\n", event.loraRecv.length);
          continue;
        }

        SendLoRaData packet{};

        memcpy(&packet, event.loraRecv.data, sizeof(packet));
        Event rxEvent;
        rxEvent.type = EVENT_LORA_RECV;
        rxEvent.loraSend = packet;
        xQueueSend(serialQueue, &rxEvent, portMAX_DELAY);
        xQueueSend(displayQueue, &rxEvent, portMAX_DELAY);
      }
      }
    }
  }
}
