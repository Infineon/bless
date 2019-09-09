# Cypress PSoC 6 Bluetooth Low Energy Middleware

### Overview

Bluetooth Low Energy (BLE) is used in very low-power network and Internet of Things (IoT) solutions using low-cost battery operated devices that can quickly connect and form simple wireless links. Target applications include HID, remote controls, sports and fitness monitors, portable medical devices and smart phone accessories, among many others.

PSoC 6 BLE Middleware contains a comprehensive API to configure the BLE Stack and the underlying chip hardware. PSoC 6 BLE Middleware incorporates a Bluetooth Core Specification v5.0 compliant protocol stack. You may access the GAP, GATT and L2CAP layers of the stack using the API. 

The standalone BT Configurator is shipped with ModusToolbox to make it easy to configure PSoC 6 BLE Middleware.

The BLE resource supports numerous SIG-adopted GATT-based Profiles and Services. Each of these can be configured as either a GATT Client or GATT Server. It generates all the necessary code for a particular Profile/Service operation, as configured in the BT Configurator.

### Features
- Multi-link supports up to four simultaneous connections in any combination of roles
- Bluetooth v5.0 compliant protocol stack
    - LE 2 Mbps
- Generic Access Profile (GAP) Features
    - Broadcaster, Observer, Peripheral and Central roles
    - User-defined advertising data
    - Bonding support for up to 128 devices
    - Security modes 1 and 2
- Generic Attribute Profile (GATT) Features
    - GATT Client and Server
    - 16-, 32-, and 128-bit UUIDs
- Special Interest Group (SIG) adopted GATT-based Profiles and Services, and quick prototype of new profile design through intuitive Bluetooth Configurator Custom Profile development;
- Security Manager features
    - Pairing methods: Just works, Passkey Entry, Out of Band, Numeric Comparison (LE Secure connections)
    - Authenticated man-in-the-middle (MITM) protection and data signing
- Logical Link Adaption Protocol (L2CAP) Connection Oriented Channel
- Link Layer (LL) Features
    - Master and Slave roles
    - 128-bit AES encryption
    - Low Duty Cycle Advertising
    - LE Ping
    - Privacy 1.2
    - Data length extension (DLE)

### Product/Asset Specific Instructions
Include cy_ble_common.h and cy_ble_event_handler.h to get access to all functions and other declarations in this library. If you are using the ModusToolbox Bluetooth Configurator, you can include cycfg_ble.h only.

### Quick Start
The [Quick Start section of the PSoC 6 BLE Middleware API Reference Guide](https://cypresssemiconductorco.github.io/bless/ble_api_reference_manual/html/page_ble_quick_start.html) describes step-by-step instructions to configure and launch PSoC 6 BLE Middleware.

### More information
The following resources contain more information:
* [PSoC 6 BLE Middleware RELEASE.md](./RELEASE.md)
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