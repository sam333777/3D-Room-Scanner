3D Scanning System
This project is a 3D scanning setup using an MSP432E401Y microcontroller, a VL53L1X time-of-flight (ToF) sensor, and a stepper motor. The system collects distance measurements while rotating the sensor and then visualizes them in Python as a 3D point cloud.
Features
MSP432E401Y microcontroller (40 MHz bus speed)
VL53L1X ToF sensor (short/medium/long range, 940 nm IR)
Stepper motor driven by ULN2003 board (360° rotation, 11.25° steps)
Pushbuttons to control rotation and scanning (active-low)
Status LEDs to indicate measurement progress
UART data transfer to PC at 115200 bps
Python visualization using NumPy + Open3D
Hardware
MSP432E401Y board
ULN2003 motor driver + stepper motor
VL53L1X ToF sensor (I²C: SDA on PB3, SCL on PB2)
Two buttons on PM0 and PM1 (active-low, pull-ups)
LEDs on PN0 and PN1
USB for programming and UART, 5–12 V supply for motor
Software
Python 3.8
Packages:
pip install pyserial open3d openpyxl
How to Use
Flash the firmware to the MSP432 using Keil (or equivalent).
Find the COM port of the device on your computer.
Edit scanning.py to match the COM port:
ser = serial.Serial('COM4', 115200)
Run the scan:
python scanning.py
Button 1 → rotate motor
Button 2 → start/stop scan
When done, run the visualization:
python visualization.py
Notes / Limitations
ToF range is 40 mm to 4000 mm
Stepper motor needs proper delay tuning (too fast → missed steps)
Serial speed limited to ~128 kbps on test setup
Small precision mismatch between MSP432 (single-precision) and Python (double-precision)
