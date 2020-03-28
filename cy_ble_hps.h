/***************************************************************************//**
* \file cy_ble_hps.h
* \version 3.40
*
* \brief
*  Contains the function prototypes and constants for HTTP Proxy service.
*
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_HPS_H
#define CY_BLE_HPS_H

#include "cy_ble_event_handler.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

/* HTTP Status Code Data Status bits  */
#define CY_BLE_HPS_HTTP_HEADERS_RECEIVED     (0x01u)
#define CY_BLE_HPS_HTTP_HEADERS_TRUNCATED    (0x02u)
#define CY_BLE_HPS_HTTP_BODY_RECEIVED        (0x04u)
#define CY_BLE_HPS_HTTP_BODY_TRUNCATED       (0x08u)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_HPS_definitions
 * \{
 */
/** HPS Characteristic indexes */
typedef enum
{
    CY_BLE_HPS_URI,                              /**< Universal Resource Identifier Characteristics index */
    CY_BLE_HPS_HTTP_HEADERS,                     /**< HTTP Headers Characteristics index */
    CY_BLE_HPS_HTTP_ENTITY_BODY,                 /**< HTTP Entity Body Characteristics index */
    CY_BLE_HPS_HTTP_CP,                          /**< HTTP Control Point Characteristics index */
    CY_BLE_HPS_HTTP_STATUS_CODE,                 /**< HTTP Status Code Characteristics index */
    CY_BLE_HPS_HTTPS_SECURITY,                   /**< HTTPS Security Characteristics index */
    CY_BLE_HPS_CHAR_COUNT                        /**< Total Count of HPS Characteristics */
}cy_en_ble_hps_char_index_t;

/** HPS Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_HPS_CCCD,                              /**< Client Characteristic Configuration Descriptor index */
    CY_BLE_HPS_DESCR_COUNT                        /**< Total Count of Descriptors */
} cy_en_ble_hps_descr_index_t;

/** HPS Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;        /**< Peer Device Handle */
    cy_en_ble_hps_char_index_t charIndex;         /**< Index of Service Characteristic */
    cy_en_ble_gatt_err_code_t  gattErrorCode;     /**< Error code received from application (optional) */
    cy_stc_ble_gatt_value_t    *value;            /**< Characteristic Value */
} cy_stc_ble_hps_char_value_t;

/** HPS Characteristic descriptor value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;        /**< Peer Device Handle */
    cy_en_ble_hps_char_index_t  charIndex;         /**< Index of Service Characteristic */
    cy_en_ble_hps_descr_index_t descrIndex;        /**< Index of Descriptor */
    cy_en_ble_gatt_err_code_t   gattErrorCode;     /**< Error code received from application (optional) */
    cy_stc_ble_gatt_value_t     *value;            /**< Characteristic Value */
} cy_stc_ble_hps_descr_value_t;

/** Structure with HTTP Proxy Service attribute handles */
typedef struct
{
    /** Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t charHandle;
    /** Array of Descriptor Handles */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_HPS_DESCR_COUNT];
} cy_stc_ble_hpss_char_t;

/** HPS Characteristic with Descriptors Handles */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t serviceHandle;                   /**< HTTP Proxy Service Handle */
    cy_stc_ble_hpss_char_t       charInfo[CY_BLE_HPS_CHAR_COUNT]; /**< Array of Characteristics and Descriptors Handles */
} cy_stc_ble_hpss_t;

/** HPS Service full Characteristic information structure */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t valueHandle;                         /**< Handle of Characteristic Value */
    uint8_t                      properties;                          /**< Properties for Value Field */
    cy_ble_gatt_db_attr_handle_t endHandle;                           /**< End handle of Characteristic */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_HPS_DESCR_COUNT]; /**< Array of Descriptor Handles */
} cy_stc_ble_hpsc_char_t;

/** Structure with Discovered Attributes Information of HTTP Proxy Service */
typedef struct
{
    /** HTTP Proxy Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;
    /** HTTP Proxy Service Characteristics info structure */
    cy_stc_ble_hpsc_char_t       charInfo[CY_BLE_HPS_CHAR_COUNT];
} cy_stc_ble_hpsc_t;

/** HTTP Requests */
typedef enum
{
    CY_BLE_HPS_HTTP_GET = 0x01u,                   /**< HTTP GET Request */
    CY_BLE_HPS_HTTP_HEAD,                          /**< HTTP HEAD Request */
    CY_BLE_HPS_HTTP_POST,                          /**< HTTP POST Request */
    CY_BLE_HPS_HTTP_PUT,                           /**< HTTP PUT Request */
    CY_BLE_HPS_HTTP_DELETE,                        /**< HTTP DELETE Request */
    CY_BLE_HPS_HTTPS_GET,                          /**< HTTS GET Request */
    CY_BLE_HPS_HTTPS_HEAD,                         /**< HTTPS HEAD Request */
    CY_BLE_HPS_HTTPS_POST,                         /**< HTTPS POST Request */
    CY_BLE_HPS_HTTPS_PUT,                          /**< HTTPS PUT Request */
    CY_BLE_HPS_HTTPS_DELETE,                       /**< HTTPS DELETE Request */
    CY_BLE_HPS_HTTP_REQ_CANCEL                     /**< HTTP CANCEL Request */
}cy_en_ble_hps_http_request_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_hpss_t *attrInfo;

} cy_stc_ble_hpss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_hpsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_hpsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/** \addtogroup group_ble_service_api_HPS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_HPSS_Init(const cy_stc_ble_hpss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_HPSC_Init(const cy_stc_ble_hpsc_config_t *config);
void Cy_BLE_HPS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_HPS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_HPSS_SetCharacteristicValue(cy_en_ble_hps_char_index_t charIndex, uint16_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HPSS_GetCharacteristicValue(cy_en_ble_hps_char_index_t charIndex, uint16_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HPSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hps_char_index_t charIndex,
                                                               cy_en_ble_hps_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HPSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hps_char_index_t charIndex,
                                                               cy_en_ble_hps_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HPSS_SendNotification(cy_stc_ble_conn_handle_t connHandle, cy_en_ble_hps_char_index_t
                                                    charIndex, uint8_t attrSize, uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_HPS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_HPSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_hps_char_index_t charIndex, uint16_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HPSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_hps_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_HPSC_SetLongCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                              cy_en_ble_hps_char_index_t charIndex, uint16_t attrSize,
                                                              uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HPSC_GetLongCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                              cy_en_ble_hps_char_index_t charIndex, uint16_t attrSize,
                                                              uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HPSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hps_char_index_t charIndex,
                                                               cy_en_ble_hps_descr_index_t descrIndex, uint8_t attrSize,
                                                               uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HPSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hps_char_index_t charIndex,
                                                               cy_en_ble_hps_descr_index_t descrIndex);
/** \} */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_hpss_config_t *cy_ble_hpssConfigPtr;
extern const cy_stc_ble_hpsc_config_t *cy_ble_hpscConfigPtr;
extern uint8_t * cy_ble_hpscRdLongBuffPtr;
extern uint16_t cy_ble_hpscRdLongBuffLen;
extern uint16_t cy_ble_hpscCurrLen;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_HPS_H */

/* [] END OF FILE */
