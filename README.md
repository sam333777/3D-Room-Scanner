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
  ```
  

  <img width="533" height="396" alt="image" src="https://github.com/user-attachments/assets/7ce85ee0-2f90-4109-b112-e448dfd109a8" />
  <img width="830" height="427" alt="image" src="https://github.com/user-attachments/assets/9790b3e8-d14c-4580-9290-7d788ea6b208" />
## Sample scan
  <img width="345" height="693" alt="image" src="https://github.com/user-attachments/assets/32d36a70-1a57-4165-b580-f12338823176" />



