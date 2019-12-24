/***************************************************************************//**
* \file cy_ble_cscs.h
* \version 3.30
*
* \brief
*  Contains the function prototypes and constants for Cycling Speed and Cadence
*  service.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_CSCS_H
#define CY_BLE_CSCS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

#define CY_BLE_CSCS_INVALID_CHAR_INDEX               (0xFFu)

/* SC Control Point Characteristic Op Codes */
#define CY_BLE_CSCS_SET_CUMMULATIVE_VALUE            (0x01u)
#define CY_BLE_CSCS_START_SENSOR_CALIBRATION         (0x02u)
#define CY_BLE_CSCS_UPDATE_SENSOR_LOCATION           (0x03u)
#define CY_BLE_CSCS_REQ_SUPPORTED_SENSOR_LOCATION    (0x04u)
#define CY_BLE_CSCS_RESPONSE_CODE                    (0x10u)

/* SC Control Point Characteristic Response Codes */
#define CY_BLE_CSCS_ERR_SUCCESS                      (0x01u)
#define CY_BLE_CSCS_ERR_OP_CODE_NOT_SUPPORTED        (0x02u)
#define CY_BLE_CSCS_ERR_INVALID_PARAMETER            (0x03u)
#define CY_BLE_CSCS_ERR_OPERATION_FAILED             (0x04u)


/*******************************************************************************
* Data Types
*******************************************************************************/
/**
 * \addtogroup group_ble_service_api_CSCS_definitions
 * \{
 */
/** Characteristic indexes */
typedef enum
{
    CY_BLE_CSCS_CSC_MEASUREMENT,                      /**< CSC Measurement Characteristic index */
    CY_BLE_CSCS_CSC_FEATURE,                          /**< CSC Feature Characteristic index */
    CY_BLE_CSCS_SENSOR_LOCATION,                      /**< CSC Sensor Location Characteristic index */
    CY_BLE_CSCS_SC_CONTROL_POINT,                     /**< CSC SC Control Point Characteristic index */
    CY_BLE_CSCS_CHAR_COUNT                            /**< Total Count of CSCS Characteristics */
} cy_en_ble_cscs_char_index_t;

/** Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_CSCS_CCCD,                                 /**< Client Characteristic Configuration Descriptor index */
    CY_BLE_CSCS_DESCR_COUNT                           /**< Total Count of Descriptors */
} cy_en_ble_cscs_descr_index_t;

/** Cycling Speed and Cadence Service Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;            /**< Peer Device Handle */
    cy_en_ble_cscs_char_index_t charIndex;             /**< Index of Cycling Speed and Cadence Service Characteristic */
    cy_stc_ble_gatt_value_t     *value;                /**< Characteristic Value */
} cy_stc_ble_cscs_char_value_t;

/** Cycling Speed and Cadence Service Characteristic descriptor Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t     connHandle;           /**< Connection handle */
    cy_en_ble_cscs_char_index_t  charIndex;            /**< Characteristic index of the Service */
    cy_en_ble_cscs_descr_index_t descrIndex;           /**< Characteristic descriptor index */
    cy_stc_ble_gatt_value_t      *value;               /**< Pointer to value of the Service Characteristic descriptor */
} cy_stc_ble_cscs_descr_value_t;

/** Characteristic with descriptors type */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t charHandle;                            /**< Handle of the Characteristic Value */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_CSCS_DESCR_COUNT];  /**< Handles of the Descriptors */
} cy_stc_ble_cscss_char_t;

/** Structure with Cycling Speed and Cadence Service attribute handles */
typedef struct
{
    /** Cycling Speed and Cadence Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Array of Cycling Speed and Cadence Service Characteristics and Descriptors handles */
    cy_stc_ble_cscss_char_t      charInfo[CY_BLE_CSCS_CHAR_COUNT];
} cy_stc_ble_cscss_t;

/** Service full Characteristic information type */
typedef struct
{
    /** Characteristic handle and properties */
    cy_stc_ble_srvr_char_info_t  charInfo;

    /** Characteristic Descriptors Handles */
    cy_ble_gatt_db_attr_handle_t descriptors[CY_BLE_CSCS_DESCR_COUNT];

    /** End handle of Characteristic */
    cy_ble_gatt_db_attr_handle_t endHandle;
} cy_stc_ble_cscsc_srvr_full_char_info_t;

/** Structure with Discovered Attributes Information of Cycling Speed and Cadence Service */
typedef struct
{
    /** Characteristics handles array */
    cy_stc_ble_cscsc_srvr_full_char_info_t characteristics[CY_BLE_CSCS_CHAR_COUNT];
} cy_stc_ble_cscsc_t;


/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_cscss_t *attrInfo;

} cy_stc_ble_cscss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_cscsc_t *attrInfo;
    /** Flash storage for discovered attributes information */
    const cy_stc_ble_cscsc_t * flashAtrInfo;
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_cscsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_CSCS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_CSCSS_Init(const cy_stc_ble_cscss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_CSCSC_Init(const cy_stc_ble_cscsc_config_t *config);
void Cy_BLE_CSCS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_CSCS_server
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_CSCSS_SetCharacteristicValue(cy_en_ble_cscs_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CSCSS_GetCharacteristicValue(cy_en_ble_cscs_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CSCSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_cscs_char_index_t charIndex,
                                                                cy_en_ble_cscs_descr_index_t descrIndex, uint8_t attrSize,
                                                                uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CSCSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                     cy_en_ble_cscs_char_index_t charIndex, uint8_t attrSize,
                                                     uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CSCSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                   cy_en_ble_cscs_char_index_t charIndex, uint8_t attrSize,
                                                   uint8_t *attrValue);

/** \} */

/**
 * \addtogroup group_ble_service_api_CSCS_client
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_CSCSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_cscs_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CSCSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_cscs_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_CSCSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_cscs_char_index_t charIndex,
                                                                cy_en_ble_cscs_descr_index_t descrIndex,
                                                                uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CSCSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_cscs_char_index_t charIndex,
                                                                cy_en_ble_cscs_descr_index_t descrIndex);
cy_en_ble_api_result_t Cy_BLE_CSCSC_StoreProfileData(void);
/** \} */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_cscss_config_t *cy_ble_cscssConfigPtr;
extern const cy_stc_ble_cscsc_config_t *cy_ble_cscscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_CSCS_H */

/* [] END OF FILE */
