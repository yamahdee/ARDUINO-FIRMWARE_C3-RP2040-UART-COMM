import machine
import time

# UART0 on Pico: GP0 is TX, GP1 is RX
uart = machine.UART(0, baudrate=115200, tx=machine.Pin(0), rx=machine.Pin(1))

print("Pico MicroPython Script Started")

while True:
    print("Sending data to ESP32...")
    
    uart.write("Ping from Pico!\n") 
    
    time.sleep(0.2) # Give the ESP32 time to process
    
    if uart.any():
        reply = uart.readline().decode('utf-8').strip()
        print("ESP32 Replied:", reply)
        
    time.sleep(2)
