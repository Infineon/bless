/***************************************************************************//**
* \file cy_ble_bas.h
* \version 3.20
*
* \brief
*  Contains the function prototypes and constants for the Battery service.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_BAS_H
#define CY_BLE_BAS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* The C binding of definitions if building with the C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

/* Battery Level Characteristic length */
#define CY_BLE_BAS_BATTERY_LEVEL_LEN          (0x01u)

/* Maximum Battery Level value */
#define CY_BLE_BAS_MAX_BATTERY_LEVEL_VALUE    (100u)

/* The maximum supported Battery Services */
#define CY_BLE_BASS_SERVICE_COUNT             (CY_BLE_CONFIG_BASS_SERVICE_COUNT)

/* The maximum supported Battery Services */
#define CY_BLE_BASC_SERVICE_COUNT             (CY_BLE_CONFIG_BASC_SERVICE_COUNT)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_BAS_definitions
 * \{
 */
/** BAS Characteristic indexes */
typedef enum
{
    CY_BLE_BAS_BATTERY_LEVEL,                            /**< Battery Level Characteristic index */
    CY_BLE_BAS_CHAR_COUNT                                /**< Total Count of Characteristics */
} cy_en_ble_bas_char_index_t;

/** BAS Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_BAS_BATTERY_LEVEL_CCCD,                       /**< Client Characteristic Configuration Descriptor index */
    CY_BLE_BAS_BATTERY_LEVEL_CPFD,                       /**< Characteristic Presentation Format Descriptor index */
    CY_BLE_BAS_DESCR_COUNT                               /**< Total Count of Descriptors */
} cy_en_ble_bas_descr_index_t;

/** Structure with Battery Service attribute handles */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t serviceHandle;          /**< Battery Service Handle */
    cy_ble_gatt_db_attr_handle_t batteryLevelHandle;     /**< Battery Level Characteristic handle */
    cy_ble_gatt_db_attr_handle_t cpfdHandle;             /**< Characteristic Presentation Format Descriptor handle */
    cy_ble_gatt_db_attr_handle_t cccdHandle;             /**< Client Characteristic Configuration Descriptor handle */
} cy_stc_ble_bass_t;

/** Structure with Discovered Attributes Information of Battery Service */
typedef struct
{
    cy_stc_ble_conn_handle_t     connHandle;             /**< Peer Device Handle */
    cy_stc_ble_srvr_char_info_t  batteryLevel;           /**< Battery Level Characteristic info */
    cy_ble_gatt_db_attr_handle_t cpfdHandle;             /**< Characteristic Presentation Format Descriptor handle */
    cy_ble_gatt_db_attr_handle_t cccdHandle;             /**< Client Characteristic Configuration Descriptor handle */
    cy_ble_gatt_db_attr_handle_t rrdHandle;              /**< Report Reference Descriptor handle */
    
} cy_stc_ble_basc_t;

/** Battery Service Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;               /**< Peer Device Handle */
    uint8_t                    serviceIndex;             /**< Service instance */
    cy_en_ble_bas_char_index_t charIndex;                /**< Index of a service Characteristic */
    cy_stc_ble_gatt_value_t    *value;                   /**< Characteristic Value */
} cy_stc_ble_bas_char_value_t;

/** Battery Service Characteristic descriptor Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;              /**< Peer Device Handle */
    uint8_t                     serviceIndex;            /**< Service instance */
    cy_en_ble_bas_char_index_t  charIndex;               /**< Index of Service Characteristic */
    cy_en_ble_bas_descr_index_t descrIndex;              /**< Index of Service Characteristic descriptor */
    cy_stc_ble_gatt_value_t     *value;                  /**< Descriptor value */
} cy_stc_ble_bas_descr_value_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_bass_t *attrInfo;

    /** The number supported Battery Services as server */
    uint8_t serviceCount;    
} cy_stc_ble_bass_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_basc_t *attrInfo;
    
    /** The number supported Battery Services as client */
    uint8_t serviceCount;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_basc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** \addtogroup group_ble_service_api_BAS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_BASS_Init(const cy_stc_ble_bass_config_t *config);
cy_en_ble_api_result_t Cy_BLE_BASC_Init(const cy_stc_ble_basc_config_t *config);
void Cy_BLE_BAS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_BAS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_BASS_SetCharacteristicValue(uint8_t serviceIndex, cy_en_ble_bas_char_index_t charIndex,
                                                          uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BASS_GetCharacteristicValue(uint8_t serviceIndex, cy_en_ble_bas_char_index_t charIndex,
                                                          uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BASS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle, uint8_t serviceIndex,
                                                               cy_en_ble_bas_char_index_t charIndex,
                                                               cy_en_ble_bas_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BASS_SendNotification(cy_stc_ble_conn_handle_t connHandle, uint8_t serviceIndex,
                                                    cy_en_ble_bas_char_index_t charIndex, uint8_t attrSize,
                                                    uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_BAS_client
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_BASC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle, uint8_t serviceIndex,
                                                          cy_en_ble_bas_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_BASC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle, uint8_t serviceIndex,
                                                               cy_en_ble_bas_char_index_t charIndex,
                                                               cy_en_ble_bas_descr_index_t descrIndex, uint8_t attrSize,
                                                               uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BASC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle, uint8_t serviceIndex,
                                                               cy_en_ble_bas_char_index_t charIndex,
                                                               cy_en_ble_bas_descr_index_t descrIndex);
/** \} */


cy_ble_gatt_db_attr_handle_t Cy_BLE_BASC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                           uint8_t serviceIndex, 
                                                                           cy_en_ble_bas_char_index_t charIndex, 
                                                                           cy_en_ble_bas_descr_index_t descrIndex);

cy_ble_gatt_db_attr_handle_t Cy_BLE_BASC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                      uint8_t serviceIndex,
                                                                      cy_en_ble_bas_char_index_t charIndex);


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_bass_config_t *cy_ble_bassConfigPtr;
extern const cy_stc_ble_basc_config_t *cy_ble_bascConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_BAS_H */

/* [] END OF FILE */
