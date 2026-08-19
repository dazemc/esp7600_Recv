#include <Arduino.h>
#include <LoRa.h>
#include <ssd1306.h>
#include <string>

#include "driver/i2c_master.h"

#define BAUD 115200
#define FREQ 915E6
#define CS 18
#define SCK 5
#define MOSI 27
#define MISO 19
#define RESET 14
#define DID0 26
#define OLED_SDA GPIO_NUM_4
#define OLED_SCL GPIO_NUM_15

QueueHandle_t eventQueue;

enum EventType {
  EVENT_LORA,
  EVENT_VOLTAGE,
  EVENT_WIFI,
};

struct LoRaPacket {
  uint8_t data[256];
  int length;
  int rssi;
  float snr;
};

struct Event {
  EventType type;
  union {
    LoRaPacket lora;
  };
};

i2c_master_bus_config_t bus_config = {
    .i2c_port = I2C_NUM_0,
    .sda_io_num = OLED_SDA,
    .scl_io_num = OLED_SCL,
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 7,
    .intr_priority = 0,
    .trans_queue_depth = 0,
    .flags =
        {
            .enable_internal_pullup = true,
            .allow_pd = false,
        },
};

void onRecieve(int packetSize) {
  if (packetSize) {
    Event event{};
    event.type = EVENT_LORA;
    event.lora.length = 0;
    event.lora.rssi = LoRa.packetRssi();
    event.lora.snr = LoRa.packetSnr();

    while (LoRa.available() && event.lora.length < sizeof(event.lora.data)) {
      event.lora.data[event.lora.length++] = LoRa.read();
    }

    xQueueSend(eventQueue, &event, 0);
  }
}

extern "C" void app_main() {
  initArduino();

  Serial.begin(BAUD);
  delay(100);
  SPI.begin(SCK, MISO, MOSI, CS);
  LoRa.setPins(CS, RESET, DID0);

  if (!LoRa.begin(FREQ)) {
    Serial.println("LoRa failed");
    return;
  }

  eventQueue = xQueueCreate(10, sizeof(Event));

  Serial.println("LoRa started");
  LoRa.onReceive(onRecieve);
  LoRa.receive();
  Event event;

  i2c_master_bus_handle_t bus_handle;

  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

  ssd1306_config_t cfg{};

  cfg.bus = SSD1306_I2C;
  cfg.width = 128;
  cfg.height = 64;

  cfg.iface.i2c.port = I2C_NUM_0;
  cfg.iface.i2c.addr = 0x3C;
  cfg.iface.i2c.rst_gpio = GPIO_NUM_16;

  ssd1306_handle_t disp;
  ESP_ERROR_CHECK(ssd1306_new_i2c(&cfg, &disp));
  int x = 0;
  int y = 0;
  while (true) {
    ssd1306_clear(disp);
    ssd1306_draw_text(disp, x, y, "Waiting...", true);
    ssd1306_display(disp);

    if (xQueueReceive(eventQueue, &event, portMAX_DELAY)) {
      switch (event.type) {
      case EVENT_LORA: {
        std::string rec = "Recieved: ";
        for (int i = 0; i < event.lora.length; i++) {
          // Serial.write(event.lora.data[i]);
          rec += static_cast<char>(event.lora.data[i]);
        }
        rec += "\nRSSI: ";
        rec += std::to_string(event.lora.rssi);
        rec += "\nSNR: ";
        rec += std::to_string(event.lora.snr);

        Serial.println(rec.c_str());
        ssd1306_clear(disp);
        ssd1306_draw_text(disp, x, y, rec.c_str(), true);
        ssd1306_display(disp);

        break;
      }

      case EVENT_VOLTAGE:
        break;

      case EVENT_WIFI:
        break;
      }
    }
  }
}
