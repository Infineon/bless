/***************************************************************************//**
* \file cy_ble_tps.h
* \version 3.50
*
* \brief
*  This file contains the function prototypes and constants used in the Tx
*  Power service.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_TPS_H
#define CY_BLE_TPS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

/* Maximum supported Tx Power Services */
#define CY_BLE_TPS_SERVICE_COUNT          (1u)
#define CY_BLE_TPS_TX_POWER_LEVEL_SIZE    (1u)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_TPS_definitions
 * \{
 */
/** TPS Characteristic indexes */
typedef enum
{
    CY_BLE_TPS_TX_POWER_LEVEL,                      /**< Tx Power Level Characteristic index */
    CY_BLE_TPS_CHAR_COUNT                           /**< Total Count of Characteristics */
} cy_en_ble_tps_char_index_t;

/** TPS Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_TPS_CCCD,                                /**< Tx Power Level Client Characteristic configuration descriptor index */
    CY_BLE_TPS_DESCR_COUNT                          /**< Total Count of Tx Power Service Characteristic descriptors */
} cy_en_ble_tps_char_descriptors_t;

/** Tx Power Service Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;          /**< Connection handle */
    cy_en_ble_tps_char_index_t charIndex;           /**< Characteristic index of Tx Power Service */
    cy_stc_ble_gatt_value_t    *value;              /**< Pointer to value of Tx Power Service Characteristic */
} cy_stc_ble_tps_char_value_t;

/** Tx Power Service Characteristic descriptor Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t         connHandle;     /**< Connection handle */
    cy_en_ble_tps_char_index_t       charIndex;      /**< Characteristic index of Tx Power Service */
    cy_en_ble_tps_char_descriptors_t descrIndex;     /**< Characteristic index Descriptor of Tx Power Service */
    cy_stc_ble_gatt_value_t          *value;         /**< Pointer to value of Tx Power Service Characteristic */
} cy_stc_ble_tps_descr_value_t;

/** Structure with Tx Power Service attribute handles */
typedef struct
{
    /** Tx Power Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Tx Power Level Characteristic handle */
    cy_ble_gatt_db_attr_handle_t txPowerLevelCharHandle;

    /** Tx Power Level Client Characteristic Configuration Descriptor handle */
    cy_ble_gatt_db_attr_handle_t txPowerLevelCccdHandle;
} cy_stc_ble_tpss_t;

/** Structure with Discovered Attributes Information of Tx Power Service */
typedef struct
{
    /** Tx Power Level Characteristic handle */
    cy_stc_ble_srvr_char_info_t  txPowerLevelChar;

    /** Tx Power Level Client Characteristic Configuration Descriptor handle */
    cy_ble_gatt_db_attr_handle_t txPowerLevelCccdHandle;
} cy_stc_ble_tpsc_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_tpss_t *attrInfo;

} cy_stc_ble_tpss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_tpsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_tpsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** \addtogroup group_ble_service_api_TPS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_TPSS_Init(const cy_stc_ble_tpss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_TPSC_Init(const cy_stc_ble_tpsc_config_t *config);
void Cy_BLE_TPS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);

/** \} */


/**
 * \addtogroup group_ble_service_api_TPS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_TPSS_SetCharacteristicValue(cy_en_ble_tps_char_index_t charIndex, uint8_t attrSize,
                                                          int8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_TPSS_GetCharacteristicValue(cy_en_ble_tps_char_index_t charIndex, uint8_t attrSize,
                                                          int8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_TPSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_tps_char_index_t charIndex,
                                                               cy_en_ble_tps_char_descriptors_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_TPSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_tps_char_index_t charIndex, uint8_t attrSize,
                                                    int8_t *attrValue);

/** \} */

/**
 * \addtogroup group_ble_service_api_TPS_client
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_TPSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_tps_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_TPSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_tps_char_index_t charIndex,
                                                               cy_en_ble_tps_char_descriptors_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_TPSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_tps_char_index_t charIndex,
                                                               cy_en_ble_tps_char_descriptors_t descrIndex);

/** \} */


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

/** \cond IGNORE */
cy_ble_gatt_db_attr_handle_t Cy_BLE_TPSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_tps_char_index_t charIndex);

cy_ble_gatt_db_attr_handle_t Cy_BLE_TPSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_tps_char_index_t charIndex,
                                                                            cy_en_ble_tps_char_descriptors_t descrIndex);
/** \endcond */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_tpss_config_t *cy_ble_tpssConfigPtr;
extern const cy_stc_ble_tpsc_config_t *cy_ble_tpscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_TPS_H */

/* [] END OF FILE */
