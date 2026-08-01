#define RX_PIN 20
#define TX_PIN 21

// HardwareSerial PicoSerial(1);

void setup()
{
    Serial.begin(115200);

    while (!Serial) {
    delay(10); 
  }
  
    Serial0.begin(115200, SERIAL_8N1, RX_PIN , TX_PIN);

    Serial.println("ESP32 Ready");
}

void loop()
{
    if (Serial0.available())
    {
        String msg = Serial0.readStringUntil('\n');

        Serial.print("Received: ");
        Serial.println(msg);

        Serial0.println("Hello Pico!");
    }
}