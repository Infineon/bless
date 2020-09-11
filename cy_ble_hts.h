/***************************************************************************//**
* \file cy_ble_hts.h
* \version 3.50
*
* \brief
*  Contains the function prototypes and constants for Health Thermometer service.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_HTS_H
#define CY_BLE_HTS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

/* Valid Range descriptor length */
#define CY_BLE_HTS_VRD_LEN                     (0x04u)

/* Health Thermometer measurement flag bits */
#define CY_BLE_HTS_MEAS_FLAG_TEMP_UNITS_BIT    (0x01u << 0u)
#define CY_BLE_HTS_MEAS_FLAG_TIME_STAMP_BIT    (0x01u << 1u)
#define CY_BLE_HTS_MEAS_FLAG_TEMP_TYPE_BIT     (0x01u << 2u)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_HTS_definitions
 * \{
 */
/** HTS Characteristic indexes */
typedef enum
{
    CY_BLE_HTS_TEMP_MEASURE,                         /**< Temperature Measurement Characteristic index */
    CY_BLE_HTS_TEMP_TYPE,                            /**< Temperature Type Characteristic index */
    CY_BLE_HTS_INTERM_TEMP,                          /**< Intermediate Temperature Characteristic index*/
    CY_BLE_HTS_MEASURE_INTERVAL,                     /**< Measurement Interval Characteristic index */
    CY_BLE_HTS_CHAR_COUNT                            /**< Total Count of HTS Characteristics */
} cy_en_ble_hts_char_index_t;

/** HTS Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_HTS_CCCD,                                 /**< Client Characteristic Configuration descriptor index */
    CY_BLE_HTS_VRD,                                  /**< Valid Range descriptor index */
    CY_BLE_HTS_DESCR_COUNT                           /**< Total Count of Descriptors */
} cy_en_ble_hts_descr_index_t;

/** HTS Characteristic with descriptors */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t charHandle;                          /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_HTS_DESCR_COUNT]; /**< Handle of Descriptor */
} cy_stc_ble_htss_char_t;

/** Structure with Health Thermometer Service attribute handles */
typedef struct
{
    /** Health Thermometer Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Health Thermometer Service Characteristic handles */
    cy_stc_ble_htss_char_t       charInfo[CY_BLE_HTS_CHAR_COUNT];
} cy_stc_ble_htss_t;

/** HTS Characteristic with descriptors */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_HTS_DESCR_COUNT]; /**< Handle of Descriptor */
    cy_ble_gatt_db_attr_handle_t valueHandle;                         /**< Handle of Report Characteristic value */
    cy_ble_gatt_db_attr_handle_t endHandle;                           /**< End handle of Characteristic */
    uint8_t                      properties;                          /**< Properties for Value Field */
} cy_stc_ble_htsc_char_t;

/** Structure with Discovered Attributes Information of Health Thermometer Service */
typedef struct
{
    /** Characteristics handles array */
    cy_stc_ble_htsc_char_t charInfo[CY_BLE_HTS_CHAR_COUNT];
} cy_stc_ble_htsc_t;

/** HTS Characteristic Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;                           /**< Peer Device Handle */
    cy_en_ble_hts_char_index_t charIndex;                            /**< Index of Service Characteristic */
    cy_stc_ble_gatt_value_t    *value;                               /**< Characteristic Value */
} cy_stc_ble_hts_char_value_t;

/** HTS Characteristic descriptor value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;                          /**< Peer Device Handle */
    cy_en_ble_hts_char_index_t  charIndex;                           /**< Index of Service Characteristic */
    cy_en_ble_hts_descr_index_t descrIndex;                          /**< Index of Descriptor */
    cy_stc_ble_gatt_value_t     *value;                              /**< Characteristic Value */
} cy_stc_ble_hts_descr_value_t;


/** The IEEE-11073 FLOAT-Type is defined as a 32-bit value
 * with a 24-bit mantissa and an 8-bit exponent. */
typedef struct
{
    int8_t  exponent;                                                /**< Base 10 exponent */
    int32_t mantissa;                                                /**< Mantissa, should be using only 24 bits */
} cy_ble_hts_float32;

/** Temperature Type measurement indicates where the temperature was measured */
typedef enum
{
    CY_BLE_HTS_TEMP_TYPE_ARMPIT = 0x01u,                             /**< Armpit */
    CY_BLE_HTS_TEMP_TYPE_BODY,                                       /**< Body (general) */
    CY_BLE_HTS_TEMP_TYPE_EAR,                                        /**< Ear (usually ear lobe) */
    CY_BLE_HTS_TEMP_TYPE_FINGER,                                     /**< Finger */
    CY_BLE_HTS_TEMP_TYPE_GI_TRACT,                                   /**< Gastro-intestinal Tract */
    CY_BLE_HTS_TEMP_TYPE_MOUTH,                                      /**< Mouth */
    CY_BLE_HTS_TEMP_TYPE_RECTUM,                                     /**< Rectum */
    CY_BLE_HTS_TEMP_TYPE_TOE,                                        /**< Toe */
    CY_BLE_HTS_TEMP_TYPE_TYMPANUM                                    /**< Tympanum (ear drum) */
} cy_stc_ble_hts_temp_type_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_htss_t *attrInfo;

} cy_stc_ble_htss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_htsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_htsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_HTS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_HTSS_Init(const cy_stc_ble_htss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_HTSC_Init(const cy_stc_ble_htsc_config_t *config);
void Cy_BLE_HTS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */


/**
 * \addtogroup group_ble_service_api_HTS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_HTSS_SetCharacteristicValue(cy_en_ble_hts_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HTSS_GetCharacteristicValue(cy_en_ble_hts_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HTSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hts_char_index_t charIndex,
                                                               cy_en_ble_hts_descr_index_t descrIndex, uint8_t attrSize,
                                                               uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HTSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hts_char_index_t charIndex,
                                                               cy_en_ble_hts_descr_index_t descrIndex, uint8_t attrSize,
                                                               uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HTSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_hts_char_index_t charIndex, uint8_t attrSize,
                                                    uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HTSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_hts_char_index_t charIndex, uint8_t attrSize,
                                                  uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_HTS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_HTSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_hts_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HTSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_hts_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_HTSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hts_char_index_t charIndex,
                                                               cy_en_ble_hts_descr_index_t descrIndex, uint8_t attrSize,
                                                               uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_HTSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hts_char_index_t charIndex,
                                                               cy_en_ble_hts_descr_index_t descrIndex);
/** \} */

cy_ble_gatt_db_attr_handle_t Cy_BLE_HTSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                           cy_en_ble_hts_char_index_t charIndex, 
                                                                           cy_en_ble_hts_descr_index_t descrIndex);

cy_ble_gatt_db_attr_handle_t Cy_BLE_HTSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                      cy_en_ble_hts_char_index_t charIndex);


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_htss_config_t *cy_ble_htssConfigPtr;
extern const cy_stc_ble_htsc_config_t *cy_ble_htscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_HTS_H */

/* [] END OF FILE */
