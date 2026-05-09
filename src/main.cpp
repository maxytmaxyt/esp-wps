#include <Arduino.h>
#include <WiFi.h>
#include "esp_wps.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display Pins für ESP32-S (Standard I2C)
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WPS Einstellungen
static esp_wps_config_t config = WPS_CONFIG_INIT_DEFAULT(WPS_TYPE_PBC);

void updateDisplay(String line1, String line2, String line3 = "") {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println(line1);
    display.println("---------------------");
    display.setTextSize(1);
    display.println(line2);
    display.println(line3);
    display.display();
}

void setup() {
    Serial.begin(115200);

    // OLED Start
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println("OLED Fehler");
        for(;;);
    }
    display.setTextColor(SSD1306_WHITE);
    updateDisplay("WPS MODUS", "Suche Router...", "Bitte Taste druecken");

    // WiFi Setup
    WiFi.mode(WIFI_MODE_STA);
    esp_wifi_wps_enable(&config);
    esp_wifi_wps_start(0);
}

void loop() {
    static bool fertig = false;

    if (WiFi.status() == WL_CONNECTED && !fertig) {
        fertig = true;
        String ssid = WiFi.SSID();
        String pass = WiFi.psk(); // Hier holen wir das Passwort

        Serial.println("Verbunden!");
        updateDisplay("ERFOLG!", "SSID: " + ssid, "PW: " + pass);
    }

    if (WiFi.status() != WL_CONNECTED && fertig) {
        fertig = false; // Reset falls Verbindung abbricht
        updateDisplay("Verloren", "Suche Reconnect...", "");
    }
    
    delay(1000);
}
