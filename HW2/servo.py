from machine import Pin, PWM
import time

servo = PWM(Pin(15))  # GP15
servo.freq(50)        # 50 Hz

def set_angle(angle):
    # Map 0–180 → duty cycle
    # Typical range for servos on Pico
    min_duty = 1638   # ~0.5 ms
    max_duty = 8192   # ~2.5 ms

    duty = int(min_duty + (angle / 180) * (max_duty - min_duty))
    servo.duty_u16(duty)

while True:
    # Sweep
    for angle in range(0, 181, 5):
        set_angle(angle)
        time.sleep(0.05)

    for angle in range(180, -1, -5):
        set_angle(angle)
        time.sleep(0.05)
