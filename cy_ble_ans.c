/***************************************************************************//**
* \file cy_ble_ans.c
* \version 3.60
*
* \brief
*  Contains the source code for the Alert Notification service.
*
********************************************************************************
* \copyright
* Copyright 2017-2021, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_ble_event_handler.h"

#if defined(CY_IP_MXBLESS)
#if CY_BLE_LIB_HOST_CORE


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_ANS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_ANSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_ANSC_EventHandler(uint32_t eventCode, void *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_ANSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_ANSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_ANSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_ANSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_ANSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_ANSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_ANSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_ANSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_ANS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_ANSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_ANSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_anscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE ANS server config structure */
const cy_stc_ble_anss_config_t *cy_ble_anssConfigPtr = NULL;

/* The pointer to a global BLE ANS client config structure */
const cy_stc_ble_ansc_config_t *cy_ble_anscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_ANSS_Init
***************************************************************************//**
*
*  This function initializes server of the alert notification service.
*
*  \param config: Configuration structure for the ANS.
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
cy_en_ble_api_result_t Cy_BLE_ANSS_Init(const cy_stc_ble_anss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_anssConfigPtr = config;

        /* Registers event handler for the ANS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_ANS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_ANSS_EventHandlerCallback = &Cy_BLE_ANSS_EventHandler;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANSC_Init
***************************************************************************//**
*
*  This function initializes client of the alert notification service.
*
*  \param config: Configuration structure for the ANS.
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
cy_en_ble_api_result_t Cy_BLE_ANSC_Init(const cy_stc_ble_ansc_config_t *config)
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
        cy_ble_anscConfigPtr = config;

        /* Registers event handler for the ANS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_ANS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_ANSC_EventHandlerCallback = &Cy_BLE_ANSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 ansServIdx = cy_ble_anscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + ansServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to ANS client structure */
                cy_stc_ble_ansc_t *anscPtr = (cy_stc_ble_ansc_t *)&cy_ble_anscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(anscPtr, 0, sizeof(cy_stc_ble_ansc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + ansServIdx].uuid =
                    CY_BLE_UUID_ALERT_NOTIFICATION_SERVICE;
            }

            cy_ble_anscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for ANS is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_ANSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_ans_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_ANS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_ANS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_ANSS_WriteEventHandler
***************************************************************************//**
*
*  Handles Write Request event for alert notification service.
*
*  \param eventParam: The pointer to the data that came with a Write Request for
*                alert notification service.
*
* \return
*  Return a value of type cy_en_ble_gatt_err_code_t:
*   * CY_BLE_GATT_ERR_NONE - The function terminated successfully.
*   * CY_BLE_GATT_ERR_INVALID_HANDLE - The handle of client configuration
*     characteristic descriptor or characteristic of alert notification service
*     is not valid.
*   * CY_BLE_GATT_ERR_UNLIKELY_ERROR - An internal stack error occurred.
*   * CY_BLE_GATT_ERR_REQUEST_NOT_SUPPORTED - The notification property of a
*     specific characteristic of alert notification service is disabled.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_ANSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_stc_ble_ans_char_value_t wrReqParam;
    cy_ble_gatt_db_attr_handle_t tmpHandle;
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    uint32_t fFlag = 0u;
    uint32_t event = (uint32_t)CY_BLE_EVT_ANSS_WRITE_CHAR;

    tmpHandle = eventParam->handleValPair.attrHandle;

    /* Client Characteristic Configuration descriptor Write Request */
    if((tmpHandle == cy_ble_anssConfigPtr->attrInfo->charInfo[CY_BLE_ANS_NEW_ALERT].descrHandle[CY_BLE_ANS_CCCD]) ||
       (tmpHandle == cy_ble_anssConfigPtr->attrInfo->charInfo[CY_BLE_ANS_UNREAD_ALERT_STATUS].descrHandle[CY_BLE_ANS_CCCD]))
    {
        /* Verify that optional notification property is enabled for characteristic */
        if(CY_BLE_IS_NOTIFICATION_SUPPORTED(cy_ble_anssConfigPtr->attrInfo->charInfo[CY_BLE_ANS_NEW_ALERT].charHandle) ||
           CY_BLE_IS_NOTIFICATION_SUPPORTED(cy_ble_anssConfigPtr->attrInfo->charInfo[CY_BLE_ANS_UNREAD_ALERT_STATUS].charHandle))
        {
            if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
            {
                event = (uint32_t)CY_BLE_EVT_ANSS_NOTIFICATION_ENABLED;
            }
            else
            {
                event = (uint32_t)CY_BLE_EVT_ANSS_NOTIFICATION_DISABLED;
            }

            if(tmpHandle == cy_ble_anssConfigPtr->attrInfo->charInfo[CY_BLE_ANS_NEW_ALERT].descrHandle[CY_BLE_ANS_CCCD])
            {
                wrReqParam.charIndex = CY_BLE_ANS_NEW_ALERT;
            }
            else
            {
                wrReqParam.charIndex = CY_BLE_ANS_UNREAD_ALERT_STATUS;
            }

            /* Value is NULL for descriptors */
            wrReqParam.value = NULL;
   
        }
        else
        {
            gattErr = CY_BLE_GATT_ERR_REQUEST_NOT_SUPPORTED;
        }
    }
    else if(tmpHandle == cy_ble_anssConfigPtr->attrInfo->charInfo[CY_BLE_ANS_ALERT_NTF_CONTROL_POINT].charHandle)
    {
        /* Validate Command ID and Category ID ranges */
        if((eventParam->handleValPair.value.val[0u] <= CY_BLE_ANS_IMM_UNREAD_ALERT_STATUS_NTF) &&
           ((eventParam->handleValPair.value.val[1u] <= CY_BLE_ANS_CAT_ID_INSTANT_MESSAGE) ||
            (eventParam->handleValPair.value.val[1u] == CY_BLE_ANS_CAT_ID_ALL)))
        {
            wrReqParam.charIndex = CY_BLE_ANS_ALERT_NTF_CONTROL_POINT;
            wrReqParam.value = &eventParam->handleValPair.value;
        }
        else /* Command ID or category ID received is invalid */
        {
            /* Erroneous request won't be notified to user but Error Response will be sent. */
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;

            gattErr = CY_BLE_GATT_ERR_ANS_COMMAND_NOT_SUPPORTED;
        }
    }
    else
    {
        /* Requested handle does not belong to alert notification service
         *  characteristic or descriptor. */
        fFlag = 1u;
    }

    if((gattErr == CY_BLE_GATT_ERR_NONE) && (fFlag == 0u))
    {
        /* Write value to GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.handleValuePair = eventParam->handleValPair;
        dbAttrValInfo.connHandle      = eventParam->connHandle;
        dbAttrValInfo.offset          = 0u;
        dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

        gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);

        if(gattErr == CY_BLE_GATT_ERR_NONE)
        {
            wrReqParam.connHandle = eventParam->connHandle;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;

            if(Cy_BLE_ANS_ApplCallback != NULL)
            {
                Cy_BLE_ANS_ApplCallback(event, &wrReqParam);
            }
        }
    }

    if(CY_BLE_GATT_ERR_NONE != gattErr)
    {
        /* Indicates that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_ANSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a characteristic value of alert notification service, which is a value
*  identified by charIndex, to the local database.
*
*  \param charIndex: The index of the service characteristic of type
*                    \ref cy_en_ble_ans_char_index_t. The valid values are,
*                       * #CY_BLE_ANS_SUPPORTED_NEW_ALERT_CAT
*                       * #CY_BLE_ANS_SUPPORTED_UNREAD_ALERT_CAT
*  \param attrSize:  The size of the characteristic value attribute.
*  \param attrValue: The pointer to characteristic value data that should be
*                    stored in the GATT database.
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
cy_en_ble_api_result_t Cy_BLE_ANSS_SetCharacteristicValue(cy_en_ble_ans_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if(charIndex < CY_BLE_ANS_CHAR_COUNT)
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_anssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
      
        /* Store data in database */
        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_SUCCESS;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets a characteristic value of alert notification service. The value is
*  identified by charIndex.
*
*  \param charIndex: The index of the service characteristic of 
*                    type \ref  cy_en_ble_ans_char_index_t. The valid values are,
*                       * #CY_BLE_ANS_NEW_ALERT
*                       * #CY_BLE_ANS_UNREAD_ALERT_STATUS
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
cy_en_ble_api_result_t Cy_BLE_ANSS_GetCharacteristicValue(cy_en_ble_ans_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if(charIndex < CY_BLE_ANS_CHAR_COUNT)
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_anssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        
        /* Read characteristic value from database */
        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_SUCCESS;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of the specified characteristic of alert
*  notification service.
*
*  \param connHandle: The connection handle
*  \param charIndex:  The index of the service characteristic of type 
*                     \ref cy_en_ble_ans_char_index_t. The valid values are,
*                       * #CY_BLE_ANS_NEW_ALERT
*                       * #CY_BLE_ANS_UNREAD_ALERT_STATUS
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_ans_descr_index_t. The valid value is,
*                       * #CY_BLE_ANS_CCCD
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  The pointer to the location where characteristic 
*                     descriptor value data should be stored.
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_ANSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_ans_char_index_t charIndex,
                                                               cy_en_ble_ans_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if(((charIndex == CY_BLE_ANS_NEW_ALERT) || (charIndex == CY_BLE_ANS_UNREAD_ALERT_STATUS)) &&
       (descrIndex == CY_BLE_ANS_CCCD))
    {
        if(cy_ble_anssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_anssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex];
            dbAttrValInfo.handleValuePair.value.len  = attrSize;
            dbAttrValInfo.handleValuePair.value.val  = attrValue;
            dbAttrValInfo.connHandle                 = connHandle;
            dbAttrValInfo.offset                     = 0u;
            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

            if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
            {
                apiResult = CY_BLE_SUCCESS;
            }
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANSS_SendNotification
***************************************************************************//**
*
*  Sends a notification with the characteristic value, as specified by its 
*  charIndex, to the client device.
*  On enabling notification successfully for a service characteristic it sends out a
*  'handle value notification', which results in #CY_BLE_EVT_ANSC_NOTIFICATION event
*  at the GATT client's end.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
*                     type \ref cy_en_ble_ans_char_index_t. The valid values are,
*                       * #CY_BLE_ANS_UNREAD_ALERT_STATUS
*                       * #CY_BLE_ANS_NEW_ALERT
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that 
*                     should be sent to the client device.
*
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
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_ANSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_ans_char_index_t charIndex,
                                                    uint8_t attrSize,
                                                    uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex == CY_BLE_ANS_NEW_ALERT) || (charIndex == CY_BLE_ANS_UNREAD_ALERT_STATUS))
    {
        /* Send Notification if it is enabled and connected */
        if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
        {
            apiResult = CY_BLE_ERROR_INVALID_STATE;
        }
        else if(!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_anssConfigPtr->attrInfo->charInfo[charIndex].
                                                 descrHandle[CY_BLE_ANS_CCCD]))
        {
            apiResult = CY_BLE_ERROR_NTF_DISABLED;
        }
        else
        {
            /* Fill all fields of Write Request structure ... */
            cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
            ntfReqParam.handleValPair.attrHandle = cy_ble_anssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
            ntfReqParam.handleValPair.value.val  = attrValue;
            ntfReqParam.handleValPair.value.len  = attrSize;
            ntfReqParam.connHandle               = connHandle;
          
            /* Send notification to client using previously filled structure */
            apiResult = Cy_BLE_GATTS_Notification(&ntfReqParam);
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_ANSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* ANS characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_anscCharUuid[CY_BLE_ANS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_SPRTD_NEW_ALRT_CTGRY,
        CY_BLE_UUID_CHAR_NEW_ALERT,
        CY_BLE_UUID_CHAR_SPRT_UNRD_ALRT_CTGRY,
        CY_BLE_UUID_CHAR_UNREAD_ALRT_STATUS,
        CY_BLE_UUID_CHAR_ALERT_NTF_CONTROL_POINT
    };

    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t ansDiscIdx = cy_ble_anscConfigPtr->serviceDiscIdx;
    uint32_t i;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == ansDiscIdx))
    {
        /* Update last characteristic endHandle to declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        for(i = (uint32_t)CY_BLE_ANS_SUPPORTED_NEW_ALERT_CAT; i < (uint32_t)CY_BLE_ANS_CHAR_COUNT; i++)
        {
            /* Get pointer (with offset) to ANS client structure with attribute handles */
            cy_stc_ble_ansc_t *anscPtr = (cy_stc_ble_ansc_t *)&cy_ble_anscConfigPtr->attrInfo[discIdx];

            if(cy_ble_anscCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                if(anscPtr->characteristics[i].charInfo.valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    anscPtr->characteristics[i].charInfo.valueHandle = discCharInfo->valueHandle;
                    anscPtr->characteristics[i].charInfo.properties = discCharInfo->properties;
                    lastEndHandle[discIdx] = &anscPtr->characteristics[i].endHandle;
                }
                else
                {
                    Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo);
                }
            }
        }

        /* Initialize the characteristic endHandle to the service endHandle. The characteristic endHandle will
         * be updated to the declaration handle of the following characteristic, in the following characteristic
         * discovery procedure. */
        if(lastEndHandle[discIdx] != NULL)
        {
            uint32_t locServCount = cy_ble_configPtr->context->discovery[discIdx].servCount;
            uint32_t discServiNum = cy_ble_configPtr->context->discServiCount;

            *lastEndHandle[discIdx] =
                cy_ble_configPtr->context->serverInfo[(discIdx * discServiNum) + locServCount].range.endHandle;
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_ANSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  This event is generated when a server successfully sends the data for
*  #CY_BLE_GATT_FIND_INFO_REQ. Based on the service UUID, an appropriate data
*  structure is populated to the service with a service callback.
*
*  \param  discDescrInfo: The pointer to descriptor information structure.
*
******************************************************************************/
static void Cy_BLE_ANSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t * discDescrInfo)
{
    uint32_t notSupportedDescr = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t ansDiscIdx = cy_ble_anscConfigPtr->serviceDiscIdx;
    uint32_t descIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == ansDiscIdx)
    {
        switch(discDescrInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_CLIENT_CONFIG:
                descIdx = (uint32_t)CY_BLE_ANS_CCCD;
                break;

            default:
                /* Not supported descriptor */
                notSupportedDescr = 1u;
                break;
        }

        if(notSupportedDescr == 0u)
        {
            /* Get pointer (with offset) to ANS client structure with attribute handles */
            cy_stc_ble_ansc_t *anscPtr = (cy_stc_ble_ansc_t *)&cy_ble_anscConfigPtr->attrInfo[discIdx];
            
            if(anscPtr->characteristics[cy_ble_configPtr->context->discovery[discIdx].charCount].descriptors[descIdx] ==
               CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                anscPtr->characteristics[cy_ble_configPtr->context->discovery[discIdx].charCount].descriptors[descIdx] =
                    discDescrInfo->descrHandle;
            }
            else
            {
                /* Duplication of the descriptor */
                Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_DESCR_DUPLICATION, &discDescrInfo);
            }
        }

        /* Indicates that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_ANSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_ANSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t ansDiscIdx = cy_ble_anscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == ansDiscIdx)
    {
        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_ANS_CHAR_COUNT) && (exitFlag == 0u))
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;

            /* Get pointer (with offset) to ANS client structure with attribute handles */
            cy_stc_ble_ansc_t *anscPtr = (cy_stc_ble_ansc_t *)&cy_ble_anscConfigPtr->attrInfo[discIdx];

            if((anscPtr->characteristics[charIdx].endHandle -
                anscPtr->characteristics[charIdx].charInfo.valueHandle) != 0u)
            {
                /* Read characteristic range */
                charRangeInfo->range.startHandle = anscPtr->characteristics[charIdx].charInfo.valueHandle + 1u;
                charRangeInfo->range.endHandle = anscPtr->characteristics[charIdx].endHandle;
                exitFlag = 1u;
            }
            else
            {
                cy_ble_configPtr->context->discovery[discIdx].charCount++;
            }
        }

        /* Indicates that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_ANSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles Notification event for Alert Notification service.
*
*  \param *eventParam: The pointer to a #cy_stc_ble_gattc_handle_value_ntf_param_t
*                                                     data structure specified by
*                                                     the event.
*
******************************************************************************/
static void Cy_BLE_ANSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_ANS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to ANS client structure with attribute handles */
        cy_stc_ble_ansc_t *anscPtr = (cy_stc_ble_ansc_t *)&cy_ble_anscConfigPtr->attrInfo[discIdx];

        if((anscPtr->characteristics[CY_BLE_ANS_NEW_ALERT].charInfo.valueHandle ==
            eventParam->handleValPair.attrHandle) ||
           (anscPtr->characteristics[CY_BLE_ANS_UNREAD_ALERT_STATUS].charInfo.valueHandle ==
            eventParam->handleValPair.attrHandle))
        {
            cy_stc_ble_ans_char_value_t ntfParam;
            ntfParam.charIndex  = CY_BLE_ANS_NEW_ALERT;
            ntfParam.connHandle = eventParam->connHandle;
            ntfParam.value      = &eventParam->handleValPair.value;

            Cy_BLE_ANS_ApplCallback((uint32_t)CY_BLE_EVT_ANSC_NOTIFICATION, (void*)&ntfParam);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_ANSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles Read Response event for Alert Notification service.
*
*  \param eventParam: The pointer to the data that came with a read response for ANS.
*
******************************************************************************/
static void Cy_BLE_ANSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t fFlag = 1u;
    uint32_t attrVal = 0u;
    cy_en_ble_ans_char_index_t idx = CY_BLE_ANS_SUPPORTED_NEW_ALERT_CAT;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_ANS_ApplCallback != NULL) &&
       (cy_ble_anscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to ANS client structure with attribute handles */
        cy_stc_ble_ansc_t *anscPtr = (cy_stc_ble_ansc_t *)&cy_ble_anscConfigPtr->attrInfo[discIdx];

        if(anscPtr->characteristics[CY_BLE_ANS_SUPPORTED_NEW_ALERT_CAT].charInfo.valueHandle ==
           cy_ble_anscReqHandle[discIdx])
        {
            idx = CY_BLE_ANS_SUPPORTED_NEW_ALERT_CAT;
        }
        else if(anscPtr->characteristics[CY_BLE_ANS_SUPPORTED_UNREAD_ALERT_CAT].charInfo.valueHandle ==
                cy_ble_anscReqHandle[discIdx])
        {
            idx = CY_BLE_ANS_SUPPORTED_UNREAD_ALERT_CAT;
        }
        else if(anscPtr->characteristics[CY_BLE_ANS_NEW_ALERT].descriptors[CY_BLE_ANS_CCCD] ==
                cy_ble_anscReqHandle[discIdx])
        {
            /* Attribute is characteristic descriptor */
            attrVal = 1u;
            idx = CY_BLE_ANS_NEW_ALERT;
        }
        else if(anscPtr->characteristics[CY_BLE_ANS_UNREAD_ALERT_STATUS].descriptors[CY_BLE_ANS_CCCD] ==
                cy_ble_anscReqHandle[discIdx])
        {
            /* Attribute is characteristic descriptor */
            attrVal = 1u;
            idx = CY_BLE_ANS_UNREAD_ALERT_STATUS;
        }
        else
        {
            /* No ANS characteristic was requested for read */
            fFlag = 0u;
        }

        if(fFlag != 0u)
        {
            /* Read response for characteristic */
            if(attrVal == 0u)
            {
                /* Fill Alert Notification service read response parameter structure with characteristic info. */
                cy_stc_ble_ans_char_value_t rdRspParam;
                rdRspParam.connHandle = eventParam->connHandle;
                rdRspParam.charIndex  = idx;
                rdRspParam.value      = &eventParam->value;

                rdRspParam.charIndex = idx;
                Cy_BLE_ANS_ApplCallback((uint32_t)CY_BLE_EVT_ANSC_READ_CHAR_RESPONSE, (void*)&rdRspParam);
            }
            else /* Read response for characteristic descriptor */
            {
                /* Fill alert notification service read response parameter structure with characteristic descriptor
                 * info. */
                cy_stc_ble_ans_descr_value_t rdRspParam;
                rdRspParam.connHandle = eventParam->connHandle;
                rdRspParam.charIndex  = idx;
                rdRspParam.descrIndex = CY_BLE_ANS_CCCD;
                rdRspParam.value      = &eventParam->value;
                rdRspParam.charIndex  = idx;
                Cy_BLE_ANS_ApplCallback((uint32_t)CY_BLE_EVT_ANSC_READ_DESCR_RESPONSE, (void*)&rdRspParam);
            }

            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            cy_ble_anscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_ANSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles Write Response event for Alert Notification service.
*
*  \param eventParam: The pointer to the cy_stc_ble_conn_handle_t data structure.
*
******************************************************************************/
static void Cy_BLE_ANSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t fFlag = 1u;
    uint32_t attrVal = 0u;
    cy_en_ble_ans_char_index_t idx = CY_BLE_ANS_SUPPORTED_NEW_ALERT_CAT;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_ANS_ApplCallback != NULL) &&
       (cy_ble_anscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        cy_stc_ble_ans_char_value_t wrRspParam = { .connHandle = *eventParam };

        /* Get pointer (with offset) to ANS client structure with attribute handles */
        cy_stc_ble_ansc_t *anscPtr = (cy_stc_ble_ansc_t *)&cy_ble_anscConfigPtr->attrInfo[discIdx];

        if(anscPtr->characteristics[CY_BLE_ANS_ALERT_NTF_CONTROL_POINT].charInfo.valueHandle ==
           cy_ble_anscReqHandle[discIdx])
        {
            idx = CY_BLE_ANS_ALERT_NTF_CONTROL_POINT;
        }
        else if(anscPtr->characteristics[CY_BLE_ANS_NEW_ALERT].descriptors[CY_BLE_ANS_CCCD] ==
                cy_ble_anscReqHandle[discIdx])
        {
            /* Attribute is characteristic descriptor */
            attrVal = 1u;
            idx = CY_BLE_ANS_NEW_ALERT;
        }
        else if(anscPtr->characteristics[CY_BLE_ANS_UNREAD_ALERT_STATUS].descriptors[CY_BLE_ANS_CCCD] ==
                cy_ble_anscReqHandle[discIdx])
        {
            /* Attribute is characteristic descriptor */
            attrVal = 1u;
            idx = CY_BLE_ANS_UNREAD_ALERT_STATUS;
        }
        else
        {
            /* No ANS characteristic was requested for write */
            fFlag = 0u;
        }

        if(fFlag != 0u)
        {
            /* Write Response for characteristic */
            if(attrVal == 0u)
            {
                wrRspParam.charIndex = idx;
                wrRspParam.value = NULL;

                Cy_BLE_ANS_ApplCallback((uint32_t)CY_BLE_EVT_ANSC_WRITE_CHAR_RESPONSE, (void*)&wrRspParam);
            }
            else /* Write Response for characteristic descriptor */
            {
                /* Fill alert notification service Write Response parameter structure with
                 * characteristic descriptor info. */
                cy_stc_ble_ans_descr_value_t wrDescRspParam;
                wrDescRspParam.connHandle = *eventParam;
                wrDescRspParam.charIndex  = idx;
                wrDescRspParam.descrIndex = CY_BLE_ANS_CCCD;
                wrDescRspParam.value      = NULL;   

                Cy_BLE_ANS_ApplCallback((uint32_t)CY_BLE_EVT_ANSC_WRITE_DESCR_RESPONSE, (void*)&wrDescRspParam);
            }

            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            cy_ble_anscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_ANSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles Error Response event for Alert Notification service.
*
*  \param eventParam: The pointer to the cy_stc_ble_gatt_err_param_t structure.
*
******************************************************************************/
static void Cy_BLE_ANSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        if(cy_ble_anscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] == eventParam->errInfo.attrHandle)
        {
            cy_ble_anscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_ANSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result, a Write Request is
*  sent to the GATT server and on successful execution of the request on the
*  server side, the #CY_BLE_EVT_ANSS_WRITE_CHAR event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
*                     type \ref cy_en_ble_ans_char_index_t. The valid values are,
*                       * #CY_BLE_ANS_UNREAD_ALERT_STATUS
*                       * #CY_BLE_ANS_NEW_ALERT
*  \param attrSize:   Size of the characteristic value attribute.
*  \param attrValue:  Pointer to the characteristic value data that should be
*                     sent to the server device.
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
*   If the ANS service-specific callback is registered
*   with \ref Cy_BLE_ANS_RegisterAttrCallback():
*   * #CY_BLE_EVT_ANSC_WRITE_CHAR_RESPONSE - If the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_ans_char_value_t.
*   
*   Otherwise (if the ANS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with event
*     parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_ANSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_ans_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    /* Get pointer (with offset) to ANS client structure with attribute handles */
    cy_stc_ble_ansc_t *anscPtr = (cy_stc_ble_ansc_t *)&cy_ble_anscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((discIdx < cy_ble_configPtr->params->maxClientCount) &&
            (attrValue != NULL) && (charIndex == CY_BLE_ANS_ALERT_NTF_CONTROL_POINT) &&
            (anscPtr->characteristics[CY_BLE_ANS_ALERT_NTF_CONTROL_POINT].charInfo.valueHandle !=
             CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Fill all fields of write command request structure ... */
        cy_stc_ble_gattc_write_cmd_req_t writeReqParam;
        writeReqParam.connHandle               = connHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.handleValPair.attrHandle = anscPtr->characteristics[CY_BLE_ANS_ALERT_NTF_CONTROL_POINT].
                                                    charInfo.valueHandle;
        
        /* Send request to write characteristic value */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);

        /* Save handle to support service-specific Write Response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_anscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }
    else
    {
        /* Validation of the input parameters failed */
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANSC_GetCharacteristicValue
***************************************************************************//**
*
*  Sends a request to the peer device to get a characteristic value, as
*  identified by its charIndex.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
                      type \ref cy_en_ble_ans_char_index_t. The valid values are,
*                       * #CY_BLE_ANS_UNREAD_ALERT_STATUS
*                       * #CY_BLE_ANS_NEW_ALERT
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
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS),
*   the following events can appear: \n
*   If the ANS service-specific callback is registered
*      with \ref Cy_BLE_ANS_RegisterAttrCallback():
*   * #CY_BLE_EVT_ANSC_READ_CHAR_RESPONSE - If the requested attribute is
*     successfully read on the peer device, the details 
*     (charIndex , value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_ans_char_value_t.
*   .
*   Otherwise (if an ANS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_ANSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_ans_char_index_t charIndex)
{
    cy_stc_ble_gattc_read_req_t readReqParam;
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(discIdx >= cy_ble_configPtr->params->maxClientCount)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Get pointer (with offset) to ANS client structure with attribute handles */
        cy_stc_ble_ansc_t *anscPtr = (cy_stc_ble_ansc_t *)&cy_ble_anscConfigPtr->attrInfo[discIdx];

        /* Select characteristic */
        switch(charIndex)
        {
            case CY_BLE_ANS_SUPPORTED_NEW_ALERT_CAT:
                readReqParam.attrHandle = anscPtr->characteristics[CY_BLE_ANS_SUPPORTED_NEW_ALERT_CAT].
                                           charInfo.valueHandle;
                break;

            case CY_BLE_ANS_SUPPORTED_UNREAD_ALERT_CAT:
                readReqParam.attrHandle = anscPtr->characteristics[CY_BLE_ANS_SUPPORTED_UNREAD_ALERT_CAT].
                                           charInfo.valueHandle;
                break;

            default:
                /* Characteristic wasn't found */
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
                break;
        }

        if(apiResult == CY_BLE_SUCCESS)
        {
            readReqParam.connHandle = connHandle;

            /* Send request to read characteristic value */
            apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_anscReqHandle[discIdx] = readReqParam.attrHandle;
            }
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to the peer device to set the characteristic descriptor of the
*  specified characteristic of alert notification service.
*
*  Internally, Write Request is sent to the GATT server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*       * #CY_BLE_EVT_ANSS_NOTIFICATION_ENABLED
*       * #CY_BLE_EVT_ANSS_NOTIFICATION_DISABLED
*
*  \param connHandle: The BLE peer device connection handle.
*  \param charIndex:  The index of the ANS characteristic.
*  \param descrIndex: The index of the ANS characteristic descriptor.
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  Pointer to the characteristic descriptor value data that should be
*                     sent to the server device.
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
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*   If successful execution (return value = #CY_BLE_SUCCESS),
*   the following events can appear: \n
*   If the ANS service-specific callback is registered
*   with \ref Cy_BLE_ANS_RegisterAttrCallback():
*   * #CY_BLE_EVT_ANSC_WRITE_DESCR_RESPONSE - If the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, descrIndex, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_ans_descr_value_t.
*   .
*   Otherwise (if an ANC service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure ( \ref cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_ANSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_ans_char_index_t charIndex,
                                                               cy_en_ble_ans_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((discIdx < cy_ble_configPtr->params->maxClientCount) && (attrValue != NULL) && 
            (descrIndex == CY_BLE_ANS_CCCD) && (attrSize == CY_BLE_CCCD_LEN))
    {
        /* Get pointer (with offset) to ANS client structure with attribute handles */
        cy_stc_ble_ansc_t *anscPtr = (cy_stc_ble_ansc_t *)&cy_ble_anscConfigPtr->attrInfo[discIdx];

        if((charIndex == CY_BLE_ANS_NEW_ALERT) || (charIndex == CY_BLE_ANS_UNREAD_ALERT_STATUS))
        {
            /* Fill all fields of Write Request structure ... */
            cy_stc_ble_gattc_write_req_t writeReqParam;
            writeReqParam.handleValPair.value.val = attrValue;
            writeReqParam.handleValPair.value.len = attrSize;
            writeReqParam.connHandle              = connHandle;
            
            if(charIndex == CY_BLE_ANS_NEW_ALERT)
            {
                writeReqParam.handleValPair.attrHandle = anscPtr->characteristics[CY_BLE_ANS_NEW_ALERT].
                                                          descriptors[CY_BLE_ANS_CCCD];
            }
            else
            {
                writeReqParam.handleValPair.attrHandle =
                    anscPtr->characteristics[CY_BLE_ANS_UNREAD_ALERT_STATUS].descriptors[CY_BLE_ANS_CCCD];
            }
            /* ... and send request to server device. */
            apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_anscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
            }
        }
    }
    else
    {
        /* Characteristic has not been discovered or has invalid fields */
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to the peer device to get the characteristic descriptor of the
*  specified characteristic of alert notification service.
*
*  \param connHandle: BLE peer device connection handle.
*  \param charIndex:  The index of the service characteristic.
*  \param descrIndex: The index of the service characteristic descriptor.
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
* \events
*   If successful execution (return value = #CY_BLE_SUCCESS),
*   the following events can appear: \n
*   If the ANS service-specific callback is registered
*   with \ref Cy_BLE_ANS_RegisterAttrCallback():
*   * #CY_BLE_EVT_ANSC_READ_DESCR_RESPONSE - If the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     descrIndex, value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_ans_descr_value_t.
*   
*   Otherwise (if an ANS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*     successfully read on the peer device, the details (handle, value, etc.) are
*     provided with an event parameter structure.
*     \ref cy_stc_ble_gattc_read_rsp_param_t.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_ANSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_ans_char_index_t charIndex,
                                                               uint8_t descrIndex)
{
    cy_stc_ble_gattc_read_req_t readReqParam;
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((discIdx < cy_ble_configPtr->params->maxClientCount) &&
            ((charIndex == CY_BLE_ANS_NEW_ALERT) || (charIndex == CY_BLE_ANS_UNREAD_ALERT_STATUS)) &&
            (descrIndex == (uint8_t)CY_BLE_ANS_CCCD))
    {
        /* Get pointer (with offset) to ANS client structure with attribute handles */
        cy_stc_ble_ansc_t *anscPtr = (cy_stc_ble_ansc_t *)&cy_ble_anscConfigPtr->attrInfo[discIdx];

        if(charIndex == CY_BLE_ANS_NEW_ALERT)
        {
            readReqParam.attrHandle = anscPtr->characteristics[CY_BLE_ANS_NEW_ALERT].
                                       descriptors[CY_BLE_ANS_CCCD];
        }
        else
        {
            readReqParam.attrHandle = anscPtr->characteristics[CY_BLE_ANS_UNREAD_ALERT_STATUS].
                                       descriptors[CY_BLE_ANS_CCCD];
        }

        readReqParam.connHandle = connHandle;
        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_anscReqHandle[discIdx] = readReqParam.attrHandle;
        }
    }
    else
    {
        /* Characteristic has not been discovered or had invalid fields */
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the alert notification service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_ANS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_ANSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_ANSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_ANSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_ANSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_ANSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the alert notification service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_ANSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_ANSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_ANSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the alert notification service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_ANSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_ANSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_ANSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_ANSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
            case CY_BLE_EVT_GATTC_ERROR_RSP:
            {
                uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(((cy_stc_ble_gatt_err_param_t*)eventParam)->connHandle);

                if((discIdx < cy_ble_configPtr->params->maxClientCount) &&
                   (cy_ble_configPtr->context->discovery[discIdx].autoDiscoveryFlag == 0u) &&
                   (((cy_stc_ble_gatt_err_param_t*)eventParam)->errInfo.errorCode != CY_BLE_GATT_ERR_ATTRIBUTE_NOT_FOUND))
                {
                    Cy_BLE_ANSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_ANSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;


            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_ANSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_ANSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
