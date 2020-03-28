/***************************************************************************//**
* \file cy_ble_lls.c
* \version 3.40
*
* \brief
*  This file contains the source code for the Link Loss service.
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

static cy_en_ble_gatt_err_code_t Cy_BLE_LLS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_LLSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_LLSC_EventHandler(uint32_t eventCode, void *eventParam);

static void Cy_BLE_LLSS_ConnectEventHandler(void);
static cy_en_ble_gatt_err_code_t Cy_BLE_LLSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_LLSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_LLSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_LLSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_LLSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_LLSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_LLS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_LLSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_LLSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_llscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE LLS server config structure */
const cy_stc_ble_llss_config_t *cy_ble_llssConfigPtr = NULL;

/* The pointer to a global BLE LLS client config structure */
const cy_stc_ble_llsc_config_t *cy_ble_llscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_LLSS_Init
***************************************************************************//**
*
*  This function initializes server of the Link Loss service.
*
*  \param config: Configuration structure for the LLS.
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
cy_en_ble_api_result_t Cy_BLE_LLSS_Init(const cy_stc_ble_llss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_llssConfigPtr = config;

        /* Registers event handler for the LLS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_LLS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_LLSS_EventHandlerCallback = &Cy_BLE_LLSS_EventHandler;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_LLSC_Init
***************************************************************************//**
*
*  This function initializes client of the Link Loss service.
*
*  \param config: Configuration structure for the LLS.
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
cy_en_ble_api_result_t Cy_BLE_LLSC_Init(const cy_stc_ble_llsc_config_t *config)
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
        cy_ble_llscConfigPtr = config;

        /* Registers event handler for the LLS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_LLS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_LLSC_EventHandlerCallback = &Cy_BLE_LLSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 llsServIdx = cy_ble_llscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + llsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to LLS client structure */
                cy_stc_ble_llsc_t *llscPtr = (cy_stc_ble_llsc_t *)&cy_ble_llscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(llscPtr, 0, sizeof(cy_stc_ble_llsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + llsServIdx].uuid =
                    CY_BLE_UUID_LINK_LOSS_SERVICE;
            }

            cy_ble_llscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_LLS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for LLS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_LLSS_WRITE_CHAR_REQ).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_lls_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_LLS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_LLS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_LLSS_ConnectEventHandler
***************************************************************************//**
*
*  Handles the Connection Indication event for Link Loss service.
*
******************************************************************************/
static void Cy_BLE_LLSS_ConnectEventHandler(void)
{
    uint8_t tmpAlertLevel = CY_BLE_NO_ALERT;

    /* Input parameters validation passed, so save Alert Level */
    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

    dbAttrValInfo.connHandle.bdHandle        = 0u;
    dbAttrValInfo.connHandle.attId           = 0u;
    dbAttrValInfo.handleValuePair.attrHandle = cy_ble_llssConfigPtr->attrInfo->alertLevelCharHandle;
    dbAttrValInfo.handleValuePair.value.len  = CY_BLE_LLS_ALERT_LEVEL_SIZE;
    dbAttrValInfo.offset                     = 0u;
    dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

    dbAttrValInfo.handleValuePair.value.val = &tmpAlertLevel;
    (void)Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
}


/******************************************************************************
* Function Name: Cy_BLE_LLSS_WriteEventHandler
***************************************************************************//**
*
*  Handles the Write Request event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*   * CY_BLE_GATT_ERR_NONE - Write is successful.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_LLSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(Cy_BLE_LLS_ApplCallback != NULL)
    {
        if(cy_ble_llssConfigPtr->attrInfo->alertLevelCharHandle == eventParam->handleValPair.attrHandle)
        {
            /* Check whether attribute handle is handle of Alert Level characteristic of
             * Link Loss service. */
            if((CY_BLE_HIGH_ALERT >= eventParam->handleValPair.value.val[0u]))
            {
                if(eventParam->handleValPair.value.len == CY_BLE_LLS_ALERT_LEVEL_SIZE)
                {
                    /* Input parameters validation passed, so save Alert Level */
                    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

                    dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                    dbAttrValInfo.connHandle      = eventParam->connHandle;
                    dbAttrValInfo.offset          = 0u;
                    dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

                    gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);

                    /* Send callback to user if no error occurred while writing Alert Level */
                    if(gattErr == CY_BLE_GATT_ERR_NONE)
                    {
                        cy_stc_ble_lls_char_value_t wrReqParam;
                        wrReqParam.connHandle = eventParam->connHandle;
                        wrReqParam.charIndex  = CY_BLE_LLS_ALERT_LEVEL;
                        wrReqParam.value      = &eventParam->handleValPair.value;

                        Cy_BLE_LLS_ApplCallback((uint32_t)CY_BLE_EVT_LLSS_WRITE_CHAR_REQ, (void*)&wrReqParam);
                    }
                }
                else
                {
                    gattErr = CY_BLE_GATT_ERR_INVALID_ATTRIBUTE_LEN;
                }
            }
            else
            {
                gattErr = CY_BLE_GATT_ERR_OUT_OF_RANGE;
            }

            /* Indicate that request was handled */
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_LLSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets an Alert Level characteristic value of the service, which is identified
*  by charIndex.
*
*  \param charIndex: The index of an Alert Level characteristic.
*
*  \param attrSize:  The size of the Alert Level characteristic value attribute.
*
*  \param attrValue: The pointer to the location where an Alert Level characteristic
*                    value data should be stored.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_LLSS_GetCharacteristicValue(cy_en_ble_lls_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((attrValue != NULL) && (charIndex == CY_BLE_LLS_ALERT_LEVEL) && (attrSize == CY_BLE_LLS_ALERT_LEVEL_SIZE))
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_llssConfigPtr->attrInfo->alertLevelCharHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        dbAttrValInfo.offset                     = 0u;
        
        /* Get Alert Level characteristic value from GATT database */
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
* Function Name: Cy_BLE_LLSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_LLSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t llsDiscIdx = cy_ble_llscConfigPtr->serviceDiscIdx;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == llsDiscIdx))
    {
        /* Get pointer (with offset) to LLS client structure with attribute handles */
        cy_stc_ble_llsc_t *llscPtr = (cy_stc_ble_llsc_t *)&cy_ble_llscConfigPtr->attrInfo[discIdx];

        if(discCharInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_ALERT_LEVEL)
        {
            /* Save Alert Level Characteristic handle */
            Cy_BLE_CheckStoreCharHandle(llscPtr->alertLevelChar);
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_LLSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_LLSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t llsDiscIdx = cy_ble_llscConfigPtr->serviceDiscIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == llsDiscIdx)
    {
        /* LLS does not have any descriptions, return CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE */
        charRangeInfo->range.startHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        charRangeInfo->range.endHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_LLSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles the Read Response event.
*
*  \param cy_stc_ble_gattc_read_rsp_param_t *eventParam: The pointer to the data structure.
*
******************************************************************************/
static void Cy_BLE_LLSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_LLS_ApplCallback != NULL) &&
       (cy_ble_llscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Fill Link Loss service read response parameter structure */
        cy_stc_ble_lls_char_value_t rdRspParam;
        rdRspParam.connHandle = eventParam->connHandle;
        rdRspParam.charIndex  = CY_BLE_LLS_ALERT_LEVEL;
        rdRspParam.value      = &eventParam->value;

        cy_ble_llscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        Cy_BLE_LLS_ApplCallback((uint32_t)CY_BLE_EVT_LLSC_READ_CHAR_RESPONSE, &rdRspParam);
    }
}


/******************************************************************************
* Function Name: Cy_BLE_LLSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles the Write Response event for the Link Loss service.
*
*  \param cy_stc_ble_conn_handle_t *eventParam: The pointer to the connection handle.
*
******************************************************************************/
static void Cy_BLE_LLSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    /* Get pointer (with offset) to LLS client structure with attribute handles */
    cy_stc_ble_llsc_t *llscPtr = (cy_stc_ble_llsc_t *)&cy_ble_llscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_LLS_ApplCallback != NULL) &&
       (llscPtr->alertLevelChar.valueHandle == cy_ble_llscReqHandle[discIdx]) &&
       (cy_ble_llscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        cy_stc_ble_lls_char_value_t wrRspParam;
        wrRspParam.connHandle = *eventParam;
        wrRspParam.charIndex  = CY_BLE_LLS_ALERT_LEVEL;
        wrRspParam.value      = NULL;

        cy_ble_llscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        Cy_BLE_LLS_ApplCallback((uint32_t)CY_BLE_EVT_LLSC_WRITE_CHAR_RESPONSE, (void*)&wrRspParam);
    }
}


/******************************************************************************
* Function Name: Cy_BLE_LLSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles the Error Response event for the Link Loss service.
*
*  \param  *eventParam: Pointer to the cy_stc_ble_gatt_err_param_t structure.
*
******************************************************************************/
static void Cy_BLE_LLSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if(cy_ble_llscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        cy_ble_llscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_LLSC_SetCharacteristicValue
***************************************************************************//**
*
*  Sets the Alert Level characteristic value of the Link Loss service, which is
*  identified by charIndex. As a result a Write Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the #CY_BLE_EVT_LLSS_WRITE_CHAR_REQ event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle: The connection handle.
*  \param charIndex: The index of the Alert Level service characteristic.
*  \param attrSize: The size of the Alert Level characteristic value attribute.
*  \param attrValue: The pointer to the Alert Level characteristic value data that
*               should be sent to the server device.
*
* \return
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
*
* \events
*  If execution is successful (return value = #CY_BLE_SUCCESS),
*  these events can appear: \n
*   If a LLS service-specific callback is registered
*      with \ref Cy_BLE_LLS_RegisterAttrCallback():
*   * #CY_BLE_EVT_LLSC_WRITE_CHAR_RESPONSE - if the requested attribute is
*     successfully written on the peer device, the details (charIndex, etc.)
*     are provided with an event parameter structure
*     of type \ref cy_stc_ble_lls_char_value_t.
*   .
*   Otherwise (if a LLS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is successfully
*     written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the requested
*     attribute on the peer device, the details are provided with an event
*     parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_LLSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_lls_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to LLS client structure with attribute handles */
    cy_stc_ble_llsc_t *llscPtr = (cy_stc_ble_llsc_t *)&cy_ble_llscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((discIdx < cy_ble_configPtr->params->maxClientCount) && (attrValue != NULL) && (charIndex == CY_BLE_LLS_ALERT_LEVEL) &&
            (attrSize == CY_BLE_LLS_ALERT_LEVEL_SIZE) && (*attrValue <= CY_BLE_HIGH_ALERT) &&
            (llscPtr->alertLevelChar.valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Fill all fields of write command request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle = llscPtr->alertLevelChar.valueHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;

        /* Send request to write Alert Level characteristic value */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);
        cy_ble_llscReqHandle[discIdx] = llscPtr->alertLevelChar.valueHandle;
    }
    else
    {
        /* Empty else */
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_LLSC_GetCharacteristicValue
***************************************************************************//**
*
*  Sends a request to get characteristic value of the Link Loss service, which
*  is identified by charIndex.
*
*  This function call can result in generation of the following events based on
*  the response from the server device:
*  * #CY_BLE_EVT_LLSC_READ_CHAR_RESPONSE
*  * #CY_BLE_EVT_GATTC_ERROR_RSP
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the Link Loss service characteristic.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The Read Request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*
*  \events
*  If execution is successful (return value = \ref CY_BLE_SUCCESS)
*  these events can appear: \n
*  If the LLS service-specific callback is registered
*      (with \ref Cy_BLE_LLS_RegisterAttrCallback):
*  * \ref CY_BLE_EVT_LLSC_READ_CHAR_RESPONSE - if the requested attribute is
*    successfully read on the peer device, the details (charIndex,
*    value, etc.) are provided with an event parameter structure
*    of type \ref cy_stc_ble_lls_char_value_t.
*  .
*  Otherwise (if a LLS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTC_READ_RSP - if the requested attribute is
*    successfully read on the peer device, the details (handle,
*    value, etc.) are provided with the event parameters structure
*    \ref cy_stc_ble_gattc_read_rsp_param_t.
*
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*    requested attribute on the peer device, the details are provided
*    with the event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_LLSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_lls_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to LLS client structure with attribute handles */
    cy_stc_ble_llsc_t *llscPtr = (cy_stc_ble_llsc_t *)&cy_ble_llscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex == CY_BLE_LLS_ALERT_LEVEL) && (discIdx < cy_ble_configPtr->params->maxClientCount))
    {
        /* Send request to write Alert Level characteristic value */
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = cy_ble_llscReqHandle[discIdx];
        readReqParam.connHandle = connHandle;

        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);
        cy_ble_llscReqHandle[discIdx] = llscPtr->alertLevelChar.valueHandle;
    }
    else
    {
        /* apiResult equals CY_BLE_ERROR_INVALID_PARAMETER */
    }

    /* Return status */
    return(apiResult);
}



/******************************************************************************
* Function Name: Cy_BLE_LLS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the LLS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_LLS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_LLSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_LLSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_LLSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_LLSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_LLSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the LLS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_LLSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_LLSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATT_CONNECT_IND:
            Cy_BLE_LLSS_ConnectEventHandler();
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_LLSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the LLS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_LLSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_LLSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_LLSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_LLSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_LLSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_LLSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
