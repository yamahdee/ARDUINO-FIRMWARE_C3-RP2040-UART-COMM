#define RX_PIN 20
#define TX_PIN 21

// HardwareSerial Serial0(1);

void setup()
{
    Serial.begin(115200);

    while (!Serial) {
    delay(10); 

    pinMode(8, OUTPUT);
  }
  
    Serial0.begin(115200, SERIAL_8N1, RX_PIN , TX_PIN);

    Serial.println("ESP32 Ready");
}

void loop()
{
    if (!Serial0.available())
        return;

    String packet = Serial0.readStringUntil('\n');
    packet.trim();

    int separator = packet.indexOf('|');

    String command;
    String data;

    if (separator >= 0)
    {
        command = packet.substring(0, separator);
        data = packet.substring(separator + 1);
    }
    else
    {
        command = packet;
        data = "";
    }

    Serial.print("Command: ");
    Serial.println(command);

    Serial.print("Data: ");
    Serial.println(data);

    if (command == "PING")
    {
        Serial0.println("PONG|");
    }
    else if (command == "STATUS")
    {
        Serial0.println("READY|");
    }
    else if (command == "LED")
    {
        Serial.println("LED command received");
        Serial0.println("OK|");
        digitalWrite(8, HIGH);
    }
    else
    {
        Serial0.println("ERROR|UNKNOWN_COMMAND");
    }
}