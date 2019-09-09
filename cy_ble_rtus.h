/***************************************************************************//**
* \file cy_ble_rtus.h
* \version 3.20
*
* \brief
*  Contains the function prototypes and constants for Reference Time Update
*  service.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_RTUS_H
#define CY_BLE_RTUS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

#define CY_BLE_RTUS_TIME_UPDATE_CP_SIZE          (0x01u)

#define CY_BLE_RTUS_INVALID_CHAR_INDEX           (0xFFu)

/* Time Update Control Point Characteristic Values */
#define CY_BLE_RTUS_GET_REF_UPDATE               (0x01u)
#define CY_BLE_RTUS_CANCEL_REF_UPDATE            (0x02u)

/* Time Update State Characteristic Values
 * Current State field
 */
#define CY_BLE_RTUS_CURR_STATE_IDLE              (0x00u)
#define CY_BLE_RTUS_CURR_STATE_UPDATE_PENDING    (0x01u)

/* Result field */
#define CY_BLE_RTUS_RESULT_SUCCESSFUL            (0x00u)
#define CY_BLE_RTUS_RESULT_CANCELED              (0x01u)
#define CY_BLE_RTUS_RESULT_NO_CONN               (0x02u)
#define CY_BLE_RTUS_RESULT_REF_ERROR             (0x03u)
#define CY_BLE_RTUS_RESULT_TIMEOUT               (0x04u)
#define CY_BLE_RTUS_RESULT_NO_UPDATE_ATTEMPT     (0x05u)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_RTUS_definitions
 * \{
 */
/** RTUS Characteristic indexes */
typedef enum
{
    CY_BLE_RTUS_TIME_UPDATE_CONTROL_POINT,            /**< Time Update Control Point Characteristic index */
    CY_BLE_RTUS_TIME_UPDATE_STATE,                    /**< Time Update State Characteristic index */
    CY_BLE_RTUS_CHAR_COUNT                            /**< Total Count of RTUS Characteristics */
} cy_en_ble_rtus_char_index_t;

/** Reference Time Update Service Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;           /**< Peer Device Handle */
    cy_en_ble_rtus_char_index_t charIndex;            /**< Index of Reference Time Update Service Characteristic */
    cy_stc_ble_gatt_value_t     *value;               /**< Characteristic Value */
} cy_stc_ble_rtus_char_value_t;

/** Time Update State Characteristic structure */
typedef struct
{
    uint8_t currentState;               /**< Current state */
    uint8_t result;                     /**< Result of Time update */
} cy_stc_ble_rtus_time_update_state_t;

/** Structure with Reference Time Update Service attribute handles */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t serviceHandle;           /**< Handle of the Reference Time Update Service */
    cy_ble_gatt_db_attr_handle_t timeUpdateCpHandle;      /**< Handle of the Time Update Control Point Characteristic */
    cy_ble_gatt_db_attr_handle_t timeUpdateStateHandle;   /**< Handle of the Time Update State Characteristic */
} cy_stc_ble_rtuss_t;

/** Structure with Discovered Attributes Information of Reference Time Update Service */
typedef struct
{
    /** Characteristic handle and properties */
    cy_stc_ble_srvr_char_info_t charInfo[CY_BLE_RTUS_CHAR_COUNT];
} cy_stc_ble_rtusc_t;


/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_rtuss_t *attrInfo;

} cy_stc_ble_rtuss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_rtusc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_rtusc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_RTUS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_RTUSS_Init(const cy_stc_ble_rtuss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_RTUSC_Init(const cy_stc_ble_rtusc_config_t *config);
void Cy_BLE_RTUS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_RTUS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_RTUSS_SetCharacteristicValue(cy_en_ble_rtus_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_RTUSS_GetCharacteristicValue(cy_en_ble_rtus_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_RTUS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_RTUSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_rtus_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_RTUSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_rtus_char_index_t charIndex);
/** \} */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_rtuss_config_t *cy_ble_rtussConfigPtr;
extern const cy_stc_ble_rtusc_config_t *cy_ble_rtuscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_RTUS_H */

/* [] END OF FILE */
