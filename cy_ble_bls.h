/***************************************************************************//**
* \file cy_ble_bls.h
* \version 3.50
*
* \brief
*  This file contains the function prototypes and constants used in
*  the Blood Pressure Profile.
*
********************************************************************************
* \copyright
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_BLS_H
#define CY_BLE_BLS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* The C binding of definitions if building with the C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* Data Types
*******************************************************************************/
/**
 * \addtogroup group_ble_service_api_BLS_definitions
 * \{
 */

/** Service Characteristics indexes */
typedef enum
{
    CY_BLE_BLS_BPM,              /**< Blood Pressure Measurement Characteristic index */
    CY_BLE_BLS_ICP,              /**< Intermediate Cuff Pressure Context Characteristic index */
    CY_BLE_BLS_BPF,              /**< Blood Pressure Feature Characteristic index */
    CY_BLE_BLS_CHAR_COUNT        /**< Total Count of BLS Characteristics */
}cy_en_ble_bls_char_index_t;

/** Service Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_BLS_CCCD,             /**< Client Characteristic Configuration descriptor index */
    CY_BLE_BLS_DESCR_COUNT       /**< Total Count of BLS descriptors */
}cy_en_ble_bls_descr_index_t;

/** Characteristic with descriptors */
typedef struct
{
    /** Blood Pressure Service Characteristic's handle */
    cy_ble_gatt_db_attr_handle_t charHandle;

    /** Blood Pressure Service Characteristic descriptor's handle */
    cy_ble_gatt_db_attr_handle_t cccdHandle;
}cy_stc_ble_blss_char_t;

/** Structure with Blood Pressure Service attribute handles */
typedef struct
{
    /** Blood Pressure Service Handle*/
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Array of Blood Pressure Service Characteristics + Descriptors Handles */
    cy_stc_ble_blss_char_t       charInfo[CY_BLE_BLS_CHAR_COUNT];
} cy_stc_ble_blss_t;

/** Blood Pressure Client Server's Characteristic structure type */
typedef struct
{
    /** Properties for the value field */
    uint8_t                      properties;

    /** Handle of Server database attribute value entry */
    cy_ble_gatt_db_attr_handle_t valueHandle;

    /** Blood Pressure Client Characteristic config descriptor's handle */
    cy_ble_gatt_db_attr_handle_t cccdHandle;

    /** Characteristic's end handle */
    cy_ble_gatt_db_attr_handle_t endHandle;
}cy_stc_ble_blsc_char_t;

/** Structure with Discovered Attributes Information of Blood Pressure Service */
typedef struct
{
    /** Structure with Characteristic handles + properties of Blood Pressure Service */
    cy_stc_ble_blsc_char_t charInfo[CY_BLE_BLS_CHAR_COUNT];
}cy_stc_ble_blsc_t;

/** Blood Pressure Service Characteristic value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;                      /**< Peer Device Handle */
    cy_en_ble_bls_char_index_t charIndex;                       /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t    *value;                          /**< Characteristic Value */
} cy_stc_ble_bls_char_value_t;

/** Blood Pressure Service Characteristic descriptor value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;                     /**< Peer Device Handle */
    cy_en_ble_bls_char_index_t  charIndex;                      /**< Index of Service Characteristic */
    cy_en_ble_bls_descr_index_t descrIndex;                     /**< Index of Service Characteristic descriptor */
    cy_stc_ble_gatt_value_t     *value;                         /**< Descriptor value */
} cy_stc_ble_bls_descr_value_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_blss_t *attrInfo;

} cy_stc_ble_blss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_blsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_blsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** \addtogroup group_ble_service_api_BLS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_BLSS_Init(const cy_stc_ble_blss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_BLSC_Init(const cy_stc_ble_blsc_config_t *config);
void Cy_BLE_BLS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_BLS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_BLSS_SetCharacteristicValue(cy_en_ble_bls_char_index_t charIndex,
                                                          uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BLSS_GetCharacteristicValue(cy_en_ble_bls_char_index_t charIndex,
                                                          uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BLSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bls_char_index_t charIndex,
                                                               cy_en_ble_bls_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BLSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_bls_char_index_t charIndex,
                                                    uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BLSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_bls_char_index_t charIndex,
                                                  uint8_t attrSize, uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_BLS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_BLSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_bls_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_BLSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bls_char_index_t charIndex,
                                                               cy_en_ble_bls_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BLSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bls_char_index_t charIndex,
                                                               cy_en_ble_bls_descr_index_t descrIndex);
/** \} */

cy_ble_gatt_db_attr_handle_t Cy_BLE_BLSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_bls_char_index_t charIndex);

cy_ble_gatt_db_attr_handle_t Cy_BLE_BLSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_bls_char_index_t charIndex,
                                                                            cy_en_ble_bls_descr_index_t descrIndex);


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_blss_config_t *cy_ble_blssConfigPtr;
extern const cy_stc_ble_blsc_config_t *cy_ble_blscConfigPtr;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_BLS_H */

/* [] END OF FILE */
