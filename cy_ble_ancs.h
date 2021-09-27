/***************************************************************************//**
* \file cy_ble_ancs.h
* \version 3.60
*
* \brief
*  This file contains the function prototypes and constants used in
*  the Apple Notification Center (ANC) service.
*
********************************************************************************
* \copyright
* Copyright 2017-2021, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_ANCS_H
#define CY_BLE_ANCS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* The C binding of definitions if building with the C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

#define CY_BLE_ANCS_FLAG_PROCESS    (0x01u)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_ANCS_definitions
 * \{
 */

/** ANC Service Characteristics indexes */
typedef enum
{
    CY_BLE_ANCS_NS,                 /**< Notification Source Characteristic index */
    CY_BLE_ANCS_CP,                 /**< Control Point Characteristic index */
    CY_BLE_ANCS_DS,                 /**< Data Source Characteristic index */
    CY_BLE_ANCS_CHAR_COUNT          /**< Total Count of ANCS Characteristics */
}cy_en_ble_ancs_char_index_t;

/** ANC Service Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_ANCS_CCCD,               /**< Client Characteristic Configuration descriptor index */
    CY_BLE_ANCS_DESCR_COUNT         /**< Total Count of ANCS descriptors */
}cy_en_ble_ancs_descr_index_t;

/** ANC Service Characteristic structure type */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t charHandle;                           /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_ANCS_DESCR_COUNT]; /**< Handle of Descriptor */
}cy_stc_ble_ancss_char_t;

/** Structure with ANC Service attribute handles */
typedef struct
{
    /** ANC Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** ANC Service Characteristics info array */
    cy_stc_ble_ancss_char_t      charInfo[CY_BLE_ANCS_CHAR_COUNT];
} cy_stc_ble_ancss_t;

/** ANCS client Characteristic structure type */
typedef struct
{
    /** Properties for Value Field */
    uint8_t                      properties;

    /** Handle of server database attribute value entry */
    cy_ble_gatt_db_attr_handle_t valueHandle;

    /** ANCS client char. descriptor handle */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_ANCS_DESCR_COUNT];

    /** Characteristic end handle */
    cy_ble_gatt_db_attr_handle_t endHandle;
}cy_stc_ble_ancsc_char_t;

/** Structure with Discovered Attributes Information of ANC Service */
typedef struct
{
    cy_stc_ble_ancsc_char_t charInfo[CY_BLE_ANCS_CHAR_COUNT];   /**< Characteristics handle + properties array */
}cy_stc_ble_ancsc_t;

/** ANCS Characteristic value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;                     /**< Peer Device Handle */
    cy_en_ble_ancs_char_index_t charIndex;                      /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t     *value;                         /**< Characteristic Value */
    cy_en_ble_gatt_err_code_t   gattErrorCode;                  /**< GATT error code for access control */
} cy_stc_ble_ancs_char_value_t;

/** ANCS Characteristic descriptor value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t     connHandle;                    /**< Peer Device Handle */
    cy_en_ble_ancs_char_index_t  charIndex;                     /**< Index of Service Characteristic */
    cy_en_ble_ancs_descr_index_t descrIndex;                    /**< Index of Service Characteristic descriptor */
    cy_stc_ble_gatt_value_t      *value;                        /**< Descriptor value */
} cy_stc_ble_ancs_descr_value_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_ancss_t *attrInfo;

} cy_stc_ble_ancss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_ancsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_ancsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** \addtogroup group_ble_service_api_ANCS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_ANCSS_Init(const cy_stc_ble_ancss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_ANCSC_Init(const cy_stc_ble_ancsc_config_t *config);
void Cy_BLE_ANCS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_ANCS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_ANCSS_SetCharacteristicValue(cy_en_ble_ancs_char_index_t charIndex,
                                                           uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_ANCSS_GetCharacteristicValue(cy_en_ble_ancs_char_index_t charIndex,
                                                           uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_ANCSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_ancs_char_index_t charIndex,
                                                                cy_en_ble_ancs_descr_index_t descrIndex, uint8_t attrSize,
                                                                uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_ANCSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                     cy_en_ble_ancs_char_index_t charIndex, uint8_t attrSize,
                                                     uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_ANCS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_ANCSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_ancs_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_ANCSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_ancs_char_index_t charIndex,
                                                                cy_en_ble_ancs_descr_index_t descrIndex, uint8_t attrSize,
                                                                uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_ANCSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_ancs_char_index_t charIndex,
                                                                cy_en_ble_ancs_descr_index_t descrIndex);
/** \} */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_uuid128_t cy_ble_ancscServUuid;
extern const cy_stc_ble_uuid128_t cy_ble_ancscCharUuid[CY_BLE_ANCS_CHAR_COUNT];

extern const cy_stc_ble_ancss_config_t *cy_ble_ancssConfigPtr;
extern const cy_stc_ble_ancsc_config_t *cy_ble_ancscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_ANCS_H */

/* [] END OF FILE */
