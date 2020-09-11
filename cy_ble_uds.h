/***************************************************************************//**
* \file cy_ble_uds.h
* \version 3.50
*
* \brief
*  This file contains the function prototypes and constants used in
*  the User Data service.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_UDS_H
#define CY_BLE_UDS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

#define CY_BLE_UDS_FLAG_PROCESS       (0x01u)
#define CY_BLE_UDS_LONG_CHAR_COUNT    (3u) /* Count of long UDS Characteristics */


/*******************************************************************************
* Data Types
*******************************************************************************/
/**
 * \addtogroup group_ble_service_api_UDS_definitions
 * \{
 */
/** UDS Service Characteristics indexes */
typedef enum
{
    CY_BLE_UDS_FNM,          /**< First Name Characteristic index */
    CY_BLE_UDS_LNM,          /**< Last Name Characteristic index */
    CY_BLE_UDS_EML,          /**< Email Address Characteristic index */
    CY_BLE_UDS_AGE,          /**< Age Characteristic index */
    CY_BLE_UDS_DOB,          /**< Date of Birth Characteristic index */
    CY_BLE_UDS_GND,          /**< Gender Characteristic index */
    CY_BLE_UDS_WGT,          /**< Weight Characteristic index */
    CY_BLE_UDS_HGT,          /**< Height Characteristic index */
    CY_BLE_UDS_VO2,          /**< VO2 Max Characteristic index */
    CY_BLE_UDS_HRM,          /**< Heart Rate Max Characteristic index */
    CY_BLE_UDS_RHR,          /**< Resting Heart Rate Characteristic index */
    CY_BLE_UDS_MRH,          /**< Maximum Recommended Heart Rate Characteristic index */
    CY_BLE_UDS_AET,          /**< Aerobic Threshold Characteristic index */
    CY_BLE_UDS_ANT,          /**< Anaerobic Threshold Characteristic index */
    CY_BLE_UDS_STP,          /**< Sport Type for Aerobic and Anaerobic Thresholds Characteristic index */
    CY_BLE_UDS_DTA,          /**< Date of Threshold Assessment Characteristic index */
    CY_BLE_UDS_WCC,          /**< Waist Circumference Characteristic index */
    CY_BLE_UDS_HCC,          /**< Hip Circumference Characteristic index */
    CY_BLE_UDS_FBL,          /**< Fat Burn Heart Rate Lower Limit Characteristic index */
    CY_BLE_UDS_FBU,          /**< Fat Burn Heart Rate Upper Limit Characteristic index */
    CY_BLE_UDS_AEL,          /**< Aerobic Heart Rate Lower Limit Characteristic index */
    CY_BLE_UDS_AEU,          /**< Aerobic Heart Rate Upper Limit Characteristic index */
    CY_BLE_UDS_ANL,          /**< Anaerobic Heart Rate Lower Limit Characteristic index */
    CY_BLE_UDS_ANU,          /**< Anaerobic Heart Rate Upper Limit Characteristic index */
    CY_BLE_UDS_5ZL,          /**< Five Zone Heart Rate Limits Characteristic index */
    CY_BLE_UDS_3ZL,          /**< Three Zone Heart Rate Limits Characteristic index */
    CY_BLE_UDS_2ZL,          /**< Two Zone Heart Rate Limit Characteristic index */
    CY_BLE_UDS_DCI,          /**< Database Change Increment Characteristic index */
    CY_BLE_UDS_UIX,          /**< User Index Characteristic index */
    CY_BLE_UDS_UCP,          /**< User Control Point Characteristic index */
    CY_BLE_UDS_LNG,          /**< Language Characteristic index */
    CY_BLE_UDS_CHAR_COUNT    /**< Total Count of UDS Characteristics */
}cy_en_ble_uds_char_index_t;

/** UDS Service Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_UDS_CCCD,           /**< Client Characteristic Configuration descriptor index */
    CY_BLE_UDS_DESCR_COUNT     /**< Total Count of UDS descriptors */
}cy_en_ble_uds_descr_index_t;

/** User Data Server Characteristic structure type */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t charHandle;                           /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_UDS_DESCR_COUNT];  /**< Handle of Descriptor */
}cy_stc_ble_udss_char_t;

/** Structure with User Data Service attribute handles */
typedef struct
{
    /** User Data Service Handle*/
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** User Data Service Characteristics info array */
    cy_stc_ble_udss_char_t       charInfo[CY_BLE_UDS_CHAR_COUNT];
} cy_stc_ble_udss_t;

/** User Data Client Characteristic structure type */
typedef struct
{
    /** Properties for Value Field */
    uint8_t                      properties;

    /** Handle of server database attribute value entry */
    cy_ble_gatt_db_attr_handle_t valueHandle;

    /** User Data client Characteristic Descriptor Handle */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_UDS_DESCR_COUNT];

    /** Characteristic End Handle */
    cy_ble_gatt_db_attr_handle_t endHandle;
}cy_stc_ble_udsc_char_t;

/** Structure with Discovered Attributes Information of User Data Service */
typedef struct
{
    cy_stc_ble_udsc_char_t charInfo[CY_BLE_UDS_CHAR_COUNT];   /**< Characteristics handle + properties array */
}cy_stc_ble_udsc_t;

/** UDS Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;                      /**< Peer Device Handle */
    cy_en_ble_uds_char_index_t charIndex;                       /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t    *value;                          /**< Characteristic Value */
    cy_en_ble_gatt_err_code_t  gattErrorCode;                   /**< GATT error code for access control */
} cy_stc_ble_uds_char_value_t;

/** UDS Characteristic descriptor Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;                     /**< Peer Device Handle */
    cy_en_ble_uds_char_index_t  charIndex;                      /**< Index of Service Characteristic */
    cy_en_ble_uds_descr_index_t descrIndex;                     /**< Index of Service Characteristic descriptor */
    cy_stc_ble_gatt_value_t     *value;                         /**< Descriptor value */
} cy_stc_ble_uds_descr_value_t;


/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_udss_t *attrInfo;

} cy_stc_ble_udss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_udsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_udsc_config_t;

/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** \addtogroup group_ble_service_api_UDS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_UDSS_Init(const cy_stc_ble_udss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_UDSC_Init(const cy_stc_ble_udsc_config_t *config);
void Cy_BLE_UDS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_UDS_server
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_UDSS_SetCharacteristicValue(cy_en_ble_uds_char_index_t charIndex,
                                                          uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_UDSS_GetCharacteristicValue(cy_en_ble_uds_char_index_t charIndex,
                                                          uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_UDSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_uds_char_index_t charIndex,
                                                               cy_en_ble_uds_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_UDSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_uds_char_index_t charIndex, uint8_t attrSize,
                                                    uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_UDSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_uds_char_index_t charIndex, uint8_t attrSize,
                                                  uint8_t *attrValue);

/** \} */

/**
 * \addtogroup group_ble_service_api_UDS_client
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_UDSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_uds_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_UDSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_uds_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_UDSC_GetLongCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                              cy_en_ble_uds_char_index_t charIndex, uint16_t attrSize,
                                                              uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_UDSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_uds_char_index_t charIndex,
                                                               cy_en_ble_uds_descr_index_t descrIndex, uint8_t attrSize,
                                                               uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_UDSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_uds_char_index_t charIndex,
                                                               cy_en_ble_uds_descr_index_t descrIndex);

/** \} */

/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_udss_config_t *cy_ble_udssConfigPtr;
extern const cy_stc_ble_udsc_config_t *cy_ble_udscConfigPtr;

extern uint8_t cy_ble_udsFlag;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_UDS_H */

/* [] END OF FILE */
