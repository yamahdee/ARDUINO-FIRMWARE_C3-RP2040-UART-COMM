from machine import UART, Pin
import time

uart = UART(
    0,
    baudrate=115200,
    tx=Pin(0),
    rx=Pin(1)
)

def send(command, data=""):
    uart.write(f"{command}|{data}\n")

while True:

    send("LED")

    time.sleep(1)

    while uart.any():

        response = uart.readline()

        if response:

            print(response.decode().strip())

    time.sleep(2)