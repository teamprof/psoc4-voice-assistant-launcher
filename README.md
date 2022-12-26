# Voice Assistant Launcher (PSoC4 and EZ-BLE firmware)
### Existing Voice Assistance (e.g. Google Assistance/Siri) keeps capturing data from microphone all the time, in order to detect the trigger words/phrases like "Hey Google" / "Siri". This causes a privacy concern as user may be unwilling to be listened by the Voice Assistance 24x7. 
### In additional, unintentional trigger may happen by a third party whose speaks the trigger words/phrases but captured by the Voice Assistance.
### This PSoC4 firmware leverages the Infineon's CapSense technology to implement a voice trigger commander. 
### User moves his/her finger on the CapSense trackpad with a rotate-clocksize gesture. The PSoC 4100S sends an "EventLaunch" message to Android device via the EZ-BLE module. The VoiceAssistantLauncher APP installed on the Android device launches Google Assistant once received the "EventLaunch" message.
---

[![License: LGPL v3](https://img.shields.io/badge/License-LGPL_v3-blue.svg)](https://github.com/teamprof/psoc4-voice-assistant-launcher/blob/main/LICENSE)

<a href="https://www.buymeacoffee.com/teamprofnet" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/v2/default-yellow.png" alt="Buy Me A Coffee" style="height: 38px !important;width: 168px !important;" ></a>

---

## Prerequisites
### Hardware
1. CY8CKIT-041-41XX PSoC™ 4100S CapSense Pioneer Kit (https://www.infineon.com/cms/en/product/evaluation-boards/cy8ckit-041-41xx/)
   [![CY8CKIT-041-41XX](https://www.infineon.com/export/sites/default/_images/product/evaluation-boards/CY8CKIT-041-41XX_0.png_391500115.png)](https://www.infineon.com/export/sites/default/_images/product/evaluation-boards/CY8CKIT-041-41XX_0.png_391500115.png)

2. A mobile phone running with Android version 8.0 or above
[![Android 8](images/android8.jpg)](https://www.android.com/versions/oreo-8-0/)

### Software
1. [`PSoC creator 4.4](https://www.infineon.com/cms/en/design-support/tools/sdk/psoc-software/psoc-creator/)


---
## System diagram
[![System Diagram](images/system-diagram.jpg)](https://github.com/teamprof/psoc4-voice-assistant-launcher/tree/main/images/system-diagram.jpg)

---

## Compile/build and program firmware 
1. Download and decompress the firmware source from github
2. Launch PSoC Creator 4.4 on Windows
3. Open the decompressed workspace. (There are two projects: EZ-BLE_PRoC_Module and VoiceAssistantLauncher on the workspace) 
4. Set 'EZ-BLE_PRoC_Module' as active project. 
5. Clean and build EZ-BLE_PRoC_Module 
[![Build BLE module](images/build-ble-module.jpg)](https://github.com/teamprof/psoc4-voice-assistant-launcher/tree/main/images/build-ble-module.jpg)
6. Switch KitProg2 to EZ-BLE PRoC Module (SW5 to bottom position)
[![KitProg2 switch](images/KitProg2-switch.jpg)](https://github.com/teamprof/psoc4-voice-assistant-launcher/tree/main/images/KitProg2-switch.jpg)
7. Connect CY8CKIT-041-41XX to PC with an USB cable. Program the BLE_PRoC_Module firmware to CY8CKIT-041-41XX's EZ-BLE PRoC Module

8. Unplug the USB cable between CY8CKIT-041-41XX and PC
9. Switch KitProg2 back to PSoC 4100S (SW5 to top position)
10. Set 'VoiceAssistantLauncher' as active project. 
11. Clean and build VoiceAssistantLauncher
[![Build Voice Assistant](images/build-voice-assistant.jpg)](https://github.com/teamprof/psoc4-voice-assistant-launcher/tree/main/images/build-voice-assistant.jpg)
12. Connect CY8CKIT-041-41XX to PC with an USB cable again. Program the VoiceAssistantLauncher firmware to CY8CKIT-041-41XX's PSoC 4100S
13. Press the "Reset" button on CY8CKIT-041-41XX
[![Reset button](images/button-reset.jpg)](https://github.com/teamprof/psoc4-voice-assistant-launcher/tree/main/images/button-reset.jpg)
14. Draw a clockwise circle in the trackpad area with a finger. The Green LED should be on if everything goes smooth.
[![Trackpad](images/trackpad.jpg)](https://github.com/teamprof/psoc4-voice-assistant-launcher/tree/main/images/trackpad.jpg)
14. Draw a counter-clockwise circle in the trackpad area with a finger. The Green LED should be off.
---

## TODO: Install mobile App on Android device 

## TODO: demo Video 

## TODO: demo code

## TODO: Debug Log
The PSoC 4100S firmware includes debug log via UART1. Simply launch a Serial Terminal (e.g. TeraTerm) and connect to CY8CKIT's COM port at "115200, 8N1" to see the log message.

## Issues

Submit issues to: [Voice Assistant Launcher issues](https://github.com/teamprof/psoc4-voice-assistant-launcher/issues) 

---

## Contributions and Thanks

Many thanks for everyone for bug reporting, new feature suggesting, testing and contributing to the development of this library.

---

## Contributing

If you want to contribute to this project:

- Report bugs and errors
- Ask for enhancements
- Create issues and pull requests
- Tell other people about this library

---

## License

- The library is licensed under GNU LESSER GENERAL PUBLIC LICENSE Version 3
---

## Copyright

- Copyright 2022- teamprof.net@gmail.com

