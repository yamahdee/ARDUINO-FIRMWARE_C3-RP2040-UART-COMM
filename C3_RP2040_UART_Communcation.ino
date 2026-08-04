#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <WiFi.h>

#define RX_PIN 20
#define TX_PIN 21

const char* DEFAULT_WIFI_SSID  = "USER 6419";
const char* DEFAULT_WIFI_PASSWORD  = "123456798";

const char* DEFAULT_AP_SSID  = "ESP C3";
const char* DEFAULT_AP_PASSWORD  = "123456789";

// JSON Helper
void sendJsonResponse(String status, String msg){
    JsonDocument responseDoc;
    responseDoc["status"] = status;
    if (msg != ""){
        responseDoc["msg"] =msg;
    }
    serializeJson(responseDoc, Serial0);
    Serial0.println();
}

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


bool connectAP_STA(const char* a_ssid, const char* a_password, const char* ssid, const char* password) {
    WiFi.mode(WIFI_AP_STA); // Explicitly dual mode

    if (!WiFi.softAP(a_ssid, a_password)) {
        return false;
    }

    WiFi.begin(ssid, password);
    return waitForConnection(); 
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

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, packet);

    if (error){
        sendJsonResponse("ERROR", "INVALID_JSON");
        return;
    }

    String command = doc["cmd"] | "";
    String action = doc["action"] | "";
    String ssid = doc["ssid"] | "";
    String pass = doc["pass"] | "";
    String ap_ssid = doc["ap_ssid"] | "";
    String ap_pass = doc["ap_pass"] | "";

    Serial.print("JSON CMD: "); Serial.println(command);

    if (command == "PING") {
        sendJsonResponse("OK", "PONG");
    }
    else if (command == "STATUS") {
        sendJsonResponse("OK", "READY");
    }
    // LED COMMAND
    else if (command == "LED") {
        Serial.println("LED command received");
        
        if (action == "ON") {
            digitalWrite(8, HIGH);
            sendJsonResponse("OK", "LED_ON");
        }
        else if (action == "OFF") {
            digitalWrite(8, LOW);
            sendJsonResponse("OK", "LED_OFF");
        }
        else {
            sendJsonResponse("ERROR", "INVALID_ACTION");
        }
    }
    // WIFI COMMANDS
        else if (command == "WIFI") {
        if (action == "CONNECT") {
             const char* final_ssid = ssid.length() > 0 ? ssid.c_str() : DEFAULT_WIFI_SSID;
             const char* final_pass = pass.length() > 0 ? pass.c_str() : DEFAULT_WIFI_PASSWORD;
             
            if (connectWiFi(final_ssid, final_pass)) {
                sendJsonResponse("OK", "CONNECTED_STA");
            } else {
                sendJsonResponse("ERROR", "NO_WIFI");
            }
        }
        else if (action == "START_AP") {

            const char* final_ssid = ssid.length() > 0 ? ssid.c_str() : DEFAULT_AP_SSID;
            const char* final_pass = pass.length() > 0 ? pass.c_str() : DEFAULT_AP_PASSWORD;

            if (connectAP(final_ssid, final_pass)) {
                sendJsonResponse("OK", "AP_ACTIVE");
            } else {
                sendJsonResponse("ERROR", "AP_FAILED");
            }
        }
        else if (action == "START_AP_STA") {
            const char* final_ap_ssid = ap_ssid.length() > 0 ? ap_ssid.c_str() : DEFAULT_AP_SSID;
            const char* final_ap_pass = ap_pass.length() > 0 ? ap_pass.c_str() : DEFAULT_AP_PASSWORD;
            const char* final_ssid = ssid.length() > 0 ? ssid.c_str() : DEFAULT_WIFI_SSID;
            const char* final_pass = pass.length() > 0 ? pass.c_str() : DEFAULT_WIFI_PASSWORD;
            

            if (connectAP_STA(final_ap_ssid, final_ap_pass, final_ssid, final_pass)) {
                sendJsonResponse("OK", "AP_STA_ACTIVE");
            } else {
                sendJsonResponse("ERROR", "AP_STA_FAILED");
            }
        }
        else if (action == "DISCONNECT") {
            // Smart disconnect: shuts down whatever is running cleanly
            disconnectWiFi();
            disconnectAP();
            sendJsonResponse("OK", "DISCONNECTED");
        }
        else if (action == "STATUS") {
            // Enhanced status reporting back across the UART link
            String currentMode = "";
            switch(WiFi.getMode()) {
                case WIFI_OFF: currentMode = "OFF"; break;
                case WIFI_STA: currentMode = "STA"; break;
                case WIFI_AP:  currentMode = "AP"; break;
                case WIFI_AP_STA: currentMode = "AP_STA"; break;
            }
            String connectionState = (WiFi.status() == WL_CONNECTED)? "CONNECTED" : "DISCONNECTED";
            sendJsonResponse("OK", "MODE:" + currentMode + "|STATUS:" + connectionState);
        }
        else if (action == "OFF") {
            if (offWiFi()) {
                sendJsonResponse("OK", "WIFI_OFF");
            }
        }
        else {
            sendJsonResponse("ERROR","UNKNOWN_WIFI_SUBCOMMAND");
        }
    }

    else {
        sendJsonResponse("ERROR", "UNKNOWN_COMMAND");
    }
}
