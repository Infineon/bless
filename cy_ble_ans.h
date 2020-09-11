/***************************************************************************//**
* \file cy_ble_ans.h
* \version 3.50
*
* \brief
*  Contains the function prototypes and constants for the Alert Notification
*  service.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_ANS_H
#define CY_BLE_ANS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

/* The C binding of definitions if building with the C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

#define CY_BLE_ANS_INVALID_CHAR_INDEX              (0xFFu)

/** Alert Notification Characteristic Command ID */
#define CY_BLE_ANS_EN_NEW_ALERT_NTF                (0x00u) /** Enable New Incoming Alert Notification */
#define CY_BLE_ANS_EN_UNREAD_ALERT_STATUS_NTF      (0x01u) /** Enable Unread Category Status Notification */
#define CY_BLE_ANS_DIS_NEW_ALERT_NTF               (0x02u) /** Disable New Incoming Alert Notification */
#define CY_BLE_ANS_DIS_UNREAD_ALERT_STATUS_NTF     (0x03u) /** Disable Unread Category Status Notification */
#define CY_BLE_ANS_IMM_NEW_ALERT_NTF               (0x04u) /** Notify New Incoming Alert immediately */
#define CY_BLE_ANS_IMM_UNREAD_ALERT_STATUS_NTF     (0x05u) /** Notify Unread Category Status immediately */

/* Alert Notification Characteristic Category ID */
#define CY_BLE_ANS_CAT_ID_SIMPLE_ALERT             (0x00u) /* Simple Alert: General text alert or non-text alert */
#define CY_BLE_ANS_CAT_ID_EMAIL                    (0x01u) /* Email: Alert when Email messages arrives */
#define CY_BLE_ANS_CAT_ID_NEWS                     (0x02u) /* News: News feeds such as RSS, Atom */
#define CY_BLE_ANS_CAT_ID_CALL                     (0x03u) /* Call: Incoming call */
#define CY_BLE_ANS_CAT_ID_MISSED_CALL              (0x04u) /* Missed call: Missed Call */
#define CY_BLE_ANS_CAT_ID_SMS_MMS                  (0x05u) /* SMS/MMS: SMS/MMS message arrives */
#define CY_BLE_ANS_CAT_ID_VOICE_MAIL               (0x06u) /* Voice mail: Voice mail */
#define CY_BLE_ANS_CAT_ID_SCHEDULE                 (0x07u) /* Schedule: Alert occurred on calendar, planner */
#define CY_BLE_ANS_CAT_ID_HI_PRFIORITIZED_ALERT    (0x08u) /* High Prioritized Alert: Alert that should be handled as
                                                            * high priority */
#define CY_BLE_ANS_CAT_ID_INSTANT_MESSAGE          (0x09u) /* Instant Message: Alert for incoming instant messages */
#define CY_BLE_ANS_CAT_ID_ALL                      (0xFFu) /* Category ID - All categories */
#define CY_BLE_ANS_CHAR_VALUE_OFFSET               (0x02u)


/*******************************************************************************
* Data Types
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_ANS_definitions
 * \{
 */
/** ANS Characteristic indexes */
typedef enum
{
    CY_BLE_ANS_SUPPORTED_NEW_ALERT_CAT,              /**< Supported New Alert Category Characteristic index */
    CY_BLE_ANS_NEW_ALERT,                            /**< New Alert Characteristic index */
    CY_BLE_ANS_SUPPORTED_UNREAD_ALERT_CAT,           /**< Supported Unread Alert Category Characteristic index */
    CY_BLE_ANS_UNREAD_ALERT_STATUS,                  /**< Unread Alert Status Characteristic index */
    CY_BLE_ANS_ALERT_NTF_CONTROL_POINT,              /**< Alert Notification Control Point Characteristic index */
    CY_BLE_ANS_CHAR_COUNT                            /**< Total Count of ANS Characteristics */
}cy_en_ble_ans_char_index_t;

/** ANS Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_ANS_CCCD,                                  /**< Client Characteristic Configuration Descriptor index */
    CY_BLE_ANS_DESCR_COUNT                            /**< Total Count of Descriptors */
}cy_en_ble_ans_descr_index_t;

/** Alert Notification Service Characteristic Value Parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t   connHandle;            /**< Peer Device Handle */
    cy_en_ble_ans_char_index_t charIndex;             /**< Index of Alert Notification Service Characteristic */
    cy_stc_ble_gatt_value_t    *value;                /**< Pointer to Characteristic Value */
} cy_stc_ble_ans_char_value_t;

/** Alert Notification Service Characteristic descriptor Value parameter structure */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;           /**< Connection handle */
    cy_en_ble_ans_char_index_t  charIndex;            /**< Characteristic index of Service */
    cy_en_ble_ans_descr_index_t descrIndex;           /**< Service Characteristic descriptor index */
    cy_stc_ble_gatt_value_t     *value;               /**< Pointer to the Value of the Service Characteristic
                                                       *   Descriptor value */
} cy_stc_ble_ans_descr_value_t;

/** ANS Characteristic with descriptors */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t charHandle;                          /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_ANS_DESCR_COUNT]; /**< Handle of Descriptor */
} cy_stc_ble_anss_char_t;

/** The structure with the Alert Notification Service attribute handles */
typedef struct
{
    /** The Alert Notification Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** The array of the Alert Notification Service Characteristics + Descriptors handles */
    cy_stc_ble_anss_char_t       charInfo[CY_BLE_ANS_CHAR_COUNT];
} cy_stc_ble_anss_t;

/** Service Full Characteristic Information type */
typedef struct
{
    /** Characteristic Handle + Properties */
    cy_stc_ble_srvr_char_info_t  charInfo;

    /** End Handle of the Characteristic */
    cy_ble_gatt_db_attr_handle_t endHandle;

    /** Characteristic descriptors Handles */
    cy_ble_gatt_db_attr_handle_t descriptors[CY_BLE_ANS_DESCR_COUNT];
} cy_stc_ble_srvr_full_char_info_t;

/** The structure with discovered attributes information of the alert notification service */
typedef struct
{
    /** The structure with Characteristic handles + properties of the alert notification service */
    cy_stc_ble_srvr_full_char_info_t characteristics[CY_BLE_ANS_CHAR_COUNT];
} cy_stc_ble_ansc_t;

/** Service Configuration Structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_anss_t *attrInfo;

} cy_stc_ble_anss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes information */
    cy_stc_ble_ansc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_ansc_config_t;

/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/

/**
 * \addtogroup group_ble_service_api_ANS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_ANSS_Init(const cy_stc_ble_anss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_ANSC_Init(const cy_stc_ble_ansc_config_t *config);
void Cy_BLE_ANS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_ANS_server
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_ANSS_SetCharacteristicValue(cy_en_ble_ans_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_ANSS_GetCharacteristicValue(cy_en_ble_ans_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_ANSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_ans_char_index_t charIndex,
                                                               cy_en_ble_ans_descr_index_t
                                                               descrIndex, uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_ANSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_ans_char_index_t charIndex, uint8_t attrSize,
                                                    uint8_t *attrValue);
/** \} */

/**
 * \addtogroup group_ble_service_api_ANS_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_ANSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_ans_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_ANSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_ans_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_ANSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_ans_char_index_t charIndex,
                                                               cy_en_ble_ans_descr_index_t
                                                               descrIndex, uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_ANSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_ans_char_index_t charIndex, uint8_t descrIndex);
/** \} */


/*******************************************************************************
* External Data References
*******************************************************************************/

extern const cy_stc_ble_anss_config_t *cy_ble_anssConfigPtr;
extern const cy_stc_ble_ansc_config_t *cy_ble_anscConfigPtr;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_ANS_H */

/* [] END OF FILE */
