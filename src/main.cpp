#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_wps.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WPS Config manuell setzen (WPS_CONFIG_INIT_DEFAULT ist deprecated)
static esp_wps_config_t wps_config;

static bool wps_success = false;
static String wps_ssid  = "";
static String wps_pass  = "";

// -------------------------------------------------------
// WiFi-Event-Handler
// -------------------------------------------------------
void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("[WiFi] STA gestartet");
            break;

        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("[WiFi] Verbunden (STA)");
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("[WiFi] IP: ");
            Serial.println(WiFi.localIP());
            wps_ssid    = WiFi.SSID();
            wps_pass    = WiFi.psk();
            wps_success = true;
            break;

        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            Serial.println("[WPS] Erfolgreich – verbinde...");
            // Credentials sind jetzt im NVS; normal verbinden
            esp_wifi_wps_disable();
            delay(10);
            WiFi.begin();
            break;

        case ARDUINO_EVENT_WPS_ER_FAILED:
            Serial.println("[WPS] Fehlgeschlagen – neuer Versuch");
            esp_wifi_wps_disable();
            esp_wifi_wps_enable(&wps_config);
            esp_wifi_wps_start(0);
            break;

        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            Serial.println("[WPS] Timeout – neuer Versuch");
            esp_wifi_wps_disable();
            esp_wifi_wps_enable(&wps_config);
            esp_wifi_wps_start(0);
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            if (wps_success) {
                Serial.println("[WiFi] Verbindung verloren – Reconnect...");
                wps_success = false;
                WiFi.begin();
            }
            break;

        default:
            break;
    }
}

// -------------------------------------------------------
// Display-Hilfsfunktion
// -------------------------------------------------------
void updateDisplay(const String& line1, const String& line2, const String& line3 = "") {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(line1);
    display.println("---------------------");
    display.println(line2);
    if (line3.length() > 0) {
        display.println(line3);
    }
    display.display();
}

// -------------------------------------------------------
// Setup
// -------------------------------------------------------
void setup() {
    Serial.begin(115200);

    // OLED initialisieren
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("[OLED] Initialisierung fehlgeschlagen!");
        for (;;) { delay(100); }
    }
    updateDisplay("WPS MODUS", "Taste am Router", "druecken...");

    // WPS Config manuell befüllen
    memset(&wps_config, 0, sizeof(wps_config));
    wps_config.wps_type = WPS_TYPE_PBC;
    // Factory-Info (optional, aber sauber)
    strlcpy(wps_config.factory_info.manufacturer, "ESP32",    sizeof(wps_config.factory_info.manufacturer));
    strlcpy(wps_config.factory_info.model_number,  "ESP32",   sizeof(wps_config.factory_info.model_number));
    strlcpy(wps_config.factory_info.model_name,    "ESPRESSIF", sizeof(wps_config.factory_info.model_name));
    strlcpy(wps_config.factory_info.device_name,   "ESP32",   sizeof(wps_config.factory_info.device_name));

    // Events registrieren VOR WiFi.mode()
    WiFi.onEvent(WiFiEvent);

    WiFi.mode(WIFI_MODE_STA);
    esp_wifi_wps_enable(&wps_config);
    esp_wifi_wps_start(0);

    Serial.println("[WPS] Gestartet – bitte WPS-Taste am Router druecken");
}

// -------------------------------------------------------
// Loop
// -------------------------------------------------------
void loop() {
    static bool displayUpdated = false;

    if (wps_success && !displayUpdated) {
        displayUpdated = true;
        Serial.println("=== VERBUNDEN ===");
        Serial.println("SSID : " + wps_ssid);
        Serial.println("PW   : " + wps_pass);
        Serial.println("IP   : " + WiFi.localIP().toString());
        updateDisplay("ERFOLG!", "SSID: " + wps_ssid, "PW: " + wps_pass);
    }

    if (!wps_success && displayUpdated) {
        displayUpdated = false;
        updateDisplay("Verbindung", "verloren...", "Reconnect...");
    }

    delay(500);
}
