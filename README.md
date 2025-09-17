# 3D Scanning System  

A simple 3D scanner using an MSP432E401Y microcontroller, VL53L1X time-of-flight sensor, and a stepper motor. The sensor rotates, collects distance data, and Python scripts generate a 3D point cloud.  

---

## Features  

- MSP432E401Y microcontroller (40 MHz)  
- VL53L1X ToF sensor (40 mm–4000 mm range)  
- Stepper motor + ULN2003 driver (360° rotation in 11.25° steps)  
- Two pushbuttons for control (start/scan)  
- UART data transfer at 115200 bps  
- Python visualization with Open3D  

---

## Requirements  

- Python 3.8  
- Install dependencies:  
  ```bash
  pip install pyserial open3d openpyxl
