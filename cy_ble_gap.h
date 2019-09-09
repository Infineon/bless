/***************************************************************************//**
* \file cy_ble_gap.h
* \version 3.20
*
* \brief
*  Contains the prototypes and constants used in the BLE GAP profile.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_GAP_H
#define CY_BLE_GAP_H

#include <stdbool.h>
#include <stdint.h>

#include "cy_ble_defines.h"
#include "cy_syslib.h"

/* BLE Stack includes */
#include "cy_ble_stack_gatt.h"
#include "cy_ble_stack_gap.h"
#include "cy_ble_stack_gap_central.h"
#include "cy_ble_stack_gap_peripheral.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_common_api_gap_definitions
 * \{
 */

/** BLE state machine type */
typedef enum
{
    CY_BLE_STATE_STOPPED,                             /**< BLE is turned off */
    CY_BLE_STATE_INITIALIZING,                        /**< Initializing state */
    CY_BLE_STATE_ON,                                  /**< BLE is turned on */
    CY_BLE_STATE_CONNECTING,                          /**< Connecting */
} cy_en_ble_state_t;

/** Scanning state machine type */
typedef enum
{
    CY_BLE_SCAN_STATE_STOPPED,                        /**< Scanning is stopped */
    CY_BLE_SCAN_STATE_SCAN_INITIATED,                 /**< Scanning is initiated */
    CY_BLE_SCAN_STATE_SCANNING,                       /**< Scanning process */
    CY_BLE_SCAN_STATE_STOP_INITIATED                  /**< Stop scanning is initiated */
} cy_en_ble_scan_state_t;

/** Advertising state machine type */
typedef enum
{
    CY_BLE_ADV_STATE_STOPPED,                         /**< Advertising is stopped */
    CY_BLE_ADV_STATE_ADV_INITIATED,                   /**< Advertising is initiated */
    CY_BLE_ADV_STATE_ADVERTISING,                     /**< Advertising process */
    CY_BLE_ADV_STATE_STOP_INITIATED                   /**< Stop advertising is initiated */
} cy_en_ble_adv_state_t;

/** Connection state machine type */
typedef enum
{
    CY_BLE_CONN_STATE_DISCONNECTED,                   /**< Essentially idle state */
    CY_BLE_CONN_STATE_CLIENT_DISCONNECTED_DISCOVERED, /**< Server is disconnected but discovered */
    CY_BLE_CONN_STATE_CONNECTED,                      /**< Peer device is connected for this and following states */
    CY_BLE_CONN_STATE_CLIENT_SRVC_DISCOVERING,        /**< Server services are being discovered */
    CY_BLE_CONN_STATE_CLIENT_INCL_DISCOVERING,        /**< Server included services are being discovered */
    CY_BLE_CONN_STATE_CLIENT_CHAR_DISCOVERING,        /**< Server Characteristics are being discovered */
    CY_BLE_CONN_STATE_CLIENT_DESCR_DISCOVERING,       /**< Server char. descriptors are being discovered */
    CY_BLE_CONN_STATE_CLIENT_DISCOVERED               /**< Server is discovered */
} cy_en_ble_conn_state_t;

/** Structure with Generic Access Profile Service (GAPS) attribute handles */
typedef struct
{
    /** Service Handle*/
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Handle of the GAPS Device Name Characteristic */
    cy_ble_gatt_db_attr_handle_t deviceNameCharHandle;

    /** Handle of the GAPS Appearance Characteristic */
    cy_ble_gatt_db_attr_handle_t appearanceCharHandle;

    /** Handle of the GAPS Peripheral Preferred Connection Parameters Characteristic */
    cy_ble_gatt_db_attr_handle_t prefConnParamCharHandle;

    /** Handle of the GAPS Central Address Resolution Characteristic */
    cy_ble_gatt_db_attr_handle_t centralAddrResolutionCharHandle;

    /** Handle of the GAPS Resolvable Private Address Only Characteristic */
    cy_ble_gatt_db_attr_handle_t resolvablePrivateAddressOnly;
} cy_stc_ble_gaps_t;

/** GAP Service Characteristics server's GATT DB handles structure type */
typedef struct
{
    /** Handle of the GAPS Device Name Characteristic */
    cy_ble_gatt_db_attr_handle_t deviceNameCharHandle;

    /** Handle of the GAPS Appearance Characteristic */
    cy_ble_gatt_db_attr_handle_t appearanceCharHandle;

    /** Handle of the GAPS Peripheral Privacy Flag Parameters Characteristic */
    cy_ble_gatt_db_attr_handle_t periphPrivacyCharHandle;

    /** Handle of the GAPS Reconnection Address Characteristic */
    cy_ble_gatt_db_attr_handle_t reconnAddrCharHandle;

    /** Handle of the GAPS Peripheral Preferred Connection Parameters Characteristic */
    cy_ble_gatt_db_attr_handle_t prefConnParamCharHandle;

    /** Handle of the GAPS Central Address Resolution Characteristic */
    cy_ble_gatt_db_attr_handle_t centralAddrResolutionCharHandle;

    /** Handle of the GAPS Resolvable Private Address Only Characteristic */
    cy_ble_gatt_db_attr_handle_t resolvablePrivateAddressOnly;
} cy_stc_ble_gapc_t;

/** GAP Service configuration structure (server) */
typedef struct
{
    /** Structure with  attributes information */
    cy_stc_ble_gaps_t *attrInfo;

} cy_stc_ble_gaps_config_t;

/** GAP Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_gapc_t *attrInfo;

    /** The Discovery Service index */
    uint8_t serviceDiscIdx;

} cy_stc_ble_gapc_config_t;

/** Advertisement SIG assigned numbers */
typedef enum
{
    /** Flags */
    CY_BLE_GAP_ADV_FLAGS = 0x01u,

    /** Incomplete List of 16-bit Service Class UUIDs */
    CY_BLE_GAP_ADV_INCOMPL_16UUID,

    /** Complete List of 16-bit Service Class UUIDs */
    CY_BLE_GAP_ADV_COMPL_16UUID,

    /** Incomplete List of 32-bit Service Class UUIDs */
    CY_BLE_GAP_ADV_INCOMPL_32_UUID,

    /** Complete List of 32-bit Service Class UUIDs */
    CY_BLE_GAP_ADV_COMPL_32_UUID,

    /** Incomplete List of 128-bit Service Class UUIDs */
    CY_BLE_GAP_ADV_INCOMPL_128_UUID,

    /** Complete List of 128-bit Service Class UUIDs */
    CY_BLE_GAP_ADV_COMPL_128_UUID,

    /** Shortened Local Name */
    CY_BLE_GAP_ADV_SHORT_NAME,

    /** Complete Local Name */
    CY_BLE_GAP_ADV_COMPL_NAME,

    /** Tx Power Level */
    CY_BLE_GAP_ADV_TX_PWR_LVL,

    /** Class of Device */
    CY_BLE_GAP_ADV_CLASS_OF_DEVICE = 0x0Du,

    /** Simple Pairing Hash C */
    CY_BLE_GAP_ADV_SMPL_PAIR_HASH_C,

    /** Simple Pairing Randomizer R */
    CY_BLE_GAP_ADV_SMPL_PAIR_RANDOM_R,

    /** Device ID*/
    CY_BLE_GAP_ADV_DEVICE_ID,

    /** Security Manager TK Value */
    CY_BLE_GAP_ADV_SCRT_MNGR_TK_VAL = 0x10u,

    /** Security Manager Out of Band Flags */
    CY_BLE_GAP_ADV_SCRT_MNGR_OOB_FLAGS,

    /** Slave Connection Interval Range */
    CY_BLE_GAP_ADV_SLAVE_CONN_INTRV_RANGE,

    /** List of 16-bit Service Solicitation UUIDs */
    CY_BLE_GAP_ADV_SOLICIT_16UUID = 0x14u,

    /** List of 128-bit Service Solicitation UUIDs */
    CY_BLE_GAP_ADV_SOLICIT_128UUID,

    /** Service Data - 16-bit UUID */
    CY_BLE_GAP_ADV_SRVC_DATA_16UUID,

    /** Public Target Address */
    CY_BLE_GAP_ADV_PUBLIC_TARGET_ADDR,

    /** Random Target Address */
    CY_BLE_GAP_ADV_RANDOM_TARGET_ADDR,

    /** Appearance */
    CY_BLE_GAP_ADV_APPEARANCE,

    /** Advertising Interval */
    CY_BLE_GAP_ADV_ADVERT_INTERVAL,

    /** LE Bluetooth Device Address */
    CY_BLE_GAP_ADV_LE_BT_DEVICE_ADDR,

    /** LE Role */
    CY_BLE_GAP_ADV_LE_ROLE,

    /** Simple Pairing Hash C-256 */
    CY_BLE_GAP_ADV_SMPL_PAIR_HASH_C256,

    /** Simple Pairing Randomizer R-256 */
    CY_BLE_GAP_ADV_SMPL_PAIR_RANDOM_R256,

    /** List of 32-bit Service Solicitation UUIDs */
    CY_BLE_GAP_ADV_SOLICIT_32UUID,

    /** Service Data - 32-bit UUID */
    CY_BLE_GAP_ADV_SRVC_DATA_32UUID,

    /** Service Data - 128-bit UUID */
    CY_BLE_GAP_ADV_SRVC_DATA_128UUID,

    /** 3D Information Data */
    CY_BLE_GAP_ADV_3D_INFO_DATA = 0x3D,

    /** Manufacturer Specific Data */
    CY_BLE_GAP_ADV_MANUFACTURER_SPECIFIC_DATA = 0xFF
} cy_en_ble_gap_adv_assign_numbers_t;

/** GAPP advertising configuration parameters structure */
typedef struct
{
    /** The minimum interval for fast advertising the data and establishing the LE Connection */
    uint16_t fastAdvIntervalMin;

    /** The maximum interval for fast advertising the data and establishing the LE Connection */
    uint16_t fastAdvIntervalMax;

    /** The timeout value of  fast advertising with fast advertising interval parameters */
    uint16_t fastAdvTimeOut;

    /** Slow advertising enable */
    uint8_t  slowAdvEnable;

    /** The minimum interval for slow advertising the data and establishing the LE Connection */
    uint16_t slowAdvIntervalMin;

    /** The maximum interval for slow  advertising the data and establishing the LE Connection */
    uint16_t slowAdvIntervalMax;

    /** The timeout value of slow advertising with slow advertising interval parameters */
    uint16_t slowAdvTimeOut;
} cy_stc_ble_gapp_adv_params_t;

/** GAPC advertising configuration parameters structure */
typedef struct
{
    /** The scan interval when operating in Fast connection */
    uint16_t fastScanInterval;

    /** Defines the scan window when operating in Fast connection */
    uint16_t fastScanWindow;

    /** The timeout value of scanning with fast scan parameters */
    uint16_t fastScanTimeOut;

    /** Slow scan enable/disable */
    uint8_t  slowScanEnabled;

    /** The scan interval when operating in Slow connection */
    uint16_t slowScanInterval;

    /** Defines the scan window when operating in Slow connection */
    uint16_t slowScanWindow;

    /** The timeout value of scanning with Slow scan parameters */
    uint16_t slowScanTimeOut;

    /** The minimum permissible connection time value to be used during a connection event */
    uint16_t gapcConnectionIntervalMin;

    /** The maximum permissible connection time value to be used during a connection event */
    uint16_t gapcConnectionIntervalMax;

    /** Defines the maximum time between two received Data Packet PDUs before
     *  the connection is considered lost */
    uint16_t gapcConnectionSlaveLatency;

    /** Defines the LE link supervision timeout interval */
    uint16_t gapcConnectionTimeOut;
} cy_stc_ble_gapc_scan_params_t;

/** Store BLE Application Data parameter into flash */
typedef struct
{
    /** Source buffer */
    const uint8_t *srcBuff;

    /** Destination buffer */
    const uint8_t *destAddr;

    /** Source buffer length */
    uint32_t      buffLen;

    /** Write mode: blocking (default) / non-blocking */
    uint32_t      writeMode;

} cy_stc_ble_app_flash_param_t;
/** \} group_ble_common_api_gap_definitions */


/*******************************************************************************
* API Constants
*******************************************************************************/

/** The max supported connection count */
#define CY_BLE_MAX_SUPPORTED_CONN_COUNT            (4u)
#define CY_BLE_CCCD_CRC_BYTE                       (0x1u)        /**< CCCD CRC byte */

#define CY_BLE_AD_TYPE_MORE16UUID                  (0x02u)
#define CY_BLE_AD_TYPE_CMPL16UUID                  (0x03u)
#define CY_BLE_AD_TYPE_MORE32UUID                  (0x04u)
#define CY_BLE_AD_TYPE_CMPL32UUID                  (0x05u)
#define CY_BLE_AD_TYPE_MORE128UUID                 (0x06u)
#define CY_BLE_AD_TYPE_CMPL128UUID                 (0x07u)

#define CY_BLE_AD_TYPE_SERVICE_DATA                (0x16u)
#define CY_BLE_AD_SERVICE_DATA_OVERHEAD            (0x03u)

#define CY_BLE_DISCOVERY_IDLE                      (0x00u)
#define CY_BLE_DISCOVERY_SERVICE                   (0x01u)
#define CY_BLE_DISCOVERY_CHAR                      (0x02u)
#define CY_BLE_DISCOVERY_DESCR                     (0x03u)
#define CY_BLE_DISCOVERY_DONE                      (0x04u)

/* Cy_BLE_NextCharDscrDiscovery parameters */
#define CY_BLE_DISCOVERY_INIT                      (0x00u)
#define CY_BLE_DISCOVERY_CONTINUE                  (0x01u)

#define CY_BLE_SFLASH_DIE_X_MASK                   (0x3Fu)
#define CY_BLE_SFLASH_DIE_X_BITS                   (6u)
#define CY_BLE_SFLASH_DIE_Y_MASK                   (0x3Fu)
#define CY_BLE_SFLASH_DIE_Y_BITS                   (6u)
#define CY_BLE_SFLASH_DIE_XY_BITS                  (CY_BLE_SFLASH_DIE_X_BITS + CY_BLE_SFLASH_DIE_Y_BITS)
#define CY_BLE_SFLASH_DIE_WAFER_MASK               (0x1Fu)
#define CY_BLE_SFLASH_DIE_WAFER_BITS               (5u)
#define CY_BLE_SFLASH_DIE_XYWAFER_BITS             (CY_BLE_SFLASH_DIE_XY_BITS + CY_BLE_SFLASH_DIE_WAFER_BITS)
#define CY_BLE_SFLASH_DIE_LOT_MASK                 (0x7Fu)
#define CY_BLE_SFLASH_DIE_LOT_BITS                 (7u)

/* Device address stored by user in ROW4 of the SFLASH */
#define CY_BLE_SFLASH_DEVICE_ADDRESS_PTR           ((cy_stc_ble_gap_bd_addr_t*)(SFLASH_BLE_DEVICE_ADDRESS))
#define CY_BLE_AD_STRUCTURE_MAX_LENGTH             (31u)

/* AD types for complete, shortened local name and device address */
#define CY_BLE_SHORT_LOCAL_NAME                    (0x08u)  /**< Shortened Local Name */
#define CY_BLE_COMPLETE_LOCAL_NAME                 (0x09u)  /**< Complete Local Name */
#define CY_BLE_ADV_DEVICE_ADDR                     (0x1Bu)  /**< LE Bluetooth Device Address */

#define CY_BLE_ADVERTISING_FAST                    (0x00u)
#define CY_BLE_ADVERTISING_SLOW                    (0x01u)
#define CY_BLE_ADVERTISING_CUSTOM                  (0x02u)

#define CY_BLE_SCANNING_FAST                       (0x00u)
#define CY_BLE_SCANNING_SLOW                       (0x01u)
#define CY_BLE_SCANNING_CUSTOM                     (0x02u)

#define CY_BLE_PENDING_STACK_FLASH_WRITE_BIT       (0x01u)
#define CY_BLE_PENDING_CCCD_FLASH_WRITE_BIT        (0x02u)
#define CY_BLE_PENDING_CCCD_FLASH_CLEAR_BIT        (0x04u)
#define CY_BLE_PENDING_CCCD_FLASH_CLEAR_ALL_BIT    (0x08u)
#define CY_BLE_PENDING_CCCD_FLASH_CLEAR_MASK \
    (CY_BLE_PENDING_CCCD_FLASH_CLEAR_BIT | CY_BLE_PENDING_CCCD_FLASH_CLEAR_ALL_BIT)

/**
 * \addtogroup group_ble_common_api_macros
 * \{
 */

/* GAP Advertisement Flags */
#define CY_BLE_GAP_ADV_FLAG_LE_LIMITED_DISC_MODE        (0x01u)     /**< LE Limited Discoverable Mode. */
#define CY_BLE_GAP_ADV_FLAG_LE_GENERAL_DISC_MODE        (0x02u)     /**< LE General Discoverable Mode. */
#define CY_BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED        (0x04u)     /**< BR/EDR not supported. */
#define CY_BLE_GAP_ADV_FLAG_LE_BR_EDR_CONTROLLER        (0x08u)     /**< Simultaneous LE and BR/EDR, Controller. */
#define CY_BLE_GAP_ADV_FLAG_LE_BR_EDR_HOST              (0x10u)     /**< Simultaneous LE and BR/EDR, Host. */
#define CY_BLE_GAP_ADV_FLAGS_PACKET_LENGTH              (0x02u)     /**< Length of flags in an advertisement packet */

/* GAP Advertising interval min and max */
#define CY_BLE_GAP_ADV_ADVERT_INTERVAL_MIN              (0x0020u)   /**< Minimum Advertising interval in 625 us units,
                                                                     *   i.e. 20 ms. */
#define CY_BLE_GAP_ADV_ADVERT_INTERVAL_NONCON_MIN       (0x00A0u)   /**< Minimum Advertising interval in 625 us units
                                                                     *   for non connectable mode, i.e. 100 ms. */
#define CY_BLE_GAP_ADV_ADVERT_INTERVAL_MAX              (0x4000u)   /**< Maximum Advertising interval in 625 us units,
                                                                     *   i.e. 10.24 s. */
#define CY_BLE_GAP_ADV_ADVERT_INTERVAL_PACKET_LENGTH    (0x03u)     /**< Length of the Advertising Interval AD type in
                                                                     *   an advertisement packet */
#define CY_BLE_GAPC_CONNECTING_TIMEOUT                  (30u)       /**< Seconds */
#define CY_BLE_INVALID_CONN_HANDLE_VALUE                (0xFFu)     /**< Invalid Connection Handle Value */

/* Write mode for Cy_BLE_StoreAppData */
#define CY_BLE_STORE_DATA_MODE_BLOCKING                 (0x0u)      /**< Blocking write */
#define CY_BLE_STORE_DATA_MODE_NON_BLOCKING             (0x1u)      /**< Non-blocking write */
/** \} group_ble_common_api_macros */


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

cy_en_ble_api_result_t Cy_BLE_GAPS_Init(const cy_stc_ble_gaps_config_t *config);
cy_en_ble_api_result_t Cy_BLE_GAPC_Init(const cy_stc_ble_gapc_config_t *config);
uint32_t Cy_BLE_GetDiscoveryIdx(cy_stc_ble_conn_handle_t connHandle);


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
 * \addtogroup group_ble_common_api_functions
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_StoreAppData(const cy_stc_ble_app_flash_param_t *param);
cy_en_ble_api_result_t Cy_BLE_StoreBondingData(void);
cy_en_ble_api_result_t Cy_BLE_GAP_RemoveBondedDevice(cy_stc_ble_gap_bd_addr_t* bdAddr);
bool Cy_BLE_IsPeerConnected(uint8_t *bdAddr);
bool Cy_BLE_IsDevicePaired(cy_stc_ble_conn_handle_t *connHandle);
uint8_t Cy_BLE_GetDeviceRole(cy_stc_ble_conn_handle_t *connHandle);
cy_en_ble_api_result_t Cy_BLE_GetRssiPeer(uint8_t  bdHandle);

uint8_t Cy_BLE_GetNumOfActiveConn(void);
cy_stc_ble_conn_handle_t Cy_BLE_GetConnHandleByBdHandle(uint8_t bdHandle);

/** \} group_ble_common_api_functions */

/**
 * \addtogroup group_ble_common_api_gap_peripheral_functions
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_GAPP_StartAdvertisement(uint8_t advertisingIntervalType, uint8_t advertisingParamIndex);
cy_en_ble_api_result_t Cy_BLE_GAPP_StopAdvertisement(void);

/** \cond IGNORE */
void Cy_BLE_ChangeAdDeviceAddress(const cy_stc_ble_gap_bd_addr_t* bdAddr, uint8_t dest);
/** \endcond */
/** \} group_ble_common_api_gap_peripheral_functions */


/**
 * \addtogroup group_ble_common_api_gap_central_functions
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_GAPC_StartScan(uint8_t scanningIntervalType, uint8_t scanParamIndex);
cy_en_ble_api_result_t Cy_BLE_GAPC_StopScan(void);

cy_en_ble_api_result_t Cy_BLE_GAPC_ConnectDevice(const cy_stc_ble_gap_bd_addr_t * address, uint8_t centralParamIndex);
cy_en_ble_api_result_t Cy_BLE_GAPC_CancelDeviceConnection(void);
/** \} group_ble_common_api_gap_central_functions */


/*******************************************************************************
* External Data references [internal]
*******************************************************************************/

/* BLE state */
extern cy_en_ble_state_t      cy_ble_state;

/* BLE Advertising state */
extern cy_en_ble_adv_state_t  cy_ble_advState;

/* BLE Scanning state  */
extern cy_en_ble_scan_state_t cy_ble_scanState;

/* BLE Connection state */
extern cy_en_ble_conn_state_t cy_ble_connState[CY_BLE_MAX_SUPPORTED_CONN_COUNT];

/*
 * Variable is initialized after connection with peer device,
 * after CY_BLE_EVT_GATT_CONNECT_IND event, and could be used by application code
 * to send data to peer device.
 */
extern cy_stc_ble_conn_handle_t cy_ble_connHandle[CY_BLE_MAX_SUPPORTED_CONN_COUNT];

/* Connection parameter */
extern cy_stc_ble_gapc_conn_info_t cy_ble_connectionParameters;

/*
 * Connecting timeout is set to 30 seconds in Cy_BLE_Init function.
 * Not zero value starts timer in Cy_BLE_GAPC_ConnectDevice().
 */
extern cy_stc_ble_timer_info_t cy_ble_connectingTimeout;


/* Active Advertising configuration */
extern uint8_t cy_ble_advIndex;

/* Active Scanning configuration */
extern uint8_t cy_ble_scanIndex;

/*
 * Store information that device connected as role:
 * CY_BLE_GAP_LL_ROLE_SLAVE or CY_BLE_GAP_LL_ROLE_MASTER.
 */
extern uint8_t cy_ble_devConnRole[CY_BLE_MAX_SUPPORTED_CONN_COUNT];

/* Stored information is device paired */

extern bool cy_ble_pairStatus[CY_BLE_MAX_SUPPORTED_CONN_COUNT];

/*
 * Variable is used to detect status of pending write to flash operation for
 * BLE Stack data and CCCD.
 */
extern uint8_t cy_ble_pendingFlashWrite;

/*
 * Bonding type setting of peer device, CY_BLE_GAP_BONDING_NONE or
 * CY_BLE_GAP_BONDING. It is initialized after pairing with peer device and
 * used for cy_ble_pendingFlashWrite variable setting.
 */
extern uint8_t cy_ble_peerBonding[CY_BLE_MAX_SUPPORTED_CONN_COUNT];

extern uint8_t cy_ble_advertisingIntervalType;
extern uint8_t cy_ble_scanningIntervalType;

extern const cy_stc_ble_gaps_config_t *cy_ble_gapsConfigPtr;
extern const cy_stc_ble_gapc_config_t *cy_ble_gapcConfigPtr;

extern uint8_t cy_ble_pendingFlashClearCccdHandle;


/*******************************************************************************
* Exported Functions (MACROS)
*******************************************************************************/

/**
 * \addtogroup group_ble_common_api_functions
 * \{
*/
/******************************************************************************
* Function Name: Cy_BLE_GetState
***************************************************************************//**
*
*  This function is used to determine the current state of the PSoC 6 BLE Middleware
*  state machine :
*
*  \image html ble_middleware_state_machine.png
*
*  The PSoC 6 BLE Middleware is in the state #CY_BLE_STATE_INITIALIZING after the
*  Cy_BLE_Enable() is called and until the #CY_BLE_EVT_STACK_ON event is not
*  received. After successful initialization the state is changed to
*  #CY_BLE_STATE_ON.
*
*  The PSoC 6 BLE Middleware is in the state #CY_BLE_STATE_STOPPED
*  when the #CY_BLE_EVT_STACK_SHUTDOWN_COMPLETE event is received (after the
*  Cy_BLE_Disable() is called).
*
*  For GAP Central role when the Cy_BLE_GAPC_ConnectDevice() is called, the state
*  is changed to #CY_BLE_STATE_CONNECTING. After successful connection indicated
*  by #CY_BLE_EVT_GAP_DEVICE_CONNECTED or #CY_BLE_EVT_GAP_ENHANCE_CONN_COMPLETE
*  event or the timeout indicated by #CY_BLE_EVT_TIMEOUT event, the state is
*  changed to #CY_BLE_STATE_ON.
*
* \return
* \ref cy_en_ble_state_t : The current PSoC 6 BLE Middleware state. The following are
*                         possible states:
*
*   State                      | Description
*   -------------------------- | -----------
*   CY_BLE_STATE_STOPPED       | BLE is turned off.
*   CY_BLE_STATE_INITIALIZING  | BLE is initializing (waiting for #CY_BLE_EVT_STACK_ON event).
*   CY_BLE_STATE_ON            | BLE is turned on.
*   CY_BLE_STATE_CONNECTING    | BLE is connecting.

******************************************************************************/
__STATIC_INLINE cy_en_ble_state_t Cy_BLE_GetState(void)
{
    return(cy_ble_state);
}


/** \cond IGNORE */
/******************************************************************************
* Function Name: Cy_BLE_SetState
***************************************************************************//**
*
*  Used to set the PSoC 6 BLE Middleware state machine's state.
*
*  \param state: The desired state to which the event handler's
*   state machine should be set. The parameter state is a variable
*   of type cy_en_ble_state_t.
*
* \return
*  None
*
******************************************************************************/
__STATIC_INLINE void Cy_BLE_SetState(cy_en_ble_state_t state)
{
    cy_ble_state = state;
}
/** \endcond */


/******************************************************************************
* Function Name: Cy_BLE_GetAdvertisementState
***************************************************************************//**
*
*  This function returns the state of the link layer hardware advertisement
*  engine.
*
*  \image html ble_advertisement_state_machine.png
*
*  When Cy_BLE_GAPP_StartAdvertisement() is called, the state is set to
*  #CY_BLE_ADV_STATE_ADV_INITIATED. It automatically changes to
*  #CY_BLE_ADV_STATE_ADVERTISING when #CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP
*  event is received. When Cy_BLE_GAPP_StopAdvertisement() is called, the
*  state is set to #CY_BLE_ADV_STATE_STOP_INITIATED. It automatically changes
*  to #CY_BLE_ADV_STATE_STOPPED when the #CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP
*  event is received.
*
* \return
* \ref cy_en_ble_adv_state_t :The current advertising state. The following are
*                              possible states.
*
*               State                |         Description
*   -------------------------------- | -----------------------------
*   CY_BLE_ADV_STATE_STOPPED         | Advertising is stopped.
*   CY_BLE_ADV_STATE_ADV_INITIATED   | Advertising is initiated.
*   CY_BLE_ADV_STATE_ADVERTISING     | Advertising is initiated.
*   CY_BLE_ADV_STATE_STOP_INITIATED  | Stop advertising is initiated.
*
******************************************************************************/
__STATIC_INLINE cy_en_ble_adv_state_t Cy_BLE_GetAdvertisementState(void)
{
    return(cy_ble_advState);
}


/** \cond IGNORE */
/******************************************************************************
* Function Name: Cy_BLE_SetAdvertisementState
***************************************************************************//**
*
*  Used to set the state of the link layer hardware advertisement engine.
*
*  \param state: The desired state.
*
* \return
*  None
*
******************************************************************************/
__STATIC_INLINE void Cy_BLE_SetAdvertisementState(cy_en_ble_adv_state_t state)
{
    cy_ble_advState = state;
}
/** \endcond */


/******************************************************************************
* Function Name: Cy_BLE_GetScanState
***************************************************************************//**
*
*  This returns the state of the link layer hardware scan engine.
*
*  \image html ble_scanning_state_machine.png
*
*  When Cy_BLE_GAPC_StartScan() is called the state is set to
*  #CY_BLE_SCAN_STATE_SCAN_INITIATED. It automatically changes to
*  #CY_BLE_SCAN_STATE_SCANNING when the #CY_BLE_EVT_GAPC_SCAN_START_STOP event is
*  received.
*  When Cy_BLE_GAPC_StopScan() is called, the state is set to
*  #CY_BLE_SCAN_STATE_STOP_INITIATED. It automatically changes to
*  #CY_BLE_SCAN_STATE_STOPPED when the #CY_BLE_EVT_GAPC_SCAN_START_STOP event is
*  received.
*
*  \return
*  \ref cy_en_ble_scan_state_t : The current scan state. The following are
*                                possible states.
*
*               State                |         Description
*   -------------------------------- | -----------------------------
*   CY_BLE_SCAN_STATE_STOPPED        | Scanning is stopped.
*   CY_BLE_SCAN_STATE_SCAN_INITIATED | Scanning is initiated.
*   CY_BLE_SCAN_STATE_SCANNING       | Scanning process.
*   CY_BLE_SCAN_STATE_STOP_INITIATED | Stop scanning is initiated.
*
******************************************************************************/
__STATIC_INLINE cy_en_ble_scan_state_t Cy_BLE_GetScanState(void)
{
    return(cy_ble_scanState);
}


/** \cond IGNORE */
/******************************************************************************
* Function Name: Cy_BLE_SetScanState
***************************************************************************//**
*
*  Used to set the state of the link layer hardware scan engine.
*
*  \param state: The desired state.
*
*  \return
*  None
*
******************************************************************************/
__STATIC_INLINE void Cy_BLE_SetScanState(cy_en_ble_scan_state_t state)
{
    cy_ble_scanState = state;
}
/** \endcond */


/******************************************************************************
* Function Name: Cy_BLE_GetConnectionState
***************************************************************************//**
*  This function returns the state of the BLE link for the specified connection
*  handle.
*
*  The state is set to #CY_BLE_CONN_STATE_CONNECTED when #CY_BLE_EVT_GATT_CONNECT_IND
*  event is received with the corresponding connection handle.
*  The state is set to #CY_BLE_CONN_STATE_DISCONNECTED when the
*  #CY_BLE_EVT_GATT_DISCONNECT_IND event is received.
*
*  For GATT Client role when Cy_BLE_GATTC_StartDiscovery() is called, the state
*  indicates the current flow of the discovery procedure by <br>
*  #CY_BLE_CONN_STATE_CLIENT_SRVC_DISCOVERING,
*  #CY_BLE_CONN_STATE_CLIENT_INCL_DISCOVERING,
*  #CY_BLE_CONN_STATE_CLIENT_CHAR_DISCOVERING,
*  #CY_BLE_CONN_STATE_CLIENT_DESCR_DISCOVERING and changes to
*  #CY_BLE_CONN_STATE_CLIENT_DISCOVERED when the procedure successfully completes.
*
*  The state changes from #CY_BLE_CONN_STATE_CLIENT_DISCOVERED to
*  #CY_BLE_CONN_STATE_CLIENT_DISCONNECTED_DISCOVERED when
*  the #CY_BLE_EVT_GATT_DISCONNECT_IND event is received.
*  The state comes backs to #CY_BLE_CONN_STATE_CLIENT_DISCOVERED when
*  #CY_BLE_EVT_GATT_CONNECT_IND event is received and resolvable device
*  address is not changed.
*
*  \param connHandle: The connection handle.
*
* \return
* \ref cy_en_ble_conn_state_t : The current client state. The following are
*                               possible states.
*
*               State                                |         Description
*   ------------------------------------------------ | -----------------------------
*   CY_BLE_CONN_STATE_DISCONNECTED                   | Essentially idle state
*   CY_BLE_CONN_STATE_CLIENT_DISCONNECTED_DISCOVERED | Server is disconnected but discovered.
*   CY_BLE_CONN_STATE_CONNECTED                      | Peer device is connected for this and following states.
*   CY_BLE_CONN_STATE_CLIENT_SRVC_DISCOVERING        | Server services are being discovered.
*   CY_BLE_CONN_STATE_CLIENT_INCL_DISCOVERING        | Server included services are being discovered
*   CY_BLE_CONN_STATE_CLIENT_CHAR_DISCOVERING        | Server Characteristics are being discovered
*   CY_BLE_CONN_STATE_CLIENT_DESCR_DISCOVERING       | Server char. descriptors are being discovered
*   CY_BLE_CONN_STATE_CLIENT_DISCOVERED              | Server is discovered
*
*  \funcusage 
*  \snippet ble/snippet/main.c BLE Common API: Cy_BLE_GetConnectionState()
*
******************************************************************************/
__STATIC_INLINE cy_en_ble_conn_state_t Cy_BLE_GetConnectionState(cy_stc_ble_conn_handle_t connHandle)
{
    return((connHandle.attId < CY_BLE_MAX_SUPPORTED_CONN_COUNT) ? cy_ble_connState[connHandle.attId] :
                                                                  CY_BLE_CONN_STATE_DISCONNECTED);
}

/** \cond IGNORE */
/******************************************************************************
* Function Name: Cy_BLE_SetConnectionState
***************************************************************************//**
*
*  Used to set BLE link state for the specified connection handle.
*
*  \param connHandle: Peer Device Handle
*  \param state:  The state that is desired to be set
*
******************************************************************************/
__STATIC_INLINE void Cy_BLE_SetConnectionState(cy_stc_ble_conn_handle_t connHandle,
                                               cy_en_ble_conn_state_t state)
{
    if(connHandle.attId < CY_BLE_MAX_SUPPORTED_CONN_COUNT)
    {
        cy_ble_connState[connHandle.attId] = state;
    }
    else
    {
        /* Halt in debug mode*/
        CY_ASSERT(connHandle.attId >= CY_BLE_MAX_SUPPORTED_CONN_COUNT);
    }
}


/******************************************************************************
* Function Name: Cy_BLE_GetConnHandleIdx
***************************************************************************//**
*
*  This function searches the array of connection handles (cy_ble_connHandle[])
*  for the location (index) of a handle that corresponds to the input parameter
*  connHandle. bdHandle.
*
*  This function is designated for internal usage.
*
*  \param connHandle: The connection handle.
*
*  \return
*  retValue: the index of connHandle
*
******************************************************************************/
__STATIC_INLINE uint8_t Cy_BLE_GetConnHandleIdx(cy_stc_ble_conn_handle_t connHandle)
{
    uint8_t idx;
    uint8_t retValue = CY_BLE_MAX_SUPPORTED_CONN_COUNT;

    for(idx = 0u; idx < CY_BLE_MAX_SUPPORTED_CONN_COUNT; idx++)
    {
        if(cy_ble_connHandle[idx].bdHandle == connHandle.bdHandle)
        {
            retValue = idx;
            break;
        }
    }
    if(retValue == CY_BLE_MAX_SUPPORTED_CONN_COUNT)
    {
        /* HALT in debug mode */
        CY_ASSERT(retValue != CY_BLE_MAX_SUPPORTED_CONN_COUNT);
        retValue = 0u;
    }

    return(retValue);
}
/** \endcond */

/******************************************************************************
* Function Name: Cy_BLE_GetFlashWritePendingStatus
***************************************************************************//**
*
*  This function returns the flash Write pending status.
*
*  If this function returns a non-zero value, the application calls
*  Cy_BLE_StoreBondingData() to store pending bonding data.
*  Cy_BLE_StoreBondingData() automatically clears pending bits after the Write
*  operation completes.
*
* \return
* \ref cy_ble_pendingFlashWrite : The flash Write pending status.
*                                 Possible flags are set after:
*
*   <table>
*    <tr>
*      <th>Flags</th>
*      <th>Description</th>
*    </tr>
*    <tr>
*      <td>CY_BLE_PENDING_STACK_FLASH_WRITE_BIT</td>
*      <td>the CY_BLE_EVT_PENDING_FLASH_WRITE event.</td>
*    </tr>
*    <tr>
*      <td>CY_BLE_PENDING_CCCD_FLASH_WRITE_BIT</td>
*      <td>a Write to the CCCD event when a peer device supports bonding.
*      </td>
*    </tr>
*    <tr>
*      <td>CY_BLE_PENDING_CCCD_FLASH_CLEAR_BIT</td>
*      <td>a call to Cy_BLE_GAP_RemoveBondedDevice() with a request to
*          clear CCCD for a particular device.
*      </td>
*    </tr>
*    <tr>
*      <td>CY_BLE_PENDING_CCCD_FLASH_CLEAR_ALL_BIT</td>
*      <td>a call to Cy_BLE_GAP_RemoveBondedDevice() with a request to
*          clear CCCD for all devices.
*      </td>
*    </tr>
*   </table>
*
******************************************************************************/
__STATIC_INLINE uint8_t Cy_BLE_GetFlashWritePendingStatus(void)
{
    return(cy_ble_pendingFlashWrite);
}
/** \} group_ble_common_api_functions */


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_GAP_H */

/* [] END OF FILE */
