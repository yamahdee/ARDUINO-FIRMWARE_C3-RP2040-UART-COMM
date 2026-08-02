#include <WiFi.h>

#define RX_PIN 20
#define TX_PIN 21

const char* WIFI_SSID = "USER 6419";
const char* WIFI_PASSWORD = "123456798";

const char* ap_ssid = "ESP C3";
const char* ap_password = "123456789";

// Reusable internal connection helper (doesn't force a WiFi mode change)
bool waitForConnection() {
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > 15000)
            return false;
        delay(250);
    }
    return true;
}

bool waitToDisconnect() {
    unsigned long start = millis();
    while (WiFi.status() == WL_CONNECTED) {
        if (millis() - start > 2000)
            return false;
        delay(50);
    }
    return true;
}

bool connectWiFi(const char* ssid, const char* password) {
    if (WiFi.status() == WL_CONNECTED)
        return true;

    WiFi.mode(WIFI_STA); // Set mode specifically for standalone STA
    WiFi.begin(ssid, password);
    return waitForConnection();
}

bool disconnectWiFi() {
    if (WiFi.status() != WL_CONNECTED)
        return true;

    WiFi.disconnect();
    return waitToDisconnect();
}

bool connectAP(const char* ssid, const char* password) {
    WiFi.mode(WIFI_AP);
    return WiFi.softAP(ssid, password);
}

bool disconnectAP() {
    bool success = WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    return success;
}

// Reusing functions properly here
bool connectAP_STA(const char* a_ssid, const char* a_password, const char* w_ssid, const char* w_password) {
    WiFi.mode(WIFI_AP_STA); // Explicitly dual mode

    if (!WiFi.softAP(a_ssid, a_password)) {
        return false;
    }

    WiFi.begin(w_ssid, w_password);
    return waitForConnection(); // Reusing the exact same connection loop logic safely!
}

bool disconnectAP_STA() {
    disconnectWiFi();          // Reused helper
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);
    return true;
}

bool offWiFi() {
    WiFi.mode(WIFI_OFF);
    return true;
}

void setup() {
    Serial.begin(115200);
    pinMode(8, OUTPUT);

    while (!Serial) {
        delay(10); 
    }
    
    // Config UART (Ensure Serial0 is supported or use Serial1 depending on board definition)
    Serial0.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
    Serial.println("ESP32 Ready");

    // Initialize in Station Mode
    WiFi.mode(WIFI_STA);
    Serial.println("Ready for commands...");
}

void loop() {
    if (!Serial0.available())
        return;

    String packet = Serial0.readStringUntil('\n');
    packet.trim();

    int separator = packet.indexOf('|');

    String command;
    String data;

    if (separator >= 0) {
        command = packet.substring(0, separator);
        data = packet.substring(separator + 1);
    } else {
        command = packet;
        data = "";
    }

    Serial.print("Command: "); Serial.println(command);
    Serial.print("Data: ");    Serial.println(data);

    if (command == "PING") {
        Serial0.println("PONG|");
    }
    else if (command == "STATUS") {
        Serial0.println("READY|");
    }
    // LED COMMAND
    else if (command == "LED") {
        Serial.println("LED command received");
        
        if (data == "ON") {
            digitalWrite(8, HIGH);
            Serial0.println("OK|");
        }
        else if (data == "OFF") {
            digitalWrite(8, LOW);
            Serial0.println("OK|");
        }
        else {
            Serial0.println("ERROR|INVALID_DATA");
        }
    }
    // WIFI COMMANDS
        else if (command == "WIFI") {
        if (data == "CONNECT") {
            if (connectWiFi(WIFI_SSID, WIFI_PASSWORD)) {
                Serial0.println("OK|CONNECTED_STA");
            } else {
                Serial0.println("ERROR|NO_WIFI");
            }
        }
        else if (data == "START_AP") {
            if (connectAP(ap_ssid, ap_password)) {
                Serial0.println("OK|AP_ACTIVE");
            } else {
                Serial0.println("ERROR|AP_FAILED");
            }
        }
        else if (data == "START_AP_STA") {
            if (connectAP_STA(ap_ssid, ap_password, WIFI_SSID, WIFI_PASSWORD)) {
                Serial0.println("OK|AP_STA_ACTIVE");
            } else {
                Serial0.println("ERROR|AP_STA_FAILED");
            }
        }
        else if (data == "DISCONNECT") {
            // Smart disconnect: shuts down whatever is running cleanly
            disconnectWiFi();
            disconnectAP();
            Serial0.println("OK|DISCONNECTED");
        }
        else if (data == "STATUS") {
            // Enhanced status reporting back across the UART link
            String currentMode = "";
            switch(WiFi.getMode()) {
                case WIFI_OFF: currentMode = "OFF"; break;
                case WIFI_STA: currentMode = "STA"; break;
                case WIFI_AP:  currentMode = "AP"; break;
                case WIFI_AP_STA: currentMode = "AP_STA"; break;
            }
            
            if (WiFi.status() == WL_CONNECTED) {
                Serial0.println("OK|MODE:" + currentMode + "|STATUS:CONNECTED");
            } else {
                Serial0.println("OK|MODE:" + currentMode + "|STATUS:DISCONNECTED");
            }
        }
        else if (data == "OFF") {
            if (offWiFi()) {
                Serial0.println("OK|WIFI_OFF");
            }
        }
        else {
            Serial0.println("ERROR|UNKNOWN_WIFI_SUBCOMMAND");
        }
    }

    else {
        Serial0.println("ERROR|UNKNOWN_COMMAND");
    }
}
