/***************************************************************************//**
* \file cy_ble_ias.h
* \version 3.40
*
* \brief
*  This file contains the function prototypes and constants used in
*  the Immediate Alert service.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_IAS_H
#define CY_BLE_IAS_H


#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

#define CY_BLE_IAS_ALERT_LEVEL_SIZE    (1u)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_IAS_definitions
 * \{
 */

/** Immediate Alert Service Characteristic indexes */
typedef enum
{
    CY_BLE_IAS_ALERT_LEVEL,                      /**< Alert Level Characteristic index */
    CY_BLE_IAS_CHAR_COUNT                        /**< Total Count of Characteristics */
} cy_en_ble_ias_char_index_t;

/** Structure with Immediate Alert Service attribute handles */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t serviceHandle;           /**< Immediate Alert Service Handle*/
    cy_ble_gatt_db_attr_handle_t alertLevelCharHandle;    /**< Handle of Alert Level Characteristic */
} cy_stc_ble_iass_t;

/** Immediate Alert Service Characteristic Value parameters structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;       /**< Connection handle */
    cy_en_ble_ias_char_index_t charIndex;        /**< Characteristic index of Immediate Alert Service */
    cy_stc_ble_gatt_value_t    *value;           /**< Pointer to value of Immediate Alert Service Characteristic */
} cy_stc_ble_ias_char_value_t;

/** Structure with Discovered Attributes Information of Immediate Alert Service */
typedef struct
{
    /** Handle of Alert Level Characteristic of Immediate Alert Service */
    cy_stc_ble_srvr_char_info_t alertLevelChar;
} cy_stc_ble_iasc_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_iass_t *attrInfo;

} cy_stc_ble_iass_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_iasc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_iasc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_IAS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_IASS_Init(const cy_stc_ble_iass_config_t *config);
cy_en_ble_api_result_t Cy_BLE_IASC_Init(const cy_stc_ble_iasc_config_t *config);
void Cy_BLE_IAS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_IAS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_IASS_GetCharacteristicValue(cy_en_ble_ias_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_IAS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_IASC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_ias_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);
/** \} */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_iass_config_t *cy_ble_iassConfigPtr;
extern const cy_stc_ble_iasc_config_t *cy_ble_iascConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_IAS_H */

/* [] END OF FILE */
