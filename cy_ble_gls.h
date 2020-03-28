/***************************************************************************//**
* \file cy_ble_gls.h
* \version 3.40
*
* \brief
*  This file contains the function prototypes and constants used in the
*  Glucose Profile.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_GLS_H
#define CY_BLE_GLS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

#define CY_BLE_GLS_FLAG_PROCESS         (0x01u)
#define CY_BLE_GLS_RACP_OPCODE_ABORT    (0x03u)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_GLS_definitions
 * \{
 */
/** Service Characteristics indexes */
typedef enum
{
    CY_BLE_GLS_GLMT,           /**< Glucose Measurement Characteristic index */
    CY_BLE_GLS_GLMC,           /**< Glucose Measurement Context Characteristic index */
    CY_BLE_GLS_GLFT,           /**< Glucose Feature Characteristic index */
    CY_BLE_GLS_RACP,           /**< Record Access Control Point Characteristic index */
    CY_BLE_GLS_CHAR_COUNT      /**< Total Count of GLS Characteristics */
}cy_en_ble_gls_char_index_t;

/** Service Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_GLS_CCCD,           /**< Client Characteristic Configuration descriptor index */
    CY_BLE_GLS_DESCR_COUNT     /**< Total Count of GLS descriptors */
}cy_en_ble_gls_descr_index_t;

/** Glucose Server Characteristic structure type */
typedef struct
{
    /** Glucose Service char handle */
    cy_ble_gatt_db_attr_handle_t charHandle;

    /** Glucose Service CCCD handle */
    cy_ble_gatt_db_attr_handle_t cccdHandle;
}cy_stc_ble_glss_char_t;

/** Structure with Glucose Service attribute handles */
typedef struct
{
    /** Glucose Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Glucose Service Characteristics info array */
    cy_stc_ble_glss_char_t       charInfo[CY_BLE_GLS_CHAR_COUNT];
} cy_stc_ble_glss_t;

/** Glucose Client Characteristic structure type */
typedef struct
{
    /** Properties for Value Field */
    uint8_t                      properties;

    /** Handle of server database attribute value entry */
    cy_ble_gatt_db_attr_handle_t valueHandle;

    /** Glucose client char. descriptor handle */
    cy_ble_gatt_db_attr_handle_t cccdHandle;

    /** Characteristic End Handle */
    cy_ble_gatt_db_attr_handle_t endHandle;
}cy_stc_ble_glsc_char_t;

/** Glucose Service structure type */
typedef struct
{
    /** Characteristics handle + properties array */
    cy_stc_ble_glsc_char_t charInfo[CY_BLE_GLS_CHAR_COUNT];
}cy_stc_ble_glsc_t;

/** Glucose Service Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;                      /**< Peer Device Handle */
    cy_en_ble_gls_char_index_t charIndex;                       /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t    *value;                          /**< Characteristic Value */
} cy_stc_ble_gls_char_value_t;

/** Glucose Service Characteristic descriptor value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;                     /**< Peer Device Handle */
    cy_en_ble_gls_char_index_t  charIndex;                      /**< Index of Service Characteristic */
    cy_en_ble_gls_descr_index_t descrIndex;                     /**< Index of Service Characteristic descriptor */
    cy_stc_ble_gatt_value_t     *value;                         /**< Descriptor value */
} cy_stc_ble_gls_descr_value_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_glss_t *attrInfo;

} cy_stc_ble_glss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_glsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_glsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_GLS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_GLSS_Init(const cy_stc_ble_glss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_GLSC_Init(const cy_stc_ble_glsc_config_t *config);
void Cy_BLE_GLS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_GLS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_GLSS_SetCharacteristicValue(cy_en_ble_gls_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);
cy_en_ble_api_result_t Cy_BLE_GLSS_GetCharacteristicValue(cy_en_ble_gls_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_GLSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_gls_char_index_t charIndex,
                                                               cy_en_ble_gls_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_GLSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_gls_char_index_t charIndex, uint8_t attrSize,
                                                    uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_GLSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_gls_char_index_t charIndex, uint8_t attrSize,
                                                  uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_GLS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_GLSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_gls_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_GLSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_gls_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_GLSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_gls_char_index_t charIndex,
                                                               cy_en_ble_gls_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_GLSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_gls_char_index_t charIndex,
                                                               cy_en_ble_gls_descr_index_t descrIndex);
/** \} */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern uint8_t cy_ble_glsFlag;
extern const cy_stc_ble_glss_config_t *cy_ble_glssConfigPtr;
extern const cy_stc_ble_glsc_config_t *cy_ble_glscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_GLS_H */

/* [] END OF FILE */
