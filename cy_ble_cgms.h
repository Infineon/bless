/***************************************************************************//**
* \file cy_ble_cgms.h
* \version 3.60
*
* \brief
*  This file contains the function prototypes and constants used in the
*  Continuous Glucose Monitoring service.
*
********************************************************************************
* \copyright
* Copyright 2017-2021, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#ifndef CY_BLE_CGMS_H
#define CY_BLE_CGMS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
* API Constants
*******************************************************************************/

#define CY_BLE_CGMS_FLAG_PROCESS         (0x01u)
#define CY_BLE_CGMS_RACP_OPCODE_ABORT    (0x03u)


/*******************************************************************************
* Data Types
*******************************************************************************/
/**
 * \addtogroup group_ble_service_api_CGMS_definitions
 * \{
 */

/** Service Characteristics indexes */
typedef enum
{
    CY_BLE_CGMS_CGMT,           /**< CGM Measurement Characteristic index */
    CY_BLE_CGMS_CGFT,           /**< CGM Feature Characteristic index */
    CY_BLE_CGMS_CGST,           /**< CGM Status Characteristic index */
    CY_BLE_CGMS_SSTM,           /**< CGM Session Start Time Characteristic index */
    CY_BLE_CGMS_SRTM,           /**< CGM Session Run Time Characteristic index */
    CY_BLE_CGMS_RACP,           /**< Record Access Control Point Characteristic index */
    CY_BLE_CGMS_SOCP,           /**< CGM Specific Ops Control Point Characteristic index */
    CY_BLE_CGMS_CHAR_COUNT      /**< Total Count of CGMS Characteristics */
}cy_en_ble_cgms_char_index_t;

/** Service Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_CGMS_CCCD,           /**< Client Characteristic Configuration descriptor index */
    CY_BLE_CGMS_DESCR_COUNT     /**< Total Count of CGMS descriptors */
}cy_en_ble_cgms_descr_index_t;

/** Characteristic with descriptors type */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t charHandle;                              /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_CGMS_DESCR_COUNT];    /**< Handles of Descriptors */
} cy_stc_ble_cgmss_char_t;

/** Structure with CGM Service attribute handles */
typedef struct
{
    /** CGM Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** CGM Service Characteristics info array */
    cy_stc_ble_cgmss_char_t      charInfo[CY_BLE_CGMS_CHAR_COUNT];
} cy_stc_ble_cgmss_t;

/** CGM Client Characteristic structure type */
typedef struct
{
    /** Properties for Value Field */
    uint8_t                      properties;

    /** Handle of Server database attribute value entry */
    cy_ble_gatt_db_attr_handle_t valueHandle;

    /** Characteristics Descriptors Handles */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_CGMS_DESCR_COUNT];

    /** Characteristic End Handle */
    cy_ble_gatt_db_attr_handle_t endHandle;
}cy_stc_ble_cgmsc_char_t;

/** CGM Service structure type */
typedef struct
{
    /** Characteristics handle + properties array */
    cy_stc_ble_cgmsc_char_t charInfo[CY_BLE_CGMS_CHAR_COUNT];
}cy_stc_ble_cgmsc_t;


/** CGM Service Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;                     /**< Peer Device Handle */
    cy_en_ble_cgms_char_index_t charIndex;                      /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t     *value;                         /**< Characteristic Value */
    cy_en_ble_gatt_err_code_t   gattErrorCode;                  /**< GATT error code for access control */
} cy_stc_ble_cgms_char_value_t;

/** CGM Service Characteristic descriptor value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t     connHandle;                    /**< Peer Device Handle */
    cy_en_ble_cgms_char_index_t  charIndex;                     /**< Index of Service Characteristic */
    cy_en_ble_cgms_descr_index_t descrIndex;                    /**< Index of Service Characteristic descriptor */
    cy_stc_ble_gatt_value_t      *value;                        /**< Descriptor value */
} cy_stc_ble_cgms_descr_value_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_cgmss_t *attrInfo;

} cy_stc_ble_cgmss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_cgmsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_cgmsc_config_t;
/** \} */

/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_CGMS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_CGMSS_Init(const cy_stc_ble_cgmss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_CGMSC_Init(const cy_stc_ble_cgmsc_config_t *config);
void Cy_BLE_CGMS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_CGMS_server
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_CGMSS_SetCharacteristicValue(cy_en_ble_cgms_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CGMSS_GetCharacteristicValue(cy_en_ble_cgms_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CGMSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_cgms_char_index_t charIndex,
                                                                cy_en_ble_cgms_descr_index_t descrIndex,
                                                                uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CGMSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_cgms_char_index_t charIndex,
                                                                cy_en_ble_cgms_descr_index_t descrIndex,
                                                                uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CGMSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                     cy_en_ble_cgms_char_index_t charIndex, uint8_t attrSize,
                                                     uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CGMSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                   cy_en_ble_cgms_char_index_t charIndex, uint8_t attrSize,
                                                   uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_CGMS_client
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_CGMSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_cgms_char_index_t charIndex,
                                                           uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CGMSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_cgms_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_CGMSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_cgms_char_index_t charIndex,
                                                                cy_en_ble_cgms_descr_index_t descrIndex, uint8_t attrSize,
                                                                uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CGMSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_cgms_char_index_t charIndex,
                                                                cy_en_ble_cgms_descr_index_t descrIndex);

/** \} */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_cgmss_config_t *cy_ble_cgmssConfigPtr;
extern const cy_stc_ble_cgmsc_config_t *cy_ble_cgmscConfigPtr;
extern uint8_t cy_ble_cgmsFlag;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_CGMS_H */

/* [] END OF FILE */
