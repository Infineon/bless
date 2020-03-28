/***************************************************************************//**
* \file cy_ble_wss.h
* \version 3.40
*
* \brief
*  Contains the function prototypes and constants for Weight Scale service.
*
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_WSS_H
#define CY_BLE_WSS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

#define CY_BLE_WSS_UNKNOWN_USER    (0xFFu)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_WSS_definitions
 * \{
 */

/** WSS Characteristic indexes */
typedef enum
{
    CY_BLE_WSS_WEIGHT_SCALE_FEATURE,              /**< Weight Scale Feature Characteristic index */
    CY_BLE_WSS_WEIGHT_MEASUREMENT,                /**< Weight Measurement Characteristic index */
    CY_BLE_WSS_CHAR_COUNT                         /**< Total Count of WSS Characteristics */
}cy_en_ble_wss_char_index_t;

/** WSS Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_WSS_CCCD,                              /**< Client Characteristic Configuration Descriptor index */
    CY_BLE_WSS_DESCR_COUNT                        /**< Total Count of Descriptors */
}cy_en_ble_wss_descr_index_t;

/** WSS Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;        /**< Peer Device Handle */
    cy_en_ble_wss_char_index_t charIndex;         /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t    *value;            /**< Characteristic Value */
} cy_stc_ble_wss_char_value_t;

/** WSS Characteristic descriptor value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;       /**< Peer Device Handle */
    cy_en_ble_wss_char_index_t  charIndex;        /**< Index of Service Characteristic */
    cy_en_ble_wss_descr_index_t descrIndex;       /**< Index of Descriptor */
    cy_stc_ble_gatt_value_t     *value;           /**< Characteristic Value */
} cy_stc_ble_wss_descr_value_t;

/** Structure with Weight Scale Service Attribute Handles */
typedef struct
{
    /** Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t charHandle;

    /** Array of Descriptor Handles */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_WSS_DESCR_COUNT];
} cy_stc_ble_wsss_char_t;

/** WSS Characteristic with Descriptors Handles */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t serviceHandle;                   /**< Weight Scale Service Handle */
    cy_stc_ble_wsss_char_t       charInfo[CY_BLE_WSS_CHAR_COUNT]; /**< Array of Characteristics and Descriptors Handles */
} cy_stc_ble_wsss_t;

/** WSS Service Full Characteristic information structure */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t valueHandle;                         /**< Handle of Characteristic Value */
    uint8_t                      properties;                          /**< Properties for Value Field */
    cy_ble_gatt_db_attr_handle_t endHandle;                           /**< End Handle of Characteristic */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_WSS_DESCR_COUNT]; /**< Array of Descriptor Handles */
} cy_stc_ble_wssc_char_t;

/** Structure with Discovered Attributes Information of Weight Scale Service */
typedef struct
{
    /** Weight Scale Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Weight Scale Service Characteristics info structure */
    cy_stc_ble_wssc_char_t       charInfo[CY_BLE_WSS_CHAR_COUNT];
} cy_stc_ble_wssc_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_wsss_t *attrInfo;

} cy_stc_ble_wsss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_wssc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_wssc_config_t;

/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/


/** \addtogroup group_ble_service_api_WSS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_WSSS_Init(const cy_stc_ble_wsss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_WSSC_Init(const cy_stc_ble_wssc_config_t *config);
void Cy_BLE_WSS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_WSS_server
 * \{
 */
uint8_t Cy_BLE_WSS_GetAdUserIdListSize(uint8_t advertisingParamIndex);

cy_en_ble_api_result_t Cy_BLE_WSS_SetAdUserId(uint8_t listSize, const uint8_t userIdList[],
                                              uint8_t advertisingParamIndex);


cy_en_ble_api_result_t Cy_BLE_WSSS_SetCharacteristicValue(cy_en_ble_wss_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_WSSS_GetCharacteristicValue(cy_en_ble_wss_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_WSSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_wss_char_index_t charIndex,
                                                               cy_en_ble_wss_descr_index_t descrIndex, uint8_t attrSize,
                                                               uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_WSSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_wss_char_index_t charIndex,
                                                               cy_en_ble_wss_descr_index_t descrIndex, uint8_t attrSize,
                                                               uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_WSSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_wss_char_index_t charIndex, uint8_t attrSize,
                                                  uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_WSS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_WSSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_wss_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_WSSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_wss_char_index_t charIndex,
                                                               cy_en_ble_wss_descr_index_t
                                                               descrIndex, uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_WSSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_wss_char_index_t charIndex,
                                                               cy_en_ble_wss_descr_index_t
                                                               descrIndex);
/** \} */



/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_wsss_config_t *cy_ble_wsssConfigPtr;
extern const cy_stc_ble_wssc_config_t *cy_ble_wsscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_WSS_H */

/* [] END OF FILE */
