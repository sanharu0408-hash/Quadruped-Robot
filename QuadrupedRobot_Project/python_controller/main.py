import sys
import serial
import pygame
import time

# --- Configuration ---
SERIAL_PORT = "COM3"
BAUD_RATE = 115200
DEADZONE = 0.2        # Joystick deadzone
LOOP_HZ = 20          # Loop frequency (Hz)

# Button Mapping (Standard Xbox Controller Layout)
BTN_A = 0
BTN_B = 1
BTN_X = 2
BTN_Y = 3

# --- Initialization ---
pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("[Error] No controller connected.")
    sys.exit()

controller = pygame.joystick.Joystick(0)
controller.init()

print(f"Connecting to Arduino on {SERIAL_PORT}...")
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
    time.sleep(1.5)  # Wait for connection to stabilize
    print("👉 Connected! You can now control the robot.")
except Exception as e:
    print(f"[Error] Failed to connect to serial port: {e}")
    sys.exit()

status_msg = "Idle"

print("\n--- Operation Guide ---")
print(f" B Button: Toggle Walking Mode (w)")
print(f" A Button: kick (b)")
print(f" X Button: Jump (j)")
print(f" Y Button: Wave Hand (h)")
print("------------------------\n")

print_counter = 0
clock = pygame.time.Clock()

try:
    while True:
        clock.tick(LOOP_HZ)
        pygame.event.pump()

        # --- Handle Button Inputs ---
        for event in pygame.event.get():
            if event.type == pygame.JOYBUTTONDOWN:
                if event.button == BTN_B:
                    ser.write(b"w\n")
                    status_msg = "Sent: Walking Mode Toggle"
                elif event.button == BTN_A:
                    ser.write(b"b\n")
                    status_msg = "Sent: kick (b)"
                elif event.button == BTN_X:
                    ser.write(b"j\n")
                    status_msg = "Sent: Jump (j) 💥"
                elif event.button == BTN_Y:
                    ser.write(b"h\n")
                    status_msg = "Sent: Wave (h) 👋"

        # --- Handle Stick Inputs ---
        # Axis 1: Vertical (Invert so up is positive)
        # Axis 2: Horizontal
        raw_Y = -controller.get_axis(1)
        raw_X = controller.get_axis(2)

        # Apply deadzone and scale
        stick_Y = round(raw_Y * 5.0, 2) if abs(raw_Y) > DEADZONE else 0.0
        stick_X = round(raw_X * 5.0, 2) if abs(raw_X) > DEADZONE else 0.0

        # Send stick data to Arduino
        msg = f"f{stick_Y:.2f},r{stick_X:.2f}\n"
        ser.write(msg.encode())

        # --- Update Terminal Display ---
        print_counter += 1
        if print_counter % 2 == 0:  # Reduce flickering
            output = f"\r[Status] {status_msg:<30} | [Stick] Forward:{stick_Y:+.2f} Turn:{stick_X:+.2f}"
            sys.stdout.write(output)
            sys.stdout.flush()

except KeyboardInterrupt:
    print("\n\nShutting down...")
finally:
    ser.close()
    pygame.quit()
    print("Cleaned up resources. Goodbye.")
