/***************************************************************************//**
* \file cy_ble_bms.h
* \version 3.50
*
* \brief
*  This file contains the function prototypes and constants used in the
*  Bond Management service.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_BMS_H
#define CY_BLE_BMS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* The C binding of definitions if building with the C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

/** Bond Management Control Point Characteristic supports Op Codes */
#define CY_BLE_BMS_BMCP_OPC_RD    (0x03u) /* Delete bond of requesting device (LE transport only). */
#define CY_BLE_BMS_BMCP_OPC_AB    (0x06u) /* Delete all bonds on server (LE transport only). */
#define CY_BLE_BMS_BMCP_OPC_BA    (0x09u) /* Delete all but the active bond on server (LE transport only). */

/** Bond Management Control Point Characteristic supports Op Codes */
#define CY_BLE_BMS_BMFT_RD        (0x00000010u) /** Delete bond  of current connection (LE transport only) supported. */
#define CY_BLE_BMS_BMFT_RC        (0x00000020u) /** Authorization Code required for feature above. */
#define CY_BLE_BMS_BMFT_AB        (0x00000400u) /** Remove all bonds on server (LE transport only) supported. */
#define CY_BLE_BMS_BMFT_AC        (0x00000800u) /** Authorization Code required for feature above. */
#define CY_BLE_BMS_BMFT_BA        (0x00010000u) /** Remove all but the active bond on server (LE transport only) supported. */
#define CY_BLE_BMS_BMFT_BC        (0x00020000u) /** Authorization Code required for feature above. */

#define CY_BLE_BMS_FLAG_LW        (0x01u)       /** Long Write Procedure */


/*******************************************************************************
* Data Types
*******************************************************************************/
/**
 * \addtogroup group_ble_service_api_BMS_definitions
 * \{
 */
/** Service Characteristics indexes */
typedef enum
{
    CY_BLE_BMS_BMCP,            /**< Bond Management Control Point Characteristic index */
    CY_BLE_BMS_BMFT,            /**< Bond Management Feature Characteristic index */
    CY_BLE_BMS_CHAR_COUNT       /**< Total Count of BMS Characteristics */
}cy_en_ble_bms_char_index_t;

/** Service Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_BMS_CEPD,            /**< Characteristic Extended Properties descriptor index */
    CY_BLE_BMS_DESCR_COUNT      /**< Total Count of BMS descriptors */
}cy_en_ble_bms_descr_index_t;

/** Characteristic with descriptors type */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t charHandle;                             /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_BMS_DESCR_COUNT];    /**< Handles of Descriptors */
} cy_stc_ble_bmss_char_t;

/** Structure with Service attribute handles */
typedef struct
{
    /** Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Service Characteristics info array */
    cy_stc_ble_bmss_char_t       charInfo[CY_BLE_BMS_CHAR_COUNT];
} cy_stc_ble_bmss_t;

/** Client Characteristic structure type */
typedef struct
{
    /** Properties for the value field */
    uint8_t                      properties;

    /** Handle of Server database attribute value entry */
    cy_ble_gatt_db_attr_handle_t valueHandle;

    /** Characteristics Descriptors Handles */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_BMS_DESCR_COUNT];

    /** Characteristic End Handle */
    cy_ble_gatt_db_attr_handle_t endHandle;
}cy_stc_ble_bmsc_char_t;

/** Service structure type */
typedef struct
{
    /** Characteristics handle + properties array */
    cy_stc_ble_bmsc_char_t charInfo[CY_BLE_BMS_CHAR_COUNT];
}cy_stc_ble_bmsc_t;

/** Service Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;         /**< Peer Device Handle */
    cy_en_ble_bms_char_index_t charIndex;          /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t    *value;             /**< Characteristic Value */
    cy_en_ble_gatt_err_code_t  gattErrorCode;      /**< GATT error code for checking the authorization code */
} cy_stc_ble_bms_char_value_t;

/** Service Characteristic descriptor Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;        /**< Peer Device Handle */
    cy_en_ble_bms_char_index_t  charIndex;         /**< Index of Service Characteristic */
    cy_en_ble_bms_descr_index_t descrIndex;        /**< Index of Service Characteristic descriptor */
    cy_stc_ble_gatt_value_t     *value;            /**< Descriptor value */
} cy_stc_ble_bms_descr_value_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_bmss_t *attrInfo;

} cy_stc_ble_bmss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_bmsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_bmsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** \addtogroup group_ble_service_api_BMS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_BMSS_Init(const cy_stc_ble_bmss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_BMSC_Init(const cy_stc_ble_bmsc_config_t *config);
void Cy_BLE_BMS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_BMS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_BMSS_SetCharacteristicValue(cy_en_ble_bms_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BMSS_GetCharacteristicValue(cy_en_ble_bms_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BMSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bms_char_index_t charIndex,
                                                               cy_en_ble_bms_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BMSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bms_char_index_t charIndex,
                                                               cy_en_ble_bms_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_BMS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_BMSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_bms_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_BMSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_bms_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BMSC_ReliableWriteCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                                    cy_en_ble_bms_char_index_t charIndex,
                                                                    uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_BMSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bms_char_index_t charIndex,
                                                               cy_en_ble_bms_descr_index_t
                                                               descrIndex);
/** \} */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_bmss_config_t *cy_ble_bmssConfigPtr;
extern const cy_stc_ble_bmsc_config_t *cy_ble_bmscConfigPtr;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_BMS_H */

/* [] END OF FILE */
