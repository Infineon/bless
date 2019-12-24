/***************************************************************************//**
* \file cy_ble_bts.h
* \version 3.30
*
* \brief
*  Contains the function prototypes and constants for the Bootloader service.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_BTS_H
#define CY_BLE_BTS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

/* Maximum supported Bootloader Services */
#define CY_BLE_BT_SERVICE_COUNT    (0x01u)


/*******************************************************************************
* Data Types
*******************************************************************************/

/** BTS Characteristic indexes */
typedef enum
{
    CY_BLE_BTS_BT_SERVICE,           /**< Bootloader service Characteristic index */
    CY_BLE_BTS_CHAR_COUNT            /**< Total Count of Characteristics */
}cy_en_ble_bts_char_index_t;

/** BTS Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_BTS_BT_SERVICE_CCCD,      /**< Client Characteristic Configuration descriptor index */
    CY_BLE_BTS_DESCR_COUNT           /**< Total Count of Descriptors */
}cy_en_ble_bts_descr_index_t;

/** Contains information about the Bootloader Characteristic structure */
typedef struct
{
    /** Bootloader Characteristic handle */
    cy_ble_gatt_db_attr_handle_t btServiceCharHandle;

    /** Bootloader Characteristic Descriptors Handles */
    cy_ble_gatt_db_attr_handle_t btServiceCharDescriptors[CY_BLE_BTS_DESCR_COUNT];
} cy_stc_ble_bts_info_t;

/** Structure with Bootloader Service attribute handles. */
typedef struct
{
    /** Handle of a Bootloader Service */
    cy_ble_gatt_db_attr_handle_t btServiceHandle;

    /** Information about Bootloader Characteristics */
    cy_stc_ble_bts_info_t        btServiceInfo[CY_BLE_BTS_CHAR_COUNT];
} cy_stc_ble_btss_t;

/** BTS Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;          /**< Peer Device Handle */
    cy_en_ble_bts_char_index_t charIndex;           /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t    *value;              /**< Characteristic Value */
} cy_stc_ble_bts_char_value_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_btss_t *attrInfo;

} cy_stc_ble_btss_config_t;


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

cy_en_ble_api_result_t Cy_BLE_BTSS_Init(const cy_stc_ble_btss_config_t *config);

void Cy_BLE_BTS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);

cy_en_ble_api_result_t Cy_BLE_BTSS_Init(const cy_stc_ble_btss_config_t *config);

cy_en_ble_api_result_t Cy_BLE_BTSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_bts_char_index_t charIndex, uint32_t attrSize, const
                                                    uint8_t *attrValue);


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_btss_config_t *cy_ble_btssConfigPtr;
extern uint16_t cy_ble_cmdLength;
extern uint8_t  cy_ble_cmdReceivedFlag;
extern uint8_t *cy_ble_btsBuffPtr;
extern cy_stc_ble_conn_handle_t btConnHandle;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_BTS_H */


/* [] END OF FILE */
