/***************************************************************************//**
* \file cy_ble_hrs.h
* \version 3.40
*
* \brief
*  This file contains the function prototypes and constants used in
*  the Heart Rate service.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_HRS_H
#define CY_BLE_HRS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

/* Definitions for Body Sensor Location Characteristic */
#define CY_BLE_HRS_BSL_CHAR_LEN             (1u)

/* Definitions for ControlPoint Characteristic */
#define CY_BLE_HRS_CPT_CHAR_LEN             (1u)
#define CY_BLE_HRS_RESET_ENERGY_EXPENDED    (0x01u)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_HRS_definitions
 * \{
 */
/** HRS Characteristics indexes */
typedef enum
{
    CY_BLE_HRS_HRM,          /**< Heart Rate Measurement Characteristic index */
    CY_BLE_HRS_BSL,          /**< Body Sensor Location Characteristic index */
    CY_BLE_HRS_CPT,          /**< Control Point Characteristic index */
    CY_BLE_HRS_CHAR_COUNT    /**< Total Count of HRS Characteristics */
}cy_en_ble_hrs_char_index_t;

/** HRS Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_HRS_HRM_CCCD,    /**< Heart Rate Measurement client char. config. descriptor index */
    CY_BLE_HRS_DESCR_COUNT  /**< Total Count of HRS HRM descriptors */
}cy_en_ble_hrs_descr_index_t;

/** Structure with Heart Rate Service attribute handles */
typedef struct
{
    /** Heart Rate Service Handle*/
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Heart Rate Service Characteristics handles and properties array */
    cy_ble_gatt_db_attr_handle_t charHandle[CY_BLE_HRS_CHAR_COUNT];

    /** Heart Rate Measurement client char. config. descriptor Handle */
    cy_ble_gatt_db_attr_handle_t hrmCccdHandle;
} cy_stc_ble_hrss_t;

/** Structure with Discovered Attributes Information of Heart Rate Service */
typedef struct
{
    /** Heart Rate Service Characteristics handles and properties array */
    cy_stc_ble_srvr_char_info_t  charInfo[CY_BLE_HRS_CHAR_COUNT];

    /** Heart Rate Measurement client char. config. descriptor Handle */
    cy_ble_gatt_db_attr_handle_t hrmCccdHandle;
}cy_stc_ble_hrsc_t;

/** HRS Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;                      /**< Peer Device Handle */
    cy_en_ble_hrs_char_index_t charIndex;                       /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t    *value;                          /**< Characteristic Value */
} cy_stc_ble_hrs_char_value_t;

/** HRS Characteristic descriptor value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;                     /**< Peer Device Handle */
    cy_en_ble_hrs_char_index_t  charIndex;                      /**< Index of Service Characteristic */
    cy_en_ble_hrs_descr_index_t descrIndex;                     /**< Index of Service Characteristic descriptor */
    cy_stc_ble_gatt_value_t     *value;                         /**< Descriptor value */
} cy_stc_ble_hrs_descr_value_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_hrss_t *attrInfo;

} cy_stc_ble_hrss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_hrsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_hrsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_HRS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_HRSS_Init(const cy_stc_ble_hrss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_HRSC_Init(const cy_stc_ble_hrsc_config_t *config);
void Cy_BLE_HRS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_HRS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_HRSS_SetCharacteristicValue(cy_en_ble_hrs_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HRSS_GetCharacteristicValue(cy_en_ble_hrs_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HRSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hrs_char_index_t charIndex,
                                                               cy_en_ble_hrs_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HRSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_hrs_char_index_t charIndex, uint8_t attrSize,
                                                    uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_HRS_client
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_HRSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_hrs_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HRSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_hrs_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_HRSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hrs_char_index_t charIndex,
                                                               cy_en_ble_hrs_descr_index_t descrIndex, uint8_t attrSize,
                                                               uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HRSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hrs_char_index_t charIndex,
                                                               cy_en_ble_hrs_descr_index_t descrIndex);
/** \} */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_hrss_config_t *cy_ble_hrssConfigPtr;
extern const cy_stc_ble_hrsc_config_t *cy_ble_hrscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_HRS_H */

/* [] END OF FILE */
