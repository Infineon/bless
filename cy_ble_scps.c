/***************************************************************************//**
* \file cy_ble_scps.c
* \version 3.50
*
* \brief
*  Contains the source code for the Scan Parameter service.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
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

static cy_en_ble_gatt_err_code_t Cy_BLE_SCPSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_SCPSC_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_SCPS_EventHandler(uint32_t eventCode, void *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_SCPSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_SCPSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_SCPSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_SCPSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_SCPSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_SCPSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_SCPSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_SCPSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_SCPS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_SCPSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_SCPSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_scpscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to the global BLE SCPS server config structure */
const cy_stc_ble_scpss_config_t *cy_ble_scpssConfigPtr = NULL;

/* The pointer to the global BLE SCPS client config structure */
const cy_stc_ble_scpsc_config_t *cy_ble_scpscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_SCPSS_Init
***************************************************************************//**
*
*  This function initializes server of the SCPS service.
*
*  \param config: Configuration structure for the Scan Parameter service.
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
cy_en_ble_api_result_t Cy_BLE_SCPSS_Init(const cy_stc_ble_scpss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_scpssConfigPtr = config;

        /* Registers event handler for the SCPS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_SCPS_EventHandler);
    }

    /* Registers a pointer to client event handler function */
    Cy_BLE_SCPSS_EventHandlerCallback = &Cy_BLE_SCPSS_EventHandler;

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_SCPSC_Init
***************************************************************************//**
*
*  This function initializes client of the Scan Parameter service.
*
*  \param config: Configuration structure for the Scan Parameter service.
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
cy_en_ble_api_result_t Cy_BLE_SCPSC_Init(const cy_stc_ble_scpsc_config_t *config)
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
        cy_ble_scpscConfigPtr = config;

        /* Registers event handler for the SCPS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_SCPS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_SCPSC_EventHandlerCallback = &Cy_BLE_SCPSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt = cy_ble_configPtr->context->discServiCount;
            uint32 scpsServIdx = cy_ble_scpscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + scpsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to SCPS client structure */
                cy_stc_ble_scpsc_t *scpscPtr = (cy_stc_ble_scpsc_t *)&cy_ble_scpscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(scpscPtr, 0, sizeof(cy_stc_ble_scpsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + scpsServIdx].uuid =
                    CY_BLE_UUID_SCAN_PARAM_SERVICE;
            }

            cy_ble_scpscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_SCPS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for SCPS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_SCPSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_scps_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_SCPS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_SCPS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_SCPSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a characteristic value of the Scan Parameters service, which is
*  identified by charIndex.
*
*  \param charIndex: The index of the service characteristic.
*              * CY_BLE_SCPS_SCAN_INT_WIN - The Scan Interval Window characteristic index
*              * CY_BLE_SCPS_SCAN_REFRESH - The Scan Refresh characteristic index
*
*  \param attrSize: The size of the characteristic value attribute.
*
*  \param attrValue: The pointer to the characteristic value data that should be
*               stored to the GATT database.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | An optional characteristic is absent
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_SCPSS_SetCharacteristicValue(cy_en_ble_scps_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    cy_ble_gatt_db_attr_handle_t charValueHandle;

    if(charIndex >= CY_BLE_SCPS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        uint16_t scanInterval;
        uint16_t locScanWindow;

        if(charIndex == CY_BLE_SCPS_SCAN_INT_WIN)
        {
            scanInterval = Cy_BLE_Get16ByPtr(attrValue);
            locScanWindow = Cy_BLE_Get16ByPtr(attrValue + sizeof(scanInterval));

            if( (scanInterval < CY_BLE_SCAN_INTERVAL_WINDOW_MIN) || (scanInterval > CY_BLE_SCAN_INTERVAL_WINDOW_MAX) ||
                (locScanWindow < CY_BLE_SCAN_INTERVAL_WINDOW_MIN) || (locScanWindow > scanInterval) ||
                (attrSize > CY_BLE_INTERVAL_WINDOW_CHAR_LEN) )
            {
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
            }
            else
            {
                charValueHandle = cy_ble_scpssConfigPtr->attrInfo->intervalWindowCharHandle;
            }
        }
        else    /* Scan Refresh characteristic */
        {
            if(attrSize > CY_BLE_REFRESH_CHAR_LEN)
            {
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
            }
            else
            {
                charValueHandle = cy_ble_scpssConfigPtr->attrInfo->refreshCharHandle;
            }
        }
    }

    if(apiResult == CY_BLE_SUCCESS)
    {
        if(charValueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
        else
        {
            /* Store data in database */
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.connHandle.bdHandle        = 0u;
            dbAttrValInfo.connHandle.attId           = 0u;
            dbAttrValInfo.handleValuePair.attrHandle = charValueHandle;
            dbAttrValInfo.handleValuePair.value.len  = attrSize;
            dbAttrValInfo.handleValuePair.value.val  = attrValue;
            dbAttrValInfo.offset                     = 0u;
            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

            if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
            {
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return(apiResult);
}


/*******************************************************************************
* Function Name: Cy_BLE_SCPSS_GetCharacteristicValue
****************************************************************************//**
*
*  Gets a characteristic value of the Scan Parameters service, which is identified
*  by charIndex.
*
*  \param charIndex: The index of the service characteristic.
*     * CY_BLE_SCPS_SCAN_INT_WIN - The Scan Interval Window characteristic index
*     * CY_BLE_SCPS_SCAN_REFRESH - The Scan Refresh characteristic index
*
*  \param attrSize:  The size of the characteristic value attribute.
*
*  \param attrValue: The pointer to the location where characteristic value data
*                    should be stored.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | An optional characteristic is absent
*
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_SCPSS_GetCharacteristicValue(cy_en_ble_scps_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;
    cy_ble_gatt_db_attr_handle_t charValueHandle;

    if(charIndex >= CY_BLE_SCPS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        if(charIndex == CY_BLE_SCPS_SCAN_INT_WIN)
        {
            if(attrSize > CY_BLE_INTERVAL_WINDOW_CHAR_LEN)
            {
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
            }
            else
            {
                charValueHandle = cy_ble_scpssConfigPtr->attrInfo->intervalWindowCharHandle;
            }
        }
        else    /* Scan Refresh characteristic */
        {
            if(attrSize > CY_BLE_REFRESH_CHAR_LEN)
            {
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
            }
            else
            {
                charValueHandle = cy_ble_scpssConfigPtr->attrInfo->refreshCharHandle;
            }
        }
    }

    if(apiResult == CY_BLE_SUCCESS)
    {
        if(charValueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
        else
        {
            /* Read characteristic value from database */
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.connHandle.bdHandle        = 0u;
            dbAttrValInfo.connHandle.attId           = 0u;
            dbAttrValInfo.handleValuePair.attrHandle = charValueHandle;
            dbAttrValInfo.handleValuePair.value.len  = attrSize;
            dbAttrValInfo.handleValuePair.value.val  = attrValue;
            dbAttrValInfo.offset                     = 0u;
            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

            if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
            {
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return(apiResult);
}


/*******************************************************************************
* Function Name: Cy_BLE_SCPSS_GetCharacteristicDescriptor
****************************************************************************//**
*
*  Gets a characteristic descriptor of the specified characteristic of the
*  Scan Parameters service.
*
*  \param connHandle: The connection handle
*  \param charIndex:  The index of the characteristic.
*       * CY_BLE_SCPS_SCAN_REFRESH - The Scan Refresh characteristic index
*
*  \param descrIndex: The index of the descriptor.
*       * CY_BLE_SCPS_SCAN_REFRESH_CCCD - The Client Characteristic
*         Configuration descriptor index of the Scan Refresh characteristic
*
*  \param attrSize:   The size of the characteristic value attribute.
*
*  \param attrValue:  The pointer to the location where the characteristic descriptor
*                     value data should be stored.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | An optional descriptor is absent
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_SCPSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_scps_char_index_t charIndex,
                                                                cy_en_ble_scps_descr_index_t descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((charIndex != CY_BLE_SCPS_SCAN_REFRESH) || (descrIndex >= CY_BLE_SCPS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        if(cy_ble_scpssConfigPtr->attrInfo->refreshCccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
        else
        {
            /* Get data from database */
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_scpssConfigPtr->attrInfo->refreshCccdHandle;
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
    }

    return(apiResult);
}


/*******************************************************************************
* Function Name: Cy_BLE_SCPSS_WriteEventHandler
****************************************************************************//**
*
*  Handles the Write Request event for the service.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_GATT_ERR_NONE                     | Write Request handled successfully.
*   CY_BLE_GATT_ERR_UNLIKELY_ERROR           |  Internal error while writing attribute value
*
*******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_SCPSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    cy_stc_ble_scps_char_value_t locChar = { .connHandle = eventParam->connHandle };

    if(Cy_BLE_SCPS_ApplCallback != NULL)
    {
        if((eventParam->handleValPair.attrHandle == cy_ble_scpssConfigPtr->attrInfo->refreshCccdHandle) ||
           (eventParam->handleValPair.attrHandle == cy_ble_scpssConfigPtr->attrInfo->intervalWindowCharHandle))
        {
            /* Store value to database */
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.handleValuePair = eventParam->handleValPair;
            dbAttrValInfo.connHandle      = eventParam->connHandle;
            dbAttrValInfo.offset          = 0u;
            dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

            gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);

            if(gattErr == CY_BLE_GATT_ERR_NONE)
            {
                /* Client Characteristic Configuration descriptor Write Request */
                if(eventParam->handleValPair.attrHandle == cy_ble_scpssConfigPtr->attrInfo->refreshCccdHandle)
                {
                    uint32_t eventCode;
                    locChar.charIndex = CY_BLE_SCPS_SCAN_REFRESH;
                    locChar.value = NULL;

                    if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                    {
                        eventCode = (uint32_t)CY_BLE_EVT_SCPSS_NOTIFICATION_ENABLED;
                    }
                    else
                    {
                        eventCode = (uint32_t)CY_BLE_EVT_SCPSS_NOTIFICATION_DISABLED;
                    }
                    Cy_BLE_SCPS_ApplCallback(eventCode, &locChar);
                }
                else /* Scan Interval Window characteristic write without response request */
                {
                    locChar.charIndex = CY_BLE_SCPS_SCAN_INT_WIN;
                    locChar.value = &eventParam->handleValPair.value;
                    Cy_BLE_SCPS_ApplCallback((uint32_t)CY_BLE_EVT_SCPSS_SCAN_INT_WIN_WRITE_CHAR, &locChar);
                }
            }
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }

    return(gattErr);
}


/*******************************************************************************
* Function Name: Cy_BLE_SCPSS_SendNotification
****************************************************************************//**
*
*  This function notifies the client that the server requires the Scan Interval
*  Window Characteristic to be written with the latest values upon notification.
*
*  On enabling notification successfully for a service characteristic it sends out a
*  'Handle Value Notification' which results in #CY_BLE_EVT_SCPSC_NOTIFICATION event
*  at the GATT Client's end.
*
*  \param connHandle: The connection handle
*
*  \param charIndex: The index of the characteristic.
*       * CY_BLE_SCPS_SCAN_REFRESH - The Scan Refresh characteristic index
*
*  \param attrSize: The size of the characteristic value attribute.
*
*  \param attrValue: The pointer to the characteristic value data that should be
*                    sent to the client device.
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_NTF_DISABLED                | Notification is not enabled by the client.
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_SCPSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                     cy_en_ble_scps_char_index_t charIndex,
                                                     uint8_t attrSize,
                                                     uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    if((charIndex != CY_BLE_SCPS_SCAN_REFRESH) || (attrSize != CY_BLE_REFRESH_CHAR_LEN))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Send Notification if it is enabled and connected */
        if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
        {
            apiResult = CY_BLE_ERROR_INVALID_STATE;
        }
        else if((cy_ble_scpssConfigPtr->attrInfo->refreshCccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE) ||
                (!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_scpssConfigPtr->attrInfo->refreshCccdHandle)))
        {
            apiResult = CY_BLE_ERROR_NTF_DISABLED;
        }
        else
        {
            /* Fill all fields of Write Request structure ... */
            cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
            ntfReqParam.handleValPair.attrHandle = cy_ble_scpssConfigPtr->attrInfo->refreshCharHandle;
            ntfReqParam.handleValPair.value.val  = attrValue;
            ntfReqParam.handleValPair.value.len  = attrSize;
            ntfReqParam.connHandle               = connHandle;

            /* ... and send notification to client device. */
            apiResult = Cy_BLE_GATTS_Notification(&ntfReqParam);
        }
    }

    return(apiResult);
}


/*******************************************************************************
* Function Name: Cy_BLE_SCPSC_DiscoverCharacteristicsEventHandler
****************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP event.
*  Based on the service UUID, an appropriate data structure is populated using the
*  data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
*******************************************************************************/
static void Cy_BLE_SCPSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t scpsDiscIdx = cy_ble_scpscConfigPtr->serviceDiscIdx;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == scpsDiscIdx))
    {
        /* Get pointer (with offset) to SCPS client structure with attribute handles */
        cy_stc_ble_scpsc_t *scpscPtr = (cy_stc_ble_scpsc_t *)&cy_ble_scpscConfigPtr->attrInfo[discIdx];

        switch(discCharInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_SCAN_REFRESH:
                Cy_BLE_CheckStoreCharHandle(scpscPtr->refreshChar);
                break;

            case CY_BLE_UUID_CHAR_SCAN_WINDOW:
                Cy_BLE_CheckStoreCharHandle(scpscPtr->intervalWindowChar);
                break;

            default:
                break;
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/*******************************************************************************
* Function Name: Cy_BLE_SCPSC_DiscoverCharDescriptorsEventHandler
****************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  This event is generated when the server successfully sends the data for
*  #CY_BLE_GATT_FIND_INFO_REQ. Based on the service UUID, an appropriate data
*  structure is populated to the service with a service callback.
*
*  \param  discDescrInfo: The pointer to descriptor information structure.
*
*******************************************************************************/
static void Cy_BLE_SCPSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t scpsDiscIdx = cy_ble_scpscConfigPtr->serviceDiscIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == scpsDiscIdx)
    {
        /* Get pointer (with offset) to SCPS client structure with attribute handles */
        cy_stc_ble_scpsc_t *scpscPtr = (cy_stc_ble_scpsc_t *)&cy_ble_scpscConfigPtr->attrInfo[discIdx];


        if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_CLIENT_CONFIG)
        {
            Cy_BLE_CheckStoreCharDescrHandle(scpscPtr->refreshCccdHandle);
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_SCPSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_SCPSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t scpsDiscIdx = cy_ble_scpscConfigPtr->serviceDiscIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == scpsDiscIdx)
    {
        uint32 servCnt = cy_ble_configPtr->context->discServiCount;

        /* Get pointer (with offset) to SCPS client structure with attribute handles */
        cy_stc_ble_scpsc_t *scpscPtr = (cy_stc_ble_scpsc_t *)&cy_ble_scpscConfigPtr->attrInfo[discIdx];

        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        if((cy_ble_configPtr->context->discovery[discIdx].charCount == 0u) &&
           (cy_ble_configPtr->context->serverInfo[(discIdx * servCnt) + scpsDiscIdx].range.endHandle != 0u))
        {
            charRangeInfo->range.startHandle = scpscPtr->refreshChar.valueHandle + 1u;
            charRangeInfo->range.endHandle =
                cy_ble_configPtr->context->serverInfo[(discIdx * servCnt) + scpsDiscIdx].range.endHandle;
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/*******************************************************************************
* Function Name: Cy_BLE_SCPSC_SetCharacteristicValue
****************************************************************************//**
*
*  Sets a characteristic value of the Scan Parameters service, which is
*  identified by charIndex.
*
*  This function call can result in generation of the following events based on
*  the response from the server device:
*  * #CY_BLE_EVT_GATTC_WRITE_RSP
*  * #CY_BLE_EVT_GATTC_ERROR_RSP
*
*  The #CY_BLE_EVT_SCPSS_SCAN_INT_WIN_WRITE_CHAR event is received by the peer
*  device on invoking this function.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     sent to the server device.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_SCPSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_scps_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex != CY_BLE_SCPS_SCAN_INT_WIN) ||
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Get pointer (with offset) to SCPS client structure with attribute handles */
        cy_stc_ble_scpsc_t *scpscPtr = (cy_stc_ble_scpsc_t *)&cy_ble_scpscConfigPtr->attrInfo[discIdx];

        if(scpscPtr->intervalWindowChar.valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            /* Fill all fields of write command request structure ... */
            cy_stc_ble_gattc_write_cmd_req_t writeCmdParam;
            writeCmdParam.handleValPair.attrHandle = scpscPtr->intervalWindowChar.valueHandle;
            writeCmdParam.handleValPair.value.val  = attrValue;
            writeCmdParam.handleValPair.value.len  = attrSize;
            writeCmdParam.connHandle               = connHandle;

            /* ... and send request to server device. */
            apiResult = Cy_BLE_GATTC_WriteWithoutResponse(&writeCmdParam);
        }
        else
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
    }

    return(apiResult);
}


/*******************************************************************************
* Function Name: Cy_BLE_SCPSC_SetCharacteristicDescriptor
****************************************************************************//**
*
*  Sets characteristic descriptor of specified characteristic of the Scan
*  Parameters service.
*
*  Internally, Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*  * #CY_BLE_EVT_SCPSS_NOTIFICATION_ENABLED
*  * #CY_BLE_EVT_SCPSS_NOTIFICATION_DISABLED
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic.
*  \param descrIndex: The index of the service characteristic descriptor.
*  \param attrSize:   The size of the descriptor value attribute.
*  \param attrValue:  The pointer to the characteristic descriptor value data that
*                     should be sent to the server device.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*  If the SCPS service-specific callback is registered
*  with \ref Cy_BLE_SCPS_RegisterAttrCallback():
*  * \ref CY_BLE_EVT_SCPSC_WRITE_DESCR_RESPONSE - In case if the requested attribute is
*    successfully written on the peer device, the details (charIndex, etc.) are
*    provided with event parameter structure of type \ref cy_stc_ble_scps_descr_value_t.
*  .
*  Otherwise, if the SCPS service-specific callback is not registered:
*  * \ref CY_BLE_EVT_GATTC_WRITE_RSP - In case if the requested attribute is
*    successfully written on the peer device.
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*    requested attribute on the peer device, the details are provided with
*    event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_SCPSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_scps_char_index_t charIndex,
                                                                cy_en_ble_scps_descr_index_t descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex != CY_BLE_SCPS_SCAN_REFRESH) || (descrIndex >= CY_BLE_SCPS_DESCR_COUNT) ||
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Get pointer (with offset) to SCPS client structure with attribute handles */
        cy_stc_ble_scpsc_t *scpscPtr = (cy_stc_ble_scpsc_t *)&cy_ble_scpscConfigPtr->attrInfo[discIdx];

        if(scpscPtr->refreshChar.valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
        else
        {
            /* Fill all fields of Write Request structure ... */
            cy_stc_ble_gattc_write_req_t writeReqParam;
            writeReqParam.handleValPair.attrHandle = scpscPtr->refreshCccdHandle;
            writeReqParam.handleValPair.value.val  = attrValue;
            writeReqParam.handleValPair.value.len  = attrSize;
            writeReqParam.connHandle               = connHandle;

            /* ... and send request to server device. */
            apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

            /* Save handle to support service-specific read response from device */
            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_scpscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
            }
        }
    }

    return(apiResult);
}


/*******************************************************************************
* Function Name: Cy_BLE_SCPSC_GetCharacteristicDescriptor
****************************************************************************//**
*
*  Gets characteristic descriptor of specified characteristic of the Scan
*  Parameters service.
*
*  This function call can result in generation of the following events based on
*  the response from the server device:
*  * #CY_BLE_EVT_SCPSC_READ_DESCR_RESPONSE
*  * #CY_BLE_EVT_GATTC_ERROR_RSP
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of a service Characteristic.
*  \param descrIndex: The index of a service characteristic descriptor.
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
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the SCPS service-specific callback is registered
*   with \ref Cy_BLE_SCPS_RegisterAttrCallback():
*   * #CY_BLE_EVT_SCPSC_READ_DESCR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details
*     (charIndex, descrIndex etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_scps_descr_value_t.
*   .
*   Otherwise (if an SCPS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with
*     an event parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_SCPSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_scps_char_index_t charIndex,
                                                                cy_en_ble_scps_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex != CY_BLE_SCPS_SCAN_REFRESH) || (descrIndex >= CY_BLE_SCPS_DESCR_COUNT) ||
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Get pointer (with offset) to SCPS client structure with attribute handles */
        cy_stc_ble_scpsc_t *scpscPtr = (cy_stc_ble_scpsc_t *)&cy_ble_scpscConfigPtr->attrInfo[discIdx];

        if(scpscPtr->refreshChar.valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
        else
        {
            /* Fill all fields of Read Request structure ... */
            cy_stc_ble_gattc_read_req_t readReqParam;
            readReqParam.attrHandle = scpscPtr->refreshCccdHandle;
            readReqParam.connHandle = connHandle;

            /* ... and send request to server device. */
            apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

            /* Save handle to support service-specific read response from device */
            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_scpscReqHandle[discIdx] = scpscPtr->refreshCccdHandle;
            }
        }
    }

    return(apiResult);
}


/*******************************************************************************
* Function Name: Cy_BLE_SCPSC_NotificationEventHandler
****************************************************************************//**
*
*  Handles the Notification event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
*******************************************************************************/
static void Cy_BLE_SCPSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx >= cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_SCPS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to SCPS client structure with attribute handles */
        cy_stc_ble_scpsc_t *scpscPtr = (cy_stc_ble_scpsc_t *)&cy_ble_scpscConfigPtr->attrInfo[discIdx];

        if(scpscPtr->refreshChar.valueHandle == eventParam->handleValPair.attrHandle)
        {
            cy_stc_ble_scps_char_value_t locCharValue;
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.charIndex  = CY_BLE_SCPS_SCAN_REFRESH;
            locCharValue.value      = &eventParam->handleValPair.value;

            Cy_BLE_SCPS_ApplCallback((uint32_t)CY_BLE_EVT_SCPSC_NOTIFICATION, &locCharValue);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/*******************************************************************************
* Function Name: Cy_BLE_SCPSC_ReadResponseEventHandler
****************************************************************************//**
*
*  Handles the Read Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
*******************************************************************************/
static void Cy_BLE_SCPSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx >= cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_SCPS_ApplCallback != NULL) &&
       (cy_ble_scpscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to SCPS client structure with attribute handles */
        cy_stc_ble_scpsc_t *scpscPtr = (cy_stc_ble_scpsc_t *)&cy_ble_scpscConfigPtr->attrInfo[discIdx];

        if(scpscPtr->refreshCccdHandle == cy_ble_scpscReqHandle[discIdx])
        {
            cy_stc_ble_scps_descr_value_t locDescrValue;
            locDescrValue.connHandle = eventParam->connHandle;
            locDescrValue.charIndex  = CY_BLE_SCPS_SCAN_REFRESH;
            locDescrValue.descrIndex = CY_BLE_SCPS_SCAN_REFRESH_CCCD;
            locDescrValue.value      = &eventParam->value;

            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            cy_ble_scpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_SCPS_ApplCallback((uint32_t)CY_BLE_EVT_SCPSC_READ_DESCR_RESPONSE, &locDescrValue);
        }
    }
}


/*******************************************************************************
* Function Name: Cy_BLE_SCPSC_WriteResponseEventHandler
****************************************************************************//**
*
*  Handles the Write Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
*******************************************************************************/
static void Cy_BLE_SCPSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx >= cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_SCPS_ApplCallback != NULL) &&
       (cy_ble_scpscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to SCPS client structure with attribute handles */
        cy_stc_ble_scpsc_t *scpscPtr = (cy_stc_ble_scpsc_t *)&cy_ble_scpscConfigPtr->attrInfo[discIdx];

        if(scpscPtr->refreshCccdHandle == cy_ble_scpscReqHandle[discIdx])
        {
            cy_stc_ble_scps_descr_value_t locDescrValue;
            locDescrValue.connHandle = *eventParam;
            locDescrValue.charIndex  = CY_BLE_SCPS_SCAN_REFRESH;
            locDescrValue.descrIndex = CY_BLE_SCPS_SCAN_REFRESH_CCCD;
            locDescrValue.value      = NULL;

            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            cy_ble_scpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_SCPS_ApplCallback((uint32_t)CY_BLE_EVT_SCPSC_WRITE_DESCR_RESPONSE, &locDescrValue);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_SCPSC_GetCharacteristicValueHandle
***************************************************************************//**
*
*  Returns the discovered peer device characteristic value handle.
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of a service characteristic.
*
*  \return
*   Returns characteristic value handle of type cy_ble_gatt_db_attr_handle_t.
*   * CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE: when a peer device doesn't have an optional characteristic
*
******************************************************************************/
cy_ble_gatt_db_attr_handle_t Cy_BLE_SCPSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_scps_char_index_t charIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to SCPS client structure with attribute handles */
    cy_stc_ble_scpsc_t *scpscPtr = (cy_stc_ble_scpsc_t *)&cy_ble_scpscConfigPtr->attrInfo[discIdx];

    if((discIdx >= cy_ble_configPtr->params->maxClientCount) || (charIndex >= CY_BLE_SCPS_CHAR_COUNT))
    {
        returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }
    else
    {
        if(charIndex == CY_BLE_SCPS_SCAN_INT_WIN)
        {
            returnHandle = scpscPtr->refreshChar.valueHandle;
        }
        else
        {
            returnHandle = scpscPtr->intervalWindowChar.valueHandle;
        }
    }
    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_SCPSC_GetCharacteristicDescriptorHandle
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
*  * CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE: when a peer device doesn't have an optional descriptor
*
******************************************************************************/
cy_ble_gatt_db_attr_handle_t Cy_BLE_SCPSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_scps_char_index_t charIndex,
                                                                            cy_en_ble_scps_descr_index_t descrIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to SCPS client structure with attribute handles */
    cy_stc_ble_scpsc_t *scpscPtr = (cy_stc_ble_scpsc_t *)&cy_ble_scpscConfigPtr->attrInfo[discIdx];

    if((discIdx >= cy_ble_configPtr->params->maxClientCount) || (charIndex >= CY_BLE_SCPS_CHAR_COUNT) ||
       (descrIndex >= CY_BLE_SCPS_DESCR_COUNT))
    {
        returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }
    else
    {
        returnHandle = scpscPtr->refreshCccdHandle;
    }
    return(returnHandle);
}


/*******************************************************************************
* Function Name: Cy_BLE_SCPSC_ErrorResponseEventHandler
****************************************************************************//**
*
*  Handles the Error Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
*******************************************************************************/
static void Cy_BLE_SCPSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        if(cy_ble_scpscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] == eventParam->errInfo.attrHandle)
        {
            cy_ble_scpscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_SCPS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the Scan Parameter service.
*
*  \param eventCode:   the event code
*  \param eventParam:  the event parameters
*
* \return
*  A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_SCPS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_SCPSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_SCPSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_SCPSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_SCPSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_SCPSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the SCPS service
*
*  \param eventCode:   the event code
*  \param eventParam:  the event parameters
*
* \return
*  A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_SCPSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_SCPSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_WRITE_CMD_REQ:
            (void)Cy_BLE_SCPSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_SCPSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the SCPS service
*
*  \param eventCode:   the event code
*  \param eventParam:  the event parameters
*
* \return
*  A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_SCPSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_SCPSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_SCPSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_SCPSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                   (((cy_stc_ble_gatt_err_param_t*)eventParam)->errInfo.errorCode !=
                        CY_BLE_GATT_ERR_ATTRIBUTE_NOT_FOUND))
                {
                    Cy_BLE_SCPSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_SCPSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_SCPSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_SCPSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
                break;

            default:
                break;
        }
    }
    return (gattErr);
}

#endif /* CY_BLE_LIB_HOST_CORE */
#endif /* CY_IP_MXBLESS */


/* [] END OF FILE */
