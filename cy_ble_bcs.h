/***************************************************************************//**
* \file cy_ble_bcs.h
* \version 3.20
*
* \brief
*  Contains the function prototypes and constants for the Body Composition service.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_BCS_H
#define CY_BLE_BCS_H

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
 * \addtogroup group_ble_service_api_BCS_definitions
 * \{
 */
/** BCS Characteristic indexes */
typedef enum
{
    CY_BLE_BCS_BODY_COMPOSITION_FEATURE,         /**< Body Composition Feature Characteristic index */
    CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT,     /**< Body Composition Measurement Characteristic index */
    CY_BLE_BCS_CHAR_COUNT                        /**< Total Count of BCS Characteristics */
}cy_en_ble_bcs_char_index_t;

/** BCS Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_BCS_CCCD,                             /**< Client Characteristic Configuration Descriptor index */
    CY_BLE_BCS_DESCR_COUNT                       /**< Total Count of Descriptors */
}cy_en_ble_bcs_descr_index_t;

/** BCS Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;       /**< Peer Device Handle */
    cy_en_ble_bcs_char_index_t charIndex;        /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t    *value;           /**< Characteristic Value */
} cy_stc_ble_bcs_char_value_t;

/** BCS Characteristic descriptor value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;      /**< Peer Device Handle */
    cy_en_ble_bcs_char_index_t  charIndex;       /**< Index of Service Characteristic */
    cy_en_ble_bcs_descr_index_t descrIndex;      /**< Index of Descriptor */
    cy_stc_ble_gatt_value_t     *value;          /**< Characteristic Value */
} cy_stc_ble_bcs_descr_value_t;

/** Structure with Body Composition service attribute handles */
typedef struct
{
    /** Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t charHandle;
    /** Array of Descriptor handles */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_BCS_DESCR_COUNT];
} cy_stc_ble_bcss_char_t;

/** BCS Characteristics info structure Handles */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t serviceHandle;                   /**< Body Composition Service Handle */
    cy_stc_ble_bcss_char_t       charInfo[CY_BLE_BCS_CHAR_COUNT]; /**< Array of Characteristics and Descriptors Handles */
} cy_stc_ble_bcss_t;

/** BCS Client Characteristic Structure type */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t valueHandle;                /**< Handle of Characteristic Value */
    uint8_t                      properties;                 /**< Properties for Value Field */
    cy_ble_gatt_db_attr_handle_t endHandle;                  /**< End Handle of a Characteristic */
} cy_stc_ble_bcsc_char_t;

/** BCS Characteristics info structure */
typedef struct
{
    /** Body Composition Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Body Composition Service Characteristics info structure */
    cy_stc_ble_bcsc_char_t       charInfo[CY_BLE_BCS_CHAR_COUNT];

    /** Body Composition Measurement Client Characteristic Configuration handle */
    cy_ble_gatt_db_attr_handle_t bodyCompositionMeasurementCccdHandle;
} cy_stc_ble_bcsc_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_bcss_t *attrInfo;

} cy_stc_ble_bcss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_bcsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_bcsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** \addtogroup group_ble_service_api_BCS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_BCSS_Init(const cy_stc_ble_bcss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_BCSC_Init(const cy_stc_ble_bcsc_config_t *config);
void Cy_BLE_BCS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_BCS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_BCSS_SetCharacteristicValue(cy_en_ble_bcs_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BCSS_GetCharacteristicValue(cy_en_ble_bcs_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BCSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bcs_char_index_t charIndex,
                                                               cy_en_ble_bcs_descr_index_t descrIndex, uint8_t attrSize,
                                                               uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BCSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bcs_char_index_t charIndex,
                                                               cy_en_ble_bcs_descr_index_t descrIndex, uint8_t attrSize,
                                                               uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BCSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_bcs_char_index_t charIndex, uint8_t attrSize,
                                                  uint8_t *attrValue);
/** \} */


/**
 * \addtogroup group_ble_service_api_BCS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_BCSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_bcs_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_BCSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bcs_char_index_t charIndex,
                                                               cy_en_ble_bcs_descr_index_t
                                                               descrIndex, uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BCSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bcs_char_index_t charIndex,
                                                               cy_en_ble_bcs_descr_index_t
                                                               descrIndex);
/** \} */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_bcss_config_t *cy_ble_bcssConfigPtr;
extern const cy_stc_ble_bcsc_config_t *cy_ble_bcscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_BCS_H */


/* [] END OF FILE */
