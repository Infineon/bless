/***************************************************************************//**
* \file cy_ble_dis.h
* \version 3.40
*
* \brief
*  Contains the function prototypes and constants for Device Information
*  service.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_DIS_H
#define CY_BLE_DIS_H

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
 * \addtogroup group_ble_service_api_DIS_definitions
 * \{
 */
/** DIS Characteristic index */
typedef enum
{
    CY_BLE_DIS_MANUFACTURER_NAME,                    /**< Manufacturer Name String Characteristic index */
    CY_BLE_DIS_MODEL_NUMBER,                         /**< Model Number String Characteristic index */
    CY_BLE_DIS_SERIAL_NUMBER,                        /**< Serial Number String Characteristic index*/
    CY_BLE_DIS_HARDWARE_REV,                         /**< Hardware Revision String Characteristic index */
    CY_BLE_DIS_FIRMWARE_REV,                         /**< Firmware Revision String Characteristic index */
    CY_BLE_DIS_SOFTWARE_REV,                         /**< Software Revision String Characteristic index */
    CY_BLE_DIS_SYSTEM_ID,                            /**< System ID Characteristic index */
    CY_BLE_DIS_REG_CERT_DATA,                        /**< IEEE 11073-20601 Characteristic index */
    CY_BLE_DIS_PNP_ID,                               /**< PnP ID Characteristic index */
    CY_BLE_DIS_CHAR_COUNT                            /**< Total Count of DIS Characteristics */
} cy_en_ble_dis_char_index_t;

/** Structure with Device Information Service attribute handles */
typedef struct
{
    /** Device Information Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Device Information Service Characteristic handles */
    cy_ble_gatt_db_attr_handle_t charHandle[CY_BLE_DIS_CHAR_COUNT];
} cy_stc_ble_diss_t;

/** Structure with Discovered Attributes Information of Device Information Service */
typedef struct
{
    /** Characteristics handle + properties array */
    cy_stc_ble_srvr_char_info_t charInfo[CY_BLE_DIS_CHAR_COUNT];
} cy_stc_ble_disc_t;

/** Device Information Service Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;           /**< Peer Device Handle */
    cy_en_ble_dis_char_index_t charIndex;            /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t    *value;               /**< Characteristic Value */
} cy_stc_ble_dis_char_value_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_diss_t *attrInfo;

} cy_stc_ble_diss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_disc_t *attrInfo;

    /** The Discovery Service index */
    uint8_t serviceDiscIdx;

} cy_stc_ble_disc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** \addtogroup group_ble_service_api_DIS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_DISS_Init(const cy_stc_ble_diss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_DISC_Init(const cy_stc_ble_disc_config_t *config);
void Cy_BLE_DIS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_DIS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_DISS_SetCharacteristicValue(cy_en_ble_dis_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);
cy_en_ble_api_result_t Cy_BLE_DISS_GetCharacteristicValue(cy_en_ble_dis_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);
/** \} */


/**
 * \addtogroup group_ble_service_api_DIS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_DISC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_dis_char_index_t charIndex);
/** \} */

/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_diss_config_t *cy_ble_dissConfigPtr;
extern const cy_stc_ble_disc_config_t *cy_ble_discConfigPtr;


/*******************************************************************************
* Macro Functions
*******************************************************************************/

#if CY_BLE_LIB_HOST_CORE
/******************************************************************************
* Function Name: Cy_BLE_DISC_GetCharacteristicValueHandle
***************************************************************************//**
*
*  Returns the discovered peer device Characteristic value handle.
*
*  \param serviceIndex: The index of the service instance.
*  \param charIndex: The index of a service Characteristic.
*
* \return
*  Returns Characteristic value handle of type cy_ble_gatt_db_attr_handle_t.
*  * CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE: when a peer device doesn't have
*                                           an optional Characteristic
*
******************************************************************************/
__STATIC_INLINE cy_ble_gatt_db_attr_handle_t Cy_BLE_DISC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                                      cy_en_ble_dis_char_index_t charIndex)
{
    cy_ble_gatt_db_attr_handle_t locAttrHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;


    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && ((charIndex) < CY_BLE_DIS_CHAR_COUNT))
    {
        /* Get pointer (with offset) to DIS client structure with attribute handles */
        cy_stc_ble_disc_t *discPtr = (cy_stc_ble_disc_t *)&cy_ble_discConfigPtr->attrInfo[discIdx];

        /* Get attribute handles */
        locAttrHandle = discPtr[discIdx].charInfo[charIndex].valueHandle;
    }

    return (locAttrHandle);
}
#endif /* CY_BLE_LIB_HOST_CORE */


/** \endcond */


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_DIS_H */

/* [] END OF FILE */
