from machine import UART, Pin
import time

uart = UART(
    0,
    baudrate=115200,
    tx=Pin(0),
    rx=Pin(1)
)


while True:
    cmd = input("ESP> ")

    uart.write(cmd + "\n")

    while not uart.any():
        pass

    print(uart.readline().decode().strip())

    while uart.any():

        response = uart.readline()

        if response:

            print(response.decode().strip())

    time.sleep(2)