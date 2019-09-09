/***************************************************************************//**
* \file cy_ble_custom.h
* \version 3.20
*
* \brief
*  Contains the function prototypes and constants for the Custom service.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_CUSTOM_H
#define CY_BLE_CUSTOM_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
* API Constants
*******************************************************************************/

/* The maximum supported Custom server services */
#define CY_BLE_CUSTOMS_SERVICE_COUNT       (CY_BLE_CONFIG_CUSTOMS_SERVICE_COUNT)

/* The maximum supported Custom client services */
#define CY_BLE_CUSTOMC_SERVICE_COUNT       (CY_BLE_CONFIG_CUSTOMC_SERVICE_COUNT)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_custom
 * \{
 */
/** Structure with Custom attribute information */
typedef struct
{
    /** Custom Characteristic handle */
    cy_ble_gatt_db_attr_handle_t customServCharHandle;

    /** Custom Characteristic Descriptors Handles */
    const cy_ble_gatt_db_attr_handle_t *customServCharDesc;
} cy_stc_ble_customs_info_t;

/** Structure with Custom Service attribute handles */
typedef struct
{
    /** Handle of a Custom Service */
    cy_ble_gatt_db_attr_handle_t customServHandle;

    /** Information about Custom Characteristics */
    const cy_stc_ble_customs_info_t *customServInfo;
} cy_stc_ble_customs_t;

/** Structure with Discovered Attributes Information of Custom Service Descriptors */
typedef struct
{
    /** Custom Descriptor handle */
    cy_ble_gatt_db_attr_handle_t *descHandle;

    /** Custom Descriptor 128 bit UUID */
    const void                   *uuid;

    /** UUID Format - 16-bit (0x01) or 128-bit (0x02) */
    uint8_t                      uuidFormat;
} cy_stc_ble_customc_desc_t;

/** Structure with Discovered Attributes Information of Custom Service Characteristics */
typedef struct
{
    /** Characteristic handle */
    cy_ble_gatt_db_attr_handle_t *customServCharHandle;

    /** Characteristic end handle */
    cy_ble_gatt_db_attr_handle_t *customServCharEndHandle;

    /** Custom Characteristic UUID */
    const void                   *uuid;

    /** UUID Format - 16-bit (0x01) or 128-bit (0x02) */
    uint8_t                      uuidFormat;

    /** Properties for Value Field */
    uint8_t                      *properties;

    /** Number of descriptors */
    uint8_t                      descCount;

    /** Characteristic descriptors */
    const cy_stc_ble_customc_desc_t    *customServCharDesc;
} cy_stc_ble_customc_char_t;

/** Structure with Discovered Attributes Information of Custom Service */
typedef struct
{
    /** Custom Service Handle */
    cy_ble_gatt_db_attr_handle_t *customServHandle;

    /** Custom Service UUID */
    const void                   *uuid;

    /** UUID Format - 16-bit (0x01) or 128-bit (0x02) */
    uint8_t                      uuidFormat;

    /** Number of Characteristics */
    uint8_t                      charCount;

    /** Custom Service Characteristics */
    const cy_stc_ble_customc_char_t    *customServChar;
} cy_stc_ble_customc_t;


/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_customs_t *attrInfo;

} cy_stc_ble_customs_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    const cy_stc_ble_customc_t *attrInfo;
    
    /** The number supported Custom Services as client */
    uint8_t serviceCount;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_customc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

cy_en_ble_api_result_t Cy_BLE_CUSTOMS_Init(const cy_stc_ble_customs_config_t *config);
cy_en_ble_api_result_t Cy_BLE_CUSTOMC_Init(const cy_stc_ble_customc_config_t *config);


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_customs_config_t *cy_ble_customsConfigPtr;
extern const cy_stc_ble_customc_config_t *cy_ble_customcConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_CUSTOM_H */

/* [] END OF FILE */
