/********************************************************************************************************************//**
* \file cy_ble_ds.h
* \version 3.50
*
* \brief
*  Contains the documentation data.
*
***********************************************************************************************************************
* \copyright
* Copyright 2016-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
***********************************************************************************************************************
*
* \mainpage Cypress PSoC 6 Bluetooth Low Energy Middleware Library 3.50
*
* \copydetails page_ble_general
* \copydetails page_group_ble_changelog
*
*
***********************************************************************************************************************
** \page page_ble_general General Description
**
* The Bluetooth Low Energy (BLE) middleware contains a comprehensive API to configure the BLE Stack and
* the underlying chip hardware. The standalone BT Configurator is shipped in ModusToolbox to make it easy
* to configure PSoC 6 BLE Middleware.
* 
* Include cy_ble_common.h and cy_ble_event_handler.h to get access to all functions and other declarations
* in this library. If you are using the ModusToolbox BT Configurator, you can include cycfg_ble.h only.
* 
* The PSoC 6 BLE Middleware incorporates a Bluetooth Core Specification v5.0 compliant protocol stack. You may
* access the GAP, GATT and L2CAP layers of the stack using the API.
* 
* The API are broadly categorized as follows:
*  - \ref group_ble_common_api
*  - \ref group_ble_service_api
* 
* <b>Features:</b>
*     - Multi-link supports for up to four simultaneous connections in any combination of roles
*     - Bluetooth v5.0 compliant protocol stack
*         - LE 2 Mbps
*     - Generic Access Profile (GAP) features
*         - Broadcaster, Observer, Peripheral and Central roles
*         - User-defined advertising data
*         - Bonding support for up to 128 devices
*         - Security modes 1 and 2
*     - Generic Attribute Profile (GATT) features
*         - GATT Client and Server
*         - 16-, 32-, and 128-bit UUIDs
*     - Special Interest Group (SIG) adopted GATT-based Profiles and Services, and quick prototype of new
*       profile design through intuitive BT Configurator Custom Profile development;
*     - Security Manager features
*         - Pairing methods: Just works, Passkey Entry, Out of Band, Numeric Comparison (LE Secure connections)
*         - Authenticated man-in-the-middle (MITM) protection and data signing
*     - Logical Link Control and Adaption Protocol (L2CAP) connection-oriented channel
*     - Link Layer (LL) features
*         - Master and Slave roles
*         - 128-bit AES encryption
*         - Low Duty Cycle Advertising
*         - LE Ping
*         - Privacy 1.2
*         - Data length extension (DLE)
* 
***************************************************************************      
**\section group_ble_when_use When to use the BLE
**
* BLE is used in very low-power network and Internet of Things (IoT) solutions using low-cost battery
* operated devices that can quickly connect and form simple wireless links. Target applications include
* HID, remote controls, sports and fitness monitors, portable medical devices and smart phone accessories,
* among many others. 
*
***************************************************************************      
**\section group_ble_sig_adopted SIG adopted Profiles and Services
**
* The BLE resource supports numerous SIG-adopted GATT-based Profiles and Services. Each of these can be configured for 
* either a GATT Client or GATT Server. It generates all the necessary code for a particular Profile/Service operation,
* as configured in the BLE Configurator. 
*
***************************************************************************      
**\section group_ble_custom_profile Custom Profiles
**
* You can create custom Profiles that use existing Services, and you can create custom Services with 
* custom Characteristics and Descriptors.
* 
***************************************************************************      
**\section group_ble_debug_support Debug Support
**
* For testing and debugging, the PSoC 6 BLE Middleware can be configured to use Host Controller Interface (HCI)
* mode through a UART. Refer to Initialize and Enable PSoC 6 BLE Middleware in Controller-only Mode (HCI over
* Software API).
* 
* For over-the-air verification, the Cypress CySmart Central Emulation Tool can be used for generic 
* Bluetooth host stack emulation.
* 
***************************************************************************      
**\section group_ble_architecture PSoC 6 BLE Middleware Architecture
**
* \htmlonly <style>div.image img[src="ble_architecture_single_cpu_mode.png"]{width:650px;}</style> \endhtmlonly 
* \htmlonly <style>div.image img[src="ble_architecture_hci_mode.png"]{width:650px;}</style> \endhtmlonly 
*
* PSoC 6 BLE Middleware consists of the BLE Stack, BLE Profile, Hardware Abstraction Layer (HAL), and the Link
* Layer. The following figure shows the high-level architecture of the PSoC 6 BLE Middleware, illustrating the
* relationship between each of the layers and how the application interacts with the PSoC 6 BLE Middleware. Note
* that the application uses callback functions to handle BLE events, which you can use to build your state
* machine.
* 
* The following block diagram shows the architecture of PSoC 6 BLE Middleware under Complete BLE Protocol mode.
* \image html ble_architecture_single_cpu_mode.png
*  
* The following block diagram shows the architecture of PSoC 6 BLE Middleware under Controller Only mode
* (HCI over Software API).
* \image html ble_architecture_hci_mode.png
*  
* 
* The following sub-sections give an overview of each of these layers.
*
**\subsection  group_ble_stack BLE Stack
*
* \htmlonly <style>div.image img[src="ble_stack.png"]{width:550pxpx;}</style> \endhtmlonly 
*
* The BLE stack implements the core BLE functionality as defined in Bluetooth Core Specification 5.0. The stack 
* is included as a pre-compiled library in the PSoC 6 BLE Middleware library. The BLE Stack implements the layered 
* architecture of the BLE protocol stack as shown in the following figure.
* 
* \image html ble_stack.png
* 
**\subsubsection subsubsection_ble_stack_1 Generic Access Profile (GAP) 
* The Generic Access Profile defines the procedures related to discovery of Bluetooth devices and the link-
* management aspects of connecting to Bluetooth devices. In addition, this profile includes common format 
* requirements for parameters accessible on the user interface level. 
*  
* The Generic Access Profile defines the following roles when operating over the LE physical channel:
*   - <b>Broadcaster role</b>: A device operating in the Broadcaster role can send advertising events. It is 
*   referred to as a Broadcaster. It has a transmitter and may have a receiver.
* 
*   - <b>Observer role</b>: A device operating in the Observer role is a device that receives advertising 
*   events. It is referred to as an Observer. It has a receiver and may have a transmitter. 
*
*   - <b>Peripheral role</b>: A device that accepts an LE physical link using any of the connection 
*   establishment procedures is said to have a Peripheral role. A device operating in the Peripheral 
*   role will be in the "Slave role" in the Link Layer Connection State. A device operating in the 
*   Peripheral role is referred to as a Peripheral. A Peripheral has both a transmitter and a receiver.
*
*   - <b>Central role</b>: A device that supports the Central role initiates the establishment of a physical 
*   connection. A device operating in the "Central role" will be in the "Master role" in the Link Layer 
*   Connection. A device operating in the Central role is referred to as a Central. A Central has a 
*   transmitter and a receiver.
*
**\subsubsection subsubsection_ble_stack_2 Generic Attribute Profile (GATT)
* 
* The Generic Attribute Profile defines a generic service framework using the ATT protocol layer. This 
* framework defines the procedures and formats of services and their characteristics. It defines the procedures 
* for Service, Characteristic, and Descriptor discovery, reading, writing, notifying, and indicating 
* Characteristics, as well as configuring the broadcast of Characteristics. GATT roles are:
*
* - <b>GATT Client</b>: This is the device that wants data. It initiates commands and requests to the GATT 
* Server. It can receive responses, indications, and notifications data sent by the GATT Server.
*
* - <b>GATT Server</b>: This is the device that has the data and accepts incoming commands and requests from 
* the GATT Client. It sends responses, indications, and notifications to a GATT Client. The BLE Stack 
* can support both roles simultaneously.
*
**\subsubsection subsubsection_ble_stack_3 Attribute Protocol (ATT)
* 
* The Attribute Protocol layer defines a Client/Server architecture above the BLE logical transport channel. 
* The attribute protocol allows a device referred to as the GATT Server to expose a set of attributes and their 
* associated values to a peer device referred to as the GATT Client. Attributes exposed by the GATT Server can 
* be discovered, read, and written by a GATT Client, and can be indicated and notified by the GATT Server. All 
* the transactions on attributes are atomic.
*
**\subsubsection subsubsection_ble_stack_4 Security Manager Protocol (SMP)
* 
* The Security Manager Protocol defines the procedures and behavior to manage pairing, authentication, and 
* encryption between the devices. These include:
* - Encryption and Authentication
* - Pairing and Bonding
*   - Pass Key and Out of Band bonding
* - Key Generation for a device identity resolution, data signing, and encryption
* - Pairing method selection based on the IO capability of the GAP central and GAP peripheral device.
*
**\subsubsection subsubsection_ble_stack_5 Logical Link Control Adaptation Protocol (L2CAP)  
*  
* L2CAP provides a connectionless data channel. LE L2CAP provides the following features:
* - Channel multiplexing, which manages three fixed channels. Two channels are dedicated for higher protocol 
* layers like ATT, SMP. One channel is used by the LE-L2CAP protocol signalling channel for its own use.
*
* - Segmentation and reassembly of packets whose size is up to the BLE Controller managed maximum packet size.
*
* - Connection-oriented channel over a specific application registered using the PSM (protocol service 
* multiplexer) channel. It implements credit-based flow control between two LE L2CAP entities. This feature can 
* be used for BLE applications that require transferring large chunks of data.
*
**\subsubsection subsubsection_ble_stack_6 Host Controller Interface (HCI)
*  
* The HCI layer implements a command, event, and data interface to allow link layer access from upper layers 
* such as GAP, L2CAP, and SMP.
*
**\subsubsection subsubsection_ble_stack_7 Link Layer (LL)
*  
* The Link Layer implements a command, event, and data interface to allow link layer access from upper layers 
* such as GAP, L2CAP, and SMP. The LL protocol manages the physical BLE connections between devices. It 
* supports all LL states such as Advertising, Scanning, Initiating, and Connecting (Master and Slave). It 
* implements all the key link control procedures such as LE Encryption, LE Connection Update, LE Channel 
* Update, and LE Ping. 
* 
* The Link Layer is a hardware-firmware co-implementation, where the key time critical LL functions are 
* implemented in the LL hardware. The LL firmware maintains and controls the key LL procedure state machines. 
* It supports all the BLE chip-specific low-power modes. The BLE Stack is a pre-compiled library. The 
* appropriate configuration of the BLE Stack library is linked during a build process based on the application. 
* The BLE Stack libraries are Arm Embedded Application Binary Interface (EABI) compliant and are compiled using 
* Arm compiler version 5.03.
*
**\subsection  group_ble_profile_layer Profile Layer
* 
* \htmlonly <style>div.image img[src="ble_profiles.png"]{width:1000px;}</style> \endhtmlonly 
*
* In BLE, data is organized into concepts called Profiles, Services, and Characteristics. 
* 
* - A <b>Profile</b> describes how devices connect to each other to find and use Services. The Profile 
* describes the type of application and the general expected behavior of that device. See the <b>Parameter 
* Configuration</b> section of ModusToolbox Bluetooth Configurator Guide for configuring the PSoC 6 BLE Middleware. 
* 
* - A <b>Service</b> is a collection of data entities called Characteristics. A Service is used to define a 
* certain function in a Profile. A Service may also define its relationship to other Services. A Service is 
* assigned a Universally Unique Identifier (UUID). This is 16 bits for SIG adopted Services and 128 bits for 
* custom Services. See the <b>GATT Settings Tab</b> section of ModusToolbox Bluetooth Configurator Guide for 
* information about adding Services to a Profile.
* 
* - A <b>Characteristic</b> contains a Value and the Descriptor that describes a Characteristic Value. It is an 
* attribute type for a specific piece of information within a Service. Like a Service, each Characteristic is 
* designated with a UUID; 16 bits for SIG adopted Characteristics and 128 bits for custom Characteristics. See 
* the <b>GATT Settings Tab</b> section of ModusToolbox Bluetooth Configurator Guide for information about adding 
* Characteristics and Descriptors. The following diagram shows the relationship between Profiles, Services, and 
* Characteristics in a sample BLE heart rate monitor application using a Heart Rate Profile.
* 
* \image html ble_profiles.png
* 
* The Heart Rate Profile contains a Heart Rate Service and a Device Information Service. Within the Heart Rate 
* Service, there are three Characteristics, each containing different information. The device in the diagram is 
* configured in the Sensor role, meaning that in the context of the Heart Rate Profile, the device is a GAP 
* Peripheral and a GATT Server. These concepts are explained in the \ref group_ble_stack description.
* 
* The Profile layer is provided by PSoC 6 BLE Middleware using the parameter configurations specified in the Bluetooth 
* Configurator. The Profile implements the Profile-specific attribute database and functions required for the 
* application. You can choose to configure the standard SIG adopted Profile and generate a design, or define a 
* Custom Profile required by an application.
*
**\subsection  group_ble_hal Hardware Abstraction Layer (HAL)
*
* The HAL implements the interface between the BLE stack and the underlying hardware. This layer is meant for 
* the stack only and it is not advisable to modify it.
* 
***********************************************************************************************************************
**\page page_ble_quick_start Quick Start Guide
**
* The PSoC 6 BLE Middleware can be used in various software environments such as ModusToolbox, etc. 
* Refer to \ref page_ble_toolchain page. The quickest way to get started is using the Code Examples. 
* Cypress Semiconductor continuously extends its portfolio of code examples at the [<b>Cypress Semiconductor website</b>]
* (http://www.cypress.com/) and at the [<b>Cypress Semiconductor GitHub</b>](https://github.com/cypresssemiconductor.com)
* site.
*
* You must accomplish several tasks to implement a BLE standard profile application:
*   - \ref group_ble_quick_start1
*   - \ref group_ble_quick_start2
*   - \ref group_ble_quick_start3
*   - \ref group_ble_quick_start4
*   - \ref group_ble_quick_start5
*   - \ref group_ble_quick_start6
* 
* The snippets below show an implementation of a simple Bluetooth SIG-defined standard profile called Find [Me 
* Profile (FMP)](https://www.bluetooth.org/docman/handlers/downloaddoc.ashx?doc_id=239389). BLE midlleware is 
* configured to operate in Complete BLE Protocol Single CPU mode. In this mode,the BLE functionality is 
* entirely on the CM4 CPU core. It uses a software interface for communication between the controller and host. 
* Refer to \ref page_ble_section_configuration_considerations for details on how to configure the BLE 
* middleware for operation in Complete BLE Protocol Dual CPU or Controller-only (HCI over Software API) modes.
* 
***************************************************************************      
**\section group_ble_quick_start1 PSoC 6 BLE Middleware configuration in BT Configurator
* 
* \htmlonly <style>div.image img[src="bt_config_general_tab.jpg"]{width:570px;}</style> \endhtmlonly 
* \htmlonly <style>div.image img[src="bt_config_gatt_tab.jpg"]{width:820px;}</style> \endhtmlonly 
* \htmlonly <style>div.image img[src="bt_config_gap_tab.jpg"]{width:820px;}</style> \endhtmlonly 
* \htmlonly <style>div.image img[src="bt_config_advervicement_packet.jpg"]{width:820px;}</style> \endhtmlonly 
* \htmlonly <style>div.image img[src="bt_config_advervicement_setting.jpg"]{width:820px;}</style> \endhtmlonly 
* \htmlonly <style>div.image img[src="bt_config_securiti_configuration.jpg"]{width:820px;}</style> \endhtmlonly 
*
* The [ModusToolbox BT Configurator Guide] (https://www.cypress.com/ModusToolboxBLEConfig) describes how to use 
* BT Configurator. The BT Configurator Tool can be launched in ModusToolbox IDE from the BLE personality, as 
* well as in stand-alone mode. Refer to [ModusToolbox Software Environment, Quick Start Guide, Documentation, 
* and Videos](https://www.cypress.com/products/modustoolbox-software-environment). 
* 
* For this Quick Start Guide, the PSoC 6 BLE Middleware is configured as the Find Me Target in the GAP Peripheral role 
* with the settings shown in the figures below:
* 
* <b>General Settings</b><br>
* Use the General tab for general configuration of the Bluetooth resource (e.g GAP role, max number of BLE 
* connection, etc).
* \image html bt_config_general_tab.jpg
*
* <b>GATT Settings</b><br> 
* Use the GATT Settings tab to configure Profile-specific parameters. For this Quick Start Guide we add only 
* Find Me profile (Find Me Target (GATT Server)).
* \image html bt_config_gatt_tab.jpg
*
* <b>GAP Settings</b><br> 
* Use the GAP Settings tab to configure GAP parameters, which define the general connection settings required 
* when connecting Bluetooth devices. It contains various parameters based on the item you select in the tree.
* 
* <i>General</i><br>
* In this panel you configure general GAP parameters (e.g. Public device address, Device Name,  Appearance, 
* Adv/Scan/Connections TX power level, etc).
* \image html bt_config_gap_tab.jpg
* 
* <i>Advertisement Settings</i><br>
* In this panel you configure the general parameters for advertisement configuration (e.g. Discovery mode, 
* Advertising type, Filter policy, Advertising Interval, etc).
* \image html bt_config_advervicement_setting.jpg
* 
* <i>Advertisement Packet</i><br>
* In this panel you configure the Advertisement data to be used in device advertisements.
* \image html bt_config_advervicement_packet.jpg
* 
* <i>Security Settings</i><br>
* In this panel you configure security settings for the device. The example uses a configuration that does not 
* require authentication, encryption, or authorization for data exchange.
* \image html bt_config_securiti_configuration.jpg
* 
* PSoC 6 BLE Middleware configuration is saved in files "cycfg_ble.h" and "cycfg_ble.c".
* These files must be attached to the user project.
*
***************************************************************************      
**\section group_ble_quick_start2 System and PSoC 6 BLE Middleware Initialization 
* 
* When PSoC 6 BLE MCU is reset, the application should perform system initialization which includes: setting up 
* the CPU cores for execution, enabling global interrupts, and enabling other components used in the design (
* UART, MCWDT, etc).  After the CPUs are initialized the application initializes the PSoC 6 BLE Middleware, which sets 
* up a complete BLE subsystem.
* 
* As part of the PSoC 6 BLE Middleware initialization, you must:
* - Initialize the BLESS interrupt. See \ref group_ble_section_conf_cons_intr for detailed information. The 
*   code below shows how to implement the routine (ISR) handler of the BLESS interrupt service (refer to 
*   BlessInterrupt() function). The Cy_BLE_BlessIsrHandler() function is called from BlessInterrupt() to process 
*   interrupt events generated by BLESS.
* - Register the generic event handler function to be called by the BLE stack to notify of pending events. 
*   Refer to Cy_BLE_RegisterEventCallback() function.
* - Call Cy_BLE_Init() to initialize PSoC 6 BLE Middleware. Pass the address of the configuration structure generated 
*   by the BT Configurator, cy_ble_config (exposed in cycfg_ble.c).
* - Call Cy_BLE_Enable() to enable the BLE subsystem.
* - Register service-specific event handlers (the example uses only the IAS event handler to handle Immediate 
*   Alert Service related events). Refer to Cy_BLE_IAS_RegisterAttrCallback() function.
*
* \snippet ble/sut_quick_start_guide/main.c BLE Quick Start Guide: System and PSoC 6 BLE Middleware Initialization
* 
***************************************************************************      
**\section group_ble_quick_start3 Main Loop Implementation
* The main loop in the example has a simply flow:
* - process the BLE stack pending events by a call to Cy_BLE_ProcessEvents()
* - process a received alert level: updates the LEDs based on the alert level
* - restarts the BLE stack timer to get a 1-second interval to blink the LED
* - the application goes into Low-power mode by call to the LowPowerImplementation() function.
*
* \snippet ble/sut_quick_start_guide/main.c BLE Quick Start Guide: Main Loop Implementation
* 
***************************************************************************      
**\section group_ble_quick_start4 Stack Event Handler
* 
* The BLE stack within the PSoC 6 BLE Middleware generates events. These events provide status and data to the 
* application firmware through the BLE stack event handler. 
* 
* The event handler must handle a few basic events from the stack. For the Find Me Target application in this 
* code example, the BLE stack event-handler must process the events described in table below. The actual code 
* recognizes and responds to additional events.
* 
* | BLE Stack Event Name                     | Event Description                                  | Event Handler Action 
* | :---                                     | :---                                               |:---                                                                                |
* | CY_BLE_EVT_STACK_ON                      | BLE stack initialization is completed successfully | Start advertisement and reflect the advertisement state on the LED
* | CY_BLE_EVT_GAP_DEVICE_DISCONNECTED       | BLE link with the peer device is disconnected      | Restart advertisement and reflect the advertisement state on the LED
* | CY_BLE_EVT_GAP_DEVICE_CONNECTED          | BLE link with the peer device is established       | Update the BLE link state on the LED
* | CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP | BLE stack advertisement start/stop event           | Shutdown the BLE stack
* | CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE       | BLE stack has been shutdown                        | Put the device into Hibernate mode
* | CY_BLE_EVT_GAP_AUTH_REQ                  | BLE authentication request received                | Call Cy_BLE_GAPP_AuthReqReply() to reply to the authentication request from Central
* 
* The code snippets show examples of how the event handler responds to an identified events. See the actual 
* source code for a complete understanding.
* 
* \code
* /*******************************************************************************
* * Function Name: AppCallBack
* ********************************************************************************
* * Summary: This is an event callback function to receive events from the PSoC 6 BLE Middleware.
* *
* * Parameters:
* *   event      - The event code.
* *   eventParam - The event parameters.
* *
* *******************************************************************************/
* void AppCallBack(uint32_t event, void *eventParam)
* {
*     switch(event)
*     {
*       /* Generic events (e.g CY_BLE_EVT_STACK_ON, CY_BLE_EVT_STACK_BUSY_STATUS, etc) */
*       . . .
*     
*       /* GAP Events (e.g CY_BLE_EVT_GAP_AUTH_REQ, CY_BLE_EVT_GAP_AUTH_COMPLETE, CY_BLE_EVT_GAP_DEVICE_CONNECTED, etc) */
*        . . .
*      
*       /* GATT Events (e.g CY_BLE_EVT_GATT_CONNECT_IND, CY_BLE_EVT_GATT_DISCONNECT_IND, etc) */
*        . . .
* 
*       /* Other Events */
*        . . .
*     }
* }
* \endcode
* 
* In this snippet, the handler starts advertising in response to the CY_BLE_EVT_STACK_ON event.
* \snippet ble/sut_quick_start_guide/main.c BLE Quick Start Guide: CY_BLE_EVT_STACK_ON
* 
* In this snippet, the handler responds to the "advertisement start/stop" event. The code toggles the LEDs 
* appropriately. If advertising has started, the advertisement LED turns on. The disconnect LED turns off, 
* because the device started advertising and is ready for connection. If advertising stops, the code sets the 
* LEDs appropriately, and sets a flag to enter Hibernate mode.
* \snippet ble/sut_quick_start_guide/main.c BLE Quick Start Guide: CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP
*
* In this snippet, the handler responds to the "disconnected" event. It starts advertising and sets the
* LEDs correctly.
* \snippet ble/sut_quick_start_guide/main.c BLE Quick Start Guide: CY_BLE_EVT_GAP_DEVICE_DISCONNECTED
*
* In this snippet, the handler receives a timeout from BLE stack timer. In this example the BLE stack timer is 
* used to get a 1-second interval to  blink the LED.
* \snippet ble/sut_quick_start_guide/main.c BLE Quick Start Guide: CY_BLE_EVT_TIMEOUT
*
* In this snippet, the handler puts the device into Hibernate mode in response to the 
* CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE event.
* \snippet ble/sut_quick_start_guide/main.c BLE Quick Start Guide: CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE
*
* In this snippet, the handler responds to the authentication request from the Central device. The peripheral 
* device must call Cy_BLE_GAPP_AuthReqReply() to reply to the authentication request from the Central.
* \snippet ble/sut_quick_start_guide/main.c BLE Quick Start Guide: CY_BLE_EVT_GAP_AUTH_REQ
* 
***************************************************************************      
**\section group_ble_quick_start5 Service-specific Event Handler
* 
* The PSoC 6 BLE Middleware also generates events corresponding to each of the services supported by the design. For 
* the Find Me Target application, the PSoC 6 BLE Middleware generates IAS events that let the application know that 
* the Alert Level characteristic has been updated with a new value. The event handler gets the new value and 
* stores it in the variable alertLevel. The main loop toggles the alert LED based on the current alert level. 
* 
* The code snippet shows how the firmware accomplishes this task.
* \snippet ble/sut_quick_start_guide/main.c BLE Quick Start Guide: IAS Event Handler
*
***************************************************************************      
**\section group_ble_quick_start6 Implement Low-power Performance
* 
* In the application process, the CPU processes the pending BLE events. If there are no events pending, the CPU 
* enters the deep sleep power mode.
* \snippet ble/sut_quick_start_guide/main.c BLE Quick Start Guide: Low Power Implementation
*
* 
***********************************************************************************************************************
**\page page_ble_section_configuration_considerations Configuration Considerations
**
* \htmlonly <style>div.image img[src="ble_config_common_flow.png"]{width:800px; align:left;}</style> \endhtmlonly 
* 
* This section explains how to configure the Bluetooth Low Energy (BLE) middleware for operation in either:
* 1. Complete BLE Protocol Single and Dual CPU modes
* 2. Controller-only mode (HCI over Software API)
* 
* The following figure shows the common flow for PSoC 6 BLE Middleware configuration:
* \image html ble_config_common_flow.png
* 
* Refer to the following sections for BLE configuration details:
*     - \ref group_ble_section_conf_cons_gui
*     - \ref group_ble_section_conf_cons_prebuild
*         - \ref group_ble_subsection_conf_cons_libraries
*         - \ref group_ble_subsection_conf_cons_precompiled
*     - \ref group_ble_section_conf_cons_intr
*     - \ref group_ble_section_conf_cons_init_enable
*         - \ref group_ble_subsection_conf_cons_single
*         - \ref group_ble_subsection_conf_cons_dual
*     - \ref group_ble_section_conf_cons_init_enable_hci
*     - \ref group_ble_section_conf_cons_ota_share_code
*
***************************************************************************      
**\section group_ble_section_conf_cons_gui Generate Configuration in the BT Configurator
*  
* 1. Run the stand-alone BT Configurator to configure the BLE design. 
* 2. After configuring the BLE General, GAP, GATT, and Link layer settings, save the generated source 
*    and header files. 
* 3. Add these files to your application. 
* 
* The generated files contain the BLE configuration structure, cy_ble_config (type of cy_stc_ble_config_t ), 
* and a set of macros that can be used in the application. Refer to [ModusToolbox BT Configurator Tool Guide]
* (https://www.cypress.com/ModusToolboxBLEConfig). 
* 
***************************************************************************      
**\section group_ble_section_conf_cons_prebuild BLE Stack Components and CM0+ Pre-built Images
*  
* 
* The BLE stack components and the pre-compiled image for the CM0+ BLE Sub-system (BLESS) controller are parts 
* of the PSoC 6 BLE Middleware that include different variants of pre-built BLE stack libraries. 
* 
* The user may specify the BLE stack components and CM0+ pre-built image via the COMPONENTS variable in the 
* application level Makefile. The following table describes all the BLE stack components, pre-built images, and 
* the relationship between them in different BLE operating modes:
* 
* \htmlonly
* <table class="doxtable" width="90%" border="1" align="center" cellpadding="3" cellspacing="0" 
*        style="width: 100%; font-size: 14px;">
*     <tr bgcolor="#D7EFEE">
*       <td rowspan="3" valign="center" style="text-align: center;">
*         <b>Components</b><br>(libraries / pre-built images) 
*       </td>
*       <td rowspan="3" valign="center" style="text-align: center; font-weight: bold;">
*         Description
*       </td>
*       <td colspan="3" align="center" style="text-align: center; font-weight: bold;">
*         BLE Operation Modes
*       </td>
*     </tr>
*     <tr bgcolor="#D7EFEE">
*       <td colspan="2" style="text-align: center;">
*         Complete BLE Protocol</td>
*       <td rowspan="2" valign="center" nowrap="nowrap" style="text-align: center;">
*         BLE HCI<br>(over Software API)
*       </td>
*     </tr>
*     <tr bgcolor="#D7EFEE">
*       <td valign="center" nowrap="nowrap" style="text-align: center;">
*         BLE Dual<br>CPU Mode<br>(Host part)
*       </td>
*       <td valign="center" nowrap="nowrap" style="text-align: center;">
*         BLE Single<br>CPU Mode
*       </td>
*     </tr>
*     <tr>
*       <td colspan="1">BLESS_HOST_IPC</td>
*       <td colspan="1">
*         BLE stack host (over IPC) pre-built libraries that run on the CM4 core in BLE Dual CPU 
*         mode. Must be complemented with the CM0+ BLESS controller pre-compiled image (CM0P_BLESS).
*         There are soft FP and hard FP variants.
*       </td>
*       <td colspan="1" style="text-align: center;"><strong>Y</strong></td>
*       <td> </td>
*       <td> </td>
*     </tr>
*     <tr>
*       <td colspan="1">BLESS_HOST</td>
*       <td colspan="1">
*         BLE stack host (over software transport interface) pre-built libraries that run on the 
*         CM4 core in BLE Single CPU mode. Must be complemented with the BLE Stack controller (over 
*         software transport interface) component (BLESS CONTROLLER). There are soft FP and hard FP 
*         variants.
*       </td>
*       <td> </td>
*       <td colspan="1" style="text-align: center;"><strong>Y</strong></td>
*       <td> </td>
*     </tr>
*     <tr>
*       <td colspan="1">BLESS_CONTROLLER</td>
*       <td colspan="1">
*         BLE stack controller (over software transport interface) pre-built libraries that run on 
*         the CM4 core in BLE Single CPU or Host Controller Interface (HCI) modes. There are soft 
*         FP and hard FP variants.
*       </td>
*       <td> </td>
*       <td colspan="1" style="text-align: center;"><strong>Y</strong></td>
*       <td colspan="1" style="text-align: center;"><strong>Y</strong></td>
*     </tr>
*     <tr bgcolor="#F4F4EF">
*       <td colspan="1">CM0P_BLESS</td>
*       <td colspan="1">
*         This image has the BLE controller implementation. It starts the CM4 core at 
*         CY_CORTEX_M4_APPL_ADDR=0x10020000. It then goes into a while loop where it processes BLE 
*         controller events and puts the CM0+ core into CPU deep sleep.
*       </td>
*       <td colspan="1" style="text-align: center;"><strong>Y</strong></td>
*       <td> </td>
*       <td> </td>
*     </tr>
*     <tr bgcolor="#F4F4EF">
*       <td colspan="1">CM0P_SLEEP</td>
*       <td colspan="1">
*         This image starts the CM4 core at CY_CORTEX_M4_APPL_ADDR=0x10002000 and puts the CM0+ 
*         core into CPU deep sleep.
*       </td>
*       <td> </td>
*       <td colspan="1" style="text-align: center;"><strong>(Y)</strong></td>
*       <td colspan="1" style="text-align: center;"><strong>(Y)</strong></td>
*     </tr>
*     <tr bgcolor="#F4F4EF">
*       <td colspan="1">Other CM0+ images</td>
*       <td colspan="1">
*         Pre-compiled application images executed on the Cortex M0+ core of the PSoC 6 Dual-core 
*         MCU. The images are provided as C arrays ready to be compiled as part of the Cortex M4 
*         application. The Cortex M0+ application code is placed in internal flash by the Cortex M4 
*         linker script.
*       </td>
*       <td> </td>
*       <td colspan="1" style="text-align: center;"><strong>(Y)</strong></td>
*       <td colspan="1" style="text-align: center;"><strong>(Y)</strong></td>
*     </tr>
* </table>
* \endhtmlonly
* 
* The following fragments of the application level Makefile show how to enable different BLE modes.
* \htmlonly
* <table class="doxtable" width="90%" border="1" align="center" cellpadding="3" cellspacing="0" style="width: 100%; font-size: 14px;">
*     <tr bgcolor="#D7EFEE" >
*       <td style="text-align: center; font-weight: bold;">Dual CPU mode</td>
*       <td style="text-align: center; font-weight: bold;">Single CPU mode</td>
*       <td style="text-align: center; font-weight: bold;">HCI mode</td>
*     </tr>
*     <tr bgcolor="#FFFFFF" valign="top">
*       <td>APPNAME=BLE_APP_DUAL_CORE<br>
*           COMPONENTS=BLESS_HOST_IPC CM0_BLESS<br>         
*           # NOTE: CM0_BLESS - is a pre-built image with<br>
*           # BLESS controller<br>
*       </td>
*       <td>APPNAME=BLE_APP_SINGLE_CORE<br>
*           COMPONENTS=BLESS_HOST BLESS_CONTROLLER<br>
*       </td>
*       <td>APPNAME=BLE_HCI_APP<br>
*           COMPONENTS=BLESS_CONTROLLER<br>
*       </td>
*     </tr> 
* </table>
* \endhtmlonly
* 
**\subsection group_ble_subsection_conf_cons_libraries BLE Stack Libraries
* 
* The BLE stack libraries are compliant with the Arm Embedded Application Binary Interface
* (EABI). They are compiled with the Arm compiler version 5.03. The following table shows 
* the mapping between the BLE stack libraries and the user-configured COMPONENT:
* 
* \htmlonly
* <table class="doxtable" width="90%" border="1" align="center" cellpadding="3" cellspacing="0" style="width: 100%; font-size: 14px;">
*     <tr bgcolor="#D7EFEE" style="text-align: center; font-weight: bold;">
*       <td>COMPONENT Name</td>
*       <td>Library Name/Path in PSoC 6 BLE Middleware</td>
*       <td>Used for<br>Toolchains<br>(WCHAR)</td>
*       <td>SOFT/HARD<br>Floating<br>Point</td>
*     </tr>
*     <tr>
*       <td rowspan="6" align="center">BLESS_HOST_IPC</td>
*       <td>COMPONENT_BLESS_HOST_IPC/COMPONENT_SOFTFP/TOOLCHAIN_GCC_ARM/cy_ble_stack_host.a</td>
*       <td rowspan="2" align="center">GCC<br>(WCHAR 32)</td>
*       <td align="center">SOFTFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_HOST_IPC/COMPONENT_HARDFP/TOOLCHAIN_GCC_ARM/cy_ble_stack_host.a</td>
*       <td align="center">HARDFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_HOST_IPC/COMPONENT_SOFTFP/TOOLCHAIN_IAR/cy_ble_stack_host.a</td>
*       <td rowspan="2" align="center">IAR<br>(WCHAR 32)</td>
*       <td align="center">SOFTFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_HOST_IPC/COMPONENT_HARDFP/TOOLCHAIN_IAR/cy_ble_stack_host.a</td>
*       <td align="center">HARDFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_HOST_IPC/COMPONENT_SOFTFP/TOOLCHAIN_ARM/cy_ble_stack_host.ar</td>
*       <td rowspan="2" align="center">Arm<br>Compiler 6<br>(WCHAR 16)</td>
*       <td align="center">SOFTFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_HOST_IPC/COMPONENT_HARDFP/TOOLCHAIN_ARM/cy_ble_stack_host.ar</td>
*       <td align="center">HARDFP</td>
*     </tr>
*     <tr>
*       <td rowspan="6" align="center">BLESS_HOST</td>
*       <td>COMPONENT_BLESS_HOST/COMPONENT_SOFTFP/TOOLCHAIN_GCC_ARM/cy_ble_stack_host.a</td>
*       <td rowspan="2" align="center">GCC<br>(WCHAR 32)</td>
*       <td align="center">SOFTFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_HOST/COMPONENT_HARDFP/TOOLCHAIN_GCC_ARM/ cy_ble_stack_host.a</td>
*       <td align="center">HARDFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_HOST/COMPONENT_SOFTFP/TOOLCHAIN_IAR/ cy_ble_stack_host.a</td>
*       <td rowspan="2" align="center">IAR<br>(WCHAR 32)</td>
*       <td align="center">SOFTFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_HOST/COMPONENT_HARDFP/TOOLCHAIN_IAR/cy_ble_stack_host.a</td>
*       <td align="center">HARDFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_HOST/COMPONENT_SOFTFP/TOOLCHAIN_ARM/cy_ble_stack_host.ar</td>
*       <td rowspan="2" align="center">Arm<br>Compiler 6<br>(WCHAR 16)</td>
*       <td align="center">SOFTFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_HOST/COMPONENT_HARDFP/TOOLCHAIN_ARM/cy_ble_stack_host.ar</td>
*       <td align="center">HARDFP</td>
*     </tr>
*     <tr>
*       <td rowspan="12" align="center">BLESS_CONTROLLER</td>
*       <td>COMPONENT_BLESS_CONTROLLER/COMPONENT_SOFTFP/TOOLCHAIN_GCC_ARM/cy_ble_stack_controller.a</td>
*       <td rowspan="4" align="center">GCC<br>(WCHAR 32)</td>
*       <td rowspan="2" align="center">SOFTFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_CONTROLLER/COMPONENT_SOFTFP/TOOLCHAIN_GCC_ARM/cy_ble_stack_manager.a</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_CONTROLLER/COMPONENT_HARDFP/TOOLCHAIN_GCC_ARM/cy_ble_stack_controller.a</td>
*       <td rowspan="2" align="center">HARDFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_CONTROLLER/COMPONENT_HARDFP/TOOLCHAIN_GCC_ARM/cy_ble_stack_manager.a</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_CONTROLLER/COMPONENT_SOFTFP/TOOLCHAIN_IAR/cy_ble_stack_controller.a</td>
*       <td rowspan="4" align="center">IAR<br>(WCHAR 32)</td>
*       <td rowspan="2" align="center">SOFTFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_CONTROLLER/COMPONENT_SOFTFP/TOOLCHAIN_IAR/cy_ble_stack_manager.a</td>
*     </tr>
*     <tr>
*       <td>COMPONENT_BLESS_CONTROLLER/COMPONENT_HARDFP/TOOLCHAIN_IAR/cy_ble_stack_controller.a</td>
*       <td rowspan="2" align="center">HARDFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_CONTROLLER/COMPONENT_HARDFP/TOOLCHAIN_IAR/cy_ble_stack_manager.a</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_CONTROLLER/COMPONENT_SOFTFP/TOOLCHAIN_ARM/cy_ble_stack_controller.ar</td>
*       <td rowspan="4" align="center">Arm<br>Compiler 6<br>(WCHAR 16)</td>
*       <td rowspan="2" align="center">SOFTFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_CONTROLLER/COMPONENT_SOFTFP/TOOLCHAIN_ARM/cy_ble_stack_manager.ar</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_CONTROLLER/COMPONENT_HARDFP/TOOLCHAIN_ARM/cy_ble_stack_controller.ar</td>
*       <td rowspan="2" align="center">HARDFP</td>
*     </tr>
*     <tr>     
*       <td>COMPONENT_BLESS_CONTROLLER/COMPONENT_HARDFP/TOOLCHAIN_ARM/cy_ble_stack_manager.ar</td>
*     </tr>
* </table>
* \endhtmlonly
* 
**\subsection  group_ble_subsection_conf_cons_precompiled CM0+ BLESS Controller Pre-compiled Image
*
* Pre-compiled BLESS controller image executed on the Cortex M0+ core of the PSoC 6 dual- core MCU. The image 
* is provided as C arrays ready to be compiled as part of the Cortex M4 application. The Cortex M0+ application 
* code is placed in internal flash by the Cortex M4 linker script. This image is used only in BLE Dual CPU 
* mode. In this mode, the BLE functionality is split between CM0+ (controller) and CM4 (host). It uses IPC for 
* communication between two CPU cores where both the controller and host run.
* 
* A BLESS controller pre-built image executes the following steps: 
* 
* - configures a BLESS interrupt 
* - registers an IPC-pipe callback for the PSoC 6 BLE Middleware to initialize and enable the BLE  controller when 
*   the PSoC 6 BLE Middleware operates in BLE Dual CPU mode 
* - starts the CM4 core at CY_CORTEX_M4_APPL_ADDR=0x10020000 
* - goes into a while loop where it processes BLE controller events and puts the CM0+ core into CPU deep sleep. 
* 
* To use this image, update the ram, flash, and FLASH_CM0P_SIZE values in the linker script for CM4:
* 
* Example for the GCC compiler:
* \code
*   ...
*   MEMORY
*   {
*       ...
*       ram       (rwx)   : ORIGIN = 0x08003000, LENGTH = 0x044800
*       flash     (rx)    : ORIGIN = 0x10000000, LENGTH = 0x100000
*       ... 
*   }
*   ...
*   /* The size and start addresses of the Cortex-M0+ application image */
*   FLASH_CM0P_SIZE  = 0x20000;
*   ...
* \endcode
* 
* Example for the IAR compiler:
* \code
*   ...
*   /* RAM */
*   define symbol __ICFEDIT_region_IRAM1_start__ = 0x08003000;
*   define symbol __ICFEDIT_region_IRAM1_end__   = 0x08047800;
*   /* Flash */
*   define symbol __ICFEDIT_region_IROM1_start__ = 0x10000000;
*   define symbol __ICFEDIT_region_IROM1_end__   = 0x10100000;
*   ...
*   /* The size and start addresses of the Cortex-M0+ application image */
*   define symbol FLASH_CM0P_SIZE  = 0x20000;
*   ...
* \endcode
* 
* Example for the IAR compiler:
* \code
*   ...
*   ; RAM
*   #define RAM_START               0x08003000
*   #define RAM_SIZE                0x00044800
*   ; Flash
*   #define FLASH_START             0x10000000
*   #define FLASH_SIZE              0x00100000
*   ...
*   /* The size and start addresses of the Cortex-M0+ application image */
*   #define FLASH_CM0P_SIZE         0x20000
*   ...
* \endcode
* 
***************************************************************************
*\section  group_ble_section_conf_cons_intr Configure BLESS Interrupt 
**
* The interrupt is mandatory for PSoC 6 BLE Middleware operation. The BLESS hardware block provides interrupt sources. 
* To configure interrupts, the Cy_BLE_BlessIsrHandler() function is called in the interrupt handler to process 
* interrupt events generated by BLESS.
* 
* The BLESS interrupt is configured on the core where the BLE controller is running. The following table shows 
* details of interrupt configuration depending on the BLE core modes. 
*
* \htmlonly
* <table class="doxtable" width="90%" border="1" align="center" cellpadding="3"
*        cellspacing="0" style="width: 100%; font-size: 14px;">
*     <tr bgcolor="#D7EFEE" >
*       <td style="text-align: center; font-weight: bold;">BLE Core mode</td>
*       <td style="text-align: center; font-weight: bold;">BLE Controller core</td>
*       <td style="text-align: center; font-weight: bold;">BLESS Interrupt configuration</td>
*     </tr>
*     <tr bgcolor="#FFFFFF" valign="top">
*       <td style="text-align: center;">Dual CPU mode</td>
*       <td style="text-align: center;">CM0+</td>
*       <td>    The BLESS interrupt is configured in the CM0+ BLESS controller pre-built image.</td>
*     </tr>
*     <tr bgcolor="#FFFFFF">
*       <td valign="center" style="text-align: center;">Single CPU mode</td>
*       <td valign="center" style="text-align: center;">CM4</td>
*       <td>
*            cy_stc_sysint_t blessIsrCfg =             <br>
*            {                                         <br>&nbsp; &nbsp; &nbsp; &nbsp;      
*                 /* The BLESS interrupt */            <br>&nbsp; &nbsp; &nbsp; &nbsp;
*                 .intrSrc = bless_interrupt_IRQn,     <br>&nbsp; &nbsp; &nbsp; &nbsp;
*                                                      <br>&nbsp; &nbsp; &nbsp; &nbsp;
*                 /* The interrupt priority number */  <br>&nbsp; &nbsp; &nbsp; &nbsp;
*                 .intrPriority = 1u                   <br>
*            };<br>                     
*        <b>Note.</b> Priority level (intrPriority ) must have the highest value in system.</td>
*     </tr>
*     
* </table>
* \endhtmlonly
* 
* The following code shows how to implement the ISR handler for the BLESS interrupt service. The 
* Cy_BLE_BlessIsrHandler() function is called from BLESS ISR to process interrupt events generated by BLESS:
* \snippet ble/snippet/main.c usage Cy_BLE_BlessIsrHandler
* 
* Finally, the BLESS interrupt is configured and interrupt handler routines are hooked up to NVIC. The pointer 
* to the BLESS interrupt configuration structure (blessIsrCfg) is stored in the BLE configuration structure (
* cy_ble_config): 
* \snippet ble/snippet/main.c BLESS interrupt configure
* 
****************************************************************************
**\section  group_ble_section_conf_cons_init_enable Initialize and Enable PSoC 6 BLE Middleware in Complete BLE Protocol Mode
*
* \htmlonly <style>div.image img[src="ble_config_single_mode.png"]{width:864px; align:left;}</style> \endhtmlonly 
* 
**\subsection  group_ble_subsection_conf_cons_single Complete BLE Protocol Single CPU mode
* 
* In this mode, the BLE functionality is entirely on the CM4 CPU core. It uses a software interface for 
* communication between the controller and host. The following figure shows the configuration flow for the BLE 
* middleware in Single CPU mode. 
* 
* \image html ble_config_single_mode.png
* 
* 1. Generate configuration in the BT Configurator, refer to 
*    [ModusToolbox BT Configurator Tool Guide](https://www.cypress.com/ModusToolboxBLEConfig)
* 2. Specify the following BLE Stack Component and CM0+ Pre-built:
*     - BLESS_HOST,
*     - BLESS_CONTROLLER, 
*     - CM0P_SLEEP or other CM0+ image (e.g. CM0P_CRYPTO, etc).<br>
*    Refer to section \ref group_ble_section_conf_cons_prebuild for detailed information.
* 3. Initialize the BLESS interrupt as shown in section \ref group_ble_section_conf_cons_intr.
* 4. Initialize and enable the PSoC 6 BLE Middleware in the application code:
* <ol type="a">
*   <li>Call the Cy_BLE_RegisterEventCallback() function to register the generic callback function. The callback 
*       function is of type cy_ble_callback_t, as defined by: void (*cy_ble_callback_t)(uint32_t eventCode, void 
*       *eventParam).</li>
*   <li>Call Cy_BLE_Init(&cy_ble_config), where cy_ble_config is the configuration structure exposed in cycfg_ble.c
*       (generated by the BT Configurator).</li>
*   <li>Call Cy_BLE_Enable() to enable the BLE host and controller.</li>
*   <li>Optionally, to enable BLE low-power mode, call the Cy_BLE_EnableLowPowerMode() function.</li>
*   <li>Register the service-specific callback functions to be used. For example, use Cy_BLE_IAS_RegisterAttrCallback(
*       IasEventHandler) to register service-specific callback function IasEventHandler for the Immediate Alert 
*       service. This function is also of type cy_ble_callback_t and is passed as a parameter to the service-specific 
*       callback registration function. The callback function is used to evaluate service-specific events and to take 
*       the appropriate action as defined by your application. Then, build a service-specific state machine using 
*       these events.</li>
* </ol>
*
**\subsection  group_ble_subsection_conf_cons_dual Complete BLE Protocol Dual CPU Mode
* 
* \htmlonly <style>div.image img[src="ble_config_dual_mode.png"]{width:864px; align:left;}</style> \endhtmlonly 
* 
* In this mode, the BLE functionality is split between the CM0+ (controller) and CM4 (host) CPU cores. The 
* controller part is implemented in BLESS controller CM0+ pre-built image. It uses IPC for communication 
* between the two CPU cores that run the controller and host.The following figure shows the configuration flow 
* for the PSoC 6 BLE Middleware in Dual CPU mode:
* 
* \image html ble_config_dual_mode.png
* 
* 1. Generate configuration in the BT Configurator, refer to 
*    [ModusToolbox BT Configurator Tool Guide](https://www.cypress.com/ModusToolboxBLEConfig)
* 2. Specify the following BLE Stack Component and CM0+ Pre-built:
*     - BLESS_HOST_IPC,
*     - CM0P_BLESS.<br>
*    Refer to section \ref group_ble_section_conf_cons_prebuild for detailed information.
* 3. The BLESS interrupt is initialized in the CM0+ BLESS controller pre-built image, so you don't need to do this.
* 4. Initialize and enable the PSoC 6 BLE Middleware in the application code:
* <ol type="a">
*   <li>Call the Cy_BLE_RegisterEventCallback() function to register the generic callback function. The callback 
*       function is of type cy_ble_callback_t, as defined by: void (*cy_ble_callback_t)(uint32_t eventCode, void 
*       *eventParam).</li>
*   <li>Call Cy_BLE_Init(&cy_ble_config), where cy_ble_config is the configuration structure exposed in cycfg_ble.c
*       (generated by the BT Configurator).</li>
*   <li>Call Cy_BLE_Enable() to enable the BLE host and controller.</li>
*   <li>Optionally, to enable BLE low-power mode, call the Cy_BLE_EnableLowPowerMode() function.</li>
*   <li>Register the service-specific callback functions to be used. For example, use Cy_BLE_IAS_RegisterAttrCallback(
*       IasEventHandler) to register service-specific callback function IasEventHandler for the Immediate Alert 
*       service. This function is also of type cy_ble_callback_t and is passed as a parameter to the service-specific 
*       callback registration function. The callback function is used to evaluate service-specific events and to take 
*       the appropriate action as defined by your application. Then, build a service-specific state machine using 
*       these events.</li>
* </ol>
* 
* The following code snippet shows the BLE initialization:
* \code
* void main(void)
* {
*     /* ... Initializes the BLE interrupt (skipped in this code snippet) ...*/
*     
*     /* Registers the generic callback functions  */
*     Cy_BLE_RegisterEventCallback(AppCallBack);
*     
*     /* Initializes the BLE host */
*     Cy_BLE_Init(&cy_ble_config);
* 
*     /* Enables BLE Low-power mode (LPM)*/
*     Cy_BLE_EnableLowPowerMode();
* 
*     /* Enables BLE */
*     Cy_BLE_Enable();
* 
*     /* Registers the service-specific callback functions (e.g. IAS) */
*     Cy_BLE_IAS_RegisterAttrCallback(IasEventHandler);
*  
*     for (;;)
*     {
*         /* Cy_BLE_ProcessEvents() allows the BLE stack to process pending events */
*         Cy_BLE_ProcessEvents();
*  
*         /* The main BLE application loop ... */      
*     }
* }
* \endcode
* 
*
**\section  group_ble_section_conf_cons_init_enable_hci Initialize and Enable PSoC 6 BLE Middleware in Controller-only Mode (HCI over Software API)
*
* \htmlonly <style>div.image img[src="ble_config_controller_only_mode.png"]{width:864px; align:left;}</style> \endhtmlonly 
* \image html ble_config_controller_only_mode.png
* 
* 1. In the BT Configurator, select the option BLE Controller-only (HCI) in the General Tab and generate 
* configuration., refer to [ModusToolbox BT Configurator Tool Guide](https://www.cypress.com/ModusToolboxBLEConfig)
* 2. Specify the following BLE Stack Component and CM0+ Pre-built:
*  - BLESS_CONTROLLER, 
*  - CM0P_SLEEP or other CM0+ image (e.g. CM0P_CRYPTO, etc).<br>
* Refer to section \ref group_ble_section_conf_cons_prebuild for detailed information.
* 3. Initialize the BLESS interrupt as shown in section \ref group_ble_section_conf_cons_intr.
* 4. Initialize and enable the PSoC 6 BLE Middleware in the application code:
* <ol type="a">
*  <li>Call Cy_BLE_RegisterEventCallback(StackEventHandler) to register an event callback function to 
*      receive events from the BLE controller.</li>
*  <li>Call Cy_BLE_Init(&cy_ble_config) where cy_ble_config is the configuration structure exposed in 
*      cycfg_ble.c (generated by the BLE Configurator).</li>
*  <li>Call Cy_BLE_Enable() to enable the BLE controller.</li>
* </ol>  
* 
* <i>Send an HCI packet to the BLE stack controller</i><br>
* Use the Cy_BLE_SoftHciSendAppPkt() function to send an HCI packet to the BLE stack controller. The 
* application allocates memory for the buffer to hold the HCI packet passed as an input parameter. This 
* function copies the HCI packet into the controller's HCI buffer. Hence, the application may deallocate the 
* memory buffer created to hold the HCI packet, once the API returns.  
* 
* <i>Receive an HCI event (or ACL packet) from BLE stack controller</i><br> 
* Use the Cy_BLE_SoftHciSendAppPkt() function to send an HCI packet to the BLE stack controller. The 
* application allocates memory for the buffer to hold the HCI packet passed as an input parameter. This 
* function copies the HCI packet into the controller's HCI buffer. Hence, the application may deallocate the 
* memory buffer created to hold the HCI packet, once the API returns. 
* 
*
**\section  group_ble_section_conf_cons_ota_share_code Over-The-Air Bootloading with Code Sharing
*
* This feature is used in the over-the-air (OTA) implementation. It allows sharing BLE component code between 
* two component instances: one instance with profile-specific code and one with a stack. To configure OTA with 
* code sharing, define CY_BLE_SHARING_MODE in the project. This parameter allows choosing between the following 
* options:
* 
* \htmlonly
* <table class="doxtable">
*     <tr align="center">
*         <th>Option</th>
*         <th>Value</th>
*         <th>Description</th>
*     </tr>
*     <tr>
*         <td align="center">Disabled</td>
*         <td align="center">0</td>
*         <td>OTA with code sharing feature is disabled.</td>
*     </tr>
*     <tr>
*         <td align="center">Stack and Profiles</td>
*         <td align="center">1</td>
*         <td>This option is used to isolate the stack and the application Profiles.<br>
*             Dynamically allocate memory for the BLE stack and pass the pointer to the memory heap into
*             BLE config structure (cy_ble_config.stackParam->memoryHeapPtr). 
*             The PSoC 6 BLE Middleware provides macro CY_BLE_STACK_RAM_SIZE which defines the ram size.
* 
*             The following code snippet shows how to dynamically allocate memory for BLE stack:<br>
*         \endhtmlonly
*         \code
*             /* Allocate memory for BLE stack */
*             cy_ble_config.stackParam->memoryHeapPtr = (uint8 *)malloc(CY_BLE_STACK_RAM_SIZE);
*             
*             if(cy_ble_config.stackParam->memoryHeapPtr == NULL)
*             {
*                 Cy_SysLib_Halt(0x00u);
*             }
*         \endcode
*         \htmlonly
*             <b>Note</b> This mode requires approximately 3024 additional bytes of heap memory. 
*             If there is not enough heap memory, the PSoC 6 BLE Middleware will not work. The Heap size can 
*             be modified by editing linker scripts.
*         </td>
*     </tr>
*     <tr>
*         <td align="center">Profile only</td>
*         <td align="center">2</td>
*         <td>This option makes the middleware only have the profile-specific code. Stack is excluded.</td>
*     </tr>
* </table>
* \endhtmlonly
* 
***********************************************************************************************************************
**\page page_ble_toolchain Supported Software and Tools
**
* This version of the PSoC 6 BLE Middleware was validated for compatibility with the following Software and Tools:
* 
* \htmlonly
* <table class="doxtable">
*   <tr>
*     <th>Software and Tools</th>
*     <th>Version</th>
*   </tr>
*   <tr>
*     <td>ModusToolbox Software Environment</td>
*     <td>2.0</td>
*   </tr>
*   <tr>
*     <td>- ModusToolbox Device Configurator</td>
*     <td>2.0</td>
*   </tr>
*   <tr>
*     <td>- ModusToolbox BT Personality in Device Configurator</td>
*     <td>1.1</td>
*   </tr>
*   <tr>
*     <td>- ModusToolbox BT Configurator tool</td>
*     <td>2.0</td>
*   </tr>
*   <tr>
*     <td>GCC Compiler</td>
*     <td>7.2.1</td>
*   </tr>
*   <tr>
*     <td>IAR Compiler</td>
*     <td>8.32</td>
*   </tr>
*   <tr>
*     <td>Arm Compiler 6 <sup><b>(Note  <sup>1</sup>)</b></sup></td>
*     <td>6.11</td>
*   </tr>
* </table>
* \endhtmlonly 
* 
* <b>Note <sup>1</sup></b> The PSoC 6 BLE Middleware includes the pre-compiled libraries for
* Arm Compiler 6. They are built with the following options to be compatible with 
* ModusToolbox and MBED:<br>
*    '-fshort-enums' - Set the size of an enumeration type to the smallest
*                      data type that can hold all enumerator values.<br>
*    '-fshort-wchar' - Set the size of wchar_t to 2 bytes.<br> 
* To operate in custom environments with Arm Compiler 6, apply the above mentioned build options.
* 
***********************************************************************************************************************
**\page page_ble_section_more_information More Information
**
*
*\section group_ble_operation_flow Operation Flow
*
* A typical application code implements three operating modes: initialization, normal operation, and low-power operation.
* Initialization should happen only at system power-up, or when waking from system hibernation. After initialization,
* the PSoC 6 BLE Middleware enters normal operation and periodically enters various degrees of low-power operation to conserve
* power.
*
* \par System Initialization 
* The initialization stage happens at system power-up or when waking from system hibernation. This stage sets up the
* platform and the PSoC 6 BLE Middleware parameters. The application code should also start the PSoC 6 BLE Middleware and set up the
* callback functions for the event callbacks that happen in the other modes of operation.
*
* \par System Normal Operation 
* Upon successful initialization of the PSoC 6 BLE Middleware or hibernate wakeup sequence, the PSoC 6 BLE Middleware enters normal 
* mode. Normal operation first establishes a BLE connection if it is not already connected. It should then call 
* Cy_BLE_ProcessEvents() to process all pending BLE events. When queue of BLE events is not full (Cy_BLE_GATT_GetBusyStatus() 
* returns CY_BLE_STACK_STATE_FREE), it can transmit any data that need to be communicated and enter low-power operation,
* unless there is another pending event. In that case it should execute the normal operation flow again.
* Processing of BLE events should be performed at least once in a BLE connection event period. The BLE connection event
* is configured by the Central device while establishing a connection.
*
* \par System Low-power Operation 
* When there are no pending interrupts in normal operation, the BLE Sub-System (BLESS) can be placed in low-power mode.
* All the high frequency blocks in BLESS are shutdown and the BLE link layer timing is maintained using low frequency 
* clocks (WCO or PILO). CPU will not be able to access the BLESS registers in this mode. If an event happens at any
* time in low-power mode, it should re-enter normal operation.
*
* \note The MCU and BLESS have separate power modes and are able to go to different power modes independent of each other.
*       The check marks in the following table show the possible combination of power modes of MCU and BLES
* \par
* <table class="doxtable"> 
*   <tr>
*     <th rowspan="2">BLESS<br>Power Modes</th>
*     <th colspan="5">MCU Power Modes</th>
*   </tr>
*   <tr>
*     <td>Active</td>
*     <td>Sleep</td>
*     <td>Deep sleep</td>
*     <td>Hibernate</td>
*   </tr>
*   <tr>
*     <td>Active (idle/Tx/Rx)</td>
*     <td align="center">Y</td>
*     <td align="center">Y</td>
*     <td></td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>Deep sleep (ECO off)</td>
*     <td align="center">Y</td>
*     <td align="center">Y</td>
*     <td align="center">Y</td>
*     <td></td>
*   </tr>
*   <tr>
*     <td>Off</td>
*     <td></td>
*     <td></td>
*     <td></td>
*     <td align="center">Y</td>
*   </tr>
* </table>
* 
* To better understand of BLE operation flow refer \ref page_ble_quick_start section based on simple BLE example project.
*
**    
**\section group_ble_device_bonding Device Bonding
**
* The PSoC 6 BLE Middleware stores the link key of a connection after pairing with the remote device. If a connection is lost and
* re-established, the devices use the previously stored key for the connection. The BLE stack updates the bonding data in
* RAM while the devices are connected. If the bonding data is to be retained during shutdown, the application can use
* Cy_BLE_StoreBondingData() API to write the bonding data from RAM to the dedicated flash location, as defined by the
* PSoC 6 BLE Middleware.
* 
* Refer to the [CE215121_BLE_HID_Keyboard]
* (https://github.com/cypresssemiconductorco/PSoC-6-MCU-BLE-Connectivity-Designs/tree/master/CE215121_BLE_HID_Keyboard/CE215121_BLE_HID_Keyboard.cydsn)
*  code example for usage details.
*
**
**\section group_ble_lfclk_conf LFCLK Configuration for BLESS Deep Sleep Mode
**
* The LFCLK configuration affects the PSoC 6 BLE Middleware's ability to operate in BLESS deep sleep mode. If the WCO or PILO are 
* chosen as source clock, then the PSoC 6 BLE Middleware BLESS deep sleep mode is available for use. However, if the ILO is chosen, 
* then the PSoC 6 BLE Middleware cannot enter BLESS deep sleep.
* 
* \note The PSoC 6 BLE Middleware uses the LFCLK only during BLESS deep sleep mode, so ILO inaccuracy does not affect BLE communication.
* 
**
**\subsection group_ble_lfclk_Pilo_conf Configuring PILO for BLESS Deep Sleep Mode
**
* The PSoC 6 BLE Middleware supports using the Precision ILO (PILO) (instead of the WCO) as the LF clock source.
* 
* Using the PILO has limitations:
*     - supports only a single connection
*     - supports only the Peripheral/Slave role
*     - not supported for preproduction hardware.
* 
* Using the PILO as the LF clock source requires:
*     - configure BLE ECO (AltHF) clock in ModusToolbox Device Configurator. The recommended frequency is 16 Mhz.
*     - initial trimming (call Cy_SysClk_PiloInitialTrim() function) and measuring the step size of the PILO trimming
*       (call Cy_SysClk_PiloUpdateTrimStep function) before BLE start.
* 
* Both actions must happen during every power-up to get closest to the target frequency.
* 
* The following code snippet shows usage: 
* \snippet ble/snippet/main.c LFCLK configuration: PILO calibration
* 
* \note Call trimming functions Cy_SysClk_PiloInitialTrim() and Cy_SysClk_PiloUpdateTrimStep() before Cy_BLE_Start(), 
*       on the CPU core running the BLE controller. 
* 
** 
**\section group_ble_multiconnection_support Multi-Connection Support
**
* The PSoC 6 BLE Middleware supports up to four simultaneous Multi-Master Multi-Slave (MMMS) BLE connections in any combination of roles.
* For example, it can be a master of four slave devices (4M), a master of three slave devices and a slave of another device 
* (3M1S), or a slave of four devices (4S), or any other combination.
* To configure the maximum number of BLE connections, refer to the <b>General Tab</b> section of ModusToolbox 
* Bluetooth Configurator Guide (Maximum number of BLE connections).
* The PSoC 6 BLE Middleware supports a single instance of a GATT Server (single GATT database). The number of the Server instance
* field is always fixed to 1 in the Bluetooth Configurator dialog. You can add additional Services or a complete Server Profile 
* to the existing Server tree and build a GATT database. This single GATT database is reused across all BLE connections.
* The PSoC 6 BLE Middleware manages multiple Client Characteristic Configuration Descriptor (CCCD) values. The CCCD value for each
* active connection is unique.
* 
* The maximum number of CCCD storage SRAM data structures supported by the PSoC 6 BLE Middleware is determined by the number of 
* active BLE connections/links that you select. The PSoC 6 BLE Middleware restores CCCD values in flash for each of the bonded 
* devices while establishing a connection with a peer device.
* 
* Use the connection handle at the application level to manage the multi-connection. The connection handle
* (\ref cy_stc_ble_conn_handle_t) appears when the connection is established (as the event parameter of the 
* \ref CY_BLE_EVT_GATT_CONNECT_IND event).
* 
* To work with a particular connection, BLE APIs (BLE Stack APIs, BLE Profile APIs) provide the parameter connHandle
* (e.g., Cy_BLE_BASS_SendNotification(cy_stc_ble_conn_handle_t connHandle,... ).)
* BLE events from the AppCallback function include a connHandle in eventParam structures to distinguish to which 
* connection this event relates. The following code shows how an application can manage connection handles:
* \snippet ble/snippet/main.c AppCallback: How to manage connection handles
* 
* Loop through all connected devices:
* \snippet ble/snippet/main.c BLE Common API: Cy_BLE_GetConnectionState()
* 
* Use the Cy_BLE_GetDeviceRole(cy_stc_ble_conn_handle_t *connHandle) function to discover the role of the link-layer 
* device connected to a peer device with the connection handle indicated by the connHandle parameter.
* Write attributes (characteristic, descriptors) requests from the peer device(s) for adopted services (e.g. BAS, HIDS,
* HRS, etc) handled by the PSoC 6 BLE Middleware.
* 
* For the Custom service, write attributes requests handled by the application level in the AppCallback () callback event 
* function.
* The following code shows the handling of the Write operation by a peer device for the Custom service:
* \snippet ble/snippet/main.c AppCallback: Handling Write operation for the Custom service
* 
* The common MMMS usage are Multi-Master Single-Slave and Multi-Role when the PSoC 6 BLE Middleware is configured in all
* the GAP roles (Central, Peripheral, Observer, and Broadcaster).
* 
* <b>Multi-Master Single-Slave Usage Block Diagram</b>
* \htmlonly <style>div.image img[src="ble_multi_master_single_slave_usage_block_diagram.png"]{width:732;}</style> \endhtmlonly 
* \image html ble_multi_master_single_slave_usage_block_diagram.png
* 
* <b>Multi-Role Usage Block Diagram</b>
* \htmlonly <style>div.image img[src="ble_multi_role_usage_block_diagram.png"]{width:775;}</style> \endhtmlonly 
* \image html ble_multi_role_usage_block_diagram.png
* 
* Refer to code examples [CE215118_BLE_Multi_Master_Single_Slave](https://github.com/cypresssemiconductorco/PSoC-6-MCU-BLE-Connectivity-Designs/tree/master/CE215118_BLE_Multi_Master_Single_Slave/CE215118_BLE_Multi_Master_Single_Slave.cydsn)
* and [CE215555_BLE_Multi_Role](https://github.com/cypresssemiconductorco/PSoC-6-MCU-BLE-Connectivity-Designs/tree/master/CE215555_BLE_Multi_Role/CE215555_BLE_Multi_Role.cydsn) 
* for more details.
* 
* 
**
**\section group_ble_interrupt_notify BLE Interrupt Notification Callback
**
* The PSoC 6 BLE Middleware exposes BLE interrupt notifications to the application which indicates a different link layer and radio
* state transitions to the user from the BLESS interrupt context.
* The user registers for a particular type of a callback and the PSoC 6 BLE Middleware will call that registered callback basing 
* on the registered mask (Refer to the Cy_BLE_RegisterInterruptCallback() and Cy_BLE_UnRegisterInterruptCallback() APIs). 
* All interrupts masks are specified in the \ref cy_en_ble_interrupt_callback_feature_t enumeration.
* 
* The possible interrupts which can trigger a user callback:
*     1. <b>CY_BLE_INTR_CALLBACK_BLESS_STACK_ISR</b> - Executed on every trigger of the BLESS interrupt.
*     2. <b>CY_BLE_INTR_CALLBACK_BLESS_INTR_STAT_DSM_EXITED</b> - Executed when the BLESS exits BLESS deep sleep mode and enters 
*     BLESS active mode. A BLESS deep sleep exit can be triggered automatically by link layer hardware or by different PSoC 6 BLE Middleware 
*     data transfer APIs, which needs the BLESS to be active.
*     3. <b>CY_BLE_INTR_CALLBACK_BLELL_CONN_EXT_INTR_EARLY</b> - Executed when the BLESS connection engine in Slave mode 
*     detects a BLE packet that matches its access address. 
*     4. <b>CY_BLE_INTR_CALLBACK_BLELL_CONN_INTR_CE_RX</b> - Executed when the BLESS connection engine receives a non-empty 
*     packet from the peer device
*     5. <b>CY_BLE_INTR_CALLBACK_BLELL_CONN_INTR_CE_TX_ACK</b> - Executed when the BLESS connection engine receives an ACK 
*     packet from the peer device for the previously transmitted packet.
*     6. <b>CY_BLE_INTR_CALLBACK_BLELL_CONN_INTR_CLOSE_CE</b> - Executed when the BLESS connection engine closes the 
*     connection event. This interrupt is executed on every connection interval for connection, irrespective of data tx/rx 
*     state.
*     7. <b>CY_BLE_INTR_CALLBACK_BLESS_INTR_STAT_DSM_ENTERED</b> - Executed when the BLESS enters deep sleep mode. A user 
*     call to the Cy_SysPm_DeepSleep() function will trigger the BLESS deep sleep entry sequence. A time instance when 
*     each of these interrupts (#1 to #7) are triggered in a connection event are shown below (green).
*     8. <b>CY_BLE_INTR_CALLBACK_BLELL_SCAN_INTR_ADV_RX</b> - Executed when the BLESS scan engine receives an advertisement 
*     packet from the peer device.
*     9. <b>CY_BLE_INTR_CALLBACK_BLELL_SCAN_INTR_SCAN_RSP_RX</b> - Executed when the BLESS scan engine receives a scan
*     response packet from the peer device in response to a scan request from the scanner.
*     10. <b>CY_BLE_INTR_CALLBACK_BLELL_ADV_INTR_CONN_REQ_RX</b> - Executed when the BLESS advertisement engine receives a 
*     connection request from the peer central device.
*  
* An application can use these interrupt callbacks to know when the RF activity is about to begin/end or when the BLE 
* device changes its state from advertisement to connected, or when the BLESS transitions between active and low-power 
* modes, etc. These BLESS real-time states can be used to synchronize an application with the BLESS or prevent radio 
* interference with other peripherals etc.
*     
* \image html ble_intr_notify_feature.png
*     
* To register / un-register a callback following APIs are available:
* \ref Cy_BLE_RegisterInterruptCallback() and \ref Cy_BLE_UnRegisterInterruptCallback().
* 
* 
**
**\section group_ble_unsupported_features Unsupported Features
**
* The PSoC 6 BLE Middleware stack does not support the following optional Bluetooth v5.0 protocol features, as listed in 
* Vol 6, Part B, section 4.6 of the specification:
*     - Connection Parameters Request Procedure (Vol 6, Part B, section 4.6.2)
*     - Extended Reject Indication (Vol 6, Part B, section 4.6.3)
*     - Slave-initiated Features Exchange (Vol 6, Part B, section 4.6.4)
*     - Stable Modulation Index - Transmitter (Vol 6, Part B, section 4.6.10)
*     - Stable Modulation Index - Receiver (Vol 6, Part B, section 4.6.11)
*     - LE Extended Advertising (Vol 6, Part B, section 4.6.12)
*     - LE Periodic Advertising (Vol 6, Part B, section 4.6.13)
*     - Channel Selection Algorithm #2 (Vol 6, Part B, section 4.6.14)
*     - Minimum Number of Used Channels Procedure (Vol 6, Part B, section 4.6.15)
* 
**
**\section group_ble_lpm Low-power Modes
**
* The PSoC 6 BLE Middleware automatically registers sleep and deep sleep callback functions. These functions request the BLE Stack 
* to put Bluetooth Low Energy Sub-System (BLESS) to low-power mode. 
* 
**
**\section group_ble_io_connections External PA/LNA Support 
**
* RF front ends come in a variety of combinations. The most popular and comprehensive RF front ends come with a built-in 
* power amplifier (PA) in the transmit path, a low-noise amplifier (LNA) in the receive path, a transmit/receive bypass 
* path to bypass both the PA and LNA, and RF switches to select between transmit, receive, and bypass paths. Some front 
* ends support multiple antennas. Some front ends have a built-in low-pass filter after the power amplifier. The discrete
* front ends also have almost the same configuration. 
* 
* \image html ble_ext_pa_lna_control.png
* 
* \par Chip Enable Signal
* This signal is needed to put the front end to sleep or in standby whenever there is no radio activity. The signal is   
* ON when either PA control or LNA control is ON.    
* 
* \par Tx ENABLE
* This signal is turned ON during transmission and turned OFF when not transmitting. This signal is active a little 
* earlier than the actual start of transmission to allow for the time it takes for the Power amplifier to ramp up.
* This delay can be set in the EXT_PA_LNA_DLY_CNFG register.
* 
* \par Rx ENABLE
* This signal is needed to choose between the bypass path and the LNA path. This signal is ON during reception and OFF
* when the receiver is OFF. This output is visible if the Enable external LNA Rx control output parameter is selected 
* on the Advanced tab.
* 
* The polarity of these signals are configurable and can be set in the EXT_PA_LNA_CTRL register by 
* Cy_BLE_ConfigureExtPA() API.
* 
* 
**
**\section group_ble_additional_resources Additional Resources
**
*
* For more information, refer to the following documents:
*
** <a href="https://www.cypress.com/products/modustoolbox-software-environment">
*   <b>ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos</b></a><br>
** <a href="https://github.com/cypresssemiconductorco"><b>PSoC 6 BLE Middleware Code Examples at GITHUB</b></a><br>
** <a href="https://www.cypress.com/ModusToolboxBLEConfig"><b>ModusToolbox BT Configurator Tool Guide</b></a><br>
** <a href="https://www.cypress.com/ModusToolboxDeviceConfig"><b>ModusToolbox Device Configurator Tool Guide</b></a><br>
** <a href="https://cypresssemiconductorco.github.io/bless/ble_api_reference_manual/html/index.html">
*  <b>PSoC 6 BLE Middleware API Reference Guide</b></a><br>
** <a href="http://www.cypress.com/an210781">
*  <b>AN210781 - Getting Started with PSoC 6 MCU with Bluetooth Low, Energy, (BLE) Connectivity</b></a><br>
** <a href="https://github.com/cypresssemiconductorco.github.io/psoc6pdl/pdl_api_reference_manual/html/index.html">
*  <b>PDL API Reference</b></a><br>
** <a href="https://www.cypress.com/documentation/technical-reference-manuals/psoc-6-mcu-psoc-63-ble-architecture-technical-reference">
*   <b>PSoC 6 Technical Reference Manual</b></a><br>
** <a href="http://www.cypress.com/documentation/software-and-drivers/cysmart-bluetooth-le-test-and-debug-tool">
*  <b>CySmart - BLE Test and Debug Tool</b></a><br>
** <a href="http://www.cypress.com/cy8ckit-062-ble">
*  <b>CY8CKIT-062-BLE PSoC 6 BLE Pioneer Kit</b></a><br>
** <a href="http://www.cypress.com"><b>Cypress Semiconductor</b></a><br>
*
* \note
* The links to another software component's documentation (middleware and PDL) 
* point to GitHub to the latest available version of the software. 
* To get documentation of the specified version, download from GitHub and unzip 
* the component archive. The documentation is available in the <i>docs</i> folder.
*
***********************************************************************************************************************
**\page page_ble_section_indastry_standards Industry Standards 
** 
**\section section_ble_Qualify Bluetooth Qualification
*
* BLE solutions provided by Cypress are listed on the Bluetooth SIG website as certified solutions. The qualification 
* is modular, allowing greater flexibility to customers. The following is the list of Qualified Design IDs (QD ID) 
* and Declaration IDs.
* \htmlonly
* <table class="doxtable">
*     <tr>
*       <th>QD ID(s)</th>
*       <th>Declaration ID#</th>
*       <th>Description</th>
*     </tr>
*     <tr>
*       <td>99158</td>
*       <td><a href="https://launchstudio.bluetooth.com/ListingDetails/2312">D037716</a></td>
*       <td>PSoC 6 MCU with Bluetooth Low Energy (BLE) Connectivity delivers ultra-low-power, best-in-class flexible and 
*             built-in security for the Internet of Things (IoT). It's built on an ultra-low-power 40-nm process technology
*             and uses a dual-core Arm&reg; Cortex&reg;-M architecture, which includes an Cortex-M4 and Cortex-M0+. The
*             BLE radio is built using an ultra-low-power 55-nm process. PSoC 6 MCU with BLE Connectivity combines a BLE
*             subsystem with software-defined analog and digital peripherals, CapSense&reg;, programmable interconnect,
*             a high-performance dual-core architecture, and critical security features in a single chip.</td>
*     </tr>
* </table>
* \endhtmlonly    
* 
**
**\section section_ble_MISRA MISRA-C Compliance
**
* This section describes the MISRA-C:2004 compliance and deviations for the PSoC 6 BLE Middleware. 
* 
* <table class="doxtable">
*     <tr>
*       <th>MISRA rule</th>
*       <th>Rule Class (Required/ Advisory)</th>
*       <th>Rule Description</th>
*       <th>Description of Deviation(s)</th>
*     </tr>
*     <tr>
*       <td>1.2</td>
*       <td>R</td>
*       <td>No reliance shall be placed on undefined or unspecified behaviour.</td>
*       <td>This specific behavior is explicitly covered in rule 5.1.</td>
*     </tr>
*     <tr>
*       <td>5.1</td>
*       <td>R</td>
*       <td>Identifiers (internal and external) shall not rely on the significance of more than 31 characters.</td>
*       <td>This rule applies to ISO:C90 standard. PDL conforms to ISO:C99 that does not require this limitation.</td>
*     </tr>
*     <tr>
*       <td>5.6</td>
*       <td>A</td>
*       <td>No identifier in one name space should have the same spelling as an identifier in another name space, 
*           with the exception of structure member and union member names.</td>
*       <td>The internal structure has field with the same name as function of standard library. 
*       This is safe as any assignment to the function from standard library will be discovered at design time.
*       </td>
*     </tr>
*     <tr>
*       <td>9.3</td>
*       <td>R</td>
*       <td>In an enumerator list, the '=' construct shall not be used to explicitly initialise members other than the
*           first, unless all items are explicitly initialised.</td>
*       <td>There are enumerator lists which depend on configurations (e.g. cy_en_ble_srvi_t) and require to 
*           use the '=' construct for calculating the instance count for the multi-instances services, 
*           such as HIDS, BAS or CUSTOM</td>
*     </tr>
*     <tr>
*       <td>10.1</td>
*       <td>R</td>
*       <td>The value of an expression of integer type shall not be implicitly converted to a different underlying type
*           under some circumstances.</td>
*       <td>An operand of essentially enum type is being converted to unsigned type as a result of an arithmetic or
*           conditional operation. The conversion does not have any unintended effect.</td>
*     </tr>
*     <tr>
*       <td>10.3</td>
*       <td>R</td>
*       <td>A composite integer expression is being cast to a wider type.</td>
*       <td>Use of a Cypress defined macro to access memory-mapped objects. Calculating the clock parameters. </td>
*     </tr>
*     <tr>
*       <td>11.4</td>
*       <td>A</td>
*       <td>A cast should not be performed between a pointer to object type and a different pointer to object type.</td>
*       <td>A cast involving pointers is conducted with caution that the pointers are correctly aligned for the type of
*           object being pointed to.</td>
*     </tr>
*     <tr>
*       <td>11.5</td>
*       <td>A</td>
*       <td>A cast shall not be performed that removes any const or volatile qualification from the type addressed by a
*           pointer.</td>
*       <td>The const or volatile qualification is lost during pointer cast before passing to the stack functions.</td>
*     </tr>
*     <tr>
*       <td>13.7</td>
*       <td>R</td>
*       <td>Boolean operations whose results are invariant shall not be permitted.</td>
*       <td>A Boolean operator can yield a result that can be proven to be always "true" or always "false" in some specific
*           configurations because of generalized implementation approach.</td>
*     </tr>
*     <tr>
*       <td>16.7</td>
*       <td>A</td>
*       <td>The object addressed by the pointer parameter is not modified and so the pointer could be of type 'pointer to
*             const'.</td>
*       <td>The 'base' and 'content' parameters in Cy_BLE_DeepSleepCallback function are not used by BLE but callback type 
*             is universal for all drivers.</td>
*     </tr>
*     <tr>
*       <td>18.4</td>
*       <td>R</td>
*       <td>Unions shall not be used.</td>
*       <td>The use of deviations is acceptable for packing and unpacking of data, for example when sending and 
*           receiving messages, and implementing variant records provided that the variants are differentiated 
*           by a common field. </td>
*     </tr>
*     <tr>
*       <td>19.16</td>
*       <td>R</td>
*       <td>Preprocessing directives shall be syntactically meaningful even when excluded by the preprocessor.</td>
*       <td>The reason for this deviation is that not standard directive "warning" is used. This directive is works 
*           on all compilers which PDL supports (e.g. GCC, IAR, MDK). </td>
*     </tr>
* </table>
* 
* 
***********************************************************************************************************************
**\page page_group_ble_changelog Changelog
**
* @section section_group_ble_changelog_BLE_Middleware_Changes PSoC 6 BLE Middleware Changes
* This section lists changes made to the PSoC 6 BLE Middleware.
* 
* <table class="doxtable">
*     <tr><th><b>Version</b></th><th><b>Changes</b></th><th><b>Reason for Change</b></th></tr> 
*
*     <tr>
*         <td rowspan="7">3.50</td>
*         <td>Updated PILO part of BLESS MW HAL to be compatible with psoc6pdl 1.6.1.<br><br>
*             Refer to section \ref group_ble_lfclk_Pilo_conf for details about the requirements for using the PILO as the LF clock source.</td>
*         <td>To support PILO on the PSoC 64 Secure device.</td>
*     </tr>
*     <tr>
*         <td>Added BLE Stack controller libraries with IPC communication (BLESS_CONTROLLER_IPC) for CM0+.</td>
*         <td>Enable users to build their own CM0+ image with a BLE controller to operate in BLE Dual CPU mode.</td>
*     </tr>
*     <tr>
*         <td>Updated the procedure of processing the CY_BLE_EVT_GATTS_WRITE_REQ event: an error response 
*             (with reason INVALID_HANDLE) is sent, if the handle was processed neither by registered services nor an application BLE callback.</td>
*         <td>Improved the handling of the CY_BLE_EVT_GATTS_WRITE_REQ event to allow operation with a custom GATT database.</td>
*     </tr>   
*     <tr>
*         <td>Updated the CY_BLE_SFLASH_DIE_xxx macros (in cy_ble_gap.h) according to the PSoC 6 BLE production configuration.</td>
*         <td>The silicon-generated "Company assigned" part of the device address has a high repetition rate of the generated MAC address.
*             \note The silicon-generated "Company assigned" option does not guarantee a unique device address. For mass production, 
*                   Cypress strongly suggests that the device address be programmed into the user area (Row 0) of the Sflash location via the SWD interface. 
*                   Refer to section "Public device address" of [ModusToolbox BT Configurator Guide] (https://www.cypress.com/ModusToolboxBLEConfig) for details</td>
*     </tr>
*     <tr>
*         <td>Added preprocessor expression to verify count of GATT database entries.</td>
*         <td>BLE Stack supports maximum 512 entries of GATT database.</td>
*     </tr>
*     <tr>
*         <td>Documentation updates.</td>
*         <td>Changed the code snippet for initial PILO trimming and updating the trim step-size in section \ref group_ble_lfclk_Pilo_conf.</tr>
*     <tr>
*         <td>Updated the BLE Stack to version 5.0.8.220.</td>
*         <td>Refer to \ref section_group_ble_changelog_BLE_Stack_Changes.</td>
*     </tr>
*
*     <tr>
*         <td rowspan="3">3.40</td>
*         <td>Added BLE Stack libraries for CM0+ core. </td>
*         <td>Support BLE CM0+ Single CPU mode.</td>
*     </tr>
*     <tr>
*       <td>Updated the procedure of processing events to wait 10 ms on CY_BLE_EVT_SOFT_RESET_COMPLETE events.</td>
*       <td>Ensure that the controller is completely reset before generating the event to the application.
*           Refer to CY_BLE_EVT_SOFT_RESET_COMPLETE event documentation.</td>
*     </tr>
*     <tr>
*         <td>Updated the BLE Stack to version 5.0.7.196</td>
*         <td>Refer to \ref section_group_ble_changelog_BLE_Stack_Changes.</td>
*     </tr>
*
*     <tr>
*         <td rowspan="4">3.30</td>
*         <td>Updated the procedure of processing events to clear the cy_ble_pair_Status flags on
*            CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE and CY_BLE_EVT_SOFT_RESET_COMPLETE events.</td>
*         <td>Cy_BLE_IsDevicePaired() falsely returned True prior to authentication after a radio reset.</td>
*     </tr>
*     <tr>
*         <td>Fixed the setting of a device address in handling the CY_BLE_EVT_STACK_ON event,
*            if the BLE middleware is configured to operate only in the Broadcaster GAP role.</td>
*         <td>With the BLE configured only as a broadcaster, the configuration for a BD address 
*            in the BLE customizer was not effective.</td>
*     </tr>
*     <tr>
*         <td>Documentation updates.</td>
*         <td>Documentation update and clarification.</td>
*     </tr>   
*     <tr>
*         <td>Updated the BLE Stack to version 5.0.6.161.</td>
*         <td>Refer to \ref section_group_ble_changelog_BLE_Stack_Changes.</td>
*     </tr>
*
*     <tr>
*         <td rowspan="6">3.20</td>
*         <td>Added support ARM Compiler 6.<br>
*             The BLE libraries are built with the following options to be compatible with ModusToolbox:<br>
*             -fshort-enums - set the size of an enumeration type to the smallest data type that can hold all enumerator values<br>
*             -fshort-wchar - Set the size of wchar_t to 2 bytes<br>
*             o operate in custom environments with ARM Compiler 6, apply the above mentioned build options.</td>
*         <td>Support Arm Compiler 6.</td>
*     </tr>
*     <tr>
*         <td>Added support for BLE dual CPU mode with pre-compiled CM0 image.</td>
*         <td>Pre-compiled application images executed on Cortex M0+ core of the PSoC 6 dual-core MCU.
*              The images are provided as C arrays ready to be compiled as part of the Cortex M4 application. The Cortex M0+ application
*              code is placed to internal flash by the Cortex M4 linker script. CM0P_BLESS image is used for BLE dual CPU mode.</td>
*     </tr>
*     <tr>
*         <td>Fixed CY_BLE_ENABLE_PHY_UPDATE macros  for enable "LE 2 Mbps" feature.</td>
*         <td>"Enable LE 2 Mbps" feature does not work if it selects in BT Configurator.</td>
*     </tr>   
*     <tr>
*         <td>The PSoC 6 BLE Middleware is re-organized for supporting MT2.0 make flow.</td>
*         <td>Support new make flow.  Refer to section \ref page_ble_section_configuration_considerations for details. </td>
*     </tr>
*     <tr>
*         <td>Disable support HCI over UART in PSoC 6 BLE Middleware side.</td>
*         <td>The SoC mode library was split into three libraries: host, controller and stack manager. The new organization structure allows
*             usage of the same controller library with BLESS host, a UART application or any third party host, over a software HCI interface.</td>
*     </tr>
*     <tr>
*         <td>Updated the BLE Stack to version 5.0.5.110.</td>
*         <td>Refer to \ref section_group_ble_changelog_BLE_Stack_Changes.</td>
*     </tr>
*     
*     <tr>
*         <td rowspan="4">3.10</td>
*         <td>Disabled support BLE dual CPU mode. The following Software components are no longer supported:
*         - BLE dual CPU mode, controller on CM0+ only, Soft FP prebuilt library,<br>
*         - BLE dual CPU mode, host on CM4 only, Soft FP prebuilt library,<br>
*         - BLE dual CPU mode, host on CM4 only, Hard FP prebuilt library.<br>
*         </td>
*         <td>Refer to ModusToolbox 1.1 Release Notes.</td>
*     </tr>
*     <tr>
*         <td>Added the new SW component "BLE OTA with code sharing, Profile only". Updated documentation.</td>
*         <td>Enable the PSoC 6 BLE Middleware to support OTA with code sharing.</td>
*     </tr>
*     <tr>
*         <td>Fixed the Cy_BLE_HAL_BlessStart() function of the PSoC 6 BLE Middleware: removed the BLE ECO reset before BLESS initialization.</td>
*         <td>Fix ModusToolbox 1.0 errata.</td>
*     </tr>   
*     <tr>
*         <td>Updated the BLE Stack to version 5.0.4.946.</td>
*         <td>Refer to \ref section_group_ble_changelog_BLE_Stack_Changes.</td>
*     </tr>
*     
*     <tr>
*         <td rowspan="6">3.0</td>
*         <td>The PSoC 6 BLE Middleware is re-organized, so that it can be built as a device agnostic immutable library. <br>
*             - The BLE configure structure and BLE service configure structures were updated.<br>
*             - Changed the configuration flow. Refer to section \ref page_ble_section_configuration_considerations to configuration details.<br>
*             - Added the following functions:<br>
*             \ref Cy_BLE_RegisterEventCallback(),<br>
*             \ref Cy_BLE_EnableLowPowerMode()<br>
*             - Changed BLE Interrupt Notification Callback feature interface:<br>
*             added Cy_BLE_ConfigureIpcForInterruptCallback() <br>
*             added \ref Cy_BLE_IntrNotifyIsrHandler()</td>
*         <td>Enable the PSoC 6 BLE Middleware to be built as a device agnostic immutable library.</td>
*     </tr>
*     <tr>
*         <td>Updated the RAM memory calculation (macros CY_BLE_GATT_MTU_BUFF_COUNT)</td>
*         <td>Decreased the RAM usage by PSoC 6 BLE Middleware  </td>
*     </tr>
*     <tr>
*         <td>Decreased the ACT_LDO startup time from 4 to 10.</td>
*         <td>Support for the 10uF capacitor on VDCDC. </td>
*     </tr>
*     <tr>
*         <td>Updated the config structures cy_stc_ble_config_t. Added the pointer to the configuration parameter structure
*             for the Radio PA calibration (field paCalConfig).</td>
*         <td>Enabling the user to override the BLE radio's default TX power configurations.</td>
*     </tr>
*     <tr>
*         <td>Documentation updates. </td>
*         <td>Documentation update and clarification.</td>
*     </tr>
*     <tr>
*         <td>Updated the BLE Stack to version 5.0.3.935.</td>
*         <td>Refer to \ref section_group_ble_changelog_BLE_Stack_Changes.</td>
*     </tr>
* 
*     <tr>
*         <td rowspan="6">2.20</td>
*         <td>Added support for PILO. </td>
*         <td>PSoC 6 BLE supports Precision ILO (PILO) which can be used as an LF clock source. 
*             Refer to \ref group_ble_lfclk_conf for more details. </td>
*     </tr>
*     <tr>
*         <td>Updated the Cy_BLE_RegisterAppHostCallback() functionality.</td>
*         <td>Added support for BLE single CPU mode. </td>
*     </tr>
*     <tr>
*         <td>Updated Write Request events handler in PSoC 6 BLE Middleware. Return CY_BLE_GATT_ERR_INVALID_HANDLE if 
*             the attribute handle is out of database range. </td>
*         <td>Handling invalid range of attribute handle during Write Request event. </td>
*     </tr>
*     <tr>
*         <td>Updated flash memory calculation for Bonded device list </td>
*         <td>The function Cy_BLE_StackInit() returned CY_BLE_ERROR_INVALID_PARAMETER when the Resolving list 
*             parameter is set to 1. </td>
*     </tr>
*     <tr>
*         <td>Documentation updates. </td>
*         <td>Documentation update and clarification. Removed the Errata section. </td>
*     </tr>
*     <tr>
*         <td>Updated the BLE Stack to version 5.0.2.917.</td>
*         <td>Refer to \ref section_group_ble_changelog_BLE_Stack_Changes.</td>
*     </tr>
* 
*     <tr>
*         <td rowspan="2">2.10</td>
*         <td>Added return value CY_BLE_ERROR_FLASH_WRITE_NOT_PERMITED for following functions:
*             Cy_BLE_StoreAppData()<br>Cy_BLE_StoreBondingData()<br>
*             Added assertion in Cy_BLE_HAL_NvramWrite() to catch failure (CY_SYSPM_INVALID_STATE) during increasing 
*             core voltage from 0.9V to 1.1V for flash operation.
*         </td>
*         <td>Flash operation is not permitted with protection context (PC) value > 0 and core voltage 0.9V,
*             because of a preproduction hardware limitation.
*         </td>
*     </tr>
*     <tr>
*         <td>Documentation updates. </td>
*         <td>Documentation update and clarification. </td>
*     </tr>
*     
*     <tr>
*         <td rowspan="5">2.0<br> Production</td>
*         <td>Added the BLE user configuration file cy_ble_config.h. </td>
*         <td>To make the BLE configuration more flexible.
*             Allows redefining the configuration define(s) generated by the BLE customizer 
*             and default BLE clock defines (from cy_ble_clk.h).
*         </td>
*     </tr>
*     <tr>
*         <td>Updated the BLE Interrupt Notification Feature. </td>
*         <td>Added support for BLE dual CPU mode.</td>
*     </tr>
*     <tr>
*         <td>Updated the Cy_BLE_GATTS_ReadAttributeValueLocal() and Cy_BLE_GATTS_ReadAttributeValuePeer() functions. </td>
*         <td>Those functions have the wrong Locally/Peer initiate operation flag.</td>
*     </tr>
*     <tr>
*         <td>The following defined values for the CY_BLE_STACK_MODE 
*                 configuration parameter were deprecated:<br>
*                 CY_BLE_STACK_HOST_ONLY<br>
*                 CY_BLE_STACK_HOST_CONTR_UART<br>
*                 CY_BLE_STACK_HOST_IPC.<br>
*             The following defined values were renamed:<br>
*                 CY_BLE_STACK_DEBUG to CY_BLE_STACK_MODE_SINGLE_SOC <br>
*                 CY_BLE_STACK_RELEASE to CY_BLE_STACK_MODE_DUAL_IPC.<br>
*         </td>
*         <td>Deprecated not supported (debug) modes.<br>
*             The defines CY_BLE_CONFIG_STACK_DEBUG and CY_BLE_CONFIG_STACK_RELEASE were renamed to have more meaningful name.
*         </td>
*     </tr>
*     <tr>
*         <td>The BLE Stack was updated to version 5.0.0.898. </td>
*         <td>Refer to \ref section_group_ble_changelog_BLE_Stack_Changes.</td>
*     </tr>
*     
*     <tr>
*         <td rowspan="2" >2.0<br> BETA#2</td>
*         <td>Added new API: <br> Cy_BLE_IsDevicePaired(),<br> Cy_BLE_GetDeviceRole(),<br> Cy_BLE_IsPeerConnected(),<br>
*             Cy_BLE_GATTS_WriteAttributeValuePeer(),<br> Cy_BLE_GATTS_WriteAttributeValueLocal(),<br> 
*             Cy_BLE_GATTS_ReadAttributeValuePeer(),<br> Cy_BLE_GATTS_ReadAttributeValueLocal(),<br> Cy_BLE_GATTS_IsIndicationEnabled(),<br>
*             Cy_BLE_GATTS_SendIndication(),<br> Cy_BLE_GATTS_IsNotificationEnabled(),<br> Cy_BLE_GATTS_SendNotification(),<br>
*             Cy_BLE_GATTS_SendErrorRsp(),<br> Cy_BLE_GATTC_SendConfirmation().
*         </td>
*         <td>Improve usability</td>
*     </tr>
*     <tr>
*         <td>The BLE Stack was updated to version 5.0.0.855.</td>
*         <td>Refer to \ref section_group_ble_changelog_BLE_Stack_Changes.</td>
*     </tr>
*     
*     <tr>
*         <td rowspan="4" >2.0 <br> BETA#1</td>
*         <td>Renamed API: <br>
*             Cy_BLE_GAP_UpdateAdvScanData() to Cy_BLE_GAPP_UpdateAdvScanData(); <br>
*             Cy_BLE_SetConnectionPriority() to Cy_BLE_GAP_SetConnectionPriority(). <br>
*         </td>
*         <td>Consistent API naming scheme</td>
*     </tr>
*     <tr>
*         <td>Made the maximum number of connections configurable.</td>
*         <td>SRAM consumption reducing when extra connections are not used.</td>
*     </tr>
*     <tr>
*         <td>Added new options for the CPU Core parameter.</td>
*         <td>Support of single-core devices.
*     </tr>
*     <tr>
*         <td>The BLE Stack was updated to version 5.0.0.785.</td>
*         <td>Refer to \ref section_group_ble_changelog_BLE_Stack_Changes.</td>
*     </tr>
*     
*     <tr>
*         <td>1.0</td>
*         <td>Initial version.</td>
*         <td></td>
*     </tr>
* </table>
*     
**\section section_group_ble_changelog_BLE_Stack_Changes BLE Stack Changes
*     This section lists changes made to the BLE Stack.
* 
* <table class="doxtable">
*     <tr><th><b>Version</b></th><th><b>Changes</b></th><th><b>Reason for Change</b></th></tr>
*
*     <tr>
*         <td rowspan="3">5.0.8.220</td>
*         <td>Updated BLE ISR for SoC mode.</td>
*         <td>Enhanced the BLE ISR to handle LL Channel Map in interrupt context for SoC mode.</td>
*     </tr>
*     <tr>
*         <td>Updated low-power mode API.</td>
*         <td>Updated Cy_BLE_ControllerEnterLPM API for handling a scenario where wake-up is initiated by the hardware.</td>
*     </tr>
*     <tr>
*        <td>Re-factored Shutdown code for Dual CPU mode.</td>
*        <td>Improved the handling of Cy_BLE_StackShutdown API to avoid timing-sensitive bugs.</td>
*     </tr>
*
*     <tr>
*         <td rowspan="3">5.0.7.196</td>
*         <td>Updated the APIs to handle the interrupt locking.</td>
*         <td>A better locking mechanism to avoid potential timing-sensitive bugs.</td>
*     </tr>
*     <tr>
*         <td>Re-factored the radio initialization sequence.</td>
*         <td>It was observed that on customer boards, re-factoring the radio initialization sequence provided
*             an improved stability to the reset sequence.</td>
*     </tr>
*     <tr>
*        <td>Fix for CVE-2019-16336.</td>
*        <td>Enforced the length check on incoming LL PDUs to protect against buffer overflows caused by
*            malicious packets. Malformed Data packets will be dropped in the LL.
*            Malformed Control packets will be replied with LL_UNKNOWN_RSP. 
*            For detail, refer to the [CVE-2019-16336](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2019-16336) vulnerability page.
*     </td>
*     </tr>
*
*     <tr>
*         <td rowspan="3">5.0.6.161</td>
*         <td>Updated the radio configuration.</td>
*         <td>To support the new packages: QFN68 and BGA124.</td>
*     </tr>
*     <tr>
*         <td>Added checks for Malformed LL PDUs.</td>
*         <td>LL PDUs with the incorrect length must be replied with LL_UNKNOWN_RSP per latest BLE spec. 
*            Also, this prevents against any unknown behavior triggered due to malicious LL PDUs.</td>
*     </tr>
*     <tr>
*         <td>Documentation update.</td>
*         <td>Updated the documentation for the CY_BLE_EVT_RADIO_TEMPERATURE and CY_BLE_EVT_RADIO_VOLTAGE_LEVEL events.</td>
*     </tr>
* 
*     <tr>
*         <td rowspan="7">5.0.5.110</td>
*         <td>Optimized the ACL TX path for a better throughput.</td>
*         <td>The throughput was observed to be low when the stack libraries were linked with MDK libraries.</td>
*     </tr>
*     <tr>
*         <td>Added support for autonomous feature exchange on connection complete.</td>
*         <td>Few mobile phone (eg. Droid Turbo) negotiate DLE with TX octets > 27 bytes but do not actually support the 
*             DLE feature. This leads to interoperability issues with such phones. An autonomous feature exchange will be
*             triggered when the PSoC 6 device is in master role, and the DLE and PHY update negotiations will be triggered
*             only if the peer supports DLE/PHY update feature. In case PSoC 6 is in a slave role,
*             DLE/PHY update negotiations will not be triggered till the peer initiates a feature exchange procedure and hence
*             exchanges it's feature set.</td>
*     </tr>
*     <tr>
*         <td>Added support for configurable PA LDO settings.</td>
*         <td>It was observed that on customer boards, the output RF TX power was sub-optimal, when the factory settings for
*             PA LDO were used. To fine tune the PA LDO settings at the customer end, the same were now exposed via the 
*             paCalConfig member of stackConfig input structure to the Cy_BLE_StackInit API.</td>
*     </tr>
*     <tr>
*         <td>Updated RSSI formula.</td>
*         <td>With the previous formula, there existed a non-linearity in the measured value of RSSI at ~ -50dBm. </td>
*     </tr>
*     <tr>
*         <td>The SoC mode library was split into three libraries: host, controller and stack manager.</td>
*         <td>The new organization structure allows usage of the same controller library with BLESS host, 
*             a UART application or any third party host, over a software HCI interface. </td>
*     </tr>
*     <tr>
*         <td>Fixed issue with disconnection.</td>
*         <td>If an LL procedure is on-going, and a disconnect command is issued, the Terminate PDU is not transmitted over 
*             the air even after the procedure completes. </td>
*     </tr>
*     <tr>
*         <td>Updated Cy_BLE_StackInit API to check that if DLE is disabled, max TX and RX octets must be set to 27 Bytes.</td>
*         <td>The check for the DLE parameters was not correct for the case when DLE is disabled.</td>
*     </tr>
*     
*     <tr>
*         <td rowspan="4">5.0.4.946</td>
*         <td>Fix for CVE-2019-17061.</td>
*         <td>Updated the handling of LL PDU with LLID 0 to prevent corruption in subsequently received LL PDUs. Such packets will be dropped in the LL, but the peer will be acked for the packet. For detail, refer to the [CVE-2019-17061](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2019-17061) vulnerability page.</td>
*     </tr>
*     <tr>
*         <td>Enabled the RSSI reading after the CRC has been received.</td>
*         <td>The RSSI value was not being updated if deep sleep was enabled in the controller.</td>
*     </tr>
*     <tr>
*         <td>Optimized the ACL TX path for a better throughput.</td>
*         <td>In BLE dual CPU mode, with CM0p running at 25 MHz, the BLE throughput was observed to be sub-optimal.</td>
*     </tr>
*     <tr>
*         <td>Added support for the PDL APIs to calculate temperature and Battery monitor output.</td>
*         <td>New APIs were added to the PDL to calculate the absolute values of the die temperature and radio voltage, 
*             using the raw values from the radio.</td>
*     </tr>
*     
*     <tr>
*         <td rowspan="4">5.0.3.935</td>
*         <td>1.Added the option to override the paCal table and LoBuff table from the application.<br>
*             2.Added the option to set lobuflimit and calvallimit from the application.<br>
*             3.Updated the PA LDO settings to use the settings recommended by the designers.
*         </td>
*         <td>Additional flexibility is provided to obtain an optimal output power at the user's board.<br>
*             The new PA LDO setting recommended by the designers results in the ~ 1dBm higher output power
*             (measured using Cypress 001 kits) at the 4dBm setting.
*         </td>
*     </tr>
*     <tr>
*         <td>The local SCA with PILO is enabled to be set to 500 ppm.</td>
*         <td>The PILO frequency accuracy is of the order of 500 ppm.</td>
*     </tr>
*     <tr>
*         <td>The BLE SMP pairing vulnerability fix:<br>
*             1.Added the public key validation check for the remote key.<br>
*             2.Updated the documentation for the Cy_BLE_GAP_GenerateSetLocalP256Keys() API, 
*                to recommend the application to update the private key as mentioned in the BLE Core spec 5.0  Vol. 3 Part H, Section 2.3.6.<br>
*             3.Added the feature bit mask for the remote public key validation.
*         </td>
*         <td>The update to comply with BLE Core Specification Errata 10734.</td>
*     </tr>
*     <tr>
*         <td>1.When none of the BLE hardware engines are active, disable the hardware auto wakeup feature before entering deep sleep mode.<br>
*             2.Re-enable the hardware auto wakeup feature before entering deep sleep if any of the BLE hardware blocks are active.<br>
* 
*         </td>
*         <td>The fix for a spurious interrupt triggered by the link layer hardware when the firmware continuously enters/exits deep sleep mode.</td>
*     </tr>
*     
*     <tr>
*         <td rowspan="3">5.0.2.917</td>
*         <td>Added a new BLE event - CY_BLE_EVT_GAP_ADV_TX, which is triggered on transmission of an ADV packet (if this 
*             feature is enabled). </td>
*         <td>Support for BLE mesh.</td>
*     </tr>
*     <tr>
*         <td>Dual CPU mode: Stack Shutdown sequence modified to shut down controller before 
*             sending CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE event. </td>
*         <td>The controller was shut-down after informing the application that shutdown had been completed. </td>
*     </tr>
*     <tr>
*         <td>Updated the Cy_BLE_HAL_BlessInterruptHandler API to return status. </td>
*         <td>This return parameter indicates that user should call Cy_BLE_ProcessEvents() to process pending BLE Stack 
*             events.</td>
*     </tr>
*     
*     <tr>
*         <td rowspan="4">5.0.1.906</td>
*         <td>Added check for any pending real time procedures before allowing autonomous PHY update request. </td>
*         <td>Autonomous PHY update request was being initiated without a check for "LL control procedure collision". </td>
*     </tr>
*     <tr>
*         <td>Fixed memory allocation failure issue observed when initializing buffer pool for ACL Rx Packet. </td>
*         <td>Memory allocation failure was observed when DLE Rx Octet < TxOctet. </td>
*     </tr>
*     <tr>
*         <td>Added support to disconnect LE link whenever there is real-time LL procedure collision. </td>
*         <td>Consistent with BLE Spec 5.0 erratum 7106</td>
*     </tr>
*     <tr>
*         <td>Added support for PILO. </td>
*         <td>PSoC 6 BLE supports a Precision ILO which can be used as an LF clock source instead of the WCO. </td>
*     </tr>
*     
*     <tr>
*         <td rowspan="2">5.0.0.898</td>
*         <td>Enhancement is done to rerun the scheduler on close CE, if the event about to be scheduled is in past. </td>
*         <td>Blocking flash write operation blocks the CPU from executing any interrupts (including BLESS isr) for > 17ms,
*             Due to this, if the CI for connection is smaller than 17ms, the BLE link will disconnect.
*         </td>
*     </tr>
*     <tr>
*         <td>The calibration target value for the Max output power level is modified under the user-defined condition.</td>
*         <td>The enhancement to support the 5dBm Tx power in the BLE stack.</td>
*     </tr>
*     
*     <tr>
*         <td rowspan="3">5.0.0.855</td>
*         <td>Changed the input parameter for the Cy_BLE_SetCustomEventMask() API from (uint8* mask) to (uint32 mask).</td>
*         <td>Consistent with the bit mask scheme. </td>
*     </tr>
*     <tr>
*         <td>1.The timer context was never reset, it is made to reset during the Stack shutdown. <br>
*             2.The timer context was not freed even when the timer start failed. It is made free when the timer start 
*             fails due to any reason.
*             </td>
*         <td>The Cy_BLE_GAPC_StopScan() function returns CY_BLE_ERROR_MAX when the Cy_BLE_GAPC_StopScan() function is 
*             called to stop a previously-initiated scanning procedure.
*             This issue occurs only when the authentication procedure is occurring and the application tries to stop 
*             the scan </td>
*     </tr>
*     <tr>
*         <td>1.The DLE control procedure timer is started only if the queuing Data Length Request in the Hardware FIFO 
*             is successful.<BR>
*             2.Added checks to handle the race condition between the ACK processing and Handling data PDU's from the Host.
*         </td>
*         <td>Unexpected disconnection due to an LMP Response timeout in the embedded stress application. This issue is 
*             due to:<BR>
*             1.The DLE control procedure timer was started even if the sending Data Length Request PDU failed when queuing 
*             it in the Hardware FIFO.<BR>
*             2.Due to the race condition between the ACK processing and Handling data PDU's from the Host.
*         </td>
*     </tr>
*     
*     <tr>
*         <td>5.0.0.785</td>
*         <td>Initial BLE Stack version. </td>
*         <td> </td>
*     </tr>
* </table>    
* 
* 
************************************************************************************************************************
* \defgroup group_ble_common_api BLE Common API
************************************************************************************************************************
* \ingroup group_ble
*
* \brief
* The common API act as a general interface between the BLE application
* and the BLE Stack module. The application may use these API to control
* the underlying hardware such as radio power, data encryption and device
* bonding via the stack. It may also access the GAP, GATT and L2CAP layers
* of the stack. These are divided into the following categories:
* - \ref group_ble_common_api_functions
* - \ref group_ble_common_api_gap_functions_section
* - \ref group_ble_common_api_gatt_functions_section
* - \ref group_ble_common_api_l2cap_functions
*
* These API also use API specific definitions and data structures. These are classified
* in the following subset:
* - \ref group_ble_common_api_definitions
*
*
* \defgroup group_ble_common_api_functions BLE Common Core Functions
* \ingroup group_ble_common_api
* \brief
* The common core API are used for general BLE configuration.
* These include initialization, power management, and utilities.
*
* \defgroup group_ble_common_Whitelist_api_functions Whitelist API
* \ingroup group_ble_common_api_functions
* \brief
* The API are used for enable user to use Whitelist feature of BLE Stack.
*
* \defgroup group_ble_common_Privacy_api_functions Link Layer Privacy API
* \ingroup group_ble_common_api_functions
* \brief
* The API are used for enable user to use Link Layer Privacy feature of BLE Stack.
*
* \defgroup group_ble_common_Data_length_extension_api_functions Data Length Extension (DLE) API
* \ingroup group_ble_common_api_functions
* \brief
* The API are used for enable user to use Data Length Extension (DLE) feature of BLE Stack.
* 
* \defgroup group_ble_common_2MBPS_api_functions 2Mbps Feature API
* \ingroup group_ble_common_api_functions
* \brief
* The API are used for enable user to use 2Mbps feature of BLE Stack.
*
* \defgroup group_ble_common_LE_ping_api_functions LE Ping API
* \ingroup group_ble_common_api_functions
* \brief
* The API are used for enable user to use LE Ping feature of BLE Stack.
*
* \defgroup group_ble_common_Encryption_api_functions AES Engine API
* \ingroup group_ble_common_api_functions
* \brief
*  BLE sub system AES Engine is exposed through this API.
*
* \defgroup group_ble_common_HCI_api_functions BLE HCI API
* \ingroup group_ble_common_api_functions
* \brief
* API exposes BLE Stack's Host HCI to user, if they want to do DTM testing or use BLE Controller alone.
*
* \defgroup group_ble_common_Intr_feature_api_functions BLE Interrupt Notification Callback
* \ingroup group_ble_common_api_functions
* \brief
* API exposes BLE interrupt notifications to the application which indicates a different link layer and radio state
* transitions to the user from the BLESS interrupt context. The user registers for a particular type of a callback and 
* the PSoC 6 BLE Middleware will call that registered callback basing on the registered mask.
* Refer to section \ref group_ble_interrupt_notify.
*
* \defgroup group_ble_common_api_gap_functions_section GAP Functions
* \ingroup group_ble_common_api
* \brief
* The GAP APIs allow access to the Generic Access Profile (GAP) layer 
* of the BLE Stack. Depending on the chosen GAP role in the Bluetooth Configurator, you may
* use a subset of the supported APIs.
* 
* The GAP API names begin with Cy_BLE_GAP_. In addition to this, the APIs
* also append the GAP role initial letter in the API name.
* 
* \defgroup group_ble_common_api_gap_functions GAP Central and Peripheral Functions
* \ingroup group_ble_common_api_gap_functions_section
* \brief
* These are APIs common to both GAP Central role and GAP Peripheral role. 
* You may use them in either roles.
* 
* No letter is appended to the API name: Cy_BLE_GAP_
* 
* \defgroup group_ble_common_api_gap_central_functions GAP Central Functions
* \ingroup group_ble_common_api_gap_functions_section
* \brief
* APIs unique to designs configured as a GAP Central role. 
* 
* A letter 'C' is appended to the API name: Cy_BLE_GAPC_
* 
* \defgroup group_ble_common_api_gap_peripheral_functions GAP Peripheral Functions
* \ingroup group_ble_common_api_gap_functions_section
* \brief
* APIs unique to designs configured as a GAP Peripheral role. 
* 
* A letter 'P' is appended to the API name: Cy_BLE_GAPP_
* 
* 
* \defgroup group_ble_common_api_gatt_functions_section GATT Functions
* \ingroup group_ble_common_api
* \brief
* The GATT APIs allow access to the Generic Attribute Profile (GATT) layer 
* of the BLE Stack. Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported APIs.
* 
* The GATT API names begin with CyBle_Gatt. In addition to this, the APIs
* also append the GATT role initial letter in the API name.
* 
* \defgroup group_ble_common_api_gatt_functions GATT Client and Server Functions
* \ingroup group_ble_common_api_gatt_functions_section
* \brief
* These are APIs common to both GATT Client role and GATT Server role. 
* You may use them in either roles.
* 
* No letter is appended to the API name: Cy_BLE_GATT_
* 
* \defgroup group_ble_common_api_gatt_client_functions GATT Client Functions
* \ingroup group_ble_common_api_gatt_functions_section
* \brief
* APIs unique to designs configured as a GATT Client role. 
* 
* A letter 'C' is appended to the API name: Cy_BLE_GATTC_
* 
* \defgroup group_ble_common_api_gatt_server_functions GATT Server Functions
* \ingroup group_ble_common_api_gatt_functions_section
* \brief
* APIs unique to designs configured as a GATT Server role. 
* 
* A letter 'S' is appended to the API name: Cy_BLE_GATTS_
* 
* 
* \defgroup group_ble_common_api_l2cap_functions L2CAP Functions
* \ingroup group_ble_common_api
* \brief
* The L2CAP APIs allow access to the Logical link control and adaptation
* protocol (L2CAP) layer of the BLE Stack.
* 
* The L2CAP API names begin with Cy_BLE_L2CAP.
* 
* 
*  
* \defgroup group_ble_common_api_events BLE Common Events
* \ingroup group_ble_common_api
* \brief
* The BLE Stack generates events to notify the application on various status
* alerts concerning the stack. These can be generic stack events or can be 
* specific to GAP, GATT or L2CAP layers. The service specific events are
* handled separately in \ref group_ble_service_api_events.
*  
*
* \defgroup group_ble_common_api_definitions_section BLE Common Definitions and Data Structures
* \ingroup group_ble_common_api
* \brief
* Contains definitions and structures that are common to all BLE common API.
* Note that some of these are also used in Service-specific API.
*
* \defgroup group_ble_common_macros_error BLE Error Code
* \ingroup group_ble_common_api_definitions_section
* \brief
* Contains definitions for all the spec defined error code in Core Spec 5.0, Vol2, Part D
* 
* Related Document:
*  BLE Standard Spec - CoreV5.0
*
* \defgroup group_ble_common_api_macros_section Macros
* \ingroup group_ble_common_api_definitions_section
*
* \defgroup group_ble_common_api_macros Common
* \ingroup group_ble_common_api_macros_section
*  BLE Common macros
*
* \defgroup group_ble_common_api_macros_gatt_db BLE GATT Database
* \ingroup group_ble_common_api_macros_section
* \brief
*  BLE GATT Database macros
*
* \defgroup group_ble_common_api_macros_gatt_uuid_services BLE Services UUID 
* \ingroup group_ble_common_api_macros_section
* \brief
*   BLE Services Universal Unique Identifier (UUID) macros
*
* \defgroup group_ble_common_api_macros_gatt_uuid_char_gatt_type BLE GATT Attribute Types UUID 
* \ingroup group_ble_common_api_macros_section
* \brief
*   BLE GATT Attribute Types defined by GATT Profile UUID macros
*
* \defgroup group_ble_common_api_macros_gatt_uuid_char_desc BLE GATT Characteristic Descriptors UUID 
* \ingroup group_ble_common_api_macros_section
* \brief
*   BLE GATT Attribute Types defined by GATT Profile UUID macros
*
* \defgroup group_ble_common_api_macros_gatt_uuid_char_type BLE GATT Characteristic Types UUID 
* \ingroup group_ble_common_api_macros_section
* \brief
*   BLE GATT Characteristic Types UUID macros
*
* \defgroup group_ble_common_api_macros_appearance_values BLE Appearance values
* \ingroup group_ble_common_api_macros_section
* \brief
*   BLE Appearance values macros
*           
* \defgroup group_ble_common_api_data_struct_section Data Structures
* \ingroup group_ble_common_api_definitions_section
*
* \defgroup group_ble_common_api_definitions Common 
* \ingroup group_ble_common_api_data_struct_section
* \brief
* Contains the common definitions and data structures used in the BLE.
*
* \defgroup group_ble_common_api_gap_definitions GAP 
* \ingroup group_ble_common_api_data_struct_section
* \brief
* Contains the GAP specific definitions and data structures used in the
* GAP APIs.
*
* \defgroup group_ble_common_api_gatt_definitions GATT
* \ingroup group_ble_common_api_data_struct_section
* \brief
* Contains the GATT specific definitions and data structures used in the
* GATT APIs.
*
* \defgroup group_ble_common_api_l2cap_definitions L2CAP 
* \ingroup group_ble_common_api_data_struct_section
* \brief
* Contains the L2CAP specific definitions and data structures used in the
* L2CAP APIs.
*
* \defgroup group_ble_common_api_global_variables Global Variables 
* \ingroup group_ble_common_api
* \defgroup group_ble_service_api BLE Service-Specific API
* \ingroup group_ble
* \brief
* This section describes BLE Service-specific API. The Service
* API are only included in the design if the Service is
* added to the selected Profile in the Bluetooth Configurator.
* These are interfaces for the BLE application to use during BLE
* connectivity. The service-specific API internally use the BLE
* Stack API to achieve the Service use case.
* Refer to the <a href="https://www.bluetooth.org/en-us/specification/adopted-specifications">Special Interest Group
* Web Site</a>
* for links to the latest specifications and other documentation.
*
* Many of the API will generate Service-specific events. The events
* are also used in the Service-specific callback functions. These are
* documented in:
* - \ref group_ble_service_api_events
*
*
* \defgroup group_ble_service_api_events BLE Service-Specific Events
* \ingroup group_ble_service_api
* \brief
* The BLE Stack generates service-specific events to notify the application
* that a service-specific status change needs attention.
*
*
*
* \defgroup group_ble_service_api_ANCS Apple Notification Center Service (ANCS)
* \ingroup group_ble_service_api
* \brief
* The Apple Notification Center Service provides iOS notifications from Apple devices for accessories.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The ANCS API names begin with Cy_BLE_ANCS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_ANCS_server_client ANCS Server and Client Function
* \ingroup group_ble_service_api_ANCS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_ANCS_
*
* \defgroup group_ble_service_api_ANCS_server ANCS Server Functions
* \ingroup group_ble_service_api_ANCS
* \brief
* API unique to ANCS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_ANCSS_
*
* \defgroup group_ble_service_api_ANCS_client ANCS Client Functions
* \ingroup group_ble_service_api_ANCS
* \brief
* API unique to ANCS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_ANCSC_
*
* \defgroup group_ble_service_api_ANCS_definitions ANCS Definitions and Data Structures
* \ingroup group_ble_service_api_ANCS
* \brief
* Contains the ANCS specific definitions and data structures used
* in the ANCS API.
*
*
*
* \defgroup group_ble_service_api_ANS Alert Notification Service (ANS)
* \ingroup group_ble_service_api
* \brief
* The Alert Notification Service exposes alert information in a device.
*
* This information includes:
* - Type of alert occurring in a device
* - Additional text information such as the caller's ID or sender's ID
* - Count of new alerts
* - Count of unread alert items
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The ANS API names begin with Cy_BLE_ANS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_ANS_server_client ANS Server and Client Function
* \ingroup group_ble_service_api_ANS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_ANS_
*
* \defgroup group_ble_service_api_ANS_server ANS Server Functions
* \ingroup group_ble_service_api_ANS
* \brief
* API unique to ANS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_ANSS_
*
* \defgroup group_ble_service_api_ANS_client ANS Client Functions
* \ingroup group_ble_service_api_ANS
* \brief
* API unique to ANS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_ANSC_
*
* \defgroup group_ble_service_api_ANS_definitions ANS Definitions and Data Structures
* \ingroup group_ble_service_api_ANS
* \brief
* Contains the ANS specific definitions and data structures used
* in the ANS API.
*
*
*
* \defgroup group_ble_service_api_AIOS Automation IO Service (AIOS)
* \ingroup group_ble_service_api
* \brief
* The Automation IO Service enables a device to connect and interact with an Automation IO Module (IOM) in order to access digital and analog signals.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The AIOS API names begin with Cy_BLE_AIOS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_AIOS_server_client AIOS Server and Client Function
* \ingroup group_ble_service_api_AIOS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_AIOS_
*
* \defgroup group_ble_service_api_AIOS_server AIOS Server Functions
* \ingroup group_ble_service_api_AIOS
* \brief
* API unique to AIOS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_AIOSS_
*
* \defgroup group_ble_service_api_AIOS_client AIOS Client Functions
* \ingroup group_ble_service_api_AIOS
* \brief
* API unique to AIOS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_AIOSC_
*
* \defgroup group_ble_service_api_AIOS_definitions AIOS Definitions and Data Structures
* \ingroup group_ble_service_api_AIOS
* \brief
* Contains the AIOS specific definitions and data structures used
* in the AIOS API.
*
*
* \defgroup group_ble_service_api_BAS Battery Service (BAS)
* \ingroup group_ble_service_api
* \brief
* The Battery Service exposes the battery level of a single battery
* or set of batteries in a device.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The BAS API names begin with Cy_BLE_BAS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_BAS_server_client BAS Server and Client Function
* \ingroup group_ble_service_api_BAS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_BAS_
*
* \defgroup group_ble_service_api_BAS_server BAS Server Functions
* \ingroup group_ble_service_api_BAS
* \brief
* API unique to BAS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_BASS_
*
* \defgroup group_ble_service_api_BAS_client BAS Client Functions
* \ingroup group_ble_service_api_BAS
* \brief
* API unique to BAS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_BASC_
*
* \defgroup group_ble_service_api_BAS_definitions BAS Definitions and Data Structures
* \ingroup group_ble_service_api_BAS
* \brief
* Contains the BAS specific definitions and data structures used
* in the BAS API.
*
*
*
* \defgroup group_ble_service_api_BCS Body Composition Service (BCS)
* \ingroup group_ble_service_api
* \brief
* The Body Composition Service exposes data related to body composition from a body composition analyzer (Server)
* intended for consumer healthcare as well as sports/fitness applications.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may use a subset of the supported API.
*
* The BCS API names begin with Cy_BLE_BCS_. In addition to this, the API also append the GATT role initial letter in the
* API name.
*
* \defgroup group_ble_service_api_BCS_server_client BCS Server and Client Function
* \ingroup group_ble_service_api_BCS
* \brief
* These are API common to both GATT Client role and GATT Server role. You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_BCS_
*
* \defgroup group_ble_service_api_BCS_server BCS Server Functions
* \ingroup group_ble_service_api_BCS
* \brief
* API unique to BCS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_BCSS_
*
* \defgroup group_ble_service_api_BCS_client BCS Client Functions
* \ingroup group_ble_service_api_BCS
* \brief
* API unique to BCS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_BCSC_
*
* \defgroup group_ble_service_api_BCS_definitions BCS Definitions and Data Structures
* \ingroup group_ble_service_api_BCS
* \brief
* Contains the BCS specific definitions and data structures used in the BCS API.
*
*
*
* \defgroup group_ble_service_api_BLS Blood Pressure Service (BLS)
* \ingroup group_ble_service_api
* \brief
* The Blood Pressure Service exposes blood pressure and other data related
* to a non-invasive blood pressure monitor for consumer and professional
* healthcare applications.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The BLS API names begin with Cy_BLE_BLS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_BLS_server_client BLS Server and Client Function
* \ingroup group_ble_service_api_BLS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_BLS_
*
* \defgroup group_ble_service_api_BLS_server BLS Server Functions
* \ingroup group_ble_service_api_BLS
* \brief
* API unique to BLS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_BLSS_
*
* \defgroup group_ble_service_api_BLS_client BLS Client Functions
* \ingroup group_ble_service_api_BLS
* \brief
* API unique to BLS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_BLSC_
*
* \defgroup group_ble_service_api_BLS_definitions BLS Definitions and Data Structures
* \ingroup group_ble_service_api_BLS
* \brief
* Contains the BLS specific definitions and data structures
* used in the BLS API.
*
*
*
* \defgroup group_ble_service_api_BMS Bond Management Service (BMS)
* \ingroup group_ble_service_api
* \brief
* The Bond Management Service defines how a peer Bluetooth device can manage the storage of bond information, especially
* the deletion of it, on the Bluetooth device supporting this service.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may use a subset of the supported API.
*
* The BMS API names begin with Cy_BLE_BMS_. In addition to this, the API also append the GATT role initial letter in the
* API name.
*
* \defgroup group_ble_service_api_BMS_server_client BMS Server and Client Function
* \ingroup group_ble_service_api_BMS
* \brief
* These are API common to both GATT Client role and GATT Server role. You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_BMS_
*
* \defgroup group_ble_service_api_BMS_server BMS Server Functions
* \ingroup group_ble_service_api_BMS
* \brief
* API unique to BMS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_BMSS_
*
* \defgroup group_ble_service_api_BMS_client BMS Client Functions
* \ingroup group_ble_service_api_BMS
* \brief
* API unique to BMS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_BMSC_
*
* \defgroup group_ble_service_api_BMS_definitions BMS Definitions and Data Structures
* \ingroup group_ble_service_api_BMS
* \brief
* Contains the BMS specific definitions and data structures used in the BMS API.
*
*
*
* \defgroup group_ble_service_api_CGMS Continuous Glucose Monitoring Service (CGMS)
* \ingroup group_ble_service_api
* \brief
* The Continuous Glucose Monitoring Service exposes glucose measurement and other data related to a personal CGM sensor
* for healthcare applications.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may use a subset of the supported API.
*
* The CGMS API names begin with Cy_BLE_CGMS_. In addition to this, the API also append the GATT role initial letter in the
* API name.
*
* \defgroup group_ble_service_api_CGMS_server_client CGMS Server and Client Function
* \ingroup group_ble_service_api_CGMS
* \brief
* These are API common to both GATT Client role and GATT Server role. You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_CGMS_
*
* \defgroup group_ble_service_api_CGMS_server CGMS Server Functions
* \ingroup group_ble_service_api_CGMS
* \brief
* API unique to CGMS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_CGMSS_
*
* \defgroup group_ble_service_api_CGMS_client CGMS Client Functions
* \ingroup group_ble_service_api_CGMS
* \brief
* API unique to CGMS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_CGMSC_
*
* \defgroup group_ble_service_api_CGMS_definitions CGMS Definitions and Data Structures
* \ingroup group_ble_service_api_CGMS
* \brief
* Contains the CGMS specific definitions and data structures used in the CGMS API.
*
*
*
* \defgroup group_ble_service_api_CPS Cycling Power Service (CPS)
* \ingroup group_ble_service_api
* \brief
* The Cycling Power Service (CPS) exposes power- and force-related
* data and optionally speed- and cadence-related data from a Cycling
* Power sensor (GATT Server) intended for sports and fitness applications.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The CPS API names begin with Cy_BLE_CPS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
*
* \defgroup group_ble_service_api_CPS_server_client CPS Server and Client Function
* \ingroup group_ble_service_api_CPS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_CPS_
*
* \defgroup group_ble_service_api_CPS_server CPS Server Functions
* \ingroup group_ble_service_api_CPS
* \brief
* API unique to CPS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_CPSS_
*
* \defgroup group_ble_service_api_CPS_client CPS Client Functions
* \ingroup group_ble_service_api_CPS
* \brief
* API unique to CPS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_CPSC_
*
* \defgroup group_ble_service_api_CPS_definitions CPS Definitions and Data Structures
* \ingroup group_ble_service_api_CPS
* \brief
* Contains the CPS specific definitions and data structures
* used in the CPS API.
*
*
* \defgroup group_ble_service_api_CSCS Cycling Speed and Cadence Service (CSCS)
* \ingroup group_ble_service_api
* \brief
* The Cycling Speed and Cadence (CSC) Service exposes speed-related
* data and/or cadence-related data while using the Cycling Speed
* and Cadence sensor (Server).
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The CSCS API names begin with Cy_BLE_CSCS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
*
* \defgroup group_ble_service_api_CSCS_server_client CSCS Server and Client Function
* \ingroup group_ble_service_api_CSCS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_CSCS_
*
* \defgroup group_ble_service_api_CSCS_server CSCS Server Functions
* \ingroup group_ble_service_api_CSCS
* \brief
* API unique to CSCS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_CSCSS_
*
* \defgroup group_ble_service_api_CSCS_client CSCS Client Functions
* \ingroup group_ble_service_api_CSCS
* \brief
* API unique to CSCS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_CSCSC_
*
* \defgroup group_ble_service_api_CSCS_definitions CSCS Definitions and Data Structures
* \ingroup group_ble_service_api_CSCS
* \brief
* Contains the CSCS specific definitions and data structures
* used in the CSCS API.
*
*
* \defgroup group_ble_service_api_CTS Current Time Service (CTS)
* \ingroup group_ble_service_api
* \brief
* The Current Time Service defines how a Bluetooth device can expose time information
* to other Bluetooth devices.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The CTS API names begin with Cy_BLE_CTS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_CTS_server_client CTS Server and Client Function
* \ingroup group_ble_service_api_CTS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_CTS_
*
* \defgroup group_ble_service_api_CTS_server CTS Server Functions
* \ingroup group_ble_service_api_CTS
* \brief
* API unique to CTS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_CTSS_
*
* \defgroup group_ble_service_api_CTS_client CTS Client Functions
* \ingroup group_ble_service_api_CTS
* \brief
* API unique to CTS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_CTSC_
*
* \defgroup group_ble_service_api_CTS_definitions CTS Definitions and Data Structures
* \ingroup group_ble_service_api_CTS
* \brief
* Contains the CTS specific definitions and data structures
* used in the CTS API.
*
*
* \defgroup group_ble_service_api_DIS Device Information Service (DIS)
* \ingroup group_ble_service_api
* \brief
* The Device Information Service exposes manufacturer and/or
* vendor information about a device.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The DIS API names begin with Cy_BLE_DIS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_DIS_server_client DIS Server and Client Function
* \ingroup group_ble_service_api_DIS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_DIS_
*
* \defgroup group_ble_service_api_DIS_server DIS Server Functions
* \ingroup group_ble_service_api_DIS
* \brief
* API unique to DIS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_DISS_
*
* \defgroup group_ble_service_api_DIS_client DIS Client Functions
* \ingroup group_ble_service_api_DIS
* \brief
* API unique to DIS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_DISC_
*
* \defgroup group_ble_service_api_DIS_definitions DIS Definitions and Data Structures
* \ingroup group_ble_service_api_DIS
* \brief
* Contains the DIS specific definitions and data structures
* used in the DIS API.
*
*
*
* \defgroup group_ble_service_api_ESS Environmental Sensing Service (ESS)
* \ingroup group_ble_service_api
* \brief
* The Environmental Sensing Service exposes measurement data from an environmental sensor intended for sports and fitness
* applications.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may use a subset of the supported API.
*
* The ESS API names begin with Cy_BLE_ESS_. In addition to this, the API also append the GATT role initial letter in the
* API name.
*
* \defgroup group_ble_service_api_ESS_server_client ESS Server and Client Function
* \ingroup group_ble_service_api_ESS
* \brief
* These are API common to both GATT Client role and GATT Server role. You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_ESS_
*
* \defgroup group_ble_service_api_ESS_server ESS Server Functions
* \ingroup group_ble_service_api_ESS
* \brief
* API unique to ESS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_ESSS_
*
* \defgroup group_ble_service_api_ESS_client ESS Client Functions
* \ingroup group_ble_service_api_ESS
* \brief
* API unique to ESS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_ESSC_
*
* \defgroup group_ble_service_api_ESS_definitions ESS Definitions and Data Structures
* \ingroup group_ble_service_api_ESS
* \brief
* Contains the ESS specific definitions and data structures used in the ESS API.
*
*
*
* \defgroup group_ble_service_api_GLS Glucose Service (GLS)
* \ingroup group_ble_service_api
* \brief
* The Glucose Service exposes glucose and other data related to
* a personal glucose sensor for consumer healthcare applications
* and is not designed for clinical use.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The GLS API names begin with Cy_BLE_GLS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_GLS_server_client GLS Server and Client Function
* \ingroup group_ble_service_api_GLS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_GLS_
*
* \defgroup group_ble_service_api_GLS_server GLS Server Functions
* \ingroup group_ble_service_api_GLS
* \brief
* API unique to GLS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_GLSS_
*
* \defgroup group_ble_service_api_GLS_client GLS Client Functions
* \ingroup group_ble_service_api_GLS
* \brief
* API unique to GLS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_GLSC_
*
* \defgroup group_ble_service_api_GLS_definitions GLS Definitions and Data Structures
* \ingroup group_ble_service_api_GLS
* \brief
* Contains the GLS specific definitions and data structures
* used in the GLS API.
*
*
*
* \defgroup group_ble_service_api_HIDS HID Service (HIDS)
* \ingroup group_ble_service_api
* \brief
* The HID Service exposes data and associated formatting for
* HID Devices and HID Hosts.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The HID API names begin with Cy_BLE_HID. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_HIDS_server_client HIDS Server and Client Functions
* \ingroup group_ble_service_api_HIDS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_HIDS_
*
* \defgroup group_ble_service_api_HIDS_server HIDS Server Functions
* \ingroup group_ble_service_api_HIDS
* \brief
* API unique to HID designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_HIDSS_
*
* \defgroup group_ble_service_api_HIDS_client HIDS Client Functions
* \ingroup group_ble_service_api_HIDS
* \brief
* API unique to HID designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_HIDSC_
*
* \defgroup group_ble_service_api_HIDS_definitions HIDS Definitions and Data Structures
* \ingroup group_ble_service_api_HIDS
* \brief
* Contains the HID specific definitions and data structures
* used in the HID API.
*
*
*
* \defgroup group_ble_service_api_HRS Heart Rate Service (HRS)
* \ingroup group_ble_service_api
* \brief
* The Heart Rate Service exposes heart rate and other data
* related to a heart rate sensor intended for fitness applications.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The HRS API names begin with Cy_BLE_HRS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_HRS_server_client HRS Server and Client Function
* \ingroup group_ble_service_api_HRS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_HRS_
*
* \defgroup group_ble_service_api_HRS_server HRS Server Functions
* \ingroup group_ble_service_api_HRS
* \brief
* API unique to HRS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_HRSS_
*
* \defgroup group_ble_service_api_HRS_client HRS Client Functions
* \ingroup group_ble_service_api_HRS
* \brief
* API unique to HRS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_HRSC_
*
* \defgroup group_ble_service_api_HRS_definitions HRS Definitions and Data Structures
* \ingroup group_ble_service_api_HRS
* \brief
* Contains the HRS specific definitions and data structures
* used in the HRS API.
*
*
*
* \defgroup group_ble_service_api_HPS HTTP Proxy Service (HPS)
* \ingroup group_ble_service_api
* \brief
* The HTTP Proxy Service allows a Client device, typically a sensor,
* to communicate with a Web Server through a gateway device.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The HPS API names begin with Cy_BLE_HPS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_HPS_server_client HPS Server and Client Function
* \ingroup group_ble_service_api_HPS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_HPS_
*
* \defgroup group_ble_service_api_HPS_server HPS Server Functions
* \ingroup group_ble_service_api_HPS
* \brief
* API unique to HPS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_HPSS_
*
* \defgroup group_ble_service_api_HPS_client HPS Client Functions
* \ingroup group_ble_service_api_HPS
* \brief
* API unique to HPS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_HPSC_
*
* \defgroup group_ble_service_api_HPS_definitions HPS Definitions and Data Structures
* \ingroup group_ble_service_api_HPS
* \brief
* Contains the HPS specific definitions and data structures
* used in the HPS API.
*
*
*
* \defgroup group_ble_service_api_HTS Health Thermometer Service (HTS)
* \ingroup group_ble_service_api
* \brief
* The Health Thermometer Service exposes temperature and other
* data related to a thermometer used for healthcare applications.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The HTS API names begin with Cy_BLE_HTS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_HTS_server_client HTS Server and Client Function
* \ingroup group_ble_service_api_HTS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_HTS_
*
* \defgroup group_ble_service_api_HTS_server HTS Server Functions
* \ingroup group_ble_service_api_HTS
* \brief
* API unique to HTS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_HTSS_
*
* \defgroup group_ble_service_api_HTS_client HTS Client Functions
* \ingroup group_ble_service_api_HTS
* \brief
* API unique to HTS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_HTSC_
*
* \defgroup group_ble_service_api_HTS_definitions HTS Definitions and Data Structures
* \ingroup group_ble_service_api_HTS
* \brief
* Contains the HTS specific definitions and data structures
* used in the HTS API.
*
*
*
* \defgroup group_ble_service_api_IAS Immediate Alert Service (IAS)
* \ingroup group_ble_service_api
* \brief
* The Immediate Alert Service exposes a control point to allow a peer device to cause the device to immediately alert.
*
* The Immediate Alert Service uses the Alert Level Characteristic
* to cause an alert when it is written with a value other than "No Alert".
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The IAS API names begin with Cy_BLE_IAS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_IAS_server_client IAS Server and Client Function
* \ingroup group_ble_service_api_IAS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_IAS_
*
* \defgroup group_ble_service_api_IAS_server IAS Server Functions
* \ingroup group_ble_service_api_IAS
* \brief
* API unique to IAS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_IASS_
*
* \defgroup group_ble_service_api_IAS_client IAS Client Functions
* \ingroup group_ble_service_api_IAS
* \brief
* API unique to IAS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_IASC_
*
* \defgroup group_ble_service_api_IAS_definitions IAS Definitions and Data Structures
* \ingroup group_ble_service_api_IAS
* \brief
* Contains the IAS specific definitions and data structures
* used in the IAS API.
*
*
*
* \defgroup group_ble_service_api_IPS Indoor Positioning Service (IPS)
* \ingroup group_ble_service_api
* \brief
* The Indoor Positioning exposes coordinates and other location related information via
* an advertisement or indicates that the device address can be used for location look-up,
* enabling mobile devices to find their position.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The IPS API names begin with Cy_BLE_IPS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_IPS_server_client IPS Server and Client Function
* \ingroup group_ble_service_api_IPS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_IPS_
*
* \defgroup group_ble_service_api_IPS_server IPS Server Functions
* \ingroup group_ble_service_api_IPS
* \brief
* API unique to IPS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_IPSS_
*
* \defgroup group_ble_service_api_IPS_client IPS Client Functions
* \ingroup group_ble_service_api_IPS
* \brief
* API unique to IPS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_IPSC_
*
* \defgroup group_ble_service_api_IPS_definitions IPS Definitions and Data Structures
* \ingroup group_ble_service_api_IPS
* \brief
* Contains the IPS specific definitions and data structures
* used in the IPS API.
*
*
*
* \defgroup group_ble_service_api_LLS Link Loss Service (LLS)
* \ingroup group_ble_service_api
* \brief
* The Link Loss Service uses the Alert Level Characteristic to
* cause an alert in the device when the link is lost.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The LLS API names begin with Cy_BLE_LLS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_LLS_server_client LLS Server and Client Function
* \ingroup group_ble_service_api_LLS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_LLS_
*
* \defgroup group_ble_service_api_LLS_server LLS Server Functions
* \ingroup group_ble_service_api_LLS
* \brief
* API unique to LLS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_LLSS_
*
* \defgroup group_ble_service_api_LLS_client LLS Client Functions
* \ingroup group_ble_service_api_LLS
* \brief
* API unique to LLS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_LLSC_
*
* \defgroup group_ble_service_api_LLS_definitions LLS Definitions and Data Structures
* \ingroup group_ble_service_api_LLS
* \brief
* Contains the LLS specific definitions and data structures
* used in the LLS API.
*
*
*
* \defgroup group_ble_service_api_LNS Location and Navigation Service (LNS)
* \ingroup group_ble_service_api
* \brief
* The Location and Navigation Service exposes location and
* navigation-related data from a Location and Navigation sensor
* (Server) intended for outdoor activity applications.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The LNS API names begin with Cy_BLE_LNS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_LNS_server_client LNS Server and Client Function
* \ingroup group_ble_service_api_LNS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_LNS_
*
* \defgroup group_ble_service_api_LNS_server LNS Server Functions
* \ingroup group_ble_service_api_LNS
* \brief
* API unique to LNS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_LNSS_
*
* \defgroup group_ble_service_api_LNS_client LNS Client Functions
* \ingroup group_ble_service_api_LNS
* \brief
* API unique to LNS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_LNSC_
*
* \defgroup group_ble_service_api_LNS_definitions LNS Definitions and Data Structures
* \ingroup group_ble_service_api_LNS
* \brief
* Contains the LNS specific definitions and data structures
* used in the LNS API.
*
*
*
* \defgroup group_ble_service_api_NDCS Next DST Change Service (NDCS)
* \ingroup group_ble_service_api
* \brief
* The Next DST Change Service enables a BLE device that has knowledge about the
* next occurrence of a DST change to expose this information to
* another Bluetooth device. The Service uses the "Time with DST"
* Characteristic and the functions exposed in this Service are
* used to interact with that Characteristic.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The NDCS API names begin with Cy_BLE_NDCS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_NDCS_server_client NDCS Server and Client Functions
* \ingroup group_ble_service_api_NDCS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_NDCS_
*
* \defgroup group_ble_service_api_NDCS_server NDCS Server Functions
* \ingroup group_ble_service_api_NDCS
* \brief
* API unique to NDCS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_NDCSS_
*
* \defgroup group_ble_service_api_NDCS_client NDCS Client Functions
* \ingroup group_ble_service_api_NDCS
* \brief
* API unique to NDCS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_NDCSC_
*
* \defgroup group_ble_service_api_NDCS_definitions NDCS Definitions and Data Structures
* \ingroup group_ble_service_api_NDCS
* \brief
* Contains the NDCS specific definitions and data structures
* used in the NDCS API.
*
*
*
* \defgroup group_ble_service_api_PASS Phone Alert Status Service (PASS)
* \ingroup group_ble_service_api
* \brief
* The Phone Alert Status Service uses the Alert Status Characteristic
* and Ringer Setting Characteristic to expose the phone alert status
* and uses the Ringer Control Point Characteristic to control the
* phone's ringer into mute or enable.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The PASS API names begin with Cy_BLE_PASS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_PASS_server_client PASS Server and Client Function
* \ingroup group_ble_service_api_PASS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_PASS_
*
* \defgroup group_ble_service_api_PASS_server PASS Server Functions
* \ingroup group_ble_service_api_PASS
* \brief
* API unique to PASS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_PASSS_
*
* \defgroup group_ble_service_api_PASS_client PASS Client Functions
* \ingroup group_ble_service_api_PASS
* \brief
* API unique to PASS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_PASSC_
*
* \defgroup group_ble_service_api_PASS_definitions PASS Definitions and Data Structures
* \ingroup group_ble_service_api_PASS
* \brief
* Contains the PASS specific definitions and data structures
* used in the PASS API.
*
*
*
* \defgroup group_ble_service_api_PLXS Pulse Oximeter Service (PLXS)
* \ingroup group_ble_service_api
* \brief
* The Pulse Oximeter Service enables a Collector device to connect and interact with a pulse oximeter intended for healthcare applications.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The PLXS API names begin with Cy_BLE_PLXS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_PLXS_server_client PLXS Server and Client Function
* \ingroup group_ble_service_api_PLXS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_PLXS_
*
* \defgroup group_ble_service_api_PLXS_server PLXS Server Functions
* \ingroup group_ble_service_api_PLXS
* \brief
* API unique to PLXS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_PLXSS_
*
* \defgroup group_ble_service_api_PLXS_client PLXS Client Functions
* \ingroup group_ble_service_api_PLXS
* \brief
* API unique to PLXS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_PLXSC_
*
* \defgroup group_ble_service_api_PLXS_definitions PLXS Definitions and Data Structures
* \ingroup group_ble_service_api_PLXS
* \brief
* Contains the PLXS specific definitions and data structures used
* in the PLXS API.
*
*
*
* \defgroup group_ble_service_api_RSCS Running Speed and Cadence Service (RSCS)
* \ingroup group_ble_service_api
* \brief
* The Running Speed and Cadence (RSC) Service exposes speed,
* cadence and other data related to fitness applications such
* as the stride length and the total distance the user has
* travelled while using the Running Speed and Cadence sensor (Server).
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The RSCS API names begin with Cy_BLE_RSCS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_RSCS_server_client RSCS Server and Client Functions
* \ingroup group_ble_service_api_RSCS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_RSCS_
*
* \defgroup group_ble_service_api_RSCS_server RSCS Server Functions
* \ingroup group_ble_service_api_RSCS
* \brief
* API unique to RSCS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_RSCSS_
*
* \defgroup group_ble_service_api_RSCS_client RSCS Client Functions
* \ingroup group_ble_service_api_RSCS
* \brief
* API unique to RSCS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_RSCSC_
*
* \defgroup group_ble_service_api_RSCS_definitions RSCS Definitions and Data Structures
* \ingroup group_ble_service_api_RSCS
* \brief
* Contains the RSCS specific definitions and data structures
* used in the RSCS API.
*
*
*
* \defgroup group_ble_service_api_RTUS Reference Time Update Service (RTUS)
* \ingroup group_ble_service_api
* \brief
* The Reference Time Update Service enables a Bluetooth device that can update the
* system time using the reference time such as a GPS receiver
* to expose a control point and expose the accuracy (drift) of
* the local system time compared to the reference time source.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The RTUS API names begin with Cy_BLE_RTUS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_RTUS_server_client RTUS Server and Client Function
* \ingroup group_ble_service_api_RTUS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_RTUS_
*
* \defgroup group_ble_service_api_RTUS_server RTUS Server Functions
* \ingroup group_ble_service_api_RTUS
* \brief
* API unique to RTUS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_RTUSS_
*
* \defgroup group_ble_service_api_RTUS_client RTUS Client Functions
* \ingroup group_ble_service_api_RTUS
* \brief
* API unique to RTUS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_RTUSC_
*
* \defgroup group_ble_service_api_RTUS_definitions RTUS Definitions and Data Structures
* \ingroup group_ble_service_api_RTUS
* \brief
* Contains the RTUS specific definitions and data structures
* used in the RTUS API.
*
*
*
* \defgroup group_ble_service_api_SCPS Scan Parameters Service (ScPS)
* \ingroup group_ble_service_api
* \brief
* The Scan Parameters Service enables a Server device to expose
* a Characteristic for the GATT Client to write its scan interval
* and scan window on the Server device, and enables a Server to
* request a refresh of the GATT Client scan interval and scan window.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The ScPS API names begin with Cy_BLE_SCPS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_SCPS_server_client ScPS Server and Client Functions
* \ingroup group_ble_service_api_SCPS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_SCPS_
*
* \defgroup group_ble_service_api_SCPS_server ScPS Server Functions
* \ingroup group_ble_service_api_SCPS
* \brief
* API unique to ScPS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_SCPSS_
*
* \defgroup group_ble_service_api_SCPS_client ScPS Client Functions
* \ingroup group_ble_service_api_SCPS
* \brief
* API unique to ScPS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_SCPSC_
*
* \defgroup group_ble_service_api_SCPS_definitions ScPS Definitions and Data Structures
* \ingroup group_ble_service_api_SCPS
* \brief
* Contains the ScPS specific definitions and data structures
* used in the ScPS API.
*
*
*
* \defgroup group_ble_service_api_TPS TX Power Service (TPS)
* \ingroup group_ble_service_api
* \brief
* The Tx Power Service uses the Tx Power Level Characteristic
* to expose the current transmit power level of a device when
* in a connection.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may
* use a subset of the supported API.
*
* The TPS API names begin with Cy_BLE_TPS_. In addition to this, the API
* also append the GATT role initial letter in the API name.
*
* \defgroup group_ble_service_api_TPS_server_client TPS Server and Client Function
* \ingroup group_ble_service_api_TPS
* \brief
* These are API common to both GATT Client role and GATT Server role.
* You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_TPS_
*
* \defgroup group_ble_service_api_TPS_server TPS Server Functions
* \ingroup group_ble_service_api_TPS
* \brief
* API unique to TPS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_TPSS_
*
* \defgroup group_ble_service_api_TPS_client TPS Client Functions
* \ingroup group_ble_service_api_TPS
* \brief
* API unique to TPS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_TPSC_
*
* \defgroup group_ble_service_api_TPS_definitions TPS Definitions and Data Structures
* \ingroup group_ble_service_api_TPS
* \brief
* Contains the TPS specific definitions and data structures
* used in the TPS API.
*
*
*
* \defgroup group_ble_service_api_UDS User Data Service (UDS)
* \ingroup group_ble_service_api
* \brief
* The User Data Service exposes user-related data in the sports and fitness environment. This allows remote access and
* update of user data by a Client as well as the synchronization of user data between a Server and a Client.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may use a subset of the supported API.
*
* The UDS API names begin with Cy_BLE_UDS_. In addition to this, the API also append the GATT role initial letter in the
* API name.
*
* \defgroup group_ble_service_api_UDS_server_client UDS Server and Client Function
* \ingroup group_ble_service_api_UDS
* \brief
* These are API common to both GATT Client role and GATT Server role. You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_UDS_
*
* \defgroup group_ble_service_api_UDS_server UDS Server Functions
* \ingroup group_ble_service_api_UDS
* \brief
* API unique to UDS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_UDSS_
*
* \defgroup group_ble_service_api_UDS_client UDS Client Functions
* \ingroup group_ble_service_api_UDS
* \brief
* API unique to UDS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_UDSC_
*
* \defgroup group_ble_service_api_UDS_definitions UDS Definitions and Data Structures
* \ingroup group_ble_service_api_UDS
* \brief
* Contains the UDS specific definitions and data structures used in the UDS API.
*
*
*
*
* \defgroup group_ble_service_api_WPTS Wireless Power Transfer Service (WPTS)
* \ingroup group_ble_service_api
* \brief
* The Wireless Power Transfer Service enables communication between Power Receiver Unit and Power Transmitter Unit
* in the Wireless Power Transfer systems.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may use a subset of the supported API.
*
* The WPTS API names begin with Cy_BLE_WPTS_. In addition to this, the API also append the GATT role initial letter in the
* API name.
*
* \defgroup group_ble_service_api_WPTS_server_client WPTS Server and Client Function
* \ingroup group_ble_service_api_WPTS
* \brief
* These are API common to both GATT Client role and GATT Server role. You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_WPTS_
*
* \defgroup group_ble_service_api_WPTS_server WPTS Server Functions
* \ingroup group_ble_service_api_WPTS
* \brief
* API unique to WPTS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_WPTSS_
*
* \defgroup group_ble_service_api_WPTS_client WPTS Client Functions
* \ingroup group_ble_service_api_WPTS
* \brief
* API unique to WPTS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_WPTSC_
*
* \defgroup group_ble_service_api_WPTS_definitions WPTS Definitions and Data Structures
* \ingroup group_ble_service_api_WPTS
* \brief
* Contains the WPTS specific definitions and data structures used in the WPTS API.
*
*
*
* \defgroup group_ble_service_api_WSS Weight Scale Service (WSS)
* \ingroup group_ble_service_api
* \brief
* The Weight Scale Service exposes weight and related data from a weight scale (Server) intended for consumer healthcare
* as well as sports/fitness applications.
*
* Depending on the chosen GATT role in the Bluetooth Configurator, you may use a subset of the supported API.
*
* The WSS API names begin with Cy_BLE_WSS_. In addition to this, the API also append the GATT role initial letter in the
* API name.
*
* \defgroup group_ble_service_api_WSS_server_client WSS Server and Client Function
* \ingroup group_ble_service_api_WSS
* \brief
* These are API common to both GATT Client role and GATT Server role. You may use them in either roles.
*
* No letter is appended to the API name: Cy_BLE_WSS_
*
* \defgroup group_ble_service_api_WSS_server WSS Server Functions
* \ingroup group_ble_service_api_WSS
* \brief
* API unique to WSS designs configured as a GATT Server role.
*
* A letter 's' is appended to the API name: Cy_BLE_WSSS_
*
* \defgroup group_ble_service_api_WSS_client WSS Client Functions
* \ingroup group_ble_service_api_WSS
* \brief
* API unique to WSS designs configured as a GATT Client role.
*
* A letter 'c' is appended to the API name: Cy_BLE_WSSC_
*
* \defgroup group_ble_service_api_WSS_definitions WSS Definitions and Data Structures
* \ingroup group_ble_service_api_WSS
* \brief
* Contains the WSS specific definitions and data structures used in the WSS API.
*
*
* \defgroup group_ble_service_api_custom Custom Service
* \ingroup group_ble_service_api
* \brief
* This section contains the data structures used for Custom Services.
*
*/
