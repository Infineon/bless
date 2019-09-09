/***************************************************************************//**
* \file cy_ble_wpts.h
* \version 3.20
*
* \brief
*  Contains the function prototypes and constants for Wireless Power Transfer
*  service.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_WPTS_H
#define CY_BLE_WPTS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

/* WPT service handle offsets */
#define CY_BLE_WPTS_WPT_SERVICE_OFFSET                   (0x00u)
#define CY_BLE_WPTS_PRU_CONTROL_CHAR_DECL_OFFSET         (0x01u)
#define CY_BLE_WPTS_PRU_CONTROL_CHAR_VALUE_OFFSET        (0x02u)
#define CY_BLE_WPTS_PTU_STATIC_PAR_CHAR_DECL_OFFSET      (0x03u)
#define CY_BLE_WPTS_PTU_STATIC_PAR_CHAR_VALUE_OFFSET     (0x04u)
#define CY_BLE_WPTS_PRU_ALERT_PAR_CHAR_DECL_OFFSET       (0x05u)
#define CY_BLE_WPTS_PRU_ALERT_PAR_CHAR_VALUE_OFFSET      (0x06u)
#define CY_BLE_WPTS_PRU_ALERT_PAR_CHAR_CCCD_OFFSET       (0x07u)
#define CY_BLE_WPTS_PRU_STATIC_PAR_CHAR_DECL_OFFSET      (0x08u)
#define CY_BLE_WPTS_PRU_STATIC_PAR_CHAR_VALUE_OFFSET     (0x09u)
#define CY_BLE_WPTS_PRU_DYNAMIC_PAR_CHAR_DECL_OFFSET     (0x0Au)
#define CY_BLE_WPTS_PRU_DYNAMIC_PAR_CHAR_VALUE_OFFSET    (0x0Bu)


/*******************************************************************************
* Data Types
*******************************************************************************/
/**
 * \addtogroup group_ble_service_api_WPTS_definitions
 * \{
 */
/** WPTS Characteristic indexes */
typedef enum
{
    CY_BLE_WPTS_PRU_CONTROL,                     /**< PRU Control Characteristic index */
    CY_BLE_WPTS_PTU_STATIC_PAR,                  /**< PTU Static Parameter Characteristic index */
    CY_BLE_WPTS_PRU_ALERT,                       /**< PRU Alert Characteristic index */
    CY_BLE_WPTS_PRU_STATIC_PAR,                  /**< PRU Static Parameter Characteristic index */
    CY_BLE_WPTS_PRU_DYNAMIC_PAR,                 /**< PRU Dynamic Parameter Characteristic index */
    CY_BLE_WPTS_CHAR_COUNT                       /**< Total Count of WPTS Characteristics */
}cy_en_ble_wpts_char_index_t;

/** WPTS Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_WPTS_CCCD,                            /**< Client Characteristic Configuration Descriptor index */
    CY_BLE_WPTS_DESCR_COUNT                      /**< Total Count of Descriptors */
}cy_en_ble_wpts_descr_index_t;

/** Characteristic with Descriptors */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t charHandle;                           /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_WPTS_DESCR_COUNT]; /**< Handle of Descriptor */
} cy_stc_ble_wptss_char_t;

/** WPTS Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;         /**< Peer Device Handle */
    cy_en_ble_wpts_char_index_t charIndex;          /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t     *value;             /**< Characteristic Value */
} cy_stc_ble_wpts_char_value_t;

/** WPTS Characteristic descriptor value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t     connHandle;        /**< Peer Device Handle */
    cy_en_ble_wpts_char_index_t  charIndex;         /**< Index of Service Characteristic */
    cy_en_ble_wpts_descr_index_t descrIndex;        /**< Index of Descriptor */
    cy_stc_ble_gatt_value_t      *value;            /**< Characteristic Value */
} cy_stc_ble_wpts_descr_value_t;

/** WPTS Server Characteristic structure type */
typedef struct
{
    /** Wireless Power Transfer Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Wireless Power Transfer Characteristic handles */
    cy_stc_ble_wptss_char_t      charInfo[CY_BLE_WPTS_CHAR_COUNT];
} cy_stc_ble_wptss_t;

/** Characteristic information structure */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_WPTS_DESCR_COUNT]; /**< Handles of Descriptors */
    cy_ble_gatt_db_attr_handle_t valueHandle;                          /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t endHandle;                            /**< End Handle of a Characteristic */
    uint8_t                      properties;                           /**< Properties for Value Field */
} cy_stc_ble_wptsc_char_t;

/** WPTS Characteristic with descriptors */
typedef struct
{
    /** Wireless Power Transfer Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;
    /** Wireless Power Transfer Service Characteristics info structure */
    cy_stc_ble_wptsc_char_t      charInfo[CY_BLE_WPTS_CHAR_COUNT];
} cy_stc_ble_wptsc_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_wptss_t *attrInfo;

} cy_stc_ble_wptss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_wptsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_wptsc_config_t;

/** \} */

/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** \addtogroup group_ble_service_api_WPTS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_WPTSS_Init(const cy_stc_ble_wptss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_WPTSC_Init(const cy_stc_ble_wptsc_config_t *config);
void Cy_BLE_WPTS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_WPTS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_WPTSS_SetCharacteristicValue(cy_en_ble_wpts_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_WPTSS_GetCharacteristicValue(cy_en_ble_wpts_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_WPTSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_wpts_char_index_t charIndex,
                                                                cy_en_ble_wpts_descr_index_t descrIndex,
                                                                uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_WPTSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_wpts_char_index_t charIndex,
                                                                cy_en_ble_wpts_descr_index_t descrIndex,
                                                                uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_WPTSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                     cy_en_ble_wpts_char_index_t charIndex, uint8_t attrSize,
                                                     uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_WPTSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                   cy_en_ble_wpts_char_index_t charIndex, uint8_t attrSize,
                                                   uint8_t *attrValue);

/** \} */

/**
 * \addtogroup group_ble_service_api_WPTS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_WPTSC_Discovery(cy_ble_gatt_db_attr_handle_t servHandle, 
                                              cy_stc_ble_conn_handle_t connHandle);

cy_en_ble_api_result_t Cy_BLE_WPTSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_wpts_char_index_t charIndex, uint8_t attrSize,
                                                           uint8_t * attrValue);

cy_en_ble_api_result_t Cy_BLE_WPTSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_wpts_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_WPTSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_wpts_char_index_t charIndex,
                                                                cy_en_ble_wpts_descr_index_t descrIndex,
                                                                uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_WPTSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_wpts_char_index_t charIndex,
                                                                cy_en_ble_wpts_descr_index_t descrIndex);
/** \} */


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

/** \cond IGNORE */
cy_ble_gatt_db_attr_handle_t Cy_BLE_WPTSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                        cy_en_ble_wpts_char_index_t charIndex);

cy_ble_gatt_db_attr_handle_t Cy_BLE_WPTSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_wpts_char_index_t charIndex,
                                                                            cy_en_ble_wpts_descr_index_t descrIndex);
/** \endcond */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_wptss_config_t *cy_ble_wptssConfigPtr;
extern const cy_stc_ble_wptsc_config_t *cy_ble_wptscConfigPtr;
extern const cy_stc_ble_uuid128_t cy_ble_wptscCharUuid128[CY_BLE_WPTS_CHAR_COUNT];


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_WPTS_H */

/* [] END OF FILE */
