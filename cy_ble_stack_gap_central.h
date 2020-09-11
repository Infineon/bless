/*******************************************************************************
* \file cy_ble_stack_gap_central.h
* \version 3.50
*
* \brief
*  This file contains declarations of public BLE APIs of Generic Access Profile - Central Role.
*  It also specifies the defines, constants, and data structures required for the APIs.
*
*
* Related Document:
*  BLE Standard Spec - CoreV5.0, CSS, CSAs, ESR05, ESR06
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#ifndef CY_BLE_STACK_GAP_CENTRAL_H_
#define CY_BLE_STACK_GAP_CENTRAL_H_

/***************************************
* Common BLE Stack includes
***************************************/

#include "cy_ble_stack_gap.h"

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************
** GAP Constants
***************************************/

/** Observation and discovery procedure */
#define CY_BLE_GAPC_OBSER_PROCEDURE              0x00u /**< Observation procedure */
#define CY_BLE_GAPC_LTD_DISC_PROCEDURE           0x01u /**< Limited discovery procedure */
#define CY_BLE_GAPC_GEN_DISC_PROCEDURE           0x02u /**< General discovery procedure */

/** Type of discovery procedure used in cy_stc_ble_gapc_disc_info_t*/
#define CY_BLE_GAPC_PASSIVE_SCANNING             0x00u /**< passive scan */
#define CY_BLE_GAPC_ACTIVE_SCANNING              0x01u /**< active scan */
#define CY_BLE_GAPC_FILTER_DUP_DISABLE           0x00u /**< Filter duplicate disable */
#define CY_BLE_GAPC_FILTER_DUP_ENABLE            0x01u /**< Filter duplicate enable */

/** Scan filter policy */

/** Accept all advertisement packets except directed advertising packets not
  * addressed to this device
  */
#define CY_BLE_GAPC_ADV_ACCEPT_ALL_PKT                       0x00u

/** Accept only advertisement packets from devices where the advertiser's address
  * is in the White List.  Directed advertising packets that are not addressed for
  * this device shall be ignored.
  */
#define CY_BLE_GAPC_ADV_ACCEPT_WHITELIST_PKT                 0x01u

/** Accept all undirected advertisement packets, and directed advertising packets
  * where the initiator address is a resolvable private address, and directed advertising
  * packets addressed to this device.
  */
#define CY_BLE_GAPC_ADV_ACCEPT_DIRECTED_RPA_PKT              0x02u

/** Accept all advertisement packets from devices where the advertiser's address
  * is in the White List, and directed advertising packets where the initiator address
  * is a resolvable private address, and directed advertising packets addressed
  * to this device.
  */
#define CY_BLE_GAPC_ADV_ACCEPT_WHITELIST_DIRECTED_RPA_PKT    0x03u

/** Initiator filter policy*/
#define CY_BLE_GAPC_CONN_ALL                     0x00u /**< Filter policy- connect all */
#define CY_BLE_GAPC_CONN_WHITELIST               0x01u /**< Filter policy- connect only from White List */

/**
 \addtogroup group_ble_common_api_gap_definitions
 @{
*/

/***************************************
** Enumerated Types
***************************************/

/** Advertisement event type  */
typedef enum
{
    /** Connectable undirected advertising */
    CY_BLE_GAPC_CONN_UNDIRECTED_ADV = 0x00u,

    /** Connectable directed advertising */
    CY_BLE_GAPC_CONN_DIRECTED_ADV,

    /** Scannable undirected advertising */
    CY_BLE_GAPC_SCAN_UNDIRECTED_ADV,

    /** Non connectable undirected advertising */
    CY_BLE_GAPC_NON_CONN_UNDIRECTED_ADV,

    /** Scan Response*/
    CY_BLE_GAPC_SCAN_RSP

}cy_en_ble_gapc_adv_event_t;

/***************************************
** Exported structures and unions
***************************************/

/** Discovery (scan) parameter  */
typedef struct
{
    /** Observation and discovery procedure.
         - CY_BLE_GAPC_OBSER_PROCEDURE (Observation procedure)
         - CY_BLE_GAPC_LTD_DISC_PROCEDURE (Limited discovery procedure)
         - CY_BLE_GAPC_GEN_DISC_PROCEDURE (General discovery procedure)
     */
    uint8_t      discProcedure;

    /** Type of scan to perform
         - CY_BLE_GAPC_PASSIVE_SCANNING (Passive Scanning)
         - CY_BLE_GAPC_ACTIVE_SCANNING (Active scanning)
     */
    uint8_t      scanType;

    /** The time interval from when last LE scan is started until next subsequent LE scan.
         Range: 0x0004 to 0x4000
         Unit: 0.625 msec
         Time Range: 2.5 ms to 10.24 sec.
     */
    uint16_t     scanIntv;

    /** The time duration of scanning to be performed
         Range: 0x0004 to 0x4000
         Unit: 0.625 msec
         Time Range: 2.5 ms to 10.24 sec.
     */
    uint16_t     scanWindow;

    /** Own BD Address Type
        - CY_BLE_GAP_ADDR_TYPE_PUBLIC
        - CY_BLE_GAP_ADDR_TYPE_RANDOM
        - CY_BLE_GAP_ADDR_TYPE_PUBLIC_RPA
        - CY_BLE_GAP_ADDR_TYPE_RANDOM_RPA
     */
    uint8_t      ownAddrType;

    /** Filter policies to be applied during scanning procedure
        - CY_BLE_GAPC_ADV_ACCEPT_ALL_PKT
        - CY_BLE_GAPC_ADV_ACCEPT_WHITELIST_PKT
        - CY_BLE_GAPC_ADV_ACCEPT_DIRECTED_RPA_PKT
        - CY_BLE_GAPC_ADV_ACCEPT_WHITELIST_DIRECTED_RPA_PKT
     */
    uint8_t      scanFilterPolicy;

    /** Scan time out is in seconds. If time out is set to 0, then there will not be any time out.
     */
    uint16_t     scanTo;

    /** Filter Duplicate Advertisements. This parameter controls whether
       the Link Layer shall filter duplicate advertising reports to the Host, or whether the
       Link Layer should generate advertising reports for each packet received.
        - CY_BLE_GAPC_FILTER_DUP_DISABLE (Duplicate filtering disabled)
        - CY_BLE_GAPC_FILTER_DUP_ENABLE (Duplicate filtering enabled)

       By default, duplicate filtering is enabled
     */
    uint8_t      filterDuplicates;

} cy_stc_ble_gapc_disc_info_t;

/** Connection parameter */
typedef struct
{
    /** The time interval from when last LE scan is started until next
       subsequent LE scan.
        - Time Range: 2.5 ms to 10.24 sec.
     */
    uint16_t      scanIntv;

    /** The time duration of scanning to be performed
        - Time Range: 2.5 ms to 10.24 sec
     */
    uint16_t      scanWindow;

    /** Filter policies to be applied during connection procedure
        - CY_BLE_GAPC_CONN_ALL (White List is not used to determine with
        which advertiser to connect. Peer address is used)
        - CY_BLE_GAPC_CONN_WHITELIST (White List is used to determine with
        which advertiser to connect. Peer address shall be ignored)
     */
    uint8_t       initiatorFilterPolicy;

    /** Peer's BD address with which connection is to be established. 
        BD address should be in Little Endian format.*/
    uint8_t       peerBdAddr[CY_BLE_GAP_BD_ADDR_SIZE];

    /** Peer's BD address type
        - CY_BLE_GAP_ADDR_TYPE_PUBLIC
        - CY_BLE_GAP_ADDR_TYPE_RANDOM
        - CY_BLE_GAP_ADDR_TYPE_PUBLIC_RPA
        - CY_BLE_GAP_ADDR_TYPE_RANDOM_RPA
     */
    uint8_t       peerAddrType;

    /** Own BD address type
        - CY_BLE_GAP_ADDR_TYPE_PUBLIC
        - CY_BLE_GAP_ADDR_TYPE_RANDOM
        - CY_BLE_GAP_ADDR_TYPE_PUBLIC_RPA
        - CY_BLE_GAP_ADDR_TYPE_RANDOM_RPA
    */
    uint8_t       ownAddrType;

    /** Minimum value for the connection event interval. This shall be less than
       or equal to conn_Interval_Max. Minimum connection interval will be
         connIntvMin * 1.25 ms
        * Time Range: 7.5 ms to 4 sec
     */
    uint16_t      connIntvMin;

    /** Maximum value for the connection event interval. This shall be greater
       than or equal to conn_Interval_Min. Maximum connection interval will be
         connIntvMax * 1.25 ms
        * Time Range: 7.5 ms to 4 sec
     */
    uint16_t      connIntvMax;

    /** Slave latency for the connection in number of connection events.
        * Range: 0x0000 to 0x01F3
     */
    uint16_t      connLatency;

    /** Supervision time out for the LE Link. Supervision time out will be
       supervisionTO * 10 ms
        * Time Range: 100 msec to 32 secs
     */
    uint16_t      supervisionTO;

    /** Minimum length of connection needed for this LE connection.
        * Range: 0x0000 - 0xFFFF
     */
    uint16_t      minCeLength;

    /** Maximum length of connection needed for this LE connection.
        * Range: 0x0000 - 0xFFFF
     */
    uint16_t      maxCeLength;

}cy_stc_ble_gapc_conn_info_t;

/** Resolve peer device parameter  */
typedef struct
{
    /** Peer Bluetooth device address */
    uint8_t      * bdAddr;

    /** Peer IRK */
    uint8_t       peerIrk[CY_BLE_GAP_SMP_IRK_SIZE];

}cy_stc_ble_gapc_resolve_peer_info_t;

/** Set peer BD Address parameter  */
typedef struct
{
    /** Peer Bluetooth device address */
    cy_stc_ble_gap_bd_addr_t  remoteAddr;

    /** Peer bdHandle */
    uint8_t                bdHandle;

}cy_stc_ble_gapc_peer_bd_addr_info_t;


/* --------------------------Structure corresponding to events-------------------- */

/** Advertisement report parameter */
typedef struct
{
    /** Advertisement event type
        - Connectable undirected advertising = 0x00
        - Connectable directed advertising = 0x01
        - Scannable undirected advertising = 0x02
        - Non connectable undirected advertising = 0x03
        - Scan Response = 0x04
     */
    cy_en_ble_gapc_adv_event_t      eventType;

    /** BD address type of the device advertising.
        - CY_BLE_GAP_ADDR_TYPE_PUBLIC
        - CY_BLE_GAP_ADDR_TYPE_RANDOM
        - CY_BLE_GAP_ADDR_TYPE_PUBLIC_RPA
        - CY_BLE_GAP_ADDR_TYPE_RANDOM_RPA
    */
    uint8_t                       peerAddrType;

    /** Public Device Address or Random Device Address for
       each device that responded to scanning. */
    uint8_t*                      peerBdAddr;

    /** length of the data for each device that responded to scanning */
    uint8_t                       dataLen;

    /** Pointer to advertising or scan response data */
    uint8_t*                      data;

    /** Rssi of the responding device.
                 * Range: -85 <= N <= 0
                 * Units: dBm */
    int8_t                        rssi;

} cy_stc_ble_gapc_adv_report_param_t;

/** Direct Advertising Report received by GAP Central */
typedef struct
{
    /** Buffer containing Random Device Address of Scanner (local device)
     * This is the address to which the directed advertisements are
     * being directed.
     * BD address should be in Little Endian format.
     */
    uint8_t                       * localBdAddr;

    /** Buffer containing Device Address of advertiser sending the directed advertisement */
    uint8_t                       * peerBdAddr;

    /** Device Address type of advertiser sending the directed advertisement */
    cy_en_ble_gap_adv_addr_type_t   peerBdAddrType;


    /** Rssi of the responding device.
     * Range: -127 <= N <= +20
     * Units: dBm
     * N = 127 -> RSSI not available
     */
    int8_t                        rssi;

} cy_stc_ble_gapc_direct_adv_report_param_t;
/** @} */



/***************************************
** Exported APIs
***************************************/
/**
 \addtogroup group_ble_common_api_gap_central_functions
 @{
*/

/** \cond IGNORE */

/******************************************************************************
* Function Name: Cy_BLE_GAPC_StartDiscovery
***************************************************************************//**
*
*  This function is used by GAP Central application to discover advertising devices.
*  The application should set the parameters required for the discovery procedure.
*  This is a non-blocking function.
*
*  When the discovery procedure is started, the Stack generates
*  CY_BLE_EVT_GAPC_SCAN_START_STOP event. 
*  
*  During the discovery procedure, CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT event is generated
*  for every received Advertisement packet or Scan Response packet.
*   
*  If param->scanTo is a non-zero value, CY_BLE_EVT_TIMEOUT event is generated after 
*  the specified time out limit and the discovery procedure is stopped.
*  If 'param->scanTo' is zero, the discovery 
*  operation is performed until the Cy_BLE_GAPC_StopDiscovery() function is called.
*  
*  If 'param->scanFilterPolicy' is set to CY_BLE_GAPC_ADV_ACCEPT_DIRECTED_RPA_PKT or CY_BLE_GAPC_ADV_ACCEPT_WHITELIST_DIRECTED_RPA_PKT,
*  CY_BLE_EVT_GAPC_DIRECT_ADV_REPORT event is generated to indicate that directed advertisements have been received from 
*  the advertiser that is using a resolvable private address (RPA).
*
*  There are three discovery procedures that can be specified as a parameter to
*  this function.
*  1. Observation procedure
*     A device in the observer role receives only advertisement data from
*     advertising devices irrespective of their discoverable mode settings. 
*    'param->scanType' can be set as CY_BLE_GAPC_ACTIVE_SCANNING or CY_BLE_GAPC_PASSIVE_SCANNING.
*
*  2. Limited Discovery procedure
*     A device in the limited discovery procedure receives advertisement data
*     and scan response data from devices that are in the limited discoverable mode only.
*    'param->scanType' should be set as active scanning (0x01).
*
*  3. General Discovery procedure
*     A device performing the general discovery procedure receives the advertisement
*     data and scan response data from devices in both limited discoverable mode and
*     general discoverable mode. 
*     'param->scanType' should be set as active scanning (0x01).
*
*  \note 
*  CY_BLE_EVT_GAPC_SCAN_START_STOP event is generated after calling 
*  Cy_BLE_GAPC_StartDiscovery and Cy_BLE_GAPC_StopDiscovery APIs.
*  Application should keep track of which function call resulted in this event.
*
*   \param param:  Pointer to a variable of type cy_stc_ble_gapc_disc_info_t
*
* \return
*  cy_en_ble_api_result_t : Return value indicates if the function succeeded or
*  failed. Following are the possible error codes.
*
*   Errors codes                          | Description
*   ------------                          | -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER        | On specifying NULL as input parameter for 'param' or if any element within 'param' has an invalid value.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | Memory allocation failed.
*   CY_BLE_ERROR_INSUFFICIENT_RESOURCES   | BLE Stack resources are unavailable.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPC_StartDiscovery
(
    cy_stc_ble_gapc_disc_info_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GAPC_StopDiscovery
***************************************************************************//**
*
*  This function is used by GAP Central application to stop the discovery procedure. 
*  This is a non-blocking function.
*
*  On stopping the scan, GAP central application receives
*  CY_BLE_EVT_GAPC_SCAN_START_STOP event. 
*
*  \note 
*  CY_BLE_EVT_GAPC_SCAN_START_STOP event is generated after calling 
*  Cy_BLE_GAPC_StartDiscovery and Cy_BLE_GAPC_StopDiscovery APIs.
*  Application should keep track of which function call resulted into this event.
*
*   Errors codes                          | Description
*   ------------                          | -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | Memory allocation failed.
*   CY_BLE_ERROR_INSUFFICIENT_RESOURCES   | BLE Stack resources are unavailable.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPC_StopDiscovery
(
    void
);


/******************************************************************************
* Function Name: Cy_BLE_GAPC_InitConnection
***************************************************************************//**
*  This function is used by GAP Central application to initiate a LE connection with 
*  a peer Bluetooth device with specified connection parameters.
*  This is a non-blocking function.
*
*  The BLE Stack initiates the connection procedure and generates following events
*  when the connection is established-
*  CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE- if Link Layer Privacy is enabled.
*  CY_BLE_EVT_GAP_DEVICE_CONNECTED- if Link Layer Privacy is not enabled.
*  CY_BLE_EVT_GATT_CONNECT_IND event is also generated along with either of the above events.
*  CY_BLE_EVT_GATT_CONNECT_IND event is generated first then CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE or CY_BLE_EVT_GAP_DEVICE_CONNECTED.
*
*  If the application has set the CY_BLE_CONN_ESTB_EVENT_MASK event mask using 
*  Cy_BLE_SetCustomEventMask() function, CY_BLE_EVT_GAP_CONN_ESTB event will be generated
*  when connection is established.
*
*  If BLE Stack fails to establish connection then 'CY_BLE_EVT_GAP_DEVICE_DISCONNECTED' is generated to the application when connection is established.
*
*  This function should not be called  
*  1. until the previous call to this function has not yet resulted in the event CY_BLE_EVT_GAP_DEVICE_CONNECTED or
*     CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE 
*  2. until the previous call to Cy_BLE_GAPC_CancelConnection function is not completed. Completion of the function is informed through 
*     'CY_BLE_EVT_GAP_CREATE_CONN_CANCEL_COMPLETE' event
*  3. if discovery procedure is ongoing. Application should first stop discovery using
*     Cy_BLE_GAPC_StopDiscovery function.
*
*  If the peer GATT Peripheral is a PSoC 6 device, this function results in the CY_BLE_EVT_GATT_CONNECT_IND 
*  event and CY_BLE_EVT_GAP_DEVICE_CONNECTED or CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE at Peripheral side after connection is established.
*
*  \param param: Structure of type 'cy_stc_ble_gapc_conn_info_t' that contains the
*              connection parameters.\n
*             <b>Note</b> Any parameter of structure type cy_stc_ble_gapc_conn_info_t may be ignored,
*             if not required by a specific Bluetooth Low Energy profile or application.
*
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*   Errors codes                          | Description
*   ------------                          | -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER        | On specifying NULL as input parameter for 'param' or if any element within 'param' has an invalid value.
*   CY_BLE_ERROR_INVALID_OPERATION        | Operation is not permitted.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | Memory allocation failed.
*   CY_BLE_ERROR_INSUFFICIENT_RESOURCES   | BLE Stack resources are unavailable.
*
*  \note:
*  Note1: BLE Stack supports only one connection from a master at a time.
*  Note2: In multi connection use case, the recommended minimum connection interval per connection should be
*        greater than N * Max Time taken by individual connections to complete a BLE Connection Event (CE).
*        Min_CI = N * Average Time Per CE.
*        Average time for each CE is the amount of time taken to complete one BLE Tx and Rx transaction. This
*        time varies depending on Link Layer Data Length Extension (DLE) and BLE data rate (1 Mbps or 2 Mbps) 
*        configuration. The application can use the below timing lookup table for CE value:
*            1. If DLE is enabled and data rate is 1 Mbps, Average time = 6 ms
*            2. If DLE is enabled and data rate is 2 Mbps, Average time = 3.5 ms
*            3. If DLE is disabled and data rate is 1 Mbps, Average time = 2 ms
*            4. If DLE is disabled and data rate is 2 Mbps, Average time = 1.6 ms
*        For example, if application supports 4 BLE connections with DLE and 1 Mbps data rate, then the recommended
*        minimum connection interval for each of the connections is 4 * 6 = 24 ms.
*        Note that connection intervals less than this value will still work, but under certain circumstances, the
*        real time control procedures (connection update, channel map update, etc.) with shorter update instances might
*        result in link disconnection.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPC_InitConnection
(
    cy_stc_ble_gapc_conn_info_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GAPC_CancelConnection
***************************************************************************//**
*
* Description:
*  This function is used by GAP Central application to cancel a previously initiated 
*  connection procedure with a peer device. This is a non-blocking function.
*
*  CY_BLE_EVT_GAP_CREATE_CONN_CANCEL_COMPLETE event is generated to inform about completion 
*  of this procedure.
*
*  This function should not be used if the devices are already connected. To disconnect 
*  an existing connection, use the function Cy_BLE_GAP_Disconnect().
*
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*   Errors codes                             | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | On successful operation.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is not permitted.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_INSUFFICIENT_RESOURCES      | BLE Stack resources are unavailable.
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPC_CancelConnection
(
    void
);

/** \endcond */

/******************************************************************************
* Function Name: Cy_BLE_GAPC_ResolveDevice
***************************************************************************//**
*
*  This function enables the GAP Central application to start a address resolution procedure 
*  for a peer device that is connected using a resolvable private address. 
*  This is a non-blocking function.
*
*  Refer to Bluetooth 5.0 Core specification, Volume 3, Part C, section 10.8.2.3
*  Resolvable Private Address Resolution Procedure to understand the usage of
*  Private addresses.
*
*  'CY_BLE_EVT_GAP_RESOLVE_DEVICE_COMPLETE' event is generated to inform about the completion
*  of address resolution procedure.
*
*  \param param: Parameter is of type cy_stc_ble_gapc_resolve_peer_info_t
*   param->bdAddr: Peer Bluetooth device address of 6 bytes length, not
*           NULL terminated.
*   param->peerIrk: 128-bit IRK to be used for resolving the peer's private
*        resolvable address.
*
* \return
*  cy_en_ble_api_result_t : Return value indicates if the function succeeded or
*  failed. Following are the possible error codes.
*
*   Errors codes                          | Description
*   ------------                          | -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER        | On specifying NULL as input parameter for 'param' or 'bdAddr'.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | Memory allocation failed.
*   CY_BLE_ERROR_INSUFFICIENT_RESOURCES   | BLE Stack resources are unavailable.
*
* \note:
*  Note1: param should point to a valid memory location until completion of function
*         is indicated via 'CY_BLE_EVT_GAP_RESOLVE_DEVICE_COMPLETE' event.
*  Note2: If application is using White List filtering, update the White List
*         with the new address using Cy_BLE_AddDeviceToWhiteList() function.
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPC_ResolveDevice
(
    cy_stc_ble_gapc_resolve_peer_info_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GAPC_SetRemoteAddr
***************************************************************************//**
*
*  This function is used by GAP Central application to set the new address of peer device 
*  identified by bdHandle. No event is generated on calling this function. This is a
*  blocking function.
*
*  This function should be used when:
*  1. A peer device bonded previously with a public address and changes its BD address
*     to a resolvable private address. The application should first resolve the device address 
*     by calling the 'Cy_BLE_GAPC_ResolveDevice()' function. If address resolution is successful then 
*     set the new address using this function.
*  2. A peer device is previously bonded with a random address, then application should call 
*     this function to set the new address(public/random).
*
*  \param param: Parameter of type 'cy_stc_ble_gapc_peer_bd_addr_info_t'
*
* \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. Following are the possible error codes.
*
*   Errors codes                         | Description
*   ------------                         | -----------
*   CY_BLE_SUCCESS                       | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER       | On invalid bdHandle.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPC_SetRemoteAddr
(
    cy_stc_ble_gapc_peer_bd_addr_info_t  * param
);


/******************************************************************************
* Function Name: Cy_BLE_GAPC_ConnectionParamUpdateRequest
***************************************************************************//**
*
*  This function is used by GAP Central application to initiate the Connection Parameter 
*  Update Procedure. This function updates parameters to local BLE Controller. Local BLE Controller
*  follows the connection update procedure as defined in Bluetooth Core Specification 4.0.
*  This function does not perform parameter negotiation with peer device through LL topology as defined in 
*  Bluetooth Core Specification 4.1.
*
*  CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE event is generated to inform about 
*  completion of Connection Parameter Update procedure.
*
*  \param param: Parameter is of type 'cy_stc_ble_gap_conn_update_param_info_t'
*
* \return
*  cy_en_ble_api_result_t : Return value indicates if the function succeeded or
*  failed. Following are the possible error codes.
*
*   Errors codes                          | Description
*   ------------                          | -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER        | On specifying NULL for input parameter.
*   CY_BLE_ERROR_NO_DEVICE_ENTITY         | If connection does not exist for corresponding 'bdHandle'.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | Memory allocation failed.
*   CY_BLE_ERROR_INSUFFICIENT_RESOURCES   | BLE Stack resources are unavailable.
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GAPC_ConnectionParamUpdateRequest
(
    cy_stc_ble_gap_conn_update_param_info_t  * param
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/** @} */

#endif /* CY_BLE_STACK_GAP_CENTRAL_H_ */

/*EOF*/
