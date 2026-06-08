import math
import pygame
import serial

WIDTH = 600
HEIGHT = 600

ser = serial.Serial('COM7', 115200, timeout=0.1)

button_press_times = 0


def update():
    global button_press_times

    while ser.in_waiting:
        try:
            line = ser.readline().decode(errors='ignore').strip()

            # PRINT EVERYTHING FROM SERIAL MONITOR
            print("SERIAL:", line)

            if line.startswith("Button pressed:"):
                parts = line.split(':')
                count_str = parts[1].split()[0]
                button_press_times = int(count_str)

        except Exception as e:
            print("Error:", e)


def draw():
    screen.fill((0, 0, 0))

    def draw_eye(eye_x, eye_y):
        screen.draw.filled_circle((eye_x, eye_y), 50, color=(255, 255, 255))

        direction = button_press_times % 4

        if direction == 0:
            dx, dy = 0, -20
        elif direction == 1:
            dx, dy = 20, 0
        elif direction == 2:
            dx, dy = 0, 20
        elif direction == 3:
            dx, dy = -20, 0

        screen.draw.filled_circle(
            (eye_x + dx, eye_y + dy),
            15,
            color=(0, 0, 100)
        )

    draw_eye(WIDTH / 3, HEIGHT / 2)
    draw_eye(WIDTH * 2 / 3, HEIGHT / 2)
