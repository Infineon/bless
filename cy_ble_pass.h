/***************************************************************************//**
* \file cy_ble_pass.h
* \version 3.30
*
* \brief
*  This file contains the function prototypes and constants used in
*  the Phone Alert Status Profile.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_PASS_H
#define CY_BLE_PASS_H

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
 * \addtogroup group_ble_service_api_PASS_definitions
 * \{
 */

/** Service Characteristics indexes */
typedef enum
{
    CY_BLE_PASS_AS,          /**< Alert Status Characteristic index */
    CY_BLE_PASS_RS,          /**< Ringer Setting Characteristic index */
    CY_BLE_PASS_CP,          /**< Ringer Control Point Characteristic index */
    CY_BLE_PASS_CHAR_COUNT   /**< Total Count of PASS Characteristics */
}cy_en_ble_pass_char_index_t;

/** Service Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_PASS_CCCD,           /**< Client Characteristic Configuration descriptor index */
    CY_BLE_PASS_DESCR_COUNT     /**< Total Count of PASS descriptors */
}cy_en_ble_pass_descr_index_t;

/** \} */

/* Alert Status values */
#define CY_BLE_PASS_AS_RINGER     (0x01) /* Ringer State */
#define CY_BLE_PASS_AS_VIBRATE    (0x02) /* Vibrate State */
#define CY_BLE_PASS_AS_DISPLAY    (0x04) /* Display Alert Status */

/**
 * \addtogroup group_ble_service_api_PASS_definitions
 * \{
 */

/** Ringer Setting values */
typedef enum
{
    CY_BLE_PASS_RS_SILENT,   /**< Ringer Silent */
    CY_BLE_PASS_RS_NORMAL    /**< Ringer Normal */
}cy_en_ble_pass_rs_t;

/** Ringer Control Point values */
typedef enum
{
    CY_BLE_PASS_CP_SILENT = 1,   /**< Silent Mode */
    CY_BLE_PASS_CP_MUTE,         /**< Mute Once */
    CY_BLE_PASS_CP_CANCEL        /**< Cancel Silent Mode */
}cy_en_ble_pass_cp_t;

/** Structure with Phone Alert Status Service Characteristics and descriptors attribute handles */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t charHandle;                            /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_PASS_DESCR_COUNT];  /**< Handle of Descriptor */
}cy_stc_ble_passs_char_t;

/** Structure with Phone Alert Status Service attribute handles */
typedef struct
{
    /** Phone Alert Status Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Phone Alert Status Service Characteristics info array */
    cy_stc_ble_passs_char_t      charInfo[CY_BLE_PASS_CHAR_COUNT];
} cy_stc_ble_passs_t;

/** Phone Alert Status Client Server's Characteristic structure type */
typedef struct
{
    /** Properties for Value Field */
    uint8_t                      properties;

    /** Handle of Server database attribute value entry */
    cy_ble_gatt_db_attr_handle_t valueHandle;

    /** Phone Alert Status Client Characteristics Descriptors Handles */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_PASS_DESCR_COUNT];

    /** Characteristic End Handle */
    cy_ble_gatt_db_attr_handle_t endHandle;
}cy_stc_ble_passc_char_t;

/** Structure with Discovered Attributes Information of Phone Alert Status Service */
typedef struct
{
    cy_stc_ble_passc_char_t charInfo[CY_BLE_PASS_CHAR_COUNT];    /**< Characteristics handle and properties array */
}cy_stc_ble_passc_t;

/** Phone Alert Status Service Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;                      /**< Peer Device Handle */
    cy_en_ble_pass_char_index_t charIndex;                       /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t     *value;                          /**< Characteristic Value */
} cy_stc_ble_pass_char_value_t;

/** Phone Alert Status Service Characteristic descriptor value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t     connHandle;                     /**< Peer Device Handle */
    cy_en_ble_pass_char_index_t  charIndex;                      /**< Index of Service Characteristic */
    cy_en_ble_pass_descr_index_t descrIndex;                     /**< Index of Service Characteristic descriptor */
    cy_stc_ble_gatt_value_t      *value;                         /**< Descriptor value */
} cy_stc_ble_pass_descr_value_t;


/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_passs_t *attrInfo;

} cy_stc_ble_passs_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_passc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_passc_config_t;

/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_PASS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_PASSS_Init(const cy_stc_ble_passs_config_t *config);
cy_en_ble_api_result_t Cy_BLE_PASSC_Init(const cy_stc_ble_passc_config_t *config);
void Cy_BLE_PASS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_PASS_server
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_PASSS_SetCharacteristicValue(cy_en_ble_pass_char_index_t charIndex,
                                                           uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_PASSS_GetCharacteristicValue(cy_en_ble_pass_char_index_t charIndex,
                                                           uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_PASSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_pass_char_index_t charIndex,
                                                                cy_en_ble_pass_descr_index_t descrIndex, uint8_t attrSize,
                                                                uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_PASSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                     cy_en_ble_pass_char_index_t charIndex, uint8_t attrSize,
                                                     uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_PASS_client
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_PASSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_pass_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_PASSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_pass_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_PASSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_pass_char_index_t charIndex,
                                                                cy_en_ble_pass_descr_index_t descrIndex, uint8_t attrSize,
                                                                uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_PASSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_pass_char_index_t charIndex,
                                                                cy_en_ble_pass_descr_index_t descrIndex);

/** \} */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_passs_config_t *cy_ble_passsConfigPtr;
extern const cy_stc_ble_passc_config_t *cy_ble_passcConfigPtr;
extern uint8_t cy_ble_passFlag;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_PASS_H */

/* [] END OF FILE */
