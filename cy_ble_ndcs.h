/***************************************************************************//**
* \file cy_ble_ndcs.h
* \version 3.40
*
* \brief
*  Contains the function prototypes and constants for Next DST Change
*  service.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_NDCS_H
#define CY_BLE_NDCS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* Data Types
*******************************************************************************/
/**
 * \addtogroup group_ble_service_api_NDCS_definitions
 * \{
 */

/** Characteristic indexes */
typedef enum
{
    CY_BLE_NDCS_TIME_WITH_DST,                           /**< Time with DST Characteristic index */
    CY_BLE_NDCS_CHAR_COUNT                               /**< Total Count of NDCS Characteristics */
} cy_en_ble_ndcs_char_index_t;

/** Next DST Change Service Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;              /**< Peer Device Handle */
    cy_en_ble_ndcs_char_index_t charIndex;               /**< Index of Next DST Change Service Characteristic */
    cy_stc_ble_gatt_value_t     *value;                  /**< Characteristic Value */
} cy_stc_ble_ndcs_char_value_t;

/** Structure with Next DST Change Service Characteristic attribute handles */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t serviceHandle;          /**< Handle of the Next DST Change Service */
    cy_ble_gatt_db_attr_handle_t timeWithDst;            /**< Handle of the Time with DST Characteristic */
} cy_stc_ble_ndcss_t;

/** Structure with Discovered Attributes Information of Next DST Change Service */
typedef struct
{
    /** Characteristic handle and properties */
    cy_stc_ble_srvr_char_info_t charInfo[CY_BLE_NDCS_CHAR_COUNT];
} cy_stc_ble_ndcsc_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_ndcss_t *attrInfo;

} cy_stc_ble_ndcss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_ndcsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_ndcsc_config_t;

/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_NDCS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_NDCSS_Init(const cy_stc_ble_ndcss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_NDCSC_Init(const cy_stc_ble_ndcsc_config_t *config);
void Cy_BLE_NDCS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_NDCS_server
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_NDCSS_SetCharacteristicValue(cy_en_ble_ndcs_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_NDCSS_GetCharacteristicValue(cy_en_ble_ndcs_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);

/** \} */

/**
 * \addtogroup group_ble_service_api_NDCS_client
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_NDCSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_ndcs_char_index_t charIndex);

/** \} */

/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_ndcss_config_t *cy_ble_ndcssConfigPtr;
extern const cy_stc_ble_ndcsc_config_t *cy_ble_ndcscConfigPtr;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_NDCS_H */

/* [] END OF FILE */
