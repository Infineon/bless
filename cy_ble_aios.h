/***************************************************************************//**
* \file cy_ble_aios.h
* \version 3.50
*
* \brief
*  Contains the function prototypes and constants for the Automation Input
*  Output service.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_AIOS_H
#define CY_BLE_AIOS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

/** Trigger condition value not supported.*/
#define CY_BLE_GATT_ERR_TRIGGER_CODITION_VALUE_NOT_SUPPORTED    (0x80u)

#define CY_BLE_AIOS_UNDEFINED_ITEM                              (0x00u)
#define CY_BLE_AIOS_CHARACTERISTIC_ITEM                         (0x01u)
#define CY_BLE_AIOS_DESCRIPTOR_ITEM                             (0x02u)

#define CY_BLE_AIOS_CHAR_PRESENTATION_FORMAT_LEN                (0x07u)
#define CY_BLE_AIOS_NUM_OF_DIGITAL_DESCR_LEN                    (0x01u)
#define CY_BLE_AIOS_ANALOG_LEN                                  (0x02u)

#define CY_BLE_AIOS_VALUE_TRIGGER_DESCR_MAX_VALUE               (0x07u)
#define CY_BLE_AIOS_TIME_TRIGGER_DESCR_MAX_VALUE                (0x03u)

#define CY_BLE_AIOS_MAX_AGGREGATE_CHAR_LEN                      (512u)
#define CY_BLE_AIOS_MAX_CHAR_DESCRIPTION_VALUE                  (0xFFFFu)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_AIOS_definitions
 * \{
 */
/** AIOS Characteristic indexes */
typedef enum
{
    CY_BLE_AIOS_DIGITAL,                          /**< AIOS Digital Characteristic*/
    CY_BLE_AIOS_ANALOG,                           /**< AIOS Analog Characteristic*/
    CY_BLE_AIOS_AGGREGATE,                        /**< AIOS Aggregate Characteristic*/
    CY_BLE_AIOS_CHAR_COUNT                        /**< Total Count of AIOS Characteristics */
}cy_en_ble_aios_char_index_t;

/** AIOS Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_AIOS_CCCD,                             /**< Client Characteristic Configuration Descriptor index */
    CY_BLE_AIOS_CHAR_PRESENTATION_FORMAT,         /**< Characteristic Presentation Format Descriptor index */
    CY_BLE_AIOS_CHAR_USER_DESCRIPTION_DESCR,      /**< Characteristic User Description Descriptor index */
    CY_BLE_AIOS_CHAR_EXTENDED_PROPERTIES,         /**< Characteristic Extended Properties Descriptor index */
    CY_BLE_AIOS_VALUE_TRIGGER_SETTINGS,           /**< AIO Value Trigger Settings Descriptor index */
    CY_BLE_AIOS_TIME_TRIGGER_SETTINGS,            /**< AIO Time Trigger Settings Descriptor index */
    CY_BLE_AIOS_VRD,                              /**< Valid Range Descriptor index */
    CY_BLE_AIOS_NUM_OF_DIGITAL_DESCR = 0x06,      /**< Number of Digitals Descriptor index */
    CY_BLE_AIOS_DESCR_COUNT                       /**< Total Count of Descriptors */
}cy_en_ble_aios_descr_index_t;

/** AIOS Characteristic with descriptors */
typedef struct
{
    /** Handles of Characteristic value */
    cy_ble_gatt_db_attr_handle_t charHandle;

    /** Array of Descriptor Handles */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_AIOS_DESCR_COUNT];
} cy_stc_ble_aioss_char_t;

/** Structure to hold pointer to #cy_stc_ble_aioss_char_t */
typedef struct
{
    /** Pointer to #cy_stc_ble_aioss_char_t that holds information about specific
     * AIOS Characteristic
     */
    const cy_stc_ble_aioss_char_t *charInfoPtr;
} cy_stc_ble_aioss_char_info_ptr_t;

/** Structure with Automation Input Output Service attribute handles */
typedef struct
{
    /** Automation Input Output Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Automation Input Output Service Array with pointers to
     * Characteristic handles.
     */
    cy_stc_ble_aioss_char_info_ptr_t charInfoAddr[CY_BLE_AIOS_CHAR_COUNT];
} cy_stc_ble_aioss_t;

/** AIOS Characteristic with descriptors */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t valueHandle;                          /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t endHandle;                            /**< End handle of Characteristic */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_AIOS_DESCR_COUNT]; /**< Array of Descriptor handles */
    uint8_t                      properties;                           /**< Properties for Value Field */
} cy_stc_ble_aiosc_char_t;

/** Structure to hold pointer to cy_stc_ble_aiosc_char_t */
typedef struct
{
    /** Pointer to cy_stc_ble_aiosc_char_t that holds information about specific AIOS Characteristic. */
    cy_stc_ble_aiosc_char_t *charInfoPtr;
} cy_stc_ble_aiosc_char_info_ptr_t;

/** Structure with Discovered Attributes Information of Automation Input Output service. */
typedef struct
{
    /** Automation Input Output Service Handle */
    cy_ble_gatt_db_attr_handle_t     serviceHandle;

    /** Automation Input Output Service Array with pointers to Characteristic information. */
    cy_stc_ble_aiosc_char_info_ptr_t charInfoAddr[CY_BLE_AIOS_CHAR_COUNT];
} cy_stc_ble_aiosc_t;

/** AIOS Characteristic value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;                     /**< Peer Device Handle */
    cy_en_ble_aios_char_index_t charIndex;                      /**< Index of Service Characteristic */
    uint8_t                     charInstance;                   /**< Instance of specific service Characteristic */
    cy_stc_ble_gatt_value_t     *value;                         /**< Characteristic Value */
    cy_en_ble_gatt_err_code_t   gattErrorCode;                  /**< GATT error code for access control */
} cy_stc_ble_aios_char_value_t;

/** AIOS Characteristic descriptor value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t     connHandle;                    /**< Peer Device Handle */
    cy_en_ble_aios_char_index_t  charIndex;                     /**< Index of Service Characteristic */
    uint8_t                      charInstance;                  /**< Instance of specific service Characteristic */
    cy_en_ble_aios_descr_index_t descrIndex;                    /**< Index of Descriptor */
    cy_en_ble_gatt_err_code_t    gattErrorCode;                 /**< Error code received from application (optional) */
    cy_stc_ble_gatt_value_t      *value;                        /**< Characteristic Value */
} cy_stc_ble_aios_descr_value_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_aioss_t *attrInfo;

    /** Number of AIOS Characteristics instances for server */
    const uint8_t  *charInstances;
    
} cy_stc_ble_aioss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    const cy_stc_ble_aiosc_t *attrInfo;
    
    /** Number of AIOS Characteristics instances for server */
    const uint8_t *charInstances;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_aiosc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** \addtogroup group_ble_service_api_AIOS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_AIOSS_Init(const cy_stc_ble_aioss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_AIOSC_Init(const cy_stc_ble_aiosc_config_t *config);
void Cy_BLE_AIOS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_AIOS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_AIOSS_SetCharacteristicValue(cy_en_ble_aios_char_index_t charIndex,
                                                           uint8_t charInstance, uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_AIOSS_GetCharacteristicValue(cy_en_ble_aios_char_index_t charIndex,
                                                           uint8_t charInstance, uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_AIOSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_aios_char_index_t charIndex, uint8_t charInstance,
                                                                cy_en_ble_aios_descr_index_t descrIndex,
                                                                uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_AIOSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_aios_char_index_t charIndex, uint8_t charInstance,
                                                                cy_en_ble_aios_descr_index_t descrIndex,
                                                                uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_AIOSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                     cy_en_ble_aios_char_index_t charIndex, uint8_t charInstance, uint8_t
                                                     attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_AIOSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                   cy_en_ble_aios_char_index_t charIndex, uint8_t charInstance,
                                                   uint8_t attrSize, uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_AIOS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_AIOSC_SetCharacteristicValueWithoutResponse(cy_stc_ble_conn_handle_t connHandle,
                                                                          cy_en_ble_aios_char_index_t charIndex, uint8_t
                                                                          charInstance, uint8_t attrSize,
                                                                          uint8_t * attrValue);

cy_en_ble_api_result_t Cy_BLE_AIOSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_aios_char_index_t charIndex, uint8_t charInstance,
                                                           uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_AIOSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_aios_char_index_t charIndex, uint8_t charInstance);

cy_en_ble_api_result_t Cy_BLE_AIOSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_aios_char_index_t charIndex, uint8_t charInstance,
                                                                cy_en_ble_aios_descr_index_t descrIndex,
                                                                uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_AIOSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_aios_char_index_t charIndex, uint8_t charInstance,
                                                                cy_en_ble_aios_descr_index_t descrIndex);
/** \} */

cy_ble_gatt_db_attr_handle_t Cy_BLE_AIOSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_aios_char_index_t charIndex,
                                                                       uint8_t charInstance);

cy_ble_gatt_db_attr_handle_t Cy_BLE_AIOSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_aios_char_index_t charIndex,
                                                                            uint8_t charInstance,
                                                                            cy_en_ble_aios_descr_index_t descrIndex);


/*******************************************************************************
* Macro Functions
*******************************************************************************/

#define Cy_BLE_AIOS_Get16ByPtr    Cy_BLE_Get16ByPtr


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_aioss_config_t *cy_ble_aiossConfigPtr;
extern const cy_stc_ble_aiosc_config_t *cy_ble_aioscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_AIOS_H */

/* [] END OF FILE */
