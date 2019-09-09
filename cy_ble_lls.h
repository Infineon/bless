/***************************************************************************//**
* \file cy_ble_lls.h
* \version 3.20
*
* \brief
*  This file contains the function prototypes and constants used in the Link
*  Loss service.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_LLS_H
#define CY_BLE_LLS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

#define CY_BLE_LLS_ALERT_LEVEL_SIZE    (1u)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_LLS_definitions
 * \{
 */
/** Link Loss Service Characteristic indexes */
typedef enum
{
    CY_BLE_LLS_ALERT_LEVEL,                            /**< Alert Level Characteristic index */
    CY_BLE_LLS_CHAR_COUNT                              /**< Total Count of Characteristics */
} cy_en_ble_lls_char_index_t;

/** Link Loss Service Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;             /**< Connection handle */
    cy_en_ble_lls_char_index_t charIndex;              /**< Characteristic index of Link Loss Service */
    cy_stc_ble_gatt_value_t    *value;                 /**< Pointer to value of Link Loss Service Characteristic */
} cy_stc_ble_lls_char_value_t;

/** Structure with Link Loss Service attribute handles */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t serviceHandle;        /**< Link Loss Service Handle*/
    cy_ble_gatt_db_attr_handle_t alertLevelCharHandle; /**< Handle of Alert Level Characteristic */
} cy_stc_ble_llss_t;

/** Structure with Discovered Attributes Information of Link Loss Service */
typedef struct
{
    cy_stc_ble_srvr_char_info_t alertLevelChar;         /**< Handle of Alert Level Characteristic of Link Loss Service */
} cy_stc_ble_llsc_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_llss_t *attrInfo;

} cy_stc_ble_llss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_llsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_llsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_LLS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_LLSS_Init(const cy_stc_ble_llss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_LLSC_Init(const cy_stc_ble_llsc_config_t *config);
void Cy_BLE_LLS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */


/**
 * \addtogroup group_ble_service_api_LLS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_LLSS_GetCharacteristicValue(cy_en_ble_lls_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);
/** \} */


/**
 * \addtogroup group_ble_service_api_LLS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_LLSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_lls_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_LLSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_lls_char_index_t charIndex);
/** \} */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_llss_config_t *cy_ble_llssConfigPtr;
extern const cy_stc_ble_llsc_config_t *cy_ble_llscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_LLS_H */

/* [] END OF FILE */
