/***************************************************************************//**
* \file cy_ble_lns.h
* \version 3.40
*
* \brief
*  This file contains the function prototypes and constants used in
*  the Location and Navigation service.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_LNS_H
#define CY_BLE_LNS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

#define CY_BLE_LNS_FLAG_PROCESS    (0x01u)


/*******************************************************************************
* Data Types
*******************************************************************************/
/**
 * \addtogroup group_ble_service_api_LNS_definitions
 * \{
 */
/** LNS Service Characteristics indexes */
typedef enum
{
    CY_BLE_LNS_FT,           /**< Location and Navigation Feature Characteristic index */
    CY_BLE_LNS_LS,           /**< Location and Speed Characteristic index */
    CY_BLE_LNS_PQ,           /**< Position Quality Characteristic index */
    CY_BLE_LNS_CP,           /**< Location and Navigation Control Point Characteristic index */
    CY_BLE_LNS_NV,           /**< Navigation Characteristic index */
    CY_BLE_LNS_CHAR_COUNT    /**< Total Count of LNS Characteristics */
}cy_en_ble_lns_char_index_t;

/** LNS Service Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_LNS_CCCD,           /**< Client Characteristic Configuration descriptor index */
    CY_BLE_LNS_DESCR_COUNT     /**< Total Count of LNS descriptors */
}cy_en_ble_lns_descr_index_t;

/** Location and Navigation Server Characteristic structure type */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t charHandle;                           /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_LNS_DESCR_COUNT];  /**< Handle of Descriptor */
}cy_stc_ble_lnss_char_t;

/** Structure with Location and Navigation Service attribute handles */
typedef struct
{
    /** Location and Navigation Service Handle*/
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Location and Navigation Service Characteristics info array */
    cy_stc_ble_lnss_char_t       charInfo[CY_BLE_LNS_CHAR_COUNT];
} cy_stc_ble_lnss_t;

/** Location and Navigation Client Characteristic structure type */
typedef struct
{
    /** Properties for Value Field */
    uint8_t                      properties;

    /** Handle of server database attribute value entry */
    cy_ble_gatt_db_attr_handle_t valueHandle;

    /** Location and Navigation client char. descriptor handle */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_LNS_DESCR_COUNT];

    /** Characteristic End Handle */
    cy_ble_gatt_db_attr_handle_t endHandle;
}cy_stc_ble_lnsc_char_t;

/** Structure with Discovered Attributes Information of Location and Navigation Service */
typedef struct
{
    cy_stc_ble_lnsc_char_t charInfo[CY_BLE_LNS_CHAR_COUNT];   /**< Characteristics handle + properties array */
}cy_stc_ble_lnsc_t;

/** LNS Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;                      /**< Peer Device Handle */
    cy_en_ble_lns_char_index_t charIndex;                       /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t    *value;                          /**< Characteristic Value */
} cy_stc_ble_lns_char_value_t;

/** LNS Characteristic descriptor Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;                     /**< Peer Device Handle */
    cy_en_ble_lns_char_index_t  charIndex;                      /**< Index of Service Characteristic */
    cy_en_ble_lns_descr_index_t descrIndex;                     /**< Index of Service Characteristic descriptor */
    cy_stc_ble_gatt_value_t     *value;                         /**< Descriptor value */
} cy_stc_ble_lns_descr_value_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_lnss_t *attrInfo;

} cy_stc_ble_lnss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_lnsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_lnsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
/**
 * \addtogroup group_ble_service_api_LNS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_LNSS_Init(const cy_stc_ble_lnss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_LNSC_Init(const cy_stc_ble_lnsc_config_t *config);
void Cy_BLE_LNS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_LNS_server
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_LNSS_SetCharacteristicValue(cy_en_ble_lns_char_index_t charIndex,
                                                          uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_LNSS_GetCharacteristicValue(cy_en_ble_lns_char_index_t charIndex,
                                                          uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_LNSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_lns_char_index_t charIndex,
                                                               cy_en_ble_lns_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_LNSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_lns_char_index_t charIndex, uint8_t attrSize,
                                                    uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_LNSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_lns_char_index_t charIndex, uint8_t attrSize,
                                                  uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_LNS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_LNSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_lns_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_LNSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_lns_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_LNSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_lns_char_index_t charIndex,
                                                               cy_en_ble_lns_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_LNSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_lns_char_index_t charIndex,
                                                               cy_en_ble_lns_descr_index_t descrIndex);
/** \} */


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

/** \cond IGNORE */
cy_ble_gatt_db_attr_handle_t Cy_BLE_LNSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_lns_char_index_t charIndex);

cy_ble_gatt_db_attr_handle_t Cy_BLE_LNSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_lns_char_index_t charIndex,
                                                                            cy_en_ble_lns_descr_index_t descrIndex);
/** \endcond */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_lnss_config_t *cy_ble_lnssConfigPtr;
extern const cy_stc_ble_lnsc_config_t *cy_ble_lnscConfigPtr;
extern uint8_t cy_ble_lnsFlag;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_LNS_H */

/* [] END OF FILE */
