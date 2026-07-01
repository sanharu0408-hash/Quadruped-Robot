# 🐾 Quadruped Robot Project

https://www.youtube.com/watch?v=LkktNqY10M0

This project features a 4-legged walking robot controlled via an Arduino R4 and a Python-based PC controller.

## 📁 Project Structure

- `/Python_controller`: Python source code for PC-side control (Pygame & Serial).
- `/arduino_robot`: Arduino sketch for servo motor management (PCA9685).
- `/STL`: 3D printable parts for the robot chassis.

## 🖨️ 3D Printing Guide

Please print the following components from the `/STL` folder. 
The recommended material is PLA or PETG with at least 20% infill for durability.

| File Name | Quantity 
| :--- | :---: 
| `frame_bottom.stl` | 1 |
| `frame_top.stl` | 1 | 
| `hip_1.stl` | 2 |
| `hip_2.stl` | 2 | 
| `left_knee.stl` | 2 | 
| `right_knee.stl` | 2 | 

## 🚀 Getting Started

### 1. Arduino Setup

1. Open `arduino_robot/arduino_robot.ino` in the Arduino IDE.
2. Install the `Adafruit PWM Servo Driver` library.
3. Upload the code to your Arduino R4.

### 2. Python Controller Setup

1. Install the required libraries:
   ```bash
   pip install pygame pyserial
   ```

2. Connect your controller (Xbox/PS4/PC Controller).

3. Run the application:
   ```bash
   python controller/main.py
   ```

## 🎮 Controls

| Button | Action |
| :--- | :--- |
| **B** | Toggle Walking Mode ON/OFF |
| **A** | kick |
| **X** | Jump |
| **Y** | Wave Hand |
| **L Stick** | Move Forward / Backward |
| **R Stick** | Turn Left / Right |

## 🛠️ Requirements

- **Hardware:** Arduino R4, PCA9685 PWM Driver, 8x MG90S Servos
- **Software:** Python 3.x, Arduino IDE
