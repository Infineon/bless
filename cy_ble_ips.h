/***************************************************************************//**
* \file cy_ble_ips.h
* \version 3.60
*
* \brief
*  Contains the function prototypes and constants for the Indoor Positioning
*  service.
*
********************************************************************************
* \copyright
* Copyright 2017-2021, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_IPS_H
#define CY_BLE_IPS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

/* IPS specific GATT errors  */
#define CY_BLE_GATT_ERR_WRITE_REQ_REJECTED              (0x80u)

#define CY_BLE_IPS_AD_TYPE                              (0x25u)
#define CY_BLE_IPS_SERVICE_DATA_LENGTH                  (0x05u)

#define CY_BLE_IPS_CHARACTERISTICS_IN_ADVERTISING       (0x7Du)
#define CY_BLE_IPS_COORDINATES_IN_ADVERTISING           (0x01u)
#define CY_BLE_IPS_COORDINATES_NOT_PRESENT              (0u)
#define CY_BLE_IPS_COORDINATES_PRESENT                  (1u)
#define CY_BLE_IPS_TYPE_OF_COORDINATE_IN_ADVERTISING    (0x02u)
#define CY_BLE_IPS_WGS84_COORDINATE                     (0u)
#define CY_BLE_IPS_LOCAL_COORDINATE                     (1u)
#define CY_BLE_IPS_TX_POWER_IN_ADVERTISING              (0x04u)
#define CY_BLE_IPS_TX_POWER_NOT_PRESENT                 (0u)
#define CY_BLE_IPS_TX_POWER_PRESENT                     (1u)
#define CY_BLE_IPS_ALTITUDE_IN_ADVERTISING              (0x08u)
#define CY_BLE_IPS_ALTITUDE_NOT_PRESENT                 (0u)
#define CY_BLE_IPS_ALTITUDE_PRESENT                     (1u)
#define CY_BLE_IPS_FLOOR_NUMBER_IN_ADVERTISING          (0x10u)
#define CY_BLE_IPS_FLOOR_NUMBER_NOT_PRESENT             (0u)
#define CY_BLE_IPS_FLOOR_NUMBER_PRESENT                 (1u)
#define CY_BLE_IPS_UNCERTAINTY_NUMBER_IN_ADVERTISING    (0x20u)
#define CY_BLE_IPS_UNCERTAINTY_NUMBER_NOT_PRESENT       (0u)
#define CY_BLE_IPS_UNCERTAINTY_NUMBER_PRESENT           (1u)
#define CY_BLE_IPS_LOCATION_NAME_IN_GATT_DATABASE       (0x40u)
#define CY_BLE_IPS_LOCATION_NAME_NOT_PRESENT            (0u)
#define CY_BLE_IPS_LOCATION_NAME_PRESENT                (1u)


/*******************************************************************************
* Data Types
*******************************************************************************/
/**
 * \addtogroup group_ble_service_api_IPS_definitions
 * \{
 */
/** IPS Characteristic indexes */
typedef enum
{
    CY_BLE_IPS_INDOOR_POSITINING_CONFIG,         /**< Set of Characteristic values included in the Indoor Positioning Service AD type. */
    CY_BLE_IPS_LATITUDE,                         /**< WGS84 North coordinate of the device. */
    CY_BLE_IPS_LONGITUDE,                        /**< WGS84 East coordinate of the device. */
    CY_BLE_IPS_LOCAL_NORTH_COORDINATE,           /**< North coordinate of the device using local coordinate system. */
    CY_BLE_IPS_LOCAL_EAST_COORDINATE,            /**< East coordinate of the device using local coordinate system. */
    CY_BLE_IPS_FLOOR_NUMBER,                     /**< Describes in which floor the device is installed. */
    CY_BLE_IPS_ALTITUDE,                         /**< Altitude of the device. */
    CY_BLE_IPS_UNCERTAINTY,                      /**< Uncertainty of the location information the device exposes. */
    CY_BLE_IPS_LOCATION_NAME,                    /**< Name of the location where the device is installed. */
    CY_BLE_IPS_CHAR_COUNT                        /**< Total Count of IPS Characteristics */
}cy_en_ble_ips_char_index_t;

/** IPS Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_IPS_CEPD,                              /**< Characteristic Extended Properties descriptor index */
    CY_BLE_IPS_SCCD,                              /**< Server Characteristic Configuration Descriptor index */
    CY_BLE_IPS_DESCR_COUNT                        /**< Total Count of Descriptors */
}cy_en_ble_ips_descr_index_t;

/** IPS Characteristic with descriptors */
typedef struct
{
    /** Handles of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t charHandle;

    /** Array of Descriptor handles */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_IPS_DESCR_COUNT];
} cy_stc_ble_ipss_char_t;

/** Structure to hold pointer to cy_stc_ble_ipss_char_t */
typedef struct
{
    /** Pointer to cy_stc_ble_ipss_char_t that holds information about specific
     * IP Characteristic
     */
    cy_stc_ble_ipss_char_t *charInfoPtr;
} cy_stc_ble_ipss_char_info_ptr_t;

/** Structure with Indoor Positioning Service attribute handles */
typedef struct
{
    /** Indoor Positioning Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Indoor Positioning Service Array with pointers to
     * Characteristic handles.
     */
    cy_stc_ble_ipss_char_t charInfo[CY_BLE_IPS_CHAR_COUNT];
} cy_stc_ble_ipss_t;

/** IPS Characteristic with descriptors */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t valueHandle;                         /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t endHandle;                           /**< End handle of Characteristic */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_IPS_DESCR_COUNT]; /**< Array of Descriptor handles */
    uint8_t                      properties;                          /**< Properties for Value Field */
} cy_stc_ble_ipsc_char_t;

/** Structure to hold pointer to cy_stc_ble_ipsc_char_t */
typedef struct
{
    /** Pointer to cy_stc_ble_ipsc_char_t that holds information about specific IP Characteristic. */
    cy_stc_ble_ipsc_char_t *charInfoPtr;
} cy_stc_ble_ipsc_char_info_ptr_t;

/** Structure with Discovered Attributes Information of Indoor Positioning service. */
typedef struct
{
    /** Indoor Positioning Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Indoor Positioning Service Characteristics info array */
    cy_stc_ble_ipsc_char_t       charInfo[CY_BLE_IPS_CHAR_COUNT];
} cy_stc_ble_ipsc_t;


/** IPS Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;                      /**< Peer Device Handle */
    cy_en_ble_ips_char_index_t charIndex;                       /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t    *value;                          /**< Characteristic Value */
    cy_en_ble_gatt_err_code_t  gattErrorCode;                   /**< GATT error code for access control */
} cy_stc_ble_ips_char_value_t;

/** IPS Characteristic descriptor value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;                     /**< Peer Device Handle */
    cy_en_ble_ips_char_index_t  charIndex;                      /**< Index of Service Characteristic */
    cy_en_ble_ips_descr_index_t descrIndex;                     /**< Index of Descriptor */
    cy_en_ble_gatt_err_code_t   gattErrorCode;                  /**< Error code received from application (optional) */
    cy_stc_ble_gatt_value_t     *value;                         /**< Characteristic Value */
} cy_stc_ble_ips_descr_value_t;


/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_ipss_t *attrInfo;

} cy_stc_ble_ipss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_ipsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_ipsc_config_t;

/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** \addtogroup group_ble_service_api_IPS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_IPSS_Init(const cy_stc_ble_ipss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_IPSC_Init(const cy_stc_ble_ipsc_config_t *config);
void Cy_BLE_IPS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_IPS_server
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_IPSS_SetCharacteristicValue(cy_en_ble_ips_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_IPSS_GetCharacteristicValue(cy_en_ble_ips_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_IPSS_SetCharacteristicDescriptor(cy_en_ble_ips_char_index_t charIndex,
                                                               cy_en_ble_ips_descr_index_t descrIndex, uint8_t attrSize,
                                                               uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_IPSS_GetCharacteristicDescriptor(cy_en_ble_ips_char_index_t charIndex,
                                                               cy_en_ble_ips_descr_index_t descrIndex, uint8_t attrSize,
                                                               uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_IPS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_IPSC_SetCharacteristicValueWithoutResponse(cy_stc_ble_conn_handle_t connHandle,
                                                                         cy_en_ble_ips_char_index_t charIndex,
                                                                         uint8_t attrSize, uint8_t * attrValue);

cy_en_ble_api_result_t Cy_BLE_IPSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_ips_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_IPSC_ReliableWriteCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                                    cy_en_ble_ips_char_index_t charIndex,
                                                                    uint8_t attrSize, uint8_t * attrValue);

cy_en_ble_api_result_t Cy_BLE_IPSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_ips_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_IPSC_GetMultipleCharacteristicValues(cy_stc_ble_conn_handle_t connHandle,
                                                                   const cy_en_ble_ips_char_index_t *charIndexesList,
                                                                   uint8_t numberOfCharIndexes);

cy_en_ble_api_result_t Cy_BLE_IPSC_GetLongCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                              cy_en_ble_ips_char_index_t charIndex, uint16_t attrSize,
                                                              uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_IPSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_ips_char_index_t charIndex,
                                                               cy_en_ble_ips_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_IPSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_ips_char_index_t charIndex,
                                                               cy_en_ble_ips_descr_index_t descrIndex);
/** \} */


cy_ble_gatt_db_attr_handle_t Cy_BLE_IPSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_ips_char_index_t charIndex);

cy_ble_gatt_db_attr_handle_t Cy_BLE_IPSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_ips_char_index_t charIndex,
                                                                            cy_en_ble_ips_descr_index_t descrIndex);


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_ipss_config_t *cy_ble_ipssConfigPtr;
extern const cy_stc_ble_ipsc_config_t *cy_ble_ipscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_IPS_H */

/* [] END OF FILE */
