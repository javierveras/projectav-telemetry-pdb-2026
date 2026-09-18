# Current Sensor - KiCad 9.0

Current sensor project developed in KiCad version 9.0.

<img width="991" height="466" alt="image" src="https://github.com/user-attachments/assets/e4e84c80-9ea2-4328-bc6f-18e447f74000" />


## Description

This repository contains the schematic (`.kicad_sch`) and PCB layout (`.kicad_pcb`) of a current sensor based on the ACS712xLCTR-05B integrated circuit, capable of measuring currents up to ±5A using the Hall effect.

- **Sensor used:** ACS712xLCTR-05B (SOIC-8)
- **Operating voltage:** +5V
- **Main outputs:** 3-pin connector for power supply and analog output (VIOUT), input terminals for the current to be measured.
- **Additional components:** LED for indication, resistors, filter capacitors.

<img width="1122" height="452" alt="image" src="https://github.com/user-attachments/assets/6a6b69ff-9df6-4b39-85a9-7eb9fd6042e3" />


## How to use

1. Open the `sensor_corrente.kicad_sch` and `sensor_corrente.kicad_pcb` files with KiCad 9.0 or higher.
2. Edit, simulate, or generate the manufacturing files as needed.
3. Consult the ACS712 datasheet for electronic integration and reading code.

## License

This project is licensed under the MIT License. See the [LICENSE] file for more details.
