/***************************************************************************//**
* \file cy_ble_cps.c
* \version 3.30
*
* \brief
*  Contains the source code for Cycling Power service.
*
********************************************************************************
* \copyright
* Copyright 2017-2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_ble_event_handler.h"

#if defined(CY_IP_MXBLESS)
#if CY_BLE_LIB_HOST_CORE


/*******************************************************************************
* Internal Defines / Macro Functions
*******************************************************************************/

#define Cy_BLE_CPSC_CheckCharHandle(handle)                                  \
    do {                                                                     \
        if((handle).valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)    \
        {                                                                    \
            (handle).valueHandle = discCharInfo->valueHandle;                \
            (handle).properties = discCharInfo->properties;                  \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, \
                                &(discCharInfo));                            \
        }                                                                    \
    } while(0)


#define Cy_BLE_CPSS_ConnectEventHandler       Cy_BLE_CPSS_Init
#define Cy_BLE_CPSS_DisconnectEventHandler    Cy_BLE_CPSS_StopBroadcast
#define Cy_BLE_CPSC_DisconnectEventHandler    Cy_BLE_CPSC_StopObserve


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_CPS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_CPSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_CPSC_EventHandler(uint32_t eventCode, void *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_CPSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);
static void Cy_BLE_CPSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_CPSS_ConnParamUpdateRspEventHandler(const cy_stc_ble_l2cap_conn_update_rsp_param_t *eventParam);
static void Cy_BLE_CPSS_ConnUpdateCompleteEventHandler(const cy_stc_ble_gap_conn_param_updated_in_controller_t *eventParam);

static void Cy_BLE_CPSS_AdvertisementStartStopEventHandler(void);

static void Cy_BLE_CPSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_CPSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_CPSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_CPSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_CPSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam);
static void Cy_BLE_CPSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_CPSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_CPSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);
static void Cy_BLE_CPSC_ScanStartStopEventHandler(void);
static void Cy_BLE_CPSC_ScanProcessEventHandler(cy_stc_ble_gapc_adv_report_param_t *eventParam);
static void Cy_BLE_CPSC_TimeOutEventHandler(const cy_stc_ble_timeout_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_CPS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_CPSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_CPSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response for server */
static cy_ble_gatt_db_attr_handle_t cy_ble_cpssReqHandle;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_cpscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE CPS server config structure */
const cy_stc_ble_cpss_config_t *cy_ble_cpssConfigPtr = NULL;

/* The pointer to a global BLE CPS client config structure */
const cy_stc_ble_cpsc_config_t *cy_ble_cpscConfigPtr = NULL;


cy_stc_ble_cps_cp_adjustment_t cy_ble_cpssAdjustment;
static uint8_t cy_ble_cpssFlag;
static cy_stc_ble_gap_conn_param_updated_in_controller_t cy_ble_cpssConnParam;

static uint8_t cy_ble_cpscFlag[CY_BLE_MAX_CONNECTION_INSTANCES];
static cy_stc_ble_timer_info_t cy_ble_cpscCpTimeout[CY_BLE_MAX_CONNECTION_INSTANCES];
static uint8_t cy_ble_cpscObserverFlag;

cy_stc_ble_gapp_disc_param_t cy_ble_cpssBroadcastParam;
cy_stc_ble_gapp_disc_data_t cy_ble_cpssBroadcastData =    
{
    {   /* Length, FLAGS, BR/EDR NOT Supported */
        CY_BLE_GAP_ADV_FLAGS_PACKET_LENGTH, (uint8_t)CY_BLE_GAP_ADV_FLAGS, CY_BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED,

        /* Advertising Interval data type contains the advInterval value */
        CY_BLE_GAP_ADV_ADVERT_INTERVAL_PACKET_LENGTH, (uint8_t)CY_BLE_GAP_ADV_ADVERT_INTERVAL,
        CY_LO8(CY_BLE_GAP_ADV_ADVERT_INTERVAL_NONCON_MIN), CY_HI8(CY_BLE_GAP_ADV_ADVERT_INTERVAL_NONCON_MIN),

        /* The service data type consists of a CPS service UUID with the Cycling Power Measurement
         * characteristic value */
        CY_BLE_CPSS_BROADCAST_DATA_LEN_MIN,                                 /* Packet length */
        (uint8_t)CY_BLE_GAP_ADV_SRVC_DATA_16UUID,                           /* service data */
        CY_LO8(CY_BLE_UUID_CPS_SERVICE), CY_HI8(CY_BLE_UUID_CPS_SERVICE),   /* CPS service UUID */
        0x00u, 0x00u,                                                       /* Flags */
        0x11u, 0x11u,                                                       /* Instantaneous Power */
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u
    },

    /* Advertising data length */
    CY_BLE_CPSS_BROADCAST_DATA_LEN_MIN + CY_BLE_CPSS_BROADCAST_HEADER_LEN,
};

cy_stc_ble_gapp_disc_mode_info_t cy_ble_cpssBroadcastModeInfo =
{
    CY_BLE_GAPP_NONE_DISC_BROADCAST_MODE,           /* discMode */
    &cy_ble_cpssBroadcastParam,
    &cy_ble_cpssBroadcastData,
    NULL,
    0u,                                             /* advTo */
};


/******************************************************************************
* Function Name: Cy_BLE_CPSS_Init
***************************************************************************//**
*
*  This function initializes server of the Cycling Power service.
*
*  \param config: Configuration structure for the CPS.
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*                                failed. The following are possible error codes.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The function completed successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | On specifying NULL as input parameter.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED   | Buffer overflow in the registration callback.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSS_Init(const cy_stc_ble_cpss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_cpssConfigPtr = config;

        /* Registers event handler for the CPS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_CPS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_CPSS_EventHandlerCallback = &Cy_BLE_CPSS_EventHandler;
        
        cy_ble_cpssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        cy_ble_cpssFlag = 0u;
        cy_ble_cpssAdjustment.samplingRate = CY_BLE_CPS_SAMLING_RATE_DEFAULT;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_Init
***************************************************************************//**
*
*  This function initializes client of the Cycling Power service.
*
*  \param config: Configuration structure for the CPS.
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*                                failed. The following are possible error codes.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The function completed successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | On specifying NULL as input parameter.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED   | Buffer overflow in the registration callback.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSC_Init(const cy_stc_ble_cpsc_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if((config == NULL) || ((cy_ble_configPtr->params->gattRole & CY_BLE_GATT_CLIENT) == 0u))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        uint32_t idx;

        /* Registers a pointer to config structure */
        cy_ble_cpscConfigPtr = config;

        /* Registers event handler for the CPS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_CPS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_CPSC_EventHandlerCallback = &Cy_BLE_CPSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 cpsServIdx = cy_ble_cpscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + cpsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to CPS client structure */
                cy_stc_ble_cpsc_t *cpscPtr = (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(cpscPtr, 0, sizeof(cy_stc_ble_cpsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + cpsServIdx].uuid =
                    CY_BLE_UUID_CPS_SERVICE;
            }
            cy_ble_cpscFlag[idx] = 0u;
            cy_ble_cpscCpTimeout[idx].timeout = CY_BLE_CPS_CP_PROCEDURE_TIMEOUT;
            cy_ble_cpscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
   
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for CPS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_CPSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_cps_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_CPS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_CPS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a characteristic value of Cycling Power service, which is a value
*  identified by charIndex, to the local database.
*
*  \param charIndex: The index of the service characteristic of type
*                    \ref cy_en_ble_cps_char_index_t. The valid values are,
*                       * \ref CY_BLE_CPS_POWER_MEASURE
*                       * \ref CY_BLE_CPS_POWER_FEATURE
*                       * \ref CY_BLE_CPS_SENSOR_LOCATION
*                       * \ref CY_BLE_CPS_POWER_VECTOR
*                       * \ref CY_BLE_CPS_POWER_CP 
*  \param attrSize:  The size of the characteristic value attribute.
*  \param attrValue: The pointer to the characteristic value data that should be
*                    stored to the GATT database.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE| An optional characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSS_SetCharacteristicValue(cy_en_ble_cps_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if(charIndex >= CY_BLE_CPS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Store data in database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_cpssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets a characteristic value of Cycling Power service. The value is
*  identified by charIndex.
*
*  \param charIndex: The index of the service characteristic of type
*                    \ref cy_en_ble_cps_char_index_t. The valid values are,
*                       * \ref CY_BLE_CPS_POWER_MEASURE
*                       * \ref CY_BLE_CPS_POWER_FEATURE
*                       * \ref CY_BLE_CPS_SENSOR_LOCATION
*                       * \ref CY_BLE_CPS_POWER_VECTOR
*                       * \ref CY_BLE_CPS_POWER_CP 
*  \param attrSize:  The size of the characteristic value attribute.
*  \param attrValue: The pointer to the location where characteristic value data
*                    should be stored.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSS_GetCharacteristicValue(cy_en_ble_cps_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if(charIndex >= CY_BLE_CPS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_cpssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sets a characteristic descriptor of the specified characteristic of Cycling 
*  Power service.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cps_char_index_t. The valid values are,
*                        * \ref CY_BLE_CPS_POWER_MEASURE
*                        * \ref CY_BLE_CPS_POWER_FEATURE
*                        * \ref CY_BLE_CPS_SENSOR_LOCATION
*                        * \ref CY_BLE_CPS_POWER_VECTOR
*                        * \ref CY_BLE_CPS_POWER_CP 
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_cps_descr_index_t. The valid values are,
*                        * \ref CY_BLE_CPS_CCCD
*                        * \ref CY_BLE_CPS_SCCD 
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  The pointer to the descriptor value data that should
*                     be stored to the GATT database.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_cps_char_index_t charIndex,
                                                               cy_en_ble_cps_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((charIndex >= CY_BLE_CPS_CHAR_COUNT) || (descrIndex >= CY_BLE_CPS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_cpssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex];
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.connHandle                 = connHandle;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of the specified characteristic of Cycling 
*  Power service.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cps_char_index_t. The valid values are,
*                        * \ref CY_BLE_CPS_POWER_MEASURE
*                        * \ref CY_BLE_CPS_POWER_FEATURE
*                        * \ref CY_BLE_CPS_SENSOR_LOCATION
*                        * \ref CY_BLE_CPS_POWER_VECTOR
*                        * \ref CY_BLE_CPS_POWER_CP 
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_cps_descr_index_t. The valid values are,
*                        * \ref CY_BLE_CPS_CCCD
*                        * \ref CY_BLE_CPS_SCCD 
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  The pointer to the location where characteristic descriptor
*                     value data should be stored.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_cps_char_index_t charIndex,
                                                               cy_en_ble_cps_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((charIndex >= CY_BLE_CPS_CHAR_COUNT) || (descrIndex >= CY_BLE_CPS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_cpssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex];
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.connHandle                 = connHandle;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_WriteEventHandler
***************************************************************************//**
*
*  Handles Write Request event for CPS service.
*
*  \param void *eventParam: The pointer to the data structure specified by the event.
*
* \return
*  A return value of type cy_en_ble_gatt_err_code_t.
*   * CY_BLE_GATT_ERR_NONE - write is successful
*   * CY_BLE_GATT_ERR_PROCEDURE_ALREADY_IN_PROGRESS
*   * CY_BLE_GATT_ERR_CCCD_IMPROPERLY_CONFIGURED
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_CPSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_cps_char_index_t locCharIndex;
    cy_stc_ble_cps_char_value_t locCharValue = { .connHandle = eventParam->connHandle };
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    uint8_t locReqHandle = 0u;

    if(Cy_BLE_CPS_ApplCallback != NULL)
    {
        /* Error conditions for CP Characteristic value Write Request */
        if((eventParam->handleValPair.attrHandle == cy_ble_cpssConfigPtr->attrInfo->charInfo[CY_BLE_CPS_POWER_CP].charHandle) &&
           (!CY_BLE_IS_INDICATION_ENABLED(eventParam->connHandle.attId,
                                          cy_ble_cpssConfigPtr->attrInfo->charInfo[CY_BLE_CPS_POWER_CP].
                                           descrHandle[CY_BLE_CPS_CCCD])))
        {
            gattErr = CY_BLE_GATT_ERR_CCCD_IMPROPERLY_CONFIGURED;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else if((eventParam->handleValPair.attrHandle ==
                 cy_ble_cpssConfigPtr->attrInfo->charInfo[CY_BLE_CPS_POWER_CP].charHandle) &&
                ((cy_ble_cpssFlag & CY_BLE_CPSS_FLAG_CP_PROCEDURE_IN_PROGRESS) != 0u))
        {
            gattErr = CY_BLE_GATT_ERR_PROCEDURE_ALREADY_IN_PROGRESS;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else
        {
            for(locCharIndex = CY_BLE_CPS_POWER_MEASURE; (locCharIndex < CY_BLE_CPS_CHAR_COUNT) && (locReqHandle == 0u);
                locCharIndex++)
            {
                if((eventParam->handleValPair.attrHandle == cy_ble_cpssConfigPtr->attrInfo->charInfo[locCharIndex].
                     descrHandle[CY_BLE_CPS_CCCD]) ||
                   (eventParam->handleValPair.attrHandle == cy_ble_cpssConfigPtr->attrInfo->charInfo[locCharIndex].
                     descrHandle[CY_BLE_CPS_SCCD]) ||
                   (eventParam->handleValPair.attrHandle == cy_ble_cpssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle))
                {
                    /* Store value to database */
                    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                    
                    dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                    dbAttrValInfo.connHandle      = eventParam->connHandle;
                    dbAttrValInfo.offset          = 0u;
                    dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

                    /* Clear event handled flag to send Write Response */
                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                    locReqHandle = 1u;

                    locCharValue.charIndex = locCharIndex;
                    locCharValue.value = &eventParam->handleValPair.value;


                    gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);

                    if(gattErr == CY_BLE_GATT_ERR_NONE)
                    {
                        /* Characteristic value Write Request */
                        if(eventParam->handleValPair.attrHandle ==
                           cy_ble_cpssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle)
                        {
                            Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSS_WRITE_CHAR, &locCharValue);

                            /* In the context of the Cycling Power Control Point characteristic, a procedure is started
                             * when a write to the Cycling Power Control Point characteristic is successfully completed
                             */
                            if(locCharIndex == CY_BLE_CPS_POWER_CP)
                            {
                                cy_ble_cpssFlag |= CY_BLE_CPSS_FLAG_CP_PROCEDURE_IN_PROGRESS;
                            }
                        }

                        /* Client Characteristic Configuration descriptor Write Request */
                        else if(eventParam->handleValPair.attrHandle ==
                                cy_ble_cpssConfigPtr->attrInfo->charInfo[locCharIndex].descrHandle[CY_BLE_CPS_CCCD])
                        {
                            /* Check characteristic properties for Notification */
                            if(CY_BLE_IS_NOTIFICATION_SUPPORTED(cy_ble_cpssConfigPtr->attrInfo->charInfo[locCharIndex].
                                                                 charHandle))
                            {
                                if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                                {
                                    uint16_t requiredConnIntv;
                                    uint8_t prefConnParamCharValue[CY_BLE_PPCPC_LEN];
                                    requiredConnIntv = CY_BLE_CPS_SAMLING_RATE_TO_CONN_INTV /
                                                       cy_ble_cpssAdjustment.samplingRate;

                                    /* Cycling Power Vector characteristic has special behavior
                                     * described in CPS specification, section 3.5.1 */
                                    if((locCharIndex == CY_BLE_CPS_POWER_VECTOR) &&
                                       (cy_ble_cpssConnParam.connIntv > requiredConnIntv))
                                    {
                                        /* Read characteristic value from database */
                                        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfoCCCD;
                                        
                                        dbAttrValInfoCCCD.connHandle.bdHandle        = 0u;
                                        dbAttrValInfoCCCD.connHandle.attId           = 0u;
                                        dbAttrValInfoCCCD.handleValuePair.attrHandle = cy_ble_gapsConfigPtr->attrInfo->prefConnParamCharHandle;
                                        dbAttrValInfoCCCD.handleValuePair.value.len  = sizeof(prefConnParamCharValue);
                                        dbAttrValInfoCCCD.handleValuePair.value.val  = prefConnParamCharValue;
                                        dbAttrValInfoCCCD.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

                                        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfoCCCD) ==
                                           CY_BLE_GATT_ERR_NONE)
                                        {
                                            /* If the current connection parameters do not allow sending of
                                             * notification, request new connection parameters by using the GAP
                                             * Connection Parameter Update procedure.
                                             */
                                            cy_stc_ble_gap_conn_update_param_info_t connUpdateParam;
                                            
                                            /* Send Connection Parameter Update Request to Client */
                                            connUpdateParam.connIntvMin   = requiredConnIntv;
                                            connUpdateParam.connIntvMax   = requiredConnIntv;
                                            connUpdateParam.connLatency   = Cy_BLE_Get16ByPtr(prefConnParamCharValue +
                                                                               CY_BLE_PPCPC_SLAVE_LATENCY_OFFSET);
                                            connUpdateParam.supervisionTO = Cy_BLE_Get16ByPtr(prefConnParamCharValue +
                                                                               CY_BLE_PPCPC_SUP_TIMEOUT_OFFSET);
                                            connUpdateParam.bdHandle      = eventParam->connHandle.bdHandle;
                                            

                                            if(Cy_BLE_L2CAP_LeConnectionParamUpdateRequest(&connUpdateParam)
                                               == CY_BLE_SUCCESS)
                                            {
                                                cy_ble_cpssFlag |= CY_BLE_CPSS_FLAG_PV_PROCEDURE_IN_PROGRESS;

                                                /* Set event handled flag to not send Write Response. Response will be
                                                 * sent when Central accept the request for connection parameter update
                                                 * in Cy_BLE_CPSS_ConnParamUpdateRspEventHandler function.
                                                 */
                                                cy_ble_eventHandlerFlag |= CY_BLE_CALLBACK;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSS_NOTIFICATION_ENABLED,
                                                                &locCharValue);
                                    }
                                }
                                else
                                {
                                    Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSS_NOTIFICATION_DISABLED,
                                                            &locCharValue);
                                }
                            }

                            /* Check characteristic properties for Indication */
                            if(CY_BLE_IS_INDICATION_SUPPORTED(cy_ble_cpssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle))
                            {
                                if(CY_BLE_IS_INDICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                                {
                                    Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSS_INDICATION_ENABLED, &locCharValue);
                                }
                                else
                                {
                                    Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSS_INDICATION_DISABLED, &locCharValue);
                                }
                            }
                        }
                        /* Server Characteristic Configuration descriptor Write Request */
                        else
                        {
                            /* Check characteristic properties for Broadcast */
                            if(CY_BLE_IS_BROADCAST_SUPPORTED(cy_ble_cpssConfigPtr->attrInfo->charInfo[locCharIndex].
                                                              charHandle))
                            {
                                if(CY_BLE_IS_BROADCAST_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                                {
                                    Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSS_BROADCAST_ENABLED, &locCharValue);
                                }
                                else
                                {
                                    Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSS_BROADCAST_DISABLED, &locCharValue);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_SendNotification
***************************************************************************//**
*
*  Sends notification with a characteristic value of the CPS, which is a value
*  specified by charIndex, to the client device.
*
*  On enabling notification successfully for a service characteristic it sends out a
*  'Handle Value Notification' which results in \ref CY_BLE_EVT_CPSC_NOTIFICATION event
*  at the GATT Client's end.
*
*  \param connHandle: The connection handle
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cps_char_index_t. The valid values are,
*                        * \ref CY_BLE_CPS_POWER_MEASURE
*                        * \ref CY_BLE_CPS_POWER_FEATURE
*                        * \ref CY_BLE_CPS_SENSOR_LOCATION
*                        * \ref CY_BLE_CPS_POWER_VECTOR
*                        * \ref CY_BLE_CPS_POWER_CP 
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     sent to the client device.
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | An optional characteristic is absent.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_NTF_DISABLED                | Notification is not enabled by the client.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_cps_char_index_t charIndex,
                                                    uint8_t attrSize,
                                                    uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send Notification if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_CPS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_cpssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((!CY_BLE_IS_NOTIFICATION_SUPPORTED(cy_ble_cpssConfigPtr->attrInfo->charInfo[charIndex].charHandle)) ||
            (!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_cpssConfigPtr->attrInfo->charInfo[charIndex].
                                              descrHandle[CY_BLE_CPS_CCCD])))
    {
        apiResult = CY_BLE_ERROR_NTF_DISABLED;
    }
    else
    {
        cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
        
        /* Fill all fields of Write Request structure ... */
        ntfReqParam.handleValPair.attrHandle = cy_ble_cpssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        ntfReqParam.handleValPair.value.val  = attrValue;
        ntfReqParam.handleValPair.value.len  = attrSize;
        ntfReqParam.connHandle               = connHandle;
        
        /* Send notification to the client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Notification(&ntfReqParam);
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_SendIndication
***************************************************************************//**
*
*  Sends indication with a characteristic value of the CPS, which is a value
*  specified by charIndex, to the client device.
*
*  On enabling indication successfully it sends out a 'Handle Value Indication' which
*  results in \ref CY_BLE_EVT_CPSC_INDICATION or \ref CY_BLE_EVT_GATTC_HANDLE_VALUE_IND (if
*  service-specific callback function is not registered) event at the GATT Client's end.
*
*  \param connHandle: The connection handle
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cps_char_index_t. The valid values are,
*                        * \ref CY_BLE_CPS_POWER_MEASURE
*                        * \ref CY_BLE_CPS_POWER_FEATURE
*                        * \ref CY_BLE_CPS_SENSOR_LOCATION
*                        * \ref CY_BLE_CPS_POWER_VECTOR
*                        * \ref CY_BLE_CPS_POWER_CP 
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     sent to the client device.
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | An optional characteristic is absent.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_NTF_DISABLED                | Notification is not enabled by the client.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*  If the CPS service-specific callback is registered
*  with \ref Cy_BLE_CPS_RegisterAttrCallback():
*  * \ref CY_BLE_EVT_CPSS_INDICATION_CONFIRMED - In case if the indication is
*    successfully delivered to the peer device.
*  .
*  Otherwise, if the CPS service-specific callback is not registered:
*  * \ref CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF - In case if the indication is
*    successfully delivered to the peer device.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_cps_char_index_t charIndex,
                                                  uint8_t attrSize,
                                                  uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send Notification if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_CPS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_cpssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((!CY_BLE_IS_INDICATION_SUPPORTED(cy_ble_cpssConfigPtr->attrInfo->charInfo[charIndex].charHandle)) ||
            (!CY_BLE_IS_INDICATION_ENABLED(connHandle.attId, cy_ble_cpssConfigPtr->attrInfo->charInfo[charIndex].
                                            descrHandle[CY_BLE_CPS_CCCD])))
    {
        apiResult = CY_BLE_ERROR_IND_DISABLED;
    }
    else
    {
        cy_stc_ble_gatts_handle_value_ind_t indReqParam;
        
        /* Fill all fields of Write Request structure ... */
        indReqParam.handleValPair.attrHandle = cy_ble_cpssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        indReqParam.handleValPair.value.val  = attrValue;
        indReqParam.handleValPair.value.len  = attrSize;
        indReqParam.connHandle               = connHandle;

        /* Send indication to client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Indication(&indReqParam);
        /* Save handle to support service-specific value confirmation response from Client */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_cpssReqHandle = indReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_ConfirmationEventHandler
***************************************************************************//**
*
*  Handles a Value Confirmation request event from the BLE Stack.
*
*  *eventParam - Pointer to a structure of type 'cy_stc_ble_conn_handle_t'.
*
******************************************************************************/
static void Cy_BLE_CPSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    cy_en_ble_cps_char_index_t locCharIndex;
    cy_stc_ble_cps_char_value_t locCharValue = { .connHandle = *eventParam };
    uint8_t locReqHandle = 0u;

    if((Cy_BLE_CPS_ApplCallback != NULL) && (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_cpssReqHandle))
    {
        for(locCharIndex = CY_BLE_CPS_POWER_MEASURE; (locCharIndex < CY_BLE_CPS_CHAR_COUNT) && (locReqHandle == 0u);
            locCharIndex++)
        {
            if(cy_ble_cpssReqHandle == cy_ble_cpssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle)
            {
                locCharValue.charIndex = locCharIndex;
                locCharValue.value = NULL;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                cy_ble_cpssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

                /* The Cycling Power Control Point procedure is complete when the server indicated with the Op Code
                 * set to Response Code.
                 */
                if(locCharIndex == CY_BLE_CPS_POWER_CP)
                {
                    cy_ble_cpssFlag &= (uint8_t) ~CY_BLE_CPSS_FLAG_CP_PROCEDURE_IN_PROGRESS;
                }
                Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSS_INDICATION_CONFIRMED, &locCharValue);
                locReqHandle = 1u;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_ConnParamUpdateRspEventHandler
***************************************************************************//**
*
*  Handles the L2CAP connection parameter response event from
*  the BLE Stack.
*
*  \param uint16_t response:
*      * Accepted = 0x0000
*      * Rejected = 0x0001
*
******************************************************************************/
static void Cy_BLE_CPSS_ConnParamUpdateRspEventHandler(const cy_stc_ble_l2cap_conn_update_rsp_param_t *eventParam)
{
    if((Cy_BLE_CPS_ApplCallback != NULL) && ((cy_ble_cpssFlag & CY_BLE_CPSS_FLAG_PV_PROCEDURE_IN_PROGRESS) != 0u))
    {
        cy_stc_ble_conn_handle_t locConnHandle =
        {
            .bdHandle = eventParam->bdHandle
        };

        cy_ble_cpssFlag &= (uint8_t) ~CY_BLE_CPSS_FLAG_PV_PROCEDURE_IN_PROGRESS;
        if(eventParam->result != 0u)
        {
            /* If the client does not change the connection parameters Server shall return an ATT Error Response
             * to the Write Request with an Error Code set to the application Error Code 0x80
             * (Inappropriate Connection Parameters). */
            cy_stc_ble_gatt_err_param_t err_param;
            
            err_param.errInfo.opCode     = (cy_en_ble_gatt_pdu_t)CY_BLE_GATT_WRITE_REQ;
            err_param.errInfo.attrHandle = cy_ble_cpssConfigPtr->attrInfo->charInfo[CY_BLE_CPS_POWER_VECTOR].
                                            descrHandle[CY_BLE_CPS_CCCD];
            err_param.errInfo.errorCode  = CY_BLE_GATT_ERR_CPS_INAPPROPRIATE_CONNECTION_PARAMETERS;
            /* get connHandle form cy_ble_connHandle, to have actual value of connHandle.attId */
            err_param.connHandle         = cy_ble_connHandle[Cy_BLE_GetConnHandleIdx(locConnHandle)];
            
            /* Send Error Response */
            (void)Cy_BLE_GATTS_ErrorRsp(&err_param);
        }
        else
        {
            cy_stc_ble_cps_char_value_t locCharValue;
            
            locCharValue.connHandle = cy_ble_connHandle[Cy_BLE_GetConnHandleIdx(locConnHandle)];
            locCharValue.charIndex  = CY_BLE_CPS_POWER_VECTOR;

            cy_stc_ble_gatt_value_t locValue;
            uint8_t cccdVal[2u];
            Cy_BLE_Set16ByPtr(cccdVal, CY_BLE_CCCD_NOTIFICATION);

            /* Otherwise, the server shall respond with a Write Response and start sending notifications
             * of the Cycling Power Vector characteristic. */
            (void)Cy_BLE_GATTS_WriteRsp(locConnHandle);

            /* get connHandle form cy_ble_connHandle, to have actual value of connHandle.attId */
            locValue.len = CY_BLE_CCCD_LEN;
            locValue.val = cccdVal;
            locCharValue.value = &locValue;

            Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSS_NOTIFICATION_ENABLED, &locCharValue);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_ConnUpdateCompleteEventHandler
***************************************************************************//**
*
*  Handles the #CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE event from
*  the BLE Stack.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_CPSS_ConnUpdateCompleteEventHandler(const cy_stc_ble_gap_conn_param_updated_in_controller_t *eventParam)
{
    /* Store connection parameters */
    cy_ble_cpssConnParam = *eventParam;
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_StartBroadcast
***************************************************************************//**
*
*  This function is used to start broadcasting of the Cycling Power
*  Measurement characteristic or update broadcasting data when it was started
*  before.
*
*  It is available only in Broadcaster role.
*
*  \param advInterval: Advertising interval in 625 us units. The valid range is
*                      from \ref CY_BLE_GAP_ADV_ADVERT_INTERVAL_NONCON_MIN
*                      to \ref CY_BLE_GAP_ADV_ADVERT_INTERVAL_MAX. This parameter
*                      is ignored when when broadcasting is already started.
*  \param attrSize:    The size of the characteristic value attribute.
*                      This size is limited by maximum advertising packet length 
*                      and advertising header size.
*  \param attrValue:   The pointer to the Cycling Power Measurement characteristic
*                      that include the mandatory fields (e.g. the Flags field and 
*                      the Instantaneous Power field and depending on the Flags 
*                      field, some optional fields in a non connectable undirected 
*                      advertising event.
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. The following are possible error codes.
*
*   Error Codes                        | Description
*   ------------                       | -----------
*   CY_BLE_SUCCESS                     | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER     | On passing an invalid parameter.
*   CY_BLE_ERROR_INVALID_OPERATION     | Operation is not permitted.
*
******************************************************************************/

cy_en_ble_api_result_t Cy_BLE_CPSS_StartBroadcast(uint16_t advInterval,
                                                  uint8_t attrSize,
                                                  const uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((advInterval < CY_BLE_GAP_ADV_ADVERT_INTERVAL_NONCON_MIN) || (advInterval > CY_BLE_GAP_ADV_ADVERT_INTERVAL_MAX) ||
       (attrSize > (CY_BLE_GAP_MAX_ADV_DATA_LEN - CY_BLE_CPSS_BROADCAST_HEADER_LEN)))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Copy Cycling Power Measurement characteristic value to advertising data packet */
        (void)memcpy(&cy_ble_cpssBroadcastData.advData[CY_BLE_CPSS_BROADCAST_DATA_OFFSET], attrValue, (uint32_t)attrSize);
        cy_ble_cpssBroadcastData.advData[CY_BLE_CPSS_BROADCAST_DATA_LEN_OFFSET] = attrSize;
        cy_ble_cpssBroadcastData.advDataLen = attrSize + CY_BLE_CPSS_BROADCAST_HEADER_LEN;

        if((cy_ble_cpssFlag & CY_BLE_CPSS_FLAG_BROADCAST_IN_PROGRESS) == 0u)
        {
            /* Put advertising interval to advertising data packet */
            cy_ble_cpssBroadcastData.advData[CY_BLE_CPSS_BROADCAST_ADVERT_INTERVAL_OFFSET] = CY_LO8(advInterval);
            cy_ble_cpssBroadcastData.advData[CY_BLE_CPSS_BROADCAST_ADVERT_INTERVAL_OFFSET + 1u] = CY_HI8(advInterval);

            /* Configure advertising timeout, interval and type */
            cy_ble_cpssBroadcastParam.advIntvMin = advInterval;
            cy_ble_cpssBroadcastParam.advIntvMax = advInterval;
            cy_ble_cpssBroadcastModeInfo.advTo = 0u;
            cy_ble_cpssBroadcastParam.advType = CY_BLE_GAPP_NON_CONNECTABLE_UNDIRECTED_ADV;
            cy_ble_cpssBroadcastParam.advChannelMap = cy_ble_configPtr->discoveryModeInfo[cy_ble_advIndex].advParam->advChannelMap;

            /* Start broadcasting */
            apiResult = Cy_BLE_GAPP_EnterDiscoveryMode(&cy_ble_cpssBroadcastModeInfo);
            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_cpssFlag |= CY_BLE_CPSS_FLAG_START_BROADCAST;
            }
        }
        else
        {
            /* Update the advertisement packet if the device is in the advertising mode. */
            apiResult = Cy_BLE_GAPP_UpdateAdvScanData(&cy_ble_cpssBroadcastModeInfo);
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_StopBroadcast
***************************************************************************//**
*
*  This function is used to stop broadcasting of the Cycling Power
*  Measurement characteristic.
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. The following are possible error codes.
*
*   Error Codes                           | Description
*   ------------                          | -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | Memory allocation failed.
*   CY_BLE_ERROR_INVALID_STATE            | On calling this function not in CPS broadcasting mode.
*   CY_BLE_ERROR_INSUFFICIENT_RESOURCES   | Stack resources are unavailable.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSS_StopBroadcast(void)
{
    cy_en_ble_api_result_t apiResult;

    if((cy_ble_cpssFlag & CY_BLE_CPSS_FLAG_BROADCAST_IN_PROGRESS) != 0u)
    {
        apiResult = Cy_BLE_GAPP_ExitDiscoveryMode();
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_cpssFlag |= CY_BLE_CPSS_FLAG_STOP_BROADCAST;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_AdvertisementStartStopEventHandler
***************************************************************************//**
*
*  This function handles #CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP event for CPS.
*
******************************************************************************/
static void Cy_BLE_CPSS_AdvertisementStartStopEventHandler(void)
{
    if((cy_ble_cpssFlag & CY_BLE_CPSS_FLAG_START_BROADCAST) != 0u)
    {
        cy_ble_cpssFlag |= CY_BLE_CPSS_FLAG_BROADCAST_IN_PROGRESS;
        cy_ble_cpssFlag &= (uint8_t) ~CY_BLE_CPSS_FLAG_START_BROADCAST;
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
    if((cy_ble_cpssFlag & CY_BLE_CPSS_FLAG_STOP_BROADCAST) != 0u)
    {
        cy_ble_cpssFlag &= (uint8_t) ~(CY_BLE_CPSS_FLAG_BROADCAST_IN_PROGRESS | CY_BLE_CPSS_FLAG_STOP_BROADCAST);
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_CPSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* CPS characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_cpscCharUuid[CY_BLE_CPS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_CPS_MSRMT,         /* Cycling Power Measurement characteristic UUID */
        CY_BLE_UUID_CHAR_CPS_FEATURE,       /* Cycling Power Feature characteristic UUID */
        CY_BLE_UUID_CHAR_SENSOR_LOCATION,   /* Cycling Power Sensor Location characteristic UUID */
        CY_BLE_UUID_CHAR_CPS_VECTOR,        /* Cycling Power Vector characteristic UUID */
        CY_BLE_UUID_CHAR_CPS_CP             /* Cycling Power Control Point characteristic UUID */
    };
    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t cpsDiscIdx = cy_ble_cpscConfigPtr->serviceDiscIdx;
    uint32_t i;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == cpsDiscIdx))
    {
        /* Get pointer (with offset) to CPS client structure with attribute handles */
        cy_stc_ble_cpsc_t *cpscPtr = (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[discIdx];

        /* Update last characteristic endHandle to declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        for(i = (uint32_t)CY_BLE_CPS_POWER_MEASURE; i < (uint32_t)CY_BLE_CPS_CHAR_COUNT; i++)
        {
            if(cy_ble_cpscCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                Cy_BLE_CheckStoreCharHandle(cpscPtr->charInfo[i]);
                lastEndHandle[discIdx] = &cpscPtr->charInfo[i].endHandle;
                break;
            }
        }

        /* Init characteristic endHandle to the service endHandle.
         * Characteristic endHandle will be updated to declaration
         * handler of the following characteristic,
         * in the following characteristic discovery procedure. */
        if(lastEndHandle[discIdx] != NULL)
        {
            uint32_t locServCount = cy_ble_configPtr->context->discovery[discIdx].servCount;
            uint32_t discServiNum = cy_ble_configPtr->context->discServiCount;

            *lastEndHandle[discIdx] =
                cy_ble_configPtr->context->serverInfo[(discIdx * discServiNum) + locServCount].range.endHandle;        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  Based on the descriptor UUID, an appropriate data structure is populated using
*  the data received as part of the callback.
*
*  \param discoveryCharIndex:  The characteristic index which is discovered.
*  \param discDescrInfo:      The pointer to a descriptor information structure.
*
******************************************************************************/
static void Cy_BLE_CPSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t notSupportedDescr = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t cpsDiscIdx = cy_ble_cpscConfigPtr->serviceDiscIdx;
    uint32_t descIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == cpsDiscIdx)
    {
        switch(discDescrInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_CLIENT_CONFIG:
                descIdx = (uint32_t)CY_BLE_CPS_CCCD;
                break;

            case CY_BLE_UUID_CHAR_SERVER_CONFIG:
                descIdx = (uint32_t)CY_BLE_CPS_SCCD;
                break;

            default:
                /* Not supported descriptor */
                notSupportedDescr = 1u;
                break;
        }

        if(notSupportedDescr == 0u)
        {    
            /* Get pointer (with offset) to CPS client structure with attribute handles */
            cy_stc_ble_cpsc_t *cpscPtr = (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[discIdx];

            if(cpscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] ==
               CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                cpscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] =
                    discDescrInfo->descrHandle;
            }
            else
            {
                /* Duplication of the descriptor */
                Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_DESCR_DUPLICATION, &discDescrInfo);
            }
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_CPSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t cpsDiscIdx = cy_ble_cpscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == cpsDiscIdx)
    {   
        /* Get pointer (with offset) to CPS client structure with attribute handles */
        cy_stc_ble_cpsc_t *cpscPtr = (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[discIdx];

        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_CPS_CHAR_COUNT) && (exitFlag == 0u))
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            if((cpscPtr->charInfo[charIdx].endHandle -
                cpscPtr->charInfo[charIdx].valueHandle) != 0u)
            {
                /* Read characteristic range */
                charRangeInfo->range.startHandle = cpscPtr->charInfo[charIdx].valueHandle + 1u;
                charRangeInfo->range.endHandle = cpscPtr->charInfo[charIdx].endHandle;
                exitFlag = 1u;
            }
            else
            {
                cy_ble_configPtr->context->discovery[discIdx].charCount++;
            }
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}

/******************************************************************************
* Function Name: Cy_BLE_CPSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result a Write Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the \ref CY_BLE_EVT_CPSS_WRITE_CHAR event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cps_char_index_t. The valid values are,
*                        * \ref CY_BLE_CPS_POWER_MEASURE
*                        * \ref CY_BLE_CPS_POWER_FEATURE
*                        * \ref CY_BLE_CPS_SENSOR_LOCATION
*                        * \ref CY_BLE_CPS_POWER_VECTOR
*                        * \ref CY_BLE_CPS_POWER_CP 
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  Pointer to the characteristic value data that
*                     should be send to the server device.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*    
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the CPS service-specific callback is registered
*   with \ref Cy_BLE_CPS_RegisterAttrCallback():
*   * \ref CY_BLE_EVT_CPSC_WRITE_CHAR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_cps_char_value_t.
*   .
*   Otherwise (if the CPS service-specific callback is not registered):
*   * \ref CY_BLE_EVT_GATTC_WRITE_RSP - In case if the requested attribute is
*     successfully written on the peer device.
*
*   * \ref CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*     requested attribute on the peer device, the details are provided with event
*     parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_cps_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to CPS client structure with attribute handles */
    cy_stc_ble_cpsc_t *cpscPtr = (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_CPS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cpscPtr->charInfo[charIndex].valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;
        
        writeReqParam.handleValPair.attrHandle = cpscPtr->charInfo[charIndex].valueHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;

        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);

        /* Save handle to support service-specific Write Response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_cpscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
            /* Start CP procedure. It will ends when the Collector sends a confirmation to acknowledge the CP
             * indication sent by the CP Sensor. A procedure is considered to have timed out if a CP indication
             * is not received within the ATT transaction timeout, defined as 30 seconds.
             * #CY_BLE_EVT_CPSC_TIMEOUT event with cy_stc_ble_cps_char_value_t parameter will indicate about CP
             * procedure timeout.
             */
            if(charIndex == CY_BLE_CPS_POWER_CP)
            {
                apiResult = Cy_BLE_StartTimer(&cy_ble_cpscCpTimeout[discIdx]);
                if(apiResult == CY_BLE_SUCCESS)
                {
                    cy_ble_cpscFlag[discIdx] |= CY_BLE_CPSC_FLAG_CP_PROCEDURE_IN_PROGRESS;
                }
            }
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_GetCharacteristicValue
***************************************************************************//**
*
*  Sends a request to the peer device to get a characteristic value, as
*  identified by its charIndex.
*  The Read Response returns the characteristic Value in the Attribute Value
*  parameter.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cps_char_index_t. The valid values are,
*                        * \ref CY_BLE_CPS_POWER_MEASURE
*                        * \ref CY_BLE_CPS_POWER_FEATURE
*                        * \ref CY_BLE_CPS_SENSOR_LOCATION
*                        * \ref CY_BLE_CPS_POWER_VECTOR
*                        * \ref CY_BLE_CPS_POWER_CP 
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*    
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic
*
*  \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the CPS service-specific callback is registered
*      with \ref Cy_BLE_CPS_RegisterAttrCallback():
*   * \ref CY_BLE_EVT_CPSC_READ_CHAR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details 
*     (charIndex , value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_gatt_err_param_t.
*   .
*   Otherwise (if an CPS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*     successfully read on the peer device, the details (handle, value, etc.) are
*     provided with an event parameter structure 
*     \ref cy_stc_ble_gattc_read_rsp_param_t.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_cps_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to CPS client structure with attribute handles */
    cy_stc_ble_cpsc_t *cpscPtr = (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_CPS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cpscPtr->charInfo[charIndex].valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = cpscPtr->charInfo[charIndex].valueHandle;
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_cpscReqHandle[discIdx] = cpscPtr->charInfo[charIndex].valueHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_SetCharacteristicDescriptor
***************************************************************************//**
*

*  Sends a request to the peer device to set the characteristic descriptor of the
*  specified characteristic of Cycling Power service.
*
*  Internally, Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*  * #CY_BLE_EVT_CPSS_NOTIFICATION_ENABLED
*  * #CY_BLE_EVT_CPSS_NOTIFICATION_DISABLED
*  * #CY_BLE_EVT_CPSS_INDICATION_ENABLED
*  * #CY_BLE_EVT_CPSS_INDICATION_DISABLED
*  * #CY_BLE_EVT_CPSS_BROADCAST_ENABLED
*  * #CY_BLE_EVT_CPSS_BROADCAST_DISABLED
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cps_char_index_t. The valid values are,
*                        * \ref CY_BLE_CPS_POWER_MEASURE
*                        * \ref CY_BLE_CPS_POWER_FEATURE
*                        * \ref CY_BLE_CPS_SENSOR_LOCATION
*                        * \ref CY_BLE_CPS_POWER_VECTOR
*                        * \ref CY_BLE_CPS_POWER_CP 
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_cps_descr_index_t. The valid values are,
*                        * \ref CY_BLE_CPS_CCCD
*                        * \ref CY_BLE_CPS_SCCD 
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic descriptor value data that
*                     should be sent to the server device.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the CPS service-specific callback is registered
*   with \ref Cy_BLE_CPS_RegisterAttrCallback():
*   * \ref CY_BLE_EVT_CPSC_WRITE_DESCR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, descrIndex etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_cps_descr_value_t.
*   .
*   Otherwise (if an ANC service-specific callback is not registered):
*   * \ref CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*
*   * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure ( \ref cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_cps_char_index_t charIndex,
                                                               cy_en_ble_cps_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to CPS client structure with attribute handles */
    cy_stc_ble_cpsc_t *cpscPtr = (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_CPS_CHAR_COUNT) || (descrIndex >= CY_BLE_CPS_DESCR_COUNT) ||
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;
        
        /* Fill all fields of Write Request structure ... */
        writeReqParam.handleValPair.attrHandle = cpscPtr->charInfo[charIndex].descrHandle[descrIndex];
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;

        /* ... and send request to server device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_cpscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to the peer device to get the characteristic descriptor of the
*  specified characteristic of Cycling Power service.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cps_char_index_t. The valid values are,
*                        * \ref CY_BLE_CPS_POWER_MEASURE
*                        * \ref CY_BLE_CPS_POWER_FEATURE
*                        * \ref CY_BLE_CPS_SENSOR_LOCATION
*                        * \ref CY_BLE_CPS_POWER_VECTOR
*                        * \ref CY_BLE_CPS_POWER_CP 
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_cps_descr_index_t. The valid values are,
*                        * \ref CY_BLE_CPS_CCCD
*                        * \ref CY_BLE_CPS_SCCD *
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | This operation is not permitted on the specified attribute.
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
*  \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the CPS service-specific callback is registered
*   with \ref Cy_BLE_CPS_RegisterAttrCallback():
*   * \ref CY_BLE_EVT_CPSC_READ_DESCR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     descrIndex, value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_cps_descr_value_t.
*   .
*   Otherwise (if an CPS service-specific callback is not registered):
*   * \ref CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*     successfully read on the peer device, the details (handle, value, etc.) are
*     provided with an event parameter structure 
*     \ref cy_stc_ble_gattc_read_rsp_param_t.
*
*   * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_cps_char_index_t charIndex,
                                                               cy_en_ble_cps_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to CPS client structure with attribute handles */
    cy_stc_ble_cpsc_t *cpscPtr = (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_CPS_CHAR_COUNT) || (descrIndex >= CY_BLE_CPS_DESCR_COUNT) ||
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = cpscPtr->charInfo[charIndex].descrHandle[descrIndex];
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_cpscReqHandle[discIdx] = cpscPtr->charInfo[charIndex].descrHandle[descrIndex];
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_TimeOutEventHandler
***************************************************************************//**
*
*  Handles Timer event.
*
*  \param eventParam:  The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_CPSC_TimeOutEventHandler(const cy_stc_ble_timeout_param_t *eventParam)
{
    if(((eventParam)->reasonCode == CY_BLE_GENERIC_APP_TO) && (Cy_BLE_CPS_ApplCallback != NULL))
    {
        uint32_t idx;
        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            if(((cy_ble_cpscFlag[idx] & CY_BLE_CPSC_FLAG_CP_PROCEDURE_IN_PROGRESS) != 0u) &&
               (eventParam->timerHandle == cy_ble_cpscCpTimeout[idx].timerHandle))
            {
                cy_stc_ble_cps_char_value_t timeoutValue =
                {
                    .connHandle = cy_ble_connHandle[cy_ble_configPtr->context->discovery[idx].connIndex],
                    .charIndex  = CY_BLE_CPS_POWER_CP
                };

                cy_ble_cpscFlag[idx] &= (uint8_t) ~CY_BLE_CPSC_FLAG_CP_PROCEDURE_IN_PROGRESS;
                cy_ble_cpscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

                Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSC_TIMEOUT, &timeoutValue);
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                break;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles Notification event.
*
*  \param eventParam:  The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_CPSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    cy_en_ble_cps_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_CPS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to CPS client structure with attribute handles */
        cy_stc_ble_cpsc_t *cpscPtr = (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[discIdx];

        for(locCharIndex = CY_BLE_CPS_POWER_MEASURE; locCharIndex < CY_BLE_CPS_CHAR_COUNT; locCharIndex++)
        {
            if(cpscPtr->charInfo[locCharIndex].valueHandle == eventParam->handleValPair.attrHandle)
            {
                cy_stc_ble_cps_char_value_t notifValue;
                
                notifValue.connHandle = eventParam->connHandle;
                notifValue.charIndex  = locCharIndex;
                notifValue.value      = &eventParam->handleValPair.value;

                Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSC_NOTIFICATION, &notifValue);
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                break;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_IndicationEventHandler
***************************************************************************//**
*
*  Handles Indication event.
*
*  \param eventParam:  The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_CPSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam)
{
    cy_en_ble_cps_char_index_t locCharIndex;
    cy_stc_ble_cps_char_value_t indicationValue = { .connHandle = eventParam->connHandle };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_CPS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to CPS client structure with attribute handles */
        cy_stc_ble_cpsc_t *cpscPtr = (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[discIdx];

        for(locCharIndex = CY_BLE_CPS_POWER_MEASURE; locCharIndex < CY_BLE_CPS_CHAR_COUNT; locCharIndex++)
        {
            if(cpscPtr->charInfo[locCharIndex].valueHandle == eventParam->handleValPair.attrHandle)
            {
                /* Stop the timer. CP procedure is finished when received indication sent by the CP Sensor. */
                if((cy_ble_cpscReqHandle[discIdx] == cpscPtr->charInfo[CY_BLE_CPS_POWER_CP].valueHandle) &&
                   ((cy_ble_cpscFlag[discIdx] & CY_BLE_CPSC_FLAG_CP_PROCEDURE_IN_PROGRESS) != 0u))
                {
                    (void)Cy_BLE_StopTimer(&cy_ble_cpscCpTimeout[discIdx]);
                    cy_ble_cpscFlag[discIdx] &= (uint8_t) ~CY_BLE_CPSC_FLAG_CP_PROCEDURE_IN_PROGRESS;
                }

                indicationValue.charIndex = locCharIndex;
                indicationValue.value = &eventParam->handleValPair.value;

                Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSC_INDICATION, &indicationValue);
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                break;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles Read Response event.
*
*  \param eventParam:  The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_CPSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t locReqHandle = 0u;
    cy_en_ble_cps_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    /* Get pointer (with offset) to CPS client structure with attribute handles */
    cy_stc_ble_cpsc_t *cpscPtr = (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_CPS_ApplCallback != NULL) &&
       (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_cpscReqHandle[discIdx]))
    {
        for(locCharIndex = CY_BLE_CPS_POWER_MEASURE; (locCharIndex < CY_BLE_CPS_CHAR_COUNT) && (locReqHandle == 0u);
            locCharIndex++)
        {
            if(cy_ble_cpscReqHandle[discIdx] == cpscPtr->charInfo[locCharIndex].valueHandle)
            {
                cy_stc_ble_cps_char_value_t locCharValue;
                
                locCharValue.connHandle = eventParam->connHandle;
                locCharValue.charIndex  = locCharIndex;
                locCharValue.value      = &eventParam->value;
                
                cy_ble_cpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSC_READ_CHAR_RESPONSE, &locCharValue);
                locReqHandle = 1u;
            }
            else
            {
                cy_en_ble_cps_descr_index_t locDescIndex;

                for(locDescIndex = CY_BLE_CPS_CCCD; (locDescIndex < CY_BLE_CPS_DESCR_COUNT) &&
                    (locReqHandle == 0u); locDescIndex++)
                {
                    if(cy_ble_cpscReqHandle[discIdx] == cpscPtr->charInfo[locCharIndex].
                        descrHandle[locDescIndex])
                    {
                        cy_stc_ble_cps_descr_value_t locDescrValue;
                        
                        locDescrValue.connHandle = eventParam->connHandle;
                        locDescrValue.charIndex  = locCharIndex;
                        locDescrValue.descrIndex = locDescIndex;
                        locDescrValue.value      = &eventParam->value;

                        cy_ble_cpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                        Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSC_READ_DESCR_RESPONSE, &locDescrValue);
                        locReqHandle = 1u;
                    }
                }
            }
        }
        if(locReqHandle != 0u)
        {
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles Write Response event.
*
*  \param eventParam:  The pointer to a data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_CPSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint8_t locReqHandle = 0u;
    cy_en_ble_cps_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_CPS_ApplCallback != NULL) &&
       (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_cpscReqHandle[discIdx]))
    {
        /* Get pointer (with offset) to CPS client structure with attribute handles */
        cy_stc_ble_cpsc_t *cpscPtr = (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[discIdx];

        for(locCharIndex = CY_BLE_CPS_POWER_MEASURE; (locCharIndex < CY_BLE_CPS_CHAR_COUNT) && (locReqHandle == 0u);
            locCharIndex++)
        {
            if(cy_ble_cpscReqHandle[discIdx] == cpscPtr->charInfo[locCharIndex].valueHandle)
            {
                cy_stc_ble_cps_char_value_t locCharValue;
                
                locCharValue.connHandle = *eventParam;
                locCharValue.charIndex  = locCharIndex;
                locCharValue.value      = NULL;

                cy_ble_cpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSC_WRITE_CHAR_RESPONSE, &locCharValue);
                locReqHandle = 1u;
            }
            else
            {
                cy_en_ble_cps_descr_index_t locDescIndex;

                for(locDescIndex = CY_BLE_CPS_CCCD; (locDescIndex < CY_BLE_CPS_DESCR_COUNT) &&
                    (locReqHandle == 0u); locDescIndex++)
                {
                    if(cy_ble_cpscReqHandle[discIdx] == cpscPtr->charInfo[locCharIndex].
                        descrHandle[locDescIndex])
                    {
                        cy_stc_ble_cps_descr_value_t locDescrValue;
                        
                        locDescrValue.connHandle = *eventParam;
                        locDescrValue.charIndex  = locCharIndex;
                        locDescrValue.descrIndex = locDescIndex;
                        locDescrValue.value      = NULL;

                        cy_ble_cpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                        Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSC_WRITE_DESCR_RESPONSE, &locDescrValue);
                        locReqHandle = 1u;
                    }
                }
            }
        }
        if(locReqHandle != 0u)
        {
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_GetCharacteristicValueHandle
***************************************************************************//**
*
*  Returns the discovered peer device characteristic value handle.
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of a service characteristic.
*
*  \return
*   Returns characteristic value handle of type cy_ble_gatt_db_attr_handle_t.
*   * CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE: when a peer device doesn't have an
*     optional characteristic
*
******************************************************************************/
cy_ble_gatt_db_attr_handle_t Cy_BLE_CPSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_cps_char_index_t charIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

    if( (Cy_BLE_GetDiscoveryIdx(connHandle) < cy_ble_configPtr->params->maxClientCount) &&
            (charIndex < CY_BLE_CPS_CHAR_COUNT) )
    {
        /* Get pointer (with offset) to CPS client structure with attribute handles */
        cy_stc_ble_cpsc_t *cpscPtr =
                (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[Cy_BLE_GetDiscoveryIdx(connHandle)];

        returnHandle = cpscPtr->charInfo[charIndex].valueHandle;
    }
    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_AIOSC_GetCharacteristicDescriptorHandle
***************************************************************************//**
*
*  Returns the discovered peer device characteristic descriptor handle.
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of a service characteristic.
*  \param descrIndex:   The index of the descriptor.
*
* \return
*  Returns characteristic value handle of type cy_ble_gatt_db_attr_handle_t.
*  * CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE: when a peer device doesn't
*    have an optional descriptor
*
******************************************************************************/
cy_ble_gatt_db_attr_handle_t Cy_BLE_CPSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_cps_char_index_t charIndex,
                                                                            cy_en_ble_cps_descr_index_t descrIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    if((Cy_BLE_GetDiscoveryIdx(connHandle) < cy_ble_configPtr->params->maxClientCount) &&
           (charIndex < CY_BLE_CPS_CHAR_COUNT) && (descrIndex < CY_BLE_CPS_DESCR_COUNT))
    {
        /* Get pointer (with offset) to CPS client structure with attribute handles */
        cy_stc_ble_cpsc_t *cpscPtr =
                (cy_stc_ble_cpsc_t *)&cy_ble_cpscConfigPtr->attrInfo[Cy_BLE_GetDiscoveryIdx(connHandle)];

        returnHandle = cpscPtr->charInfo[charIndex].descrHandle[descrIndex];
    }

    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles Error Response event.
*
*  \param eventParam:  The pointer to a data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_CPSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
        if(eventParam->errInfo.attrHandle == cy_ble_cpscReqHandle[discIdx])
        {
            cy_ble_cpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}

/******************************************************************************
* Function Name: Cy_BLE_CPSC_StartObserve
***************************************************************************//**
*
*   This function is used for observing GAP peripheral devices.
*   A device performing the observer role receives only advertisement data from
*   devices irrespective of their discoverable mode settings. Advertisement
*   data received is provided by the event, \ref CY_BLE_EVT_CPSC_SCAN_PROGRESS_RESULT.
*   This procedure sets the scanType sub parameter to passive scanning.
*
*   If 'scanTo' sub-parameter is set to zero value, then passive scanning
*   procedure will continue until you call Cy_BLE_CPSC_StopObserve().
*   Possible generated events are:
*   * #CY_BLE_EVT_CPSC_SCAN_PROGRESS_RESULT
*
*  \param scanParamIndex:  The index of the scan configuration.
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. The following are possible error codes.
*
*   Error Codes                           | Description
*   ------------                          | -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_INVALID_PARAMETER        | On specifying NULL as input parameter. for 'scanInfo' or if any element within 'scanInfo' has an invalid value.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | Memory allocation failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSC_StartObserve(uint8_t scanParamIndex)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gapc_disc_info_t locDiscoveryInfo;
    
    locDiscoveryInfo.discProcedure    = CY_BLE_GAPC_OBSER_PROCEDURE;
    locDiscoveryInfo.scanType         = CY_BLE_GAPC_PASSIVE_SCANNING;
    locDiscoveryInfo.scanIntv         = cy_ble_configPtr->gapcScanParams[scanParamIndex].fastScanInterval;
    locDiscoveryInfo.scanWindow       = cy_ble_configPtr->gapcScanParams[scanParamIndex].fastScanWindow;
    locDiscoveryInfo.ownAddrType      = CY_BLE_GAP_ADDR_TYPE_PUBLIC;
    locDiscoveryInfo.scanFilterPolicy = CY_BLE_GAPC_ADV_ACCEPT_ALL_PKT;
    locDiscoveryInfo.scanTo           = 0u;          /* Disable Timeout */
    locDiscoveryInfo.filterDuplicates = CY_BLE_GAPC_FILTER_DUP_DISABLE;

    apiResult = Cy_BLE_GAPC_StartDiscovery(&locDiscoveryInfo);

    if(apiResult == CY_BLE_SUCCESS)
    {
        cy_ble_cpscObserverFlag |= CY_BLE_CPSC_FLAG_START_OBSERVE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_StopObserve
***************************************************************************//**
*
*   This function used to stop the discovery of devices.
*
*  \return
*  \ref cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. The following are possible error codes.
*
*   Error Codes                           | Description
*   ------------                          | -----------
*   CY_BLE_SUCCESS                        | On successful operation.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | Memory allocation failed.
*   CY_BLE_ERROR_INVALID_STATE            | On calling this function not in CPS observing state.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CPSC_StopObserve(void)
{
    cy_en_ble_api_result_t apiResult;

    if((cy_ble_cpscObserverFlag & CY_BLE_CPSC_FLAG_OBSERVE_IN_PROGRESS) != 0u)
    {
        apiResult = Cy_BLE_GAPC_StopDiscovery();
        cy_ble_cpscObserverFlag |= CY_BLE_CPSC_FLAG_STOP_OBSERVE;
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_ScanStartStopEventHandler
***************************************************************************//**
*
*  This function handles #CY_BLE_EVT_GAPC_SCAN_START_STOP event for CPS.
*
******************************************************************************/
static void Cy_BLE_CPSC_ScanStartStopEventHandler(void)
{
    if((cy_ble_cpscObserverFlag & CY_BLE_CPSC_FLAG_START_OBSERVE) != 0u)
    {
        cy_ble_cpscObserverFlag |= CY_BLE_CPSC_FLAG_OBSERVE_IN_PROGRESS;
        cy_ble_cpscObserverFlag &= (uint8_t) ~CY_BLE_CPSC_FLAG_START_OBSERVE;
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
    if((cy_ble_cpscObserverFlag & CY_BLE_CPSC_FLAG_STOP_OBSERVE) != 0u)
    {
        cy_ble_cpscObserverFlag &= (uint8_t) ~(CY_BLE_CPSC_FLAG_OBSERVE_IN_PROGRESS | CY_BLE_CPSC_FLAG_STOP_OBSERVE);
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_ScanProcessEventHandler
***************************************************************************//**
*
*  This function handles #CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT event for CPS.
*
*  \param eventParam:  the pointer to a data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_CPSC_ScanProcessEventHandler(cy_stc_ble_gapc_adv_report_param_t *eventParam)
{
    if((cy_ble_cpscObserverFlag & CY_BLE_CPSC_FLAG_OBSERVE_IN_PROGRESS) != 0u)
    {
        if(eventParam->eventType == CY_BLE_GAPC_NON_CONN_UNDIRECTED_ADV)
        {
            if(Cy_BLE_CPS_ApplCallback != NULL)
            {
                cy_stc_ble_cps_char_value_t broadcastValue;
                cy_stc_ble_gatt_value_t locCharValue;
                uint8_t advIndex = 0u;
                do
                {
                    /* Show Cycling Power Measurement characteristic value from service Data packet. */
                    if((eventParam->data[advIndex] > CY_BLE_CPSC_BROADCAST_DATA_OFFSET) &&
                       (eventParam->data[advIndex + 1u] == (uint8_t)CY_BLE_GAP_ADV_SRVC_DATA_16UUID))
                    {
                        broadcastValue.peerAddrType = eventParam->peerAddrType;
                        broadcastValue.peerBdAddr = eventParam->peerBdAddr;
                        broadcastValue.charIndex = CY_BLE_CPS_POWER_MEASURE;
                        broadcastValue.value = &locCharValue;
                        locCharValue.val = &eventParam->data[advIndex + CY_BLE_CPSC_BROADCAST_DATA_OFFSET];
                        locCharValue.len = (uint16_t)eventParam->data[advIndex] - (CY_BLE_CPSC_BROADCAST_DATA_OFFSET - 1u);
                        Cy_BLE_CPS_ApplCallback((uint32_t)CY_BLE_EVT_CPSC_SCAN_PROGRESS_RESULT, &broadcastValue);
                        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                    }
                    advIndex += eventParam->data[advIndex] + 1u;
                }
                while(advIndex < eventParam->dataLen);
            }
        }
        else    /* Filter for all connectable advertising packets */
        {
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CPS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the CPS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_CPS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_CPSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_CPSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_CPSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_CPSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the CPS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_CPSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            (void)Cy_BLE_CPSS_DisconnectEventHandler();
            break;
        
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_CPSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            Cy_BLE_CPSS_ConfirmationEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
            break;

        case CY_BLE_EVT_GATT_CONNECT_IND:
            cy_ble_cpssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_cpssFlag = 0u;
            break;

        case CY_BLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
            Cy_BLE_CPSS_ConnParamUpdateRspEventHandler((cy_stc_ble_l2cap_conn_update_rsp_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
            Cy_BLE_CPSS_ConnUpdateCompleteEventHandler((cy_stc_ble_gap_conn_param_updated_in_controller_t*)eventParam);
            break;

        case CY_BLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
            Cy_BLE_CPSS_AdvertisementStartStopEventHandler();
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_CPSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the CPS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_CPSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_CPSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_CPSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_CPSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
                break;

            default:
            break;
        }
    }
    else
    {
        /* Handling GATT Client events */
        switch((cy_en_ble_event_t)eventCode)
        {
            case CY_BLE_EVT_GATT_DISCONNECT_IND:
                (void)Cy_BLE_CPSC_DisconnectEventHandler();
                break;
        
             case CY_BLE_EVT_GAPC_SCAN_START_STOP:
                Cy_BLE_CPSC_ScanStartStopEventHandler();
                break;

            case CY_BLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
                Cy_BLE_CPSC_ScanProcessEventHandler((cy_stc_ble_gapc_adv_report_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_ERROR_RSP:
            {
                uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(((cy_stc_ble_gatt_err_param_t*)eventParam)->connHandle);
                if((discIdx < cy_ble_configPtr->params->maxClientCount) && (cy_ble_configPtr->context->discovery[discIdx].autoDiscoveryFlag == 0u) &&
                   (((cy_stc_ble_gatt_err_param_t*)eventParam)->errInfo.errorCode !=
                    CY_BLE_GATT_ERR_ATTRIBUTE_NOT_FOUND))
                {
                    Cy_BLE_CPSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
            break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_CPSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_IND:
                Cy_BLE_CPSC_IndicationEventHandler((cy_stc_ble_gattc_handle_value_ind_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_CPSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_CPSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
                break;

            case CY_BLE_EVT_TIMEOUT:
                Cy_BLE_CPSC_TimeOutEventHandler((cy_stc_ble_timeout_param_t*)eventParam);
                break;

            default:
            break;
        }
    }

    return(gattErr);
}

#endif /* CY_BLE_LIB_HOST_CORE */
#endif /* CY_IP_MXBLESS */


/* [] END OF FILE */

