/***************************************************************************//**
* \file cy_ble_ndcs.c
* \version 3.50
*
* \brief
*  Contains the source code for Next DST Change service.
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
static cy_en_ble_gatt_err_code_t Cy_BLE_NDCS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_NDCSC_EventHandler(uint32_t eventCode, void *eventParam);

static void Cy_BLE_NDCSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_NDCSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_NDCSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_NDCS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_NDCSC_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_ndcscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE NDCS server config structure */
const cy_stc_ble_ndcss_config_t *cy_ble_ndcssConfigPtr = NULL;

/* The pointer to a global BLE NDCS client config structure */
const cy_stc_ble_ndcsc_config_t *cy_ble_ndcscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_NDCSS_Init
***************************************************************************//**
*
*  This function initializes server of the Next DST Change service.
*
*  \param config: Configuration structure for the NDCS.
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
cy_en_ble_api_result_t Cy_BLE_NDCSS_Init(const cy_stc_ble_ndcss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_ndcssConfigPtr = config;

        /* Registers event handler for the NDCS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_NDCS_EventHandler);

    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_NDCSC_Init
***************************************************************************//**
*
*  This function initializes client of the Next DST Change service.
*
*  \param config: Configuration structure for the NDCS.
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
cy_en_ble_api_result_t Cy_BLE_NDCSC_Init(const cy_stc_ble_ndcsc_config_t *config)
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
        cy_ble_ndcscConfigPtr = config;

        /* Registers event handler for the NDCS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_NDCS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_NDCSC_EventHandlerCallback = &Cy_BLE_NDCSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 ndcsServIdx = cy_ble_ndcscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + ndcsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to NDCS client structure */
                cy_stc_ble_ndcsc_t *ndcscPtr = (cy_stc_ble_ndcsc_t *)&cy_ble_ndcscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(ndcscPtr, 0, sizeof(cy_stc_ble_ndcsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + ndcsServIdx].uuid =
                    CY_BLE_UUID_NEXT_DST_CHANGE_SERVICE;
            }

            cy_ble_ndcscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_NDCS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for NDCS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_NDCSC_READ_CHAR_RESPONSE).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_ndcs_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_NDCS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_NDCS_ApplCallback = callbackFunc;
}


/*******************************************************************************
* Function Name: Cy_BLE_NDCSS_SetCharacteristicValue
****************************************************************************//**
*
*  Sets characteristic value of the Next DST Change service, which is identified
*  by charIndex in the local database.
*
*  \param charIndex: The index of a service characteristic of type cy_en_ble_ndcs_char_index_t.
*  \param attrSize:  The size of the characteristic value attribute.
*  \param attrValue: The pointer to the characteristic value data that should be stored to the GATT database.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                       | Description
*   ------------                      | -----------
*   CY_BLE_SUCCESS                    | The request is handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER    | Validation of the input parameters failed.
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_NDCSS_SetCharacteristicValue(cy_en_ble_ndcs_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if(charIndex == CY_BLE_NDCS_TIME_WITH_DST)
    {
        /* Store data in database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_ndcssConfigPtr->attrInfo->timeWithDst;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        
        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_SUCCESS;
        }
    }

    return(apiResult);
}


/*******************************************************************************
* Function Name: Cy_BLE_NDCSS_GetCharacteristicValue
****************************************************************************//**
*
*  Gets a characteristic value of the Next DST Change service, which is
*  identified by charIndex.
*
*  \param charIndex:  The index of a service characteristic of
*                     type cy_en_ble_ndcs_char_index_t.
*  \param attrSize:   The size of the characteristic value attribute.
*
*  \param attrValue: The pointer to the location where
*                     characteristic value data should be stored.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                       | Description
*   ------------                      | -----------
*   CY_BLE_SUCCESS                    | The request is handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER    | Validation of the input parameters failed.
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_NDCSS_GetCharacteristicValue(cy_en_ble_ndcs_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if(charIndex == CY_BLE_NDCS_TIME_WITH_DST)
    {
        /* Read characteristic value from database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_ndcssConfigPtr->attrInfo->timeWithDst;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        
        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_SUCCESS;
        }
    }

    return(apiResult);
}


/*******************************************************************************
* Function Name: Cy_BLE_NDCSC_DiscoverCharacteristicsEventHandler
****************************************************************************//**
*
*  This function is called on receiving the #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
*
*******************************************************************************/
static void Cy_BLE_NDCSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t ndcsDiscIdx = cy_ble_ndcscConfigPtr->serviceDiscIdx;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == ndcsDiscIdx))
    {
        if(discCharInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_TIME_WITH_DST)
        {
            /* Get pointer (with offset) to NDCS client structure with attribute handles */
            cy_stc_ble_ndcsc_t *ndcscPtr = (cy_stc_ble_ndcsc_t *)&cy_ble_ndcscConfigPtr->attrInfo[discIdx];

            /* Using characteristic UUID store handle of requested characteristic */
            if(ndcscPtr->charInfo[CY_BLE_NDCS_TIME_WITH_DST].valueHandle ==
               CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                ndcscPtr->charInfo[CY_BLE_NDCS_TIME_WITH_DST].valueHandle = discCharInfo->valueHandle;
                ndcscPtr->charInfo[CY_BLE_NDCS_TIME_WITH_DST].properties = discCharInfo->properties;
            }
            else
            {
                Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo);
            }
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}



/******************************************************************************
* Function Name: Cy_BLE_NDCSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_NDCSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t ndcsDiscIdx = cy_ble_ndcscConfigPtr->serviceDiscIdx;
    
    if(cy_ble_configPtr->context->discovery[discIdx].servCount == ndcsDiscIdx)
    {
        /* NDCS does not have any descriptions, return CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE */
        charRangeInfo->range.startHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        charRangeInfo->range.endHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/*******************************************************************************
* Function Name: Cy_BLE_NDCSC_ReadResponseEventHandler
****************************************************************************//**
*
*  Handles Read Response event for Next DST Change service.
*
*  \param eventParam: The pointer to the data that came with a read response for NDCS.
*
*******************************************************************************/
static void Cy_BLE_NDCSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_NDCS_ApplCallback != NULL) &&
       (cy_ble_ndcscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to NDCS client structure with attribute handles */
        cy_stc_ble_ndcsc_t *ndcscPtr = (cy_stc_ble_ndcsc_t *)&cy_ble_ndcscConfigPtr->attrInfo[discIdx];

        if(ndcscPtr->charInfo[CY_BLE_NDCS_TIME_WITH_DST].valueHandle == cy_ble_ndcscReqHandle[discIdx])
        {
            /* Fill Reference Time Update service read response parameter structure with
             * characteristic info.
             */
            cy_stc_ble_ndcs_char_value_t rdRspParam;
            
            rdRspParam.connHandle = eventParam->connHandle;
            rdRspParam.charIndex  = CY_BLE_NDCS_TIME_WITH_DST;
            rdRspParam.value      = &eventParam->value;

            Cy_BLE_NDCS_ApplCallback((uint32_t)CY_BLE_EVT_NDCSC_READ_CHAR_RESPONSE, (void*)&rdRspParam);

            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            cy_ble_ndcscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/*******************************************************************************
* Function Name: Cy_BLE_NDCSC_GetCharacteristicValue
****************************************************************************//**
*
*  Sends a request to peer device to set characteristic value of the Next
*  DST Change service, which is identified by charIndex.
*
*  \param connHandle:    The connection handle.
*  \param charIndex:     The index of a service characteristic.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                           | Description
*   ------------                          | -----------
*   CY_BLE_SUCCESS                        | the request was sent successfully.
*   CY_BLE_ERROR_INVALID_STATE            | connection with the client is not established.
*   CY_BLE_ERROR_INVALID_PARAMETER        | validation of the input parameters failed.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | Memory allocation failed.
*   CY_BLE_ERROR_INVALID_OPERATION        | Operation is invalid for this characteristic.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the NDCS service-specific callback is registered
*      (with \ref Cy_BLE_NDCS_RegisterAttrCallback):
*  * #CY_BLE_EVT_NDCSC_READ_CHAR_RESPONSE - In case if the requested attribute is
*                                successfully read on the peer device,
*                                the details (charIndex , value, etc.) are
*                                provided with event parameter structure
*                                of type cy_stc_ble_ndcs_char_value_t.
*  .
*   Otherwise (if the NDCS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_READ_RSP - In case if the requested attribute is
*                                successfully read on the peer device,
*                                the details (handle, value, etc.) are
*                                provided with event parameters
*                                structure (cy_stc_ble_gattc_read_rsp_param_t).
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*                                requested attribute on the peer device,
*                                the details are provided with event parameters
*                                structure (cy_stc_ble_gatt_err_param_t).
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_NDCSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_ndcs_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex == CY_BLE_NDCS_TIME_WITH_DST) && (discIdx < cy_ble_configPtr->params->maxClientCount))
    {
        /* Get pointer (with offset) to NDCS client structure with attribute handles */
        cy_stc_ble_ndcsc_t *ndcscPtr = (cy_stc_ble_ndcsc_t *)&cy_ble_ndcscConfigPtr->attrInfo[discIdx];

        /* Send request to read characteristic value */
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = ndcscPtr->charInfo[CY_BLE_NDCS_TIME_WITH_DST].valueHandle;
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_ndcscReqHandle[discIdx] = ndcscPtr->charInfo[CY_BLE_NDCS_TIME_WITH_DST].valueHandle;
        }
    }
    else
    {
        /* apiResult equals to CY_BLE_ERROR_INVALID_PARAMETER */
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_NDCS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the NDCS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_NDCS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_NDCSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_NDCSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_NDCSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the NDCS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_NDCSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_NDCSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_NDCSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_NDCSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
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
