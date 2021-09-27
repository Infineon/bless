/***************************************************************************//**
* \file cy_ble_cps.h
* \version 3.60
*
* \brief
*  Contains the function prototypes and constants for Cycling Power service.
*
********************************************************************************
* \copyright
* Copyright 2017-2021, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef CY_BLE_CPS_H
#define CY_BLE_CPS_H

#include "cy_ble.h"

#if defined(CY_IP_MXBLESS)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*******************************************************************************
* API Constants
*******************************************************************************/

/* Control Point Procedure Timeout */
#define CY_BLE_CPS_CP_PROCEDURE_TIMEOUT              (30u)           /* Seconds */

#define CY_BLE_CPS_SAMLING_RATE_DEFAULT              (25u)           /* In Hertz with a resolution of 1 Hertz */

/* ConnIntv = 1/SampleRate(Hz)*1000ms/1.25ms(1 frame is 1.25 ms) = 800 / SampleRate */
#define CY_BLE_CPS_SAMLING_RATE_TO_CONN_INTV         (800u)

/* Server state flags */
#define CY_BLE_CPSS_FLAG_CP_PROCEDURE_IN_PROGRESS    (0x01u)
#define CY_BLE_CPSS_FLAG_PV_PROCEDURE_IN_PROGRESS    (0x02u)
#define CY_BLE_CPSS_FLAG_BROADCAST_IN_PROGRESS       (0x04u)
#define CY_BLE_CPSS_FLAG_START_BROADCAST             (0x08u)
#define CY_BLE_CPSS_FLAG_STOP_BROADCAST              (0x10u)

/* Client state flags */
#define CY_BLE_CPSC_FLAG_CP_PROCEDURE_IN_PROGRESS    (0x01u)
#define CY_BLE_CPSC_FLAG_OBSERVE_IN_PROGRESS         (0x02u)
#define CY_BLE_CPSC_FLAG_START_OBSERVE               (0x04u)
#define CY_BLE_CPSC_FLAG_STOP_OBSERVE                (0x08u)


/**
 * \addtogroup group_ble_service_api_CPS_definitions
 * \{
 */
/* Cycling Power Feature bits */
#define CY_BLE_CPS_CPF_PEDAL_BIT                      (0x01u << 0u)  /**< Pedal Power Balance Supported */
#define CY_BLE_CPS_CPF_TORQUE_BIT                     (0x01u << 1u)  /**< Accumulated Torque Supported */
#define CY_BLE_CPS_CPF_WHEEL_BIT                      (0x01u << 2u)  /**< Wheel Revolution Data Supported */
#define CY_BLE_CPS_CPF_CRANK_BIT                      (0x01u << 3u)  /**< Crank Revolution Data Supported */
#define CY_BLE_CPS_CPF_MAGNITUDES_BIT                 (0x01u << 4u)  /**< Extreme Magnitudes Supported */
#define CY_BLE_CPS_CPF_ANGLES_BIT                     (0x01u << 5u)  /**< Extreme Angles Supported */
#define CY_BLE_CPS_CPF_DEAD_SPOT_BIT                  (0x01u << 6u)  /**< Top and Bottom Dead Spot Angles Supported */
#define CY_BLE_CPS_CPF_ENERGY_BIT                     (0x01u << 7u)  /**< Accumulated Energy Supported */
#define CY_BLE_CPS_CPF_OFFSET_INDICATOR_BIT           (0x01u << 8u)  /**< Offset Compensation Indicator Supported */
#define CY_BLE_CPS_CPF_OFFSET_BIT                     (0x01u << 9u)  /**< Offset Compensation Supported */
#define CY_BLE_CPS_CPF_CPM_CONTENT_MASKING_BIT        (0x01u << 10u) /**< Cycling Power Measurement Characteristic
                                                                      *    Content Masking Supported */
#define CY_BLE_CPS_CPF_MULTIPLE_SENSOR_BIT            (0x01u << 11u) /**< Multiple Sensor Locations Supported */
#define CY_BLE_CPS_CPF_CRANK_LEN_ADJ_BIT              (0x01u << 12u) /**< Crank Length Adjustment Supported */
#define CY_BLE_CPS_CPF_CHAIN_LEN_ADJ_BIT              (0x01u << 13u) /**< Chain Length Adjustment Supported */
#define CY_BLE_CPS_CPF_CHAIN_WEIGHT_ADJ_BIT           (0x01u << 14u) /**< Chain Weight Adjustment Supported */
#define CY_BLE_CPS_CPF_SPAN_LEN_ADJ_BIT               (0x01u << 15u) /**< Span Length Adjustment Supported */
#define CY_BLE_CPS_CPF_SENSOR_MEASURE_BIT             (0x01u << 16u) /**< Sensor Measurement Context */
#define CY_BLE_CPS_CPF_SENSOR_MEASURE_FORCE           (0x00u << 16u) /**< Sensor Measurement Context: Force based */
#define CY_BLE_CPS_CPF_SENSOR_MEASURE_TORQUE          (0x01u << 16u) /**< Sensor Measurement Context: Torque based */
#define CY_BLE_CPS_CPF_INSTANTANEOUS_DIRECTION_BIT    (0x01u << 17u) /**< Instantaneous Measurement Direction Supported */
#define CY_BLE_CPS_CPF_CALL_DATE_BIT                  (0x01u << 18u) /**< Factory Calibration Date Supported */
#define CY_BLE_CPS_CPF_ENHANCED_OFFSET_BIT            (0x01u << 19u) /**< Enhanced Offset Compensation Supported */
#define CY_BLE_CPS_CPF_DISTRIBUTE_MASK                (0x03u << 20u) /**< Distribute System Support */
#define CY_BLE_CPS_CPF_DISTRIBUTE_UNSPEC              (0x00u << 20u) /**< Distribute System: Unspecified (legacy sensor)*/
#define CY_BLE_CPS_CPF_DISTRIBUTE_NOT_FOR_USE         (0x01u << 20u) /**< Distribute System: Not for use */
#define CY_BLE_CPS_CPF_DISTRIBUTE_CAN_BE_USED         (0x02u << 20u) /**< Distribute System: Can be used */


/* Cycling Power Measurement bits */
#define CY_BLE_CPS_CPM_PEDAL_PRESENT_BIT              (0x01u << 0u)  /**< Pedal Power Balance Present */
#define CY_BLE_CPS_CPM_PEDAL_REFERENCE_BIT            (0x01u << 1u)  /**< Pedal Power Balance Reference */
#define CY_BLE_CPS_CPM_TORQUE_PRESENT_BIT             (0x01u << 2u)  /**< Accumulated Torque Present */
#define CY_BLE_CPS_CPM_TORQUE_SOURCE_BIT              (0x01u << 3u)  /**< Accumulated Torque Source */
#define CY_BLE_CPS_CPM_TORQUE_SOURCE_WHEEL            (0x00u << 3u)  /**< Accumulated Torque Source: Wheel Based */
#define CY_BLE_CPS_CPM_TORQUE_SOURCE_CRANK            (0x01u << 3u)  /**< Accumulated Torque Source: Crank Based */
#define CY_BLE_CPS_CPM_WHEEL_BIT                      (0x01u << 4u)  /**< Wheel Revolution Data Present */
#define CY_BLE_CPS_CPM_CRANK_BIT                      (0x01u << 5u)  /**< Crank Revolution Data Present */
#define CY_BLE_CPS_CPM_FORCE_MAGNITUDES_BIT           (0x01u << 6u)  /**< Extreme Force Magnitudes Present */
#define CY_BLE_CPS_CPM_TORQUE_MAGNITUDES_BIT          (0x01u << 7u)  /**< Extreme Torque Magnitudes Present */
#define CY_BLE_CPS_CPM_ANGLES_BIT                     (0x01u << 8u)  /**< Extreme Angles Present */
#define CY_BLE_CPS_CPM_TOP_DEAD_SPOT_BIT              (0x01u << 9u)  /**< Top Dead Spot Angle Present */
#define CY_BLE_CPS_CPM_BOTTOM_DEAD_SPOT_BIT           (0x01u << 10u) /**< Bottom Dead Spot Angle Present */
#define CY_BLE_CPS_CPM_ENERGY_BIT                     (0x01u << 11u) /**< Accumulated Energy Present */
#define CY_BLE_CPS_CPM_OFFSET_INDICATOR_BIT           (0x01u << 12u) /**< Offset Compensation Indicator */

/* Cycling Control Point Mask of Cycling Power Measurement Characteristic content */
#define CY_BLE_CPS_CP_PEDAL_PRESENT_BIT               (0x01u << 0u)  /**< Pedal Power Balance Turn off */
#define CY_BLE_CPS_CP_TORQUE_PRESENT_BIT              (0x01u << 1u)  /**< Accumulated Torque Turn off */
#define CY_BLE_CPS_CP_WHEEL_BIT                       (0x01u << 2u)  /**< Wheel Revolution Data Turn off */
#define CY_BLE_CPS_CP_CRANK_BIT                       (0x01u << 3u)  /**< Crank Revolution Data Turn off */
#define CY_BLE_CPS_CP_MAGNITUDES_BIT                  (0x01u << 4u)  /**< Extreme Magnitudes Turn off */
#define CY_BLE_CPS_CP_ANGLES_BIT                      (0x01u << 5u)  /**< Extreme Angles Turn off */
#define CY_BLE_CPS_CP_TOP_DEAD_SPOT_BIT               (0x01u << 6u)  /**< Top Dead Spot Angle Turn off */
#define CY_BLE_CPS_CP_BOTTOM_DEAD_SPOT_BIT            (0x01u << 7u)  /**< Bottom Dead Spot Angle Turn off */
#define CY_BLE_CPS_CP_ENERGY_BIT                      (0x01u << 8u)  /**< Accumulated Energy Turn off */
#define CY_BLE_CPS_CP_ENERGY_RESERVED                 (0xFE00u)      /**< Reserved bits */

/* Cycling Power Vector bits */
#define CY_BLE_CPS_CPV_CRANK_DATA_BIT                 (0x01u << 0u)  /**< Crank Revolution Data Present */
#define CY_BLE_CPS_CPV_FIRST_CRANK_MEASURE_BIT        (0x01u << 1u)  /**< First Crank Measurement Angle Present */
#define CY_BLE_CPS_CPV_INST_FORCE_MAGN_BIT            (0x01u << 2u)  /**< Instantaneous Force Magnitude Array Present */
#define CY_BLE_CPS_CPV_INST_TORQUE_MAGN_BIT           (0x01u << 3u)  /**< Instantaneous Torque Magnitude Array Present */
#define CY_BLE_CPS_CPV_INST_MEASURE_DIR_MASK          (0x03u << 4u)  /**< Instantaneous Measurement Direction */
#define CY_BLE_CPS_CPV_INST_MEASURE_DIR_TANGENTIAL    (0x01u << 4u)  /**< Instantaneous Measurement Direction: Tangential*/
#define CY_BLE_CPS_CPV_INST_MEASURE_DIR_RADIAL        (0x02u << 4u)  /**< Instantaneous Measurement Direction: Radial */
#define CY_BLE_CPS_CPV_INST_MEASURE_DIR_LATERAL       (0x03u << 4u)  /**< Instantaneous Measurement Direction: Lateral */

/** Op Codes of the Cycling Power Control Point Characteristic */
typedef enum
{
    CY_BLE_CPS_CP_OC_SCV    = 1u,   /**< Set Cumulative Value */
    CY_BLE_CPS_CP_OC_USL    = 2u,   /**< Update Sensor Location */
    CY_BLE_CPS_CP_OC_RSSL   = 3u,   /**< Request Supported Sensor Locations */
    CY_BLE_CPS_CP_OC_SCRL   = 4u,   /**< Set Crank Length */
    CY_BLE_CPS_CP_OC_RCRL   = 5u,   /**< Request Crank Length */
    CY_BLE_CPS_CP_OC_SCHL   = 6u,   /**< Set Chain Length */
    CY_BLE_CPS_CP_OC_RCHL   = 7u,   /**< Request Chain Length */
    CY_BLE_CPS_CP_OC_SCHW   = 8u,   /**< Set Chain Weight */
    CY_BLE_CPS_CP_OC_RCHW   = 9u,   /**< Request Chain Weight */
    CY_BLE_CPS_CP_OC_SSL    = 10u,  /**< Set Span Length */
    CY_BLE_CPS_CP_OC_RSL    = 11u,  /**< Request Span Length */
    CY_BLE_CPS_CP_OC_SOC    = 12u,  /**< Start Offset Compensation */
    CY_BLE_CPS_CP_OC_MCPMCC = 13u,  /**< Mask Cycling Power Measurement Characteristic Content */
    CY_BLE_CPS_CP_OC_RSR    = 14u,  /**< Request Sampling Rate */
    CY_BLE_CPS_CP_OC_RFCD   = 15u,  /**< Request Factory Calibration Date */
    CY_BLE_CPS_CP_OC_SEOC   = 16u,  /**< Start Enhanced Offset Compensation */
    CY_BLE_CPS_CP_OC_RC     = 32u   /**< Response Code */
} cy_en_ble_cps_cp_oc_t;


/** Response Code of the Cycling Power Control Point Characteristic */
typedef enum
{
    CY_BLE_CPS_CP_RC_SUCCESS = 1u,       /**< Response for successful operation. */
    CY_BLE_CPS_CP_RC_NOT_SUPPORTED,      /**< Response if unsupported Op Code is received */
    CY_BLE_CPS_CP_RC_INVALID_PARAMETER,  /**< Response if Parameter received does not meet the requirements of the
                                          *   service or is outside of the supported range of the Sensor */
    CY_BLE_CPS_CP_RC_OPERATION_FAILED    /**< Response if the requested procedure failed */
} cy_en_ble_cps_cp_rc_t;

/** Sensor Location Characteristic value */
typedef enum
{
    CY_BLE_CPS_SL_OTHER,         /**< Sensor Location -  Other */
    CY_BLE_CPS_SL_TOP_OF_SHOE,   /**< Sensor Location -  Top of shoe */
    CY_BLE_CPS_SL_IN_SHOE,       /**< Sensor Location -  In shoe */
    CY_BLE_CPS_SL_HIP,           /**< Sensor Location -  Hip */
    CY_BLE_CPS_SL_FRONT_WHEEL,   /**< Sensor Location -  Front Wheel */
    CY_BLE_CPS_SL_LEFT_CRANK,    /**< Sensor Location -  Left Crank */
    CY_BLE_CPS_SL_RIGHT_CRANK,   /**< Sensor Location -  Right Crank */
    CY_BLE_CPS_SL_LEFT_PEDAL,    /**< Sensor Location -  Left Pedal */
    CY_BLE_CPS_SL_RIGHT_PEDAL,   /**< Sensor Location -  Right Pedal */
    CY_BLE_CPS_SL_FRONT_HUB,     /**< Sensor Location -  Front Hub */
    CY_BLE_CPS_SL_REAR_DROPOUT,  /**< Sensor Location -  Rear Dropout */
    CY_BLE_CPS_SL_CHAINSTAY,     /**< Sensor Location -  Chainstay */
    CY_BLE_CPS_SL_REAR_WHEEL,    /**< Sensor Location -  Rear Wheel */
    CY_BLE_CPS_SL_REAR_HUB,      /**< Sensor Location -  Rear Hub */
    CY_BLE_CPS_SL_CHEST,         /**< Sensor Location -  Chest */
    CY_BLE_CPS_SL_SPIDER,        /**< Sensor Location -  Spider */
    CY_BLE_CPS_SL_CHAIN_RING,    /**< Sensor Location -  Chain Ring */
    CY_BLE_CPS_SL_COUNT          /**< Total Count of SL Characteristics */
} cy_en_ble_cps_sl_value_t;

/** \} */

#define CY_BLE_CPSS_BROADCAST_ADVERT_INTERVAL_OFFSET    (0x05u)
#define CY_BLE_CPSS_BROADCAST_DATA_LEN_OFFSET           (0x07u)
#define CY_BLE_CPSS_BROADCAST_DATA_OFFSET               (0x0Bu)

#define CY_BLE_CPSS_BROADCAST_DATA_LEN_MIN              (0x07u)
#define CY_BLE_CPSS_BROADCAST_DATA_HEADER_LEN           (0x03u)
#define CY_BLE_CPSS_BROADCAST_PACKETS                   (0x03u)
#define CY_BLE_CPSS_BROADCAST_HEADER_LEN                (CY_BLE_GAP_ADV_FLAGS_PACKET_LENGTH +           \
                                                         CY_BLE_GAP_ADV_ADVERT_INTERVAL_PACKET_LENGTH + \
                                                         CY_BLE_CPSS_BROADCAST_DATA_HEADER_LEN +        \
                                                         CY_BLE_CPSS_BROADCAST_PACKETS)

#define CY_BLE_CPSC_BROADCAST_DATA_OFFSET               (0x04u)


/*******************************************************************************
* Data Types
*******************************************************************************/
/**
 * \addtogroup group_ble_service_api_CPS_definitions
 * \{
 */

/** Characteristic indexes */
typedef enum
{
    CY_BLE_CPS_POWER_MEASURE,       /**< Cycling Power Measurement Characteristic index */
    CY_BLE_CPS_POWER_FEATURE,       /**< Cycling Power Feature Characteristic index */
    CY_BLE_CPS_SENSOR_LOCATION,     /**< Sensor Location Characteristic index */
    CY_BLE_CPS_POWER_VECTOR,        /**< Cycling Power Vector Characteristic index */
    CY_BLE_CPS_POWER_CP,            /**< Cycling Power Control Point Characteristic index */
    CY_BLE_CPS_CHAR_COUNT           /**< Total Count of CPS Characteristics */
}cy_en_ble_cps_char_index_t;

/** Characteristic Descriptors indexes */
typedef enum
{
    CY_BLE_CPS_CCCD,                /**< Client Characteristic Configuration descriptor index */
    CY_BLE_CPS_SCCD,                /**< Handle of the server Characteristic Configuration descriptor */
    CY_BLE_CPS_DESCR_COUNT          /**< Total Count of Descriptors */
}cy_en_ble_cps_descr_index_t;

/** Characteristic with descriptors */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t charHandle;                          /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_CPS_DESCR_COUNT]; /**< Handle of Descriptor */
} cy_stc_ble_cpss_char_t;

/** Structure with Cycling Power Service attribute handles */
typedef struct
{
    /** Cycling Power Service Handle */
    cy_ble_gatt_db_attr_handle_t serviceHandle;

    /** Cycling Power Service Characteristic handles */
    cy_stc_ble_cpss_char_t       charInfo[CY_BLE_CPS_CHAR_COUNT];
} cy_stc_ble_cpss_t;

/** Characteristic with descriptors */
typedef struct
{
    cy_ble_gatt_db_attr_handle_t descrHandle[CY_BLE_CPS_DESCR_COUNT];   /**< Handles of Descriptors */
    cy_ble_gatt_db_attr_handle_t valueHandle;                           /**< Handle of Characteristic Value */
    cy_ble_gatt_db_attr_handle_t endHandle;                             /**< End handle of Characteristic */
    uint8_t                      properties;                            /**< Properties for Value Field */
} cy_stc_ble_cpsc_char_t;

/** Structure with Discovered Attributes Information of Cycling Power Service */
typedef struct
{
    /** Characteristic handles array */
    cy_stc_ble_cpsc_char_t charInfo[CY_BLE_CPS_CHAR_COUNT];
} cy_stc_ble_cpsc_t;

/** Characteristic Value parameter structure of Cycling Power Service */
typedef struct
{
    /** Peer Device Handle */
    cy_stc_ble_conn_handle_t connHandle;

    /** bd address type of the device advertising.
     *  - CY_BLE_GAP_ADDR_TYPE_PUBLIC
     *  - CY_BLE_GAP_ADDR_TYPE_RANDOM
     *  - CY_BLE_GAP_ADDR_TYPE_PUBLIC_RPA
     *  - CY_BLE_GAP_ADDR_TYPE_RANDOM_RPA
     */
    uint8_t peerAddrType;

    /** Public Device Address or Random Device Address for
     * each device that responded to scanning. */
    uint8_t                    *peerBdAddr;

    /** Index of Service Characteristic */
    cy_en_ble_cps_char_index_t charIndex;

    /** Characteristic Value */
    cy_stc_ble_gatt_value_t    *value;
} cy_stc_ble_cps_char_value_t;

/** Characteristic descriptor Value parameter structure of Cycling Power Service */
typedef struct
{
    cy_stc_ble_conn_handle_t    connHandle;           /**< Peer Device Handle */
    cy_en_ble_cps_char_index_t  charIndex;            /**< Index of Service Characteristic */
    cy_en_ble_cps_descr_index_t descrIndex;           /**< Index of Descriptor */
    cy_stc_ble_gatt_value_t     *value;               /**< Characteristic Value */
} cy_stc_ble_cps_descr_value_t;

/** Date/time structure of Cycling Power Service */
typedef struct
{
    uint16_t year;                                    /**< Year */
    uint8_t  month;                                   /**< Month */
    uint8_t  day;                                     /**< Day */
    uint8_t  hours;                                   /**< Time - hours */
    uint8_t  minutes;                                 /**< Time - minutes */
    uint8_t  seconds;                                 /**< Time - seconds */
}__PACKED cy_stc_ble_cps_date_time_t;

/** Adjustment structure of Cycling Power Service */
typedef struct
{
    uint16_t                   crankLength;            /**< In millimeters with a resolution of 1/2 millimeter */
    uint16_t                   chainLength;            /**< In millimeters with a resolution of 1 millimeter */
    uint16_t                   chainWeight;            /**< In grams with a resolution of 1 gram */
    uint16_t                   spanLength;             /**< In millimeters with a resolution of 1 millimeter */
    cy_stc_ble_cps_date_time_t factoryCalibrationDate; /**< Use the same format as the Date Time Characteristic */
    uint8_t                    samplingRate;           /**< In Hertz with a resolution of 1 Hertz */
    int16_t                    offsetCompensation;     /**< Either the raw force in Newton or the raw torque in 1/32
                                                        *    Newton meter based on the server capabilities.
                                                        *    0xFFFF means "Not Available" */
}__PACKED cy_stc_ble_cps_cp_adjustment_t;

/** Service configuration structure (server) */
typedef struct
{
    /** Service GATT DB Handles structure */
    const cy_stc_ble_cpss_t *attrInfo;

} cy_stc_ble_cpss_config_t;

/** Service Configuration structure (client) */
typedef struct
{
    /** Structure with Discovered Attributes Information */
    cy_stc_ble_cpsc_t *attrInfo;
    
    /** The Discovery Service index */
    uint8_t serviceDiscIdx;
    
} cy_stc_ble_cpsc_config_t;
/** \} */


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
/** \addtogroup group_ble_service_api_CPS_server_client
 * \{
 */
cy_en_ble_api_result_t Cy_BLE_CPSS_Init(const cy_stc_ble_cpss_config_t *config);
cy_en_ble_api_result_t Cy_BLE_CPSC_Init(const cy_stc_ble_cpsc_config_t *config);
void Cy_BLE_CPS_RegisterAttrCallback(cy_ble_callback_t callbackFunc);
/** \} */

/**
 * \addtogroup group_ble_service_api_CPS_server
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_CPSS_SetCharacteristicValue(cy_en_ble_cps_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CPSS_GetCharacteristicValue(cy_en_ble_cps_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CPSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_cps_char_index_t charIndex,
                                                               cy_en_ble_cps_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CPSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_cps_char_index_t charIndex,
                                                               cy_en_ble_cps_descr_index_t descrIndex,
                                                               uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CPSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_cps_char_index_t charIndex, uint8_t attrSize,
                                                    uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CPSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_cps_char_index_t charIndex, uint8_t attrSize,
                                                  uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CPSS_StartBroadcast(uint16_t advInterval, uint8_t attrSize, const uint8_t *attrValue);
cy_en_ble_api_result_t Cy_BLE_CPSS_StopBroadcast(void);
/** \} */

/**
 * \addtogroup group_ble_service_api_CPS_client
 * \{
 */

cy_en_ble_api_result_t Cy_BLE_CPSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_cps_char_index_t charIndex, uint8_t attrSize,
                                                          uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CPSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_cps_char_index_t charIndex);

cy_en_ble_api_result_t Cy_BLE_CPSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_cps_char_index_t charIndex,
                                                               cy_en_ble_cps_descr_index_t
                                                               descrIndex, uint8_t attrSize, uint8_t *attrValue);

cy_en_ble_api_result_t Cy_BLE_CPSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_cps_char_index_t charIndex,
                                                               cy_en_ble_cps_descr_index_t
                                                               descrIndex);

cy_en_ble_api_result_t Cy_BLE_CPSC_StartObserve(uint8_t scanParamIndex);

cy_en_ble_api_result_t Cy_BLE_CPSC_StopObserve(void);

/** \} */


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

/** \cond IGNORE */
cy_ble_gatt_db_attr_handle_t Cy_BLE_CPSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_cps_char_index_t charIndex);

cy_ble_gatt_db_attr_handle_t Cy_BLE_CPSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_cps_char_index_t charIndex,
                                                                            cy_en_ble_cps_descr_index_t descrIndex);
/** \endcond */


/*******************************************************************************
* External Data references
*******************************************************************************/

extern const cy_stc_ble_cpss_config_t *cy_ble_cpssConfigPtr;
extern const cy_stc_ble_cpsc_config_t *cy_ble_cpscConfigPtr;

extern cy_stc_ble_gapp_disc_mode_info_t cy_ble_cpssBroadcastModeInfo;
extern cy_stc_ble_cps_cp_adjustment_t cy_ble_cpssAdjustment;

extern cy_stc_ble_gapp_disc_param_t cy_ble_cpssBroadcastParam;
extern cy_stc_ble_gapp_disc_data_t cy_ble_cpssBroadcastData;


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CY_IP_MXBLESS */
#endif /* CY_BLE_CPS_H */


/* [] END OF FILE */
