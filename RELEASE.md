﻿# Cypress PSoC 6 Bluetooth Low Energy Middleware Library 3.60

### What's Included?
Please refer to the [README.md](./README.md) and the [API Reference Guide](https://cypresssemiconductorco.github.io/bless/ble_api_reference_manual/html/index.html) for a complete description of the PSoC 6 BLE Middleware.

The revision history of the PSoC 6 BLE Middleware is also available on the [API Reference Guide Changelog](https://cypresssemiconductorco.github.io/bless/ble_api_reference_manual/html/page_group_ble_changelog.html).

### New in this release:
* BLE Stack 5.0.10 

### Defect Fixes
* Updated channel map indication handler to optimize the Low-Power-Mode (LPM) performance of PSoC 6 BLE Controller.
* Added changes required to qualify for BLE Spec. 5.2.
* Updated Cy_BLE_Init function. Cy_BLE_Init() returns CY_BLE_ERROR_INVALID_PARAMETER when BLE configured as Observer or Broadcaster only.
* Updated the procedure of processing the Write Request event from Stack. BLE middleware returns CY_BLE_EVT_GATTS_WRITE_REQ event twice for the Custom profile.
* Updated Radio TX Power Change Handling procedure. The radio TX power change handler was incorrectly being called from interrupt context whereas the ideal location for the same is from the task context.
* Enhanced GATT Server implementation. Updated the GATT server implementation to handle large data base (> 512 entries).
* Re-factored Shutdown code for Dual CPU mode. Improved the handling of Cy_BLE_StackShutdown API to avoid timing-sensitive bugs.

Refer to section [Changelog of PSoC 6 BLE Middleware API Reference Guide](https://cypresssemiconductorco.github.io/bless/ble_api_reference_manual/html/page_group_ble_changelog.html) for details.


### Product/Asset Specific Instructions
Include cy_ble_common.h and cy_ble_event_handler.h to get access to all functions and other declarations in this library. If you are using the ModusToolbox Bluetooth Configurator, you can include cycfg_ble.h only.

### Quick Start
The [Quick Start section of the PSoC 6 BLE Middleware API Reference Guide](https://cypresssemiconductorco.github.io/bless/ble_api_reference_manual/html/page_ble_quick_start.html) describes step-by-step instructions to configure and launch BLE in [ModusToolbox](https://www.cypress.com/products/modustoolbox-software-environment).

### Known Issues
| Problem                                                  | Workaround |
| :---                                                     | :----  |
| The CY_BLE_EVT_STACK_BUSY_STATUS event may not return a CY_BLE_STACK_STATE_FREE state, if the application initiates an active connection (Peripheral/Central GAP role) along with Scan activity (GAP Observer) with a high duty cycle (scan window value is close to scan interval). | Increase the scan interval and reduce the scan window values to have a  ratio of at least 1/2. |


### Supported Software and Tools
This version of the PSoC 6 BLE Middleware was validated for compatibility with the following Software and Tools:

| Software and Tools                                      | Version |
| :---                                                    | :----:  |
| ModusToolbox Software Environment                       | 2.0     |
| - ModusToolbox Device Configurator                      | 2.0     |
| - ModusToolbox BT Personality in Device Configurator    | 1.1     |
| - ModusToolbox BT Configurator                          | 2.0     |
| GCC Compiler                                            | 7.2.1   |
| IAR Compiler                                            | 8.32    |
| ARM Compiler 6                                          | 6.11    |

### More information
The following resources contain more information:
* [PSoC 6 BLE Middleware README.md](./README.md)
* [PSoC 6 BLE Middleware API Reference Guide](https://cypresssemiconductorco.github.io/bless/ble_api_reference_manual/html/index.html)
* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)
* [PSoC 6 BLE Middleware Code Examples at GITHUB](https://github.com/cypresssemiconductorco)
* [ModusToolbox BT Configurator Tool Guide](https://www.cypress.com/ModusToolboxBLEConfig)
* [ModusToolbox Device Configurator Tool Guide](https://www.cypress.com/ModusToolboxDeviceConfig)
* [AN210781 Getting Started with PSoC 6 MCU with Bluetooth Low Energy (BLE) Connectivity](http://www.cypress.com/an210781)
* [CySmart - BLE Test and Debug Tool](http://www.cypress.com/documentation/software-and-drivers/cysmart-bluetooth-le-test-and-debug-tool)
* [CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit](http://www.cypress.com/cy8ckit-062-ble)
* [PSoC 6 Technical Reference Manual](https://www.cypress.com/documentation/technical-reference-manuals/psoc-6-mcu-psoc-63-ble-architecture-technical-reference)
* [PSoC 63 with BLE Datasheet Programmable System-on-Chip](http://www.cypress.com/ds218787)
* [Cypress Semiconductor](http://www.cypress.com)
---
© Cypress Semiconductor Corporation, 2020.
