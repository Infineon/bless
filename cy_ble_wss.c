/***************************************************************************//**
* \file cy_ble_wss.c
* \version 3.30
*
* \brief
*  Contains the source code for the Weight Scale service.
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
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_WSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_WSSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_WSSC_EventHandler(uint32_t eventCode, void *eventParam);
 
static cy_en_ble_gatt_err_code_t Cy_BLE_WSSS_WriteEventHandler(const cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);
static void Cy_BLE_WSSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam);

static void Cy_BLE_WSSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_WSSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_WSSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_WSSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam);
static void Cy_BLE_WSSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_WSSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_WSSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_WSS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_WSSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_WSSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response for server */
static cy_ble_gatt_db_attr_handle_t cy_ble_wsssReqHandle;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_wsscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE WSS server config structure */
const cy_stc_ble_wsss_config_t *cy_ble_wsssConfigPtr = NULL;

/* The pointer to a global BLE WSS client config structure */
const cy_stc_ble_wssc_config_t *cy_ble_wsscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_WSSS_Init
***************************************************************************//**
*
*  This function initializes server for the  Weight Scale service.
*
*  \param config: Configuration structure for the WSS.
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
cy_en_ble_api_result_t Cy_BLE_WSSS_Init(const cy_stc_ble_wsss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_wsssConfigPtr = config;

        /* Registers event handler for the WSS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_WSS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_WSSS_EventHandlerCallback = &Cy_BLE_WSSS_EventHandler;
        
        cy_ble_wsssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WSSC_Init
***************************************************************************//**
*
*  This function initializes client for the Weight Scale service.
*
*  \param config: Configuration structure for the WSS.
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
cy_en_ble_api_result_t Cy_BLE_WSSC_Init(const cy_stc_ble_wssc_config_t *config)
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
        cy_ble_wsscConfigPtr = config;

        /* Registers event handler for the WSS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_WSS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_WSSC_EventHandlerCallback = &Cy_BLE_WSSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 wssServIdx = cy_ble_wsscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + wssServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to WSS client structure */
                cy_stc_ble_wssc_t *wsscPtr = (cy_stc_ble_wssc_t *)&cy_ble_wsscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(wsscPtr, 0, sizeof(cy_stc_ble_wssc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + wssServIdx].uuid =
                    CY_BLE_UUID_WEIGHT_SCALE_SERVICE;
            }

            cy_ble_wsscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WSS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for WSS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_WSSS_INDICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event (e.g. pointer to \ref cy_stc_ble_wss_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_WSS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_WSS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_WSSS_WriteEventHandler
***************************************************************************//**
*
*  Handles Write Request event for Weight Scale service. 
*
*  \param void *eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_WSSS_WriteEventHandler(const cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    uint32_t event = (uint32_t)CY_BLE_EVT_WSSS_INDICATION_DISABLED;

    if(Cy_BLE_WSS_ApplCallback != NULL)
    {
        /* Client Characteristic Configuration descriptor Write Request */
        if(eventParam->handleValPair.attrHandle ==
           cy_ble_wsssConfigPtr->attrInfo->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].descrHandle[CY_BLE_WSS_CCCD])
        {
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
            dbAttrValInfo.handleValuePair = eventParam->handleValPair;
            dbAttrValInfo.connHandle      = eventParam->connHandle;
            dbAttrValInfo.offset          = 0u;
            dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;
            
            if(CY_BLE_IS_INDICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
            {
                event = (uint32_t)CY_BLE_EVT_WSSS_INDICATION_ENABLED;
            }
            gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);

            if(gattErr == CY_BLE_GATT_ERR_NONE)
            {
                cy_stc_ble_wss_char_value_t wrReqParam; 
                wrReqParam.connHandle = eventParam->connHandle;
                wrReqParam.charIndex  = CY_BLE_WSS_WEIGHT_MEASUREMENT;
                wrReqParam.value      = NULL;
                
                Cy_BLE_WSS_ApplCallback(event, &wrReqParam);
            }

            /* Clear callback flag indicating that request was handled */
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_WSSS_ConfirmationEventHandler
***************************************************************************//**
*
*  Handles a Value Confirmation request event from the BLE Stack.
*
*  \param eventParam: The pointer to a structure of type cy_stc_ble_conn_handle_t.
*
******************************************************************************/
static void Cy_BLE_WSSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    if((Cy_BLE_WSS_ApplCallback != NULL) && (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_wsssReqHandle))
    {
        /* Only Weight Measurement Characteristic has the Indication property. Check whether
         * the requested handle is the handle of the descriptor Value Change handle.
         */
        if(cy_ble_wsssReqHandle == cy_ble_wsssConfigPtr->attrInfo->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].charHandle)
        {
            /* Fill in event data and inform application about successfully
             * confirmed indication.
             */
            cy_stc_ble_wss_char_value_t locCharValue;
            locCharValue.connHandle = *eventParam;
            locCharValue.charIndex  = CY_BLE_WSS_WEIGHT_MEASUREMENT;
            locCharValue.value      = NULL;

            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            cy_ble_wsssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_WSS_ApplCallback((uint32_t)CY_BLE_EVT_WSSS_INDICATION_CONFIRMED, &locCharValue);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_WSS_SetAdUserId
***************************************************************************//**
*
*  Sets the User ID List to the advertisement packet. To be able to set the
*  User ID List with this function, the advertisement packet should be
*  configured in the Component GUI to include Weight Scale service UUID in the
*  service Data field. The service Data should have enough room to fit the
*  User ID List that is planned to be advertised. To reserve the room for the
*  User ID List, the service Data for WSS should be filled with Unknown User
*  ID - 0xFF. The amount of 0xFF's should be equal to the User List Size that is
*  planned to be advertised.
*  This function must be called when Cy_BLE_StackGetBleSsState() returns
*  CY_BLE_BLESS_STATE_EVENT_CLOSE state.
*
*  \param listSize: The size of the User List.
*  \param userIdList: The array contains a User List.
*  \param advertisingParamIndex: The index of the peripheral and broadcast
*                                configuration in the customizer. For example:
*  * CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX
*  * CY_BLE_BROADCASTER_CONFIGURATION_0_INDEX
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                    | Description
*   ------------                   | -----------
*   CY_BLE_SUCCESS                 | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER | On NULL pointer, Data length in input parameter exceeds maximum advertisement packet length.
*   CY_BLE_ERROR_INVALID_OPERATION | The advertisement packet doesn't contain the User List or it is to small or ADV event is not closed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WSS_SetAdUserId(uint8_t listSize,
                                              const uint8_t userIdList[],
                                              uint8_t advertisingParamIndex)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    uint32_t i;
    uint32_t fFlag = 0u;
    uint8_t adLength = 0u;
    uint8_t byteCounter = 0u;

    if((advertisingParamIndex >= cy_ble_configPtr->params->gappConfCount) || (userIdList == NULL) ||
       (listSize > (CY_BLE_GAP_MAX_ADV_DATA_LEN - CY_BLE_AD_SERVICE_DATA_OVERHEAD)))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        while((byteCounter < cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advData->advDataLen) &&
              (fFlag == 0u))
        {
            adLength = cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advData->advData[byteCounter];

            if(adLength != 0u)
            {
                uint16_t servUuid;

                /* Increment byte counter so that it points to AD type field */
                byteCounter++;

                servUuid = Cy_BLE_Get16ByPtr(&cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advData->
                                              advData[byteCounter + 1u]);

                /* Check whether "Service Data" AD type is found and service UUID is WSS UUID */
                if((cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advData->advData[byteCounter] ==
                    CY_BLE_AD_TYPE_SERVICE_DATA) && (servUuid == CY_BLE_UUID_WEIGHT_SCALE_SERVICE))
                {
                    /* WSS service Data was found. Set flag and exit the loop. */
                    fFlag = 1u;
                }
                else
                {
                    byteCounter += adLength;
                }
            }
            else
            {
                /* End of advertisement data structure was encountered so exit loop. */
                break;
            }
        }
        if(fFlag != 0u)
        {
            /* Check whether there is enough space to fit user index list */
            if((adLength - CY_BLE_AD_SERVICE_DATA_OVERHEAD) >= listSize)
            {
                /* Increment byte counter so that it points to data value */
                byteCounter += CY_BLE_AD_SERVICE_DATA_OVERHEAD;

                for(i = 0u; i < ((uint32_t)adLength - CY_BLE_AD_SERVICE_DATA_OVERHEAD); i++)
                {
                    if(i <= listSize)
                    {
                        /* Copy user index element to advertisement data byte */
                        cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advData->advData[byteCounter + i] =
                            userIdList[i];
                    }
                    else
                    {
                        /* Fill remaining bytes with "Unknown User" ID */
                        cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advData->advData[byteCounter + i] =
                            CY_BLE_WSS_UNKNOWN_USER;
                    }
                }

                /* We are done. Indicate success. */
                apiResult = CY_BLE_SUCCESS;

                if((Cy_BLE_GetAdvertisementState() == CY_BLE_ADV_STATE_ADVERTISING) &&
                   (cy_ble_advIndex == advertisingParamIndex))
                {
                    /* Update the advertisement packet if the device is in the advertising mode. */
                    apiResult = Cy_BLE_GAPP_UpdateAdvScanData(&cy_ble_configPtr->discoveryModeInfo[cy_ble_advIndex]);
                }
            }
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WSS_GetAdUserIdListSize
***************************************************************************//**
*
*  Returns the size (in bytes) of the User ID List in the advertisement packet.
*
*  \param advertisingParamIndex: The index of the peripheral and broadcast
*                                configuration in the customizer. For example:
*  * CY_BLE_PERIPHERAL_CONFIGURATION_0_INDEX
*  * CY_BLE_BROADCASTER_CONFIGURATION_0_INDEX
*
* \return
*  Size of User ID List.
*
******************************************************************************/
uint8_t Cy_BLE_WSS_GetAdUserIdListSize(uint8_t advertisingParamIndex)
{
    uint8_t uiCount = 0u;
    uint8_t adLength = 0u;
    uint32_t fFlag = 0u;
    uint8_t byteCounter = 0u;

    if(advertisingParamIndex >= cy_ble_configPtr->params->gappConfCount)
    {
        uiCount = 0u;
    }
    else
    {
        while((byteCounter < cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advData->advDataLen))
        {
            adLength = cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advData->advData[byteCounter];

            if(adLength != 0u)
            {
                uint16_t servUuid;

                /* Increment byte counter so that it points to AD type field */
                byteCounter++;

                servUuid = Cy_BLE_Get16ByPtr(&cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advData->
                                              advData[byteCounter + 1u]);

                /* Check whether "Service Data" AD type is found and service UUID is WSS UUID */
                if((cy_ble_configPtr->discoveryModeInfo[advertisingParamIndex].advData->advData[byteCounter] ==
                    CY_BLE_AD_TYPE_SERVICE_DATA) && (servUuid == CY_BLE_UUID_WEIGHT_SCALE_SERVICE))
                {
                    /* WSS service Data was found. Set flag and exit the loop. */
                    fFlag = 1u;
                }
                else
                {
                    byteCounter += adLength;
                }
            }
            else
            {
                /* End of advertisement data structure was encountered so exit loop. */
                break;
            }
        }
        if(fFlag != 0u)
        {
            uiCount = adLength - CY_BLE_AD_SERVICE_DATA_OVERHEAD;
        }
    }

    return(uiCount);
}


/******************************************************************************
* Function Name: Cy_BLE_WSSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a value for one of two characteristic values of the Weight Scale
*  service. The characteristic is identified by charIndex.
*
*  \param charIndex: The index of a Weight Scale service characteristic.
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
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WSSS_SetCharacteristicValue(cy_en_ble_wss_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

    dbAttrValInfo.connHandle.bdHandle       = 0u;
    dbAttrValInfo.connHandle.attId          = 0u;
    dbAttrValInfo.handleValuePair.value.len = attrSize;
    dbAttrValInfo.handleValuePair.value.val = attrValue;
    dbAttrValInfo.offset                    = 0u;
    dbAttrValInfo.flags                     = CY_BLE_GATT_DB_LOCALLY_INITIATED;

    if((attrValue != NULL) && (charIndex < CY_BLE_WSS_CHAR_COUNT))
    {
        /* Fill structure */
        if(charIndex == CY_BLE_WSS_WEIGHT_SCALE_FEATURE)
        {
            dbAttrValInfo.handleValuePair.attrHandle =
                cy_ble_wsssConfigPtr->attrInfo->charInfo[CY_BLE_WSS_WEIGHT_SCALE_FEATURE].charHandle;
        }
        else
        {
            dbAttrValInfo.handleValuePair.attrHandle =
                cy_ble_wsssConfigPtr->attrInfo->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].charHandle;
        }

        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            /* Indicate success */
            apiResult = CY_BLE_SUCCESS;
        }
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WSSS_GetCharacteristicValue
***************************************************************************//**
*
*  Reads a characteristic value of the Weight Scale service, which is identified
*  by charIndex from the GATT database.
*
*  \param charIndex:  The index of the Weight Scale service characteristic.
*  \param attrSize:   The size of the Weight Scale service characteristic value attribute.
*  \param attrValue:  The pointer to the location where characteristic value data
*                     should be stored.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WSSS_GetCharacteristicValue(cy_en_ble_wss_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

    dbAttrValInfo.connHandle.bdHandle       = 0u;
    dbAttrValInfo.connHandle.attId          = 0u;
    dbAttrValInfo.handleValuePair.value.len = attrSize;
    dbAttrValInfo.handleValuePair.value.val = attrValue;
    dbAttrValInfo.offset                    = 0u;
    dbAttrValInfo.flags                     = CY_BLE_GATT_DB_LOCALLY_INITIATED;

    if((attrValue != NULL) && (charIndex < CY_BLE_WSS_CHAR_COUNT))
    {
        if(charIndex == CY_BLE_WSS_WEIGHT_SCALE_FEATURE)
        {
            dbAttrValInfo.handleValuePair.attrHandle =
                cy_ble_wsssConfigPtr->attrInfo->charInfo[CY_BLE_WSS_WEIGHT_SCALE_FEATURE].charHandle;
        }
        else
        {
            dbAttrValInfo.handleValuePair.attrHandle =
                cy_ble_wsssConfigPtr->attrInfo->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].charHandle;
        }

        /* Get characteristic value from GATT database */
        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            /* Indicate success */
            apiResult = CY_BLE_SUCCESS;
        }
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WSSS_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sets the characteristic descriptor of the specified characteristic.
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of the service characteristic.
*  \param descrIndex:   The index of the descriptor.
*  \param attrSize:     The size of the characteristic descriptor attribute.
*  \param attrValue:    The pointer to the descriptor value data to be stored in the GATT
*                       database.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WSSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_wss_char_index_t charIndex,
                                                               cy_en_ble_wss_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex == CY_BLE_WSS_WEIGHT_MEASUREMENT) && (descrIndex == CY_BLE_WSS_CCCD) && (attrValue != NULL))
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_wsssConfigPtr->attrInfo->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].
                                                   descrHandle[CY_BLE_WSS_CCCD];
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.connHandle                 = connHandle;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
       
        /* Read characteristic value from database */
        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_SUCCESS;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WSSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Reads a a characteristic descriptor of a specified characteristic of the
*  Weight Scale service from the GATT database.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the characteristic.
*  \param descrIndex: The index of the descriptor.
*  \param attrSize:   The size of the descriptor value.
*  \param attrValue: The pointer to the location where characteristic descriptor value
*                     data should be stored.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WSSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_wss_char_index_t charIndex,
                                                               cy_en_ble_wss_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex == CY_BLE_WSS_WEIGHT_MEASUREMENT) && (descrIndex == CY_BLE_WSS_CCCD) && (attrValue != NULL))
    {
        if(cy_ble_wsssConfigPtr->attrInfo->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].descrHandle[CY_BLE_WSS_CCCD] !=
           CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_wsssConfigPtr->attrInfo->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].
                                                       descrHandle[CY_BLE_WSS_CCCD];
            dbAttrValInfo.handleValuePair.value.len  = attrSize;
            dbAttrValInfo.handleValuePair.value.val  = attrValue;
            dbAttrValInfo.connHandle                 = connHandle;
            dbAttrValInfo.offset                     = 0u;
            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

            /* Get characteristic value from GATT database */
            if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
            {
                /* Indicate success */
                apiResult = CY_BLE_SUCCESS;
            }
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WSSS_SendIndication
***************************************************************************//**
*
*  Sends an indication with a characteristic value of the Weight Scale service,
*  which is a value specified by charIndex, to the client's device.
*
*  On enabling indication successfully it sends out a 'Handle Value Indication' which
*  results in #CY_BLE_EVT_WSSC_INDICATION or #CY_BLE_EVT_GATTC_HANDLE_VALUE_IND (if
*  service-specific callback function is not registered) event at the GATT Client's end.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     sent to the client's device.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | This operation is not permitted.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | An optional characteristic is absent.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_IND_DISABLED                | Indication is not enabled by the client.

* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the WSS service-specific callback is registered
*   with \ref Cy_BLE_WSS_RegisterAttrCallback():
*   * #CY_BLE_EVT_WSSS_INDICATION_CONFIRMED - If the indication is
*     successfully delivered to the peer device.
*   
*   Otherwise (if the WSS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF - If the indication is
*     successfully delivered to the peer device.
*
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WSSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_wss_char_index_t charIndex,
                                                  uint8_t attrSize,
                                                  uint8_t *attrValue)
{
    /* Store new data in database */
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if(charIndex == CY_BLE_WSS_WEIGHT_MEASUREMENT)
    {
        /* Send indication if it is enabled and connected */
        if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
        {
            apiResult = CY_BLE_ERROR_INVALID_STATE;
        }
        else if(!CY_BLE_IS_INDICATION_ENABLED(connHandle.attId,
                                              cy_ble_wsssConfigPtr->attrInfo->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].descrHandle[
                                                  CY_BLE_WSS_CCCD]))
        {
            apiResult = CY_BLE_ERROR_IND_DISABLED;
        }
        else
        {
            /* Fill all fields of Write Request structure ... */
            cy_stc_ble_gatts_handle_value_ind_t indReqParam;
                indReqParam.handleValPair.attrHandle = cy_ble_wsssConfigPtr->attrInfo->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].charHandle;
                indReqParam.handleValPair.value.val  = attrValue;
                indReqParam.handleValPair.value.len  = attrSize;
                indReqParam.connHandle               = connHandle;

            /* Send indication to client using previously filled structure */
            apiResult = Cy_BLE_GATTS_Indication(&indReqParam);

            /* Save handle to support service-specific value confirmation response from client */
            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_wsssReqHandle = indReqParam.handleValPair.attrHandle;
            }
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WSSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_WSSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* WSS characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_wsscCharUuid[CY_BLE_WSS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_WEIGHT_SCALE_FEATURE,
        CY_BLE_UUID_CHAR_WEIGHT_MEASUREMENT
    };
    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t wssDiscIdx = cy_ble_wsscConfigPtr->serviceDiscIdx;
    uint32_t i;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == wssDiscIdx))
    {
        /* Update last characteristic endHandle to declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        /* Search through all available characteristics */
        for(i = (uint32_t)CY_BLE_WSS_WEIGHT_SCALE_FEATURE; (i < (uint32_t)CY_BLE_WSS_CHAR_COUNT); i++)
        {
            if(cy_ble_wsscCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                /* Get pointer (with offset) to WSS client structure with attribute handles */
                cy_stc_ble_wssc_t *wsscPtr = (cy_stc_ble_wssc_t *)&cy_ble_wsscConfigPtr->attrInfo[discIdx];
        
                if(wsscPtr->charInfo[i].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    wsscPtr->charInfo[i].valueHandle = discCharInfo->valueHandle;
                    wsscPtr->charInfo[i].properties = discCharInfo->properties;
                    lastEndHandle[discIdx] = &wsscPtr->charInfo[i].endHandle;
                    break;
                }
                else
                {
                    Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo);
                }
            }
        }

        /* Init characteristic endHandle to service endHandle. Characteristic endHandle
         * will be updated to the declaration handler of the following characteristic,
         * in the following characteristic discovery procedure.
         */
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
* Function Name: Cy_BLE_WSSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  Based on the descriptor UUID, an appropriate data structure is populated using
*  the data received as part of the callback.
*
*  \param discDescrInfo: The pointer to a descriptor information structure.
*
******************************************************************************/
static void Cy_BLE_WSSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t notSupportedDescr = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t wssDiscIdx = cy_ble_wsscConfigPtr->serviceDiscIdx;
    uint32_t descIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == wssDiscIdx)
    {
        switch(discDescrInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_CLIENT_CONFIG:
                descIdx = (uint32_t)CY_BLE_WSS_CCCD;
                break;

            default:
                /* Not supported descriptor */
                notSupportedDescr = 1u;
                break;
        }

        if(notSupportedDescr == 0u)
        {
            /* Get pointer (with offset) to WSS client structure with attribute handles */
            cy_stc_ble_wssc_t *wsscPtr = (cy_stc_ble_wssc_t *)&cy_ble_wsscConfigPtr->attrInfo[discIdx];
            
            if(wsscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] ==
               CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                wsscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] =
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
* Function Name: Cy_BLE_WSSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_WSSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t wssDiscIdx = cy_ble_wsscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == wssDiscIdx)
    {
        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_WSS_CHAR_COUNT) && (exitFlag == 0u))
        {
            /* Get pointer (with offset) to WSS client structure with attribute handles */
            cy_stc_ble_wssc_t *wsscPtr = (cy_stc_ble_wssc_t *)&cy_ble_wsscConfigPtr->attrInfo[discIdx];
        
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            if((wsscPtr->charInfo[charIdx].endHandle - wsscPtr->charInfo[charIdx].valueHandle) != 0u)
            {
                /* Read characteristic range */
                charRangeInfo->range.startHandle = wsscPtr->charInfo[charIdx].valueHandle + 1u;
                charRangeInfo->range.endHandle = wsscPtr->charInfo[charIdx].endHandle;
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
* Function Name: Cy_BLE_WSSC_IndicationEventHandler
***************************************************************************//**
*
*  Handles an indication event for Weight Scale service.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_WSSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_WSS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to WSS client structure with attribute handles */
        cy_stc_ble_wssc_t *wsscPtr = (cy_stc_ble_wssc_t *)&cy_ble_wsscConfigPtr->attrInfo[discIdx];
        
        if(wsscPtr->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].valueHandle ==
           eventParam->handleValPair.attrHandle)
        {
            cy_stc_ble_wss_char_value_t indicationValue;
            indicationValue.connHandle = eventParam->connHandle;
            indicationValue.charIndex  = CY_BLE_WSS_WEIGHT_MEASUREMENT;
            indicationValue.value      = &eventParam->handleValPair.value;
           
            Cy_BLE_WSS_ApplCallback((uint32_t)CY_BLE_EVT_WSSC_INDICATION, &indicationValue);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_WSSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles a Read Response event for the Weight Scale service.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_WSSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint8_t locReqHandle = 1u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_WSS_ApplCallback != NULL) &&
       (cy_ble_wsscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to WSS client structure with attribute handles */
        cy_stc_ble_wssc_t *wsscPtr = (cy_stc_ble_wssc_t *)&cy_ble_wsscConfigPtr->attrInfo[discIdx];
        
        if((cy_ble_wsscReqHandle[discIdx] == wsscPtr->charInfo[CY_BLE_WSS_WEIGHT_SCALE_FEATURE].valueHandle) ||
           (cy_ble_wsscReqHandle[discIdx] == wsscPtr->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].valueHandle))
        {
            cy_stc_ble_wss_char_value_t locCharValue;

            if(cy_ble_wsscReqHandle[discIdx] == wsscPtr->charInfo[CY_BLE_WSS_WEIGHT_SCALE_FEATURE].valueHandle)
            {
                locCharValue.charIndex = CY_BLE_WSS_WEIGHT_SCALE_FEATURE;
            }
            else
            {
                locCharValue.charIndex = CY_BLE_WSS_WEIGHT_MEASUREMENT;
            }

            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.value = &eventParam->value;
            cy_ble_wsscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_WSS_ApplCallback((uint32_t)CY_BLE_EVT_WSSC_READ_CHAR_RESPONSE, &locCharValue);
        }
        else if(cy_ble_wsscReqHandle[discIdx] ==
                wsscPtr->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].descrHandle[CY_BLE_WSS_CCCD])
        {
            cy_stc_ble_wss_descr_value_t locDescrValue;
            locDescrValue.connHandle = eventParam->connHandle;
            locDescrValue.charIndex  = CY_BLE_WSS_WEIGHT_MEASUREMENT;
            locDescrValue.descrIndex = CY_BLE_WSS_CCCD;
            locDescrValue.value      = &eventParam->value;

            cy_ble_wsscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_WSS_ApplCallback((uint32_t)CY_BLE_EVT_WSSC_READ_DESCR_RESPONSE, &locDescrValue);
        }
        else
        {
            locReqHandle = 0u;
        }

        if(locReqHandle != 0u)
        {
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_WSSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles a Write Response event for the Weight Scale service.
*
*  \param eventParam: The pointer to a data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_WSSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    /* Check whether service handler was registered and request handle is valid */
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_WSS_ApplCallback != NULL) &&
       (cy_ble_wsscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to WSS client structure with attribute handles */
        cy_stc_ble_wssc_t *wsscPtr = (cy_stc_ble_wssc_t *)&cy_ble_wsscConfigPtr->attrInfo[discIdx];
        
        if(cy_ble_wsscReqHandle[discIdx] ==
           wsscPtr->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].descrHandle[CY_BLE_WSS_CCCD])
        {
            cy_stc_ble_wss_descr_value_t locDescrValue;
            locDescrValue.connHandle = *eventParam;
            locDescrValue.charIndex  = CY_BLE_WSS_WEIGHT_MEASUREMENT;
            locDescrValue.descrIndex = CY_BLE_WSS_CCCD;
            locDescrValue.value      = NULL;

            cy_ble_wsscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_WSS_ApplCallback((uint32_t)CY_BLE_EVT_WSSC_WRITE_DESCR_RESPONSE, &locDescrValue);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_WSSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles an Error Response event for the Weight Scale service.
*
*  \param eventParam - The pointer to a data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_WSSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
        if(eventParam->errInfo.attrHandle == cy_ble_wsscReqHandle[discIdx])
        {
            cy_ble_wsscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_WSSC_GetCharacteristicValue
***************************************************************************//**
*
*  This function is used to read a characteristic value, which is a value
*  identified by charIndex, from the server.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic. Starts with zero.
*
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
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the WSS service-specific callback is registered
*      with \ref Cy_BLE_WSS_RegisterAttrCallback():
*   * #CY_BLE_EVT_WSSC_READ_CHAR_RESPONSE - If the requested attribute is
*     successfully read on the peer device, the details 
*     (charIndex , value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_wss_char_value_t.
*   .
*   Otherwise (if an WSS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_WSSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_wss_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to WSS client structure with attribute handles */
    cy_stc_ble_wssc_t *wsscPtr = (cy_stc_ble_wssc_t *)&cy_ble_wsscConfigPtr->attrInfo[discIdx];
        
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_WSS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != wsscPtr->charInfo[charIndex].valueHandle)
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = wsscPtr->charInfo[charIndex].valueHandle;
        readReqParam.connHandle = connHandle;

        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_wsscReqHandle[discIdx] = wsscPtr->charInfo[charIndex].valueHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WSSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  This function is used to write the characteristic descriptor to the server,
*  which is identified by charIndex and descrIndex.
*
*  Internally, Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*  * #CY_BLE_EVT_WSSS_INDICATION_ENABLED
*  * #CY_BLE_EVT_WSSS_INDICATION_DISABLED
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of the service characteristic. Starts with zero.
*  \param descrIndex:   The index of the service characteristic descriptor.
*  \param attrSize:     The size of the characteristic value attribute.
*  \param attrValue:    The pointer to the characteristic descriptor value data that
*                       should be sent to the server device.
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
*   If the WSS service-specific callback is registered
*   with \ref Cy_BLE_WSS_RegisterAttrCallback():
*   * #CY_BLE_EVT_WSSC_WRITE_DESCR_RESPONSE - If the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, descrIndex, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_wss_descr_value_t.
*   
*   Otherwise (if an WSS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure ( \ref cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WSSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_wss_char_index_t charIndex,
                                                               cy_en_ble_wss_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to WSS client structure with attribute handles */
    cy_stc_ble_wssc_t *wsscPtr = (cy_stc_ble_wssc_t *)&cy_ble_wsscConfigPtr->attrInfo[discIdx];
        
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_WSS_CHAR_COUNT) || (descrIndex >= CY_BLE_WSS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(wsscPtr->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].descrHandle[CY_BLE_WSS_CCCD] !=
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle = wsscPtr->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].
                                     descrHandle[CY_BLE_WSS_CCCD];
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;
        
        /* ... and send request to server's device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_wsscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WSSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to get the characteristic descriptor of the specified
*  characteristic of the service.
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of the service characteristic. Starts with zero.
*  \param descrIndex:   The index of the service characteristic descriptor.
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have
*                                               the particular descriptor.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the WSS service-specific callback is registered
*   with \ref Cy_BLE_WSS_RegisterAttrCallback():
*   * #CY_BLE_EVT_WSSC_READ_DESCR_RESPONSE - If the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     descrIndex, value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_wss_descr_value_t.
*   .
*   Otherwise (if an WSS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_WSSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_wss_char_index_t charIndex,
                                                               cy_en_ble_wss_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    
    /* Get pointer (with offset) to WSS client structure with attribute handles */
    cy_stc_ble_wssc_t *wsscPtr = (cy_stc_ble_wssc_t *)&cy_ble_wsscConfigPtr->attrInfo[discIdx];
    
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_WSS_CHAR_COUNT) || (descrIndex >= CY_BLE_WSS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(wsscPtr->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].descrHandle[CY_BLE_WSS_CCCD] !=
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = wsscPtr->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].descrHandle[CY_BLE_WSS_CCCD];
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_wsscReqHandle[discIdx] = wsscPtr->charInfo[CY_BLE_WSS_WEIGHT_MEASUREMENT].
                                             descrHandle[CY_BLE_WSS_CCCD];
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WSS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the WSS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_WSS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_WSSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_WSSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_WSSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_WSSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_WSSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the WSS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_WSSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_WSSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            Cy_BLE_WSSS_ConfirmationEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_WSSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the WSS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_WSSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_WSSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_WSSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_WSSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_WSSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

           case CY_BLE_EVT_GATTC_HANDLE_VALUE_IND:
                Cy_BLE_WSSC_IndicationEventHandler((cy_stc_ble_gattc_handle_value_ind_param_t*)eventParam);
                break;

           case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_WSSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

           case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_WSSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
