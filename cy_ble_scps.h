/***************************************************************************//**
* \file cy_ble_scps.h
* \version 3.20
*
* \brief
*  Contains the function prototypes and constants for the Scan Parameter service.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_SCPS_H
#define CY_BLE_SCPS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

#define CY_BLE_SCAN_REFRESH_ENABLED        (0x00u)
#define CY_BLE_SCAN_REFRESH_RESERVED       (0xFFu)

#define CY_BLE_REFRESH_CHAR_LEN            (0x01u)
#define CY_BLE_INTERVAL_WINDOW_CHAR_LEN    (0x04u)

#define CY_BLE_SCAN_INTERVAL_WINDOW_MIN    (0x0004u)
#define CY_BLE_SCAN_INTERVAL_WINDOW_MAX    (0x4000u)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_SCPS_definitions
 * \{
 */
/** SCPS Characteristic indexes */
typedef enum
{
    CY_BLE_SCPS_SCAN_INT_WIN,                        /**< Scan Interval Window Characteristic index */
    CY_BLE_SCPS_SCAN_REFRESH,                        /**< Scan Refresh Characteristic index */
    CY_BLE_SCPS_CHAR_COUNT                           /**< Total Count of Characteristics */
} cy_en_ble_scps_char_index_t;

/** ScPS Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_SCPS_SCAN_REFRESH_CCCD,                   /**< Client Characteristic Configuration descriptor index */
    CY_BLE_SCPS_DESCR_COUNT                          /**< Total Count of Descriptors */
} cy_en_ble_scps_descr_index_t;

/** Structure with Scan Parameters Service attribute handles */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t serviceHandle;            /**< Scan Parameter Service Handle*/
    cy_ble_gatt_db_attr_handle_t intervalWindowCharHandle; /**< Handle of Scan Interval Window Characteristic */
    cy_ble_gatt_db_attr_handle_t refreshCharHandle;        /**< Handle of Scan Refresh Characteristic */
    cy_ble_gatt_db_attr_handle_t refreshCccdHandle;        /**< Handle of Client Characteristic Configuration Descriptor */
} cy_stc_ble_scpss_t;

/** Structure with Discovered Attributes Information of Scan Parameters Service */
typedef struct
{
    cy_stc_ble_conn_handle_t     connHandle;             /**< Peer Device Handle */
    cy_stc_ble_srvr_char_info_t  intervalWindowChar;     /**< Handle + properties of Scan Interval Window Characteristic */
    cy_stc_ble_srvr_char_info_t  refreshChar;            /**< Handle + properties of Scan Refresh Characteristic */
    cy_ble_gatt_db_attr_handle_t refreshCccdHandle;      /**< Handle of Client Characteristic Configuration Descriptor */
} cy_stc_ble_scpsc_t;

/** Scan Parameters Service Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;              /**< Peer Device Handle */
    cy_en_ble_scps_char_index_t charIndex;               /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t     *value;                  /**< Characteristic Value */
} cy_stc_ble_scps_char_value_t;

/** Scan Parameters Service Characteristic descriptor Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t     connHandle;             /**< Peer Device Handle */
    cy_en_ble_scps_char_index_t  charIndex;              /**< Index of Service Characteristic */
    cy_en_ble_scps_descr_index_t descrIndex;             /**< Index of Service Characteristic descriptor */
    cy_stc_ble_gatt_value_t      *value;                 /**< Descriptor value */
} cy_stc_ble_scps_descr_value_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_scpss_t *attrInfo;

} cy_stc_ble_scpss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_scpsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_scpsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** \addtogroup group_ble_service_api_SCPS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_SCPSS_Init(const cy_stc_ble_scpss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_SCPSC_Init(const cy_stc_ble_scpsc_config_t *config);
void Cy_BLE_SCPS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_SCPS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_SCPSS_SetCharacteristicValue(cy_en_ble_scps_char_index_t charIndex,
                                                           uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_SCPSS_GetCharacteristicValue(cy_en_ble_scps_char_index_t charIndex,
                                                           uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_SCPSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_scps_char_index_t charIndex,
                                                                cy_en_ble_scps_descr_index_t descrIndex,
                                                                uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_SCPSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                     cy_en_ble_scps_char_index_t charIndex, uint8_t attrSize,
                                                     uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_SCPS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_SCPSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_scps_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t * attrValue);

cy_en_ble_api_result_t Cy_BLE_SCPSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_scps_char_index_t charIndex,
                                                                cy_en_ble_scps_descr_index_t descrIndex,
                                                                uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_SCPSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_scps_char_index_t charIndex,
                                                                cy_en_ble_scps_descr_index_t descrIndex);
/** \} */

cy_ble_gatt_db_attr_handle_t Cy_BLE_SCPSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_scps_char_index_t charIndex);
                                                                       
cy_ble_gatt_db_attr_handle_t Cy_BLE_SCPSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_scps_char_index_t charIndex,
                                                                            cy_en_ble_scps_descr_index_t descrIndex);

/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_scpss_config_t *cy_ble_scpssConfigPtr;
extern const cy_stc_ble_scpsc_config_t *cy_ble_scpscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_SCPH_H */

/* [] END OF FILE */
