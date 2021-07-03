# Video Frame
![demo.gif](images/demo.gif)


<!-- TABLE OF CONTENTS -->
<details open="open">
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#compiling">Compiling</a></li>
      </ul>
    </li>
    <li><a href="#circuit-diagram">Circuit Diagram</a></li>
    <li><a href="#disclaimer">Disclaimer</a></li>
  </ol>
</details>


<!-- ABOUT THE PROJECT -->
## About The Project
This is like a digital picture frame that pays movies at one frame per minute. Images appear on a [7-Color ePaper display](https://www.waveshare.com/5.65inch-e-paper-module-f.htm). The whole movie is stored as a sequence of jpeg images on an SD-Card. The STM32 microcontroller does the jpeg decoding and updates the display.
The device is powered by 6 AA batteries.

For more info check out the [full log on hackaday.io](https://hackaday.io/project/177197-the-slowest-video-player-with-7-colors)

<!-- GETTING STARTED -->
## Getting Started
### Prerequisites
* [STM32CubeIDE 1.4.0](https://www.st.com/en/development-tools/stm32cubeide.html)
* [dfu-util](http://dfu-util.sourceforge.net/)

### Compiling
* Open STM32CubeIDE
* Select a folder as workspace (this can be any folder in your hard-drive)
* Click on `File > Import`
* Select `General > Existing Project into Workspace` then click next
* Browse for the folder `stm32` select the project and click finish

### Loading firmware via USB bootloader
* Connect the USB cable, check that the [correct drivers](https://www.hanselman.com/blog/how-to-fix-dfuutil-stm-winusb-zadig-bootloaders-and-other-firmware-flashing-issues-on-windows) are installed
* Enter DFU mode by pressing the RESET while holding down the BOOT button
* Use the following command to flash the firmware

      dfu-util -a0 -s 0x08000000 0 -D "Video Frame.bin"

<!-- GETTING STARTED -->
## Circuit Diagram
![Schematic](images/schematic.png)


<!-- DISCLAIMER -->
## Disclaimer
The following project is shared "as is", with the sole objective of being useful. The creator of the described piece of hardware and its associated software cannot guarantee its correct functioning under any circumstance. The author of this project will not be held responsible for any material, personal or economic losses to you or any third parties that may arise from using this design. This project will not be used under any circumstances on systems whose responsibility is critical, or from which people's lives depend upon, directly or indirectly.