# CatSat: Bootloader Documentation

<p align="left">
	<!--Latest Release Version-->
	<img src="https://img.shields.io/github/v/release/CanSat-YDSP/Bootloader?style=default&logo=github&logoColor=white&color=0080ff" alt="latest-release">
	<img src="https://img.shields.io/github/last-commit/CanSat-YDSP/Bootloader?style=default&logo=git&logoColor=white&color=0080ff" alt="last-commit">
	<img src="https://img.shields.io/github/languages/top/CanSat-YDSP/Bootloader?style=default&color=0080ff" alt="repo-top-language">
	<img src="https://img.shields.io/github/languages/count/CanSat-YDSP/Bootloader?style=default&color=0080ff" alt="repo-languages">
	<img src="https://img.shields.io/github/repo-size/CanSat-YDSP/Bootloader?style=default&color=0080ff" alt="repo-size">
</p>


This bootloader is designed as part of a larger CanSat project. It is the main mechanism for updating the firmware on the CanSat. It is responsible for receiving new firmware over the air and writing it to the microcontroller's flash memory.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

The things you need before using the CanSat:

#### Hardware
* 1x CanSat

#### Software
* (Optional) Serial Monitor (e.g., PuTTY, RealTerm) for viewing debug information.
* Microchip Studio (Formerly Atmel Studio) for compiling the project.

## Installation

To install Microchip Studio and set it up for this project, follow these steps:

1. Download Arduino IDE from the [Arduino Website](https://www.arduino.cc/en/software)
2. Go to `Tools > Board > Boards Manager` and install Arduino AVR boards
3. Check the installation path of Arduino IDE
    - Right-click on the `Arduino IDE icon > Open File Location`
    - Example: ```C:\Users\johndoe\AppData\Local\Programs\Arduino IDE```
4. Locate the `avrdude.exe` path:
   - Example:  
     ```C:\Users\johndoe\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino17\bin\avrdude.exe```
5. Install Microchip Studio from the [Microchip website](https://www.microchip.com/en-us/tools-resources/develop/microchip-studio)
6. Open Microchip Studio
7. Go to `Tools > External Tools`
8. Click on `Add`
9. Fill in the following fields:
    - Title: `Bootloader`.
    - Command: `avrdude.exe` path.
        - Example: 
        ```
        C:\Users\johndoe\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino17\bin\avrdude.exe
        ```
    - Arguments: `-C` path to `avrdude.conf` file `-v -patmega2560 -carduino -PCOM` x `-b19200 -Uflash:w:"$(ProjectDir)Debug\$(TargetName).hex":i`.
        - Example: 
        ```
        -C"C:\Users\johndoe\AppData\Local\Arduino15\packages\arduino\tools\avrdude\6.3.0-arduino17\etc\avrdude.conf" -v -patmega328p -carduino -PCOM3 -b115200 -D -Uflash:w:"$(ProjectDir)Debug\$(TargetName).hex":i
        ```

10. The project files should already have the memory settings for the linker set correctly for burning the bootloader.

## Usage

Turn on the CanSat to start. Wait for approximately 5-10 seconds for the in-built LED to blink. Debugging information will be sent to the serial monitor if connected.

## Additional Documentation

Below are links to additional documentation related to the CanSat project:

For more details on software, [click here](https://github.com/CanSat-YDSP/Documentation/blob/main/software_details.pdf).

For an overview of the entire project and hardware, [click here](https://github.com/CanSat-YDSP/Documentation/blob/main/report.pdf).

## Contributing
<p align="left">
   <a href="https://github.com{/atlas-sat/Bootloader/}graphs/contributors">
      <img src="https://contrib.rocks/image?repo=CanSat-YDSP/Bootloader">
   </a>
</p>
</details>

## License

This project is not protected under any license. You are free to use, modify, and distribute the software as you see fit.