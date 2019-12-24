/***************************************************************************//**
* \file cy_ble_rtus.c
* \version 3.30
*
* \brief
*  Contains the source code for Reference Time Update service.
*
********************************************************************************
*
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
static cy_en_ble_gatt_err_code_t Cy_BLE_RTUS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_RTUSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_RTUSC_EventHandler(uint32_t eventCode, void *eventParam);

static void Cy_BLE_RTUSS_WriteCmdEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_RTUSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_RTUSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_RTUSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_RTUS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_RTUSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_RTUSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_rtuscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE RTUS server config structure */
const cy_stc_ble_rtuss_config_t *cy_ble_rtussConfigPtr = NULL;

/* The pointer to a global BLE RTUS client config structure */
const cy_stc_ble_rtusc_config_t *cy_ble_rtuscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_RTUSS_Init
***************************************************************************//**
*
*  This function initializes server of the Reference Time Update service.
*
*  \param config: Configuration structure for the RTUS.
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
cy_en_ble_api_result_t Cy_BLE_RTUSS_Init(const cy_stc_ble_rtuss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_rtussConfigPtr = config;

        /* Registers event handler for the RTUS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_RTUS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_RTUSS_EventHandlerCallback = &Cy_BLE_RTUSS_EventHandler;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_RTUSC_Init
***************************************************************************//**
*
*  This function initializes client of the Reference Time Update service.
*
*  \param config: Configuration structure for the RTUS.
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
cy_en_ble_api_result_t Cy_BLE_RTUSC_Init(const cy_stc_ble_rtusc_config_t *config)
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
        cy_ble_rtuscConfigPtr = config;

        /* Registers event handler for the RTUS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_RTUS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_RTUSC_EventHandlerCallback = &Cy_BLE_RTUSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 rtusServIdx = cy_ble_rtuscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + rtusServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to RTUS client structure */
                cy_stc_ble_rtusc_t *rtuscPtr = (cy_stc_ble_rtusc_t *)&cy_ble_rtuscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(rtuscPtr, 0, sizeof(cy_stc_ble_rtusc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + rtusServIdx].uuid =
                    CY_BLE_UUID_REF_TIME_UPDATE_SERVICE;
            }

            cy_ble_rtuscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/*******************************************************************************
* Function Name: Cy_BLE_RTUS_RegisterAttrCallback
****************************************************************************//**
*
*  Registers a callback function for Reference Time Update Service-specific
*  attribute operations.
*  Service-specific Write Requests from peer device will not be handled with
*  unregistered callback function.
*
*  \param callbackFunc:  An application layer event callback function to receive
*   events from the PSoC 6 BLE Middleware. The definition of cy_ble_callback_t
*   for RTUS is: \n
*    typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam)
*     * eventCode indicates the event that triggered this
*       callback.
*     * eventParam contains the parameters corresponding to the
*       current event.
*
*  \return
*  cy_en_ble_api_result_t : Return value indicates whether the function succeeded or
*  failed. The following are possible error codes:
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The function completed successfully.
*   CY_BLE_ERROR_INVALID_OPERATION          | This operation is not permitted
*
******************************************************************************/
void Cy_BLE_RTUS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_RTUS_ApplCallback = callbackFunc;
}


/*******************************************************************************
* Function Name: Cy_BLE_RTUSS_SetCharacteristicValue
****************************************************************************//**
*
*  Sets characteristic value of the Reference Time Update service, which is
*  identified by charIndex in the local database.
*
*  \param charIndex: The index of a service characteristic of
*                    type \ref cy_en_ble_rtus_char_index_t.
*  \param attrSize:  The size of the characteristic value attribute.
*  \param attrValue: The pointer to the characteristic value
*                     data that should be stored to the GATT database.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request is handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameters failed.
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_RTUSS_SetCharacteristicValue(cy_en_ble_rtus_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_stc_ble_rtus_time_update_state_t tUState;
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
    
    dbAttrValInfo.connHandle.bdHandle       = 0u;
    dbAttrValInfo.connHandle.attId          = 0u;
    dbAttrValInfo.handleValuePair.value.len = attrSize;
    dbAttrValInfo.handleValuePair.value.val = attrValue;
    dbAttrValInfo.offset                    = 0u;
    dbAttrValInfo.flags                     = CY_BLE_GATT_DB_LOCALLY_INITIATED;

    if((CY_BLE_RTUS_CHAR_COUNT > charIndex))
    {
        if(charIndex == CY_BLE_RTUS_TIME_UPDATE_CONTROL_POINT)
        {
            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_rtussConfigPtr->attrInfo->timeUpdateCpHandle;
            
            if(*attrValue <= CY_BLE_RTUS_CANCEL_REF_UPDATE)
            {
                apiResult = CY_BLE_SUCCESS;
            }
        }
        else
        {
            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_rtussConfigPtr->attrInfo->timeUpdateStateHandle;

            tUState = *(cy_stc_ble_rtus_time_update_state_t*)attrValue;

            if((tUState.result <= CY_BLE_RTUS_RESULT_NO_UPDATE_ATTEMPT) &&
               (tUState.currentState <= CY_BLE_RTUS_CURR_STATE_UPDATE_PENDING))
            {
                apiResult = CY_BLE_SUCCESS;
            }
        }
        /* Store data in database */
        if(apiResult == CY_BLE_SUCCESS)
        {
            if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
            {
                apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return(apiResult);
}


/*******************************************************************************
* Function Name: Cy_BLE_RTUSS_GetCharacteristicValue
****************************************************************************//**
*
*  Gets a characteristic value of the Reference Time Update service, which
*  is identified by charIndex.
*
*  \param charIndex: The index of a service characteristic of
*                    type \ref cy_en_ble_rtus_char_index_t.
*
*  \param attrSize:  The size of the characteristic value attribute.
*
*  \param attrValue: The pointer to the location where
*                     characteristic value data should be stored.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_RTUSS_GetCharacteristicValue(cy_en_ble_rtus_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
    
    dbAttrValInfo.connHandle.bdHandle       = 0u;
    dbAttrValInfo.connHandle.attId          = 0u;
    dbAttrValInfo.handleValuePair.value.len = attrSize;
    dbAttrValInfo.handleValuePair.value.val = attrValue;
    dbAttrValInfo.offset                      = 0u;
    dbAttrValInfo.flags                     = CY_BLE_GATT_DB_LOCALLY_INITIATED;

    if(CY_BLE_RTUS_CHAR_COUNT > charIndex)
    {
        /* Read characteristic value from database */
        if(charIndex == CY_BLE_RTUS_TIME_UPDATE_CONTROL_POINT)
        {
            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_rtussConfigPtr->attrInfo->timeUpdateCpHandle;
        }
        else
        {
            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_rtussConfigPtr->attrInfo->timeUpdateStateHandle;
        }

        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_SUCCESS;
        }
    }

    return(apiResult);
}


/*******************************************************************************
* Function Name: Cy_BLE_RTUSS_WriteCmdEventHandler
****************************************************************************//**
*
*  Handles the Write Without Response Request event for the Reference Time Update
*  service.
*
*  \param cy_stc_ble_gatts_write_cmd_req_param_t * eventParam: The pointer to a data structure
*                                                  specified by the event.
*
*
*******************************************************************************/
static void Cy_BLE_RTUSS_WriteCmdEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    if(Cy_BLE_RTUS_ApplCallback != NULL)
    {
        /* Check whether attribute handle is handle of Time Update Control Point
         * Characteristic of Reference Time Update service. */
        if((cy_ble_rtussConfigPtr->attrInfo->timeUpdateCpHandle == eventParam->handleValPair.attrHandle) &&
           (CY_BLE_RTUS_TIME_UPDATE_CP_SIZE == eventParam->handleValPair.value.len) &&
           (CY_BLE_RTUS_CANCEL_REF_UPDATE >= eventParam->handleValPair.value.val[0u]))
        {
            /* Input parameters validation passed, so save Time Update Control Point value */
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
            
            dbAttrValInfo.handleValuePair = eventParam->handleValPair;
            dbAttrValInfo.connHandle      = eventParam->connHandle;
            dbAttrValInfo.offset          = 0u;
            dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

            if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
            {
                cy_stc_ble_rtus_char_value_t wrCmdParam;
                wrCmdParam.connHandle = eventParam->connHandle;
                wrCmdParam.charIndex  = CY_BLE_RTUS_TIME_UPDATE_CONTROL_POINT;
                wrCmdParam.value      = &eventParam->handleValPair.value;
                
                /* Send callback to user if no error occurred while writing Time Update Control
                 * Point.
                 */
                Cy_BLE_RTUS_ApplCallback((uint32_t)CY_BLE_EVT_RTUSS_WRITE_CHAR_CMD, (void*)&wrCmdParam);
                cy_ble_eventHandlerFlag &= (uint8_t)(~CY_BLE_CALLBACK);
            }
        }
        /* As this handler handles Write Without Response request the Error Response
         * can't be sent for the client. The erroneous value will be with
         * CY_BLE_EVT_GATTS_WRITE_CMD_REQ event. User will decide how to handle it. */
    }
}


/*******************************************************************************
* Function Name: Cy_BLE_RTUSC_DiscoverCharacteristicsEventHandler
****************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
*
*******************************************************************************/
static void Cy_BLE_RTUSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* RTUS characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_rtusCharUuid[CY_BLE_RTUS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_TIME_UPDATE_CONTROL_POINT,
        CY_BLE_UUID_CHAR_TIME_UPDATE_STATE
    };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t rtusDiscIdx = cy_ble_rtuscConfigPtr->serviceDiscIdx;
    uint32_t i;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == rtusDiscIdx))
    {
        for(i = 0u; i < (uint32_t)CY_BLE_RTUS_CHAR_COUNT; i++)
        {
            if(cy_ble_rtusCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                /* Get pointer (with offset) to RTUS client structure with attribute handles */
                cy_stc_ble_rtusc_t *rtuscPtr = (cy_stc_ble_rtusc_t *)&cy_ble_rtuscConfigPtr->attrInfo[discIdx];

                if(rtuscPtr->charInfo[i].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    rtuscPtr->charInfo[i].valueHandle = discCharInfo->valueHandle;
                    rtuscPtr->charInfo[i].properties = discCharInfo->properties;
                }
                else
                {
                    Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo);
                }
            }
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}



/******************************************************************************
* Function Name: Cy_BLE_RTUSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_RTUSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t rtusDiscIdx = cy_ble_rtuscConfigPtr->serviceDiscIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == rtusDiscIdx)
    {
        /* RTUS does not have any descriptions, return CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE */
        charRangeInfo->range.startHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        charRangeInfo->range.endHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/*******************************************************************************
* Function Name: Cy_BLE_RTUSC_ReadResponseEventHandler
****************************************************************************//**
*
*  Handles Read Response event for Reference Time Update service.
*
*  \param eventParam: The pointer to the data that came with a read response for RTUS.
*
*
******************************************************************************/
static void Cy_BLE_RTUSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_RTUS_ApplCallback != NULL) &&
       (cy_ble_rtuscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to RTUS client structure with attribute handles */
        cy_stc_ble_rtusc_t *rtuscPtr = (cy_stc_ble_rtusc_t *)&cy_ble_rtuscConfigPtr->attrInfo[discIdx];

        if(rtuscPtr->charInfo[CY_BLE_RTUS_TIME_UPDATE_STATE].valueHandle == cy_ble_rtuscReqHandle[discIdx])
        {
            /* Fill Reference Time Update service read response parameter structure with
             * characteristic info.
             */
            cy_stc_ble_rtus_char_value_t rdRspParam;
            
            rdRspParam.connHandle = eventParam->connHandle;
            rdRspParam.charIndex  = CY_BLE_RTUS_TIME_UPDATE_STATE;
            rdRspParam.value      = &eventParam->value;
            
            Cy_BLE_RTUS_ApplCallback((uint32_t)CY_BLE_EVT_RTUSC_READ_CHAR_RESPONSE, (void*)&rdRspParam);

            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            cy_ble_rtuscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_RTUSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result a Write Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the #CY_BLE_EVT_RTUSS_WRITE_CHAR_CMD event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     sent to the server device.
*
* \return
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
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_RTUSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_rtus_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    
    /* Get pointer (with offset) to RTUS client structure with attribute handles */
    cy_stc_ble_rtusc_t *rtuscPtr = (cy_stc_ble_rtusc_t *)&cy_ble_rtuscConfigPtr->attrInfo[discIdx];
    
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((discIdx < cy_ble_configPtr->params->maxClientCount) && (attrValue != NULL) && 
            (charIndex == CY_BLE_RTUS_TIME_UPDATE_CONTROL_POINT) &&
            (rtuscPtr->charInfo[CY_BLE_RTUS_TIME_UPDATE_CONTROL_POINT].valueHandle !=
             CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Fill all fields of write command request structure ... */
        cy_stc_ble_gattc_write_cmd_req_t wrReqParam;
        
        wrReqParam.handleValPair.value.val  = attrValue;
        wrReqParam.handleValPair.value.len  = attrSize;
        wrReqParam.handleValPair.attrHandle = rtuscPtr->charInfo[CY_BLE_RTUS_TIME_UPDATE_CONTROL_POINT].valueHandle;
        wrReqParam.connHandle               = connHandle;
        
        /* Send request to write characteristic value */
        apiResult = Cy_BLE_GATTC_WriteWithoutResponse(&wrReqParam);
    }
    else
    {
        /* apiResult equals to CY_BLE_ERROR_INVALID_PARAMETER */
    }

    return(apiResult);
}


/*******************************************************************************
* Function Name: Cy_BLE_RTUSC_GetCharacteristicValue
****************************************************************************//**
*
*  Sends a request to a peer device to set characteristic value of the Reference
*  Time Update service, which is identified by charIndex.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of a service characteristic.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The Read Request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the RTUS service-specific callback is registered
*      (with \ref Cy_BLE_RTUS_RegisterAttrCallback):
*  * #CY_BLE_EVT_RTUSC_READ_CHAR_RESPONSE - In case if the requested attribute is
*                                successfully read on the peer device,
*                                the details (charIndex , value, etc.) are
*                                provided with event parameter structure
*                                of type cy_stc_ble_rtus_char_value_t.
*  .
*   Otherwise (if the RTUS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_RTUSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_rtus_char_index_t charIndex)
{
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
        if(charIndex == CY_BLE_RTUS_TIME_UPDATE_STATE)
        {
            
            /* Get pointer (with offset) to RTUS client structure with attribute handles */
            cy_stc_ble_rtusc_t *rtuscPtr = (cy_stc_ble_rtusc_t *)&cy_ble_rtuscConfigPtr->attrInfo[discIdx];
            
            cy_stc_ble_gattc_read_req_t readReqParam;
            readReqParam.attrHandle = rtuscPtr->charInfo[CY_BLE_RTUS_TIME_UPDATE_STATE].valueHandle;
            readReqParam.connHandle = connHandle;

            /* Send request to read characteristic value */
            apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_rtuscReqHandle[discIdx] = rtuscPtr->charInfo[CY_BLE_RTUS_TIME_UPDATE_STATE].valueHandle;
            }
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_RTUS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the Reference Time Update service.
*
*  \param eventCode:  The event code
*  \param eventParam: The event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_RTUS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_RTUSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_RTUSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_RTUSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_RTUSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_RTUSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the Reference Time Update service
*
*  \param eventCode:  The event code
*  \param eventParam: The event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_RTUSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_CMD_REQ:
            Cy_BLE_RTUSS_WriteCmdEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;
    
        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_RTUSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the Reference Time Update service
*
*  \param eventCode:  The event code
*  \param eventParam: The event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_RTUSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_RTUSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_RTUSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                Cy_BLE_RTUSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
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
