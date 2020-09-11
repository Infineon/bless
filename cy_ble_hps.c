/***************************************************************************//**
* \file cy_ble_hps.c
* \version 3.50
*
* \brief
*  Contains the source code for HTTP Proxy service.
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

static cy_en_ble_gatt_err_code_t Cy_BLE_HPS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_HPSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_HPSC_EventHandler(uint32_t eventCode, void *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_HPSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);
static void Cy_BLE_HPSS_PrepareWriteRequestEventHandler(const cy_stc_ble_gatts_prep_write_req_param_t *eventParam);
static void Cy_BLE_HPSS_ExecuteWriteRequestEventHandler(cy_stc_ble_gatts_exec_write_req_t *eventParam);

static void Cy_BLE_HPSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_HPSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_HPSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_HPSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_HPSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_HPSC_ReadLongRespEventHandler(const cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_HPSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_HPSC_ExecuteWriteResponseEventHandler(const cy_stc_ble_gattc_exec_write_rsp_t *eventParam);
static void Cy_BLE_HPSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_HPS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_HPSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_HPSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response for server */
static cy_ble_gatt_db_attr_handle_t cy_ble_hpssReqHandle;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_hpscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* Variables for Read Long Characteristic Values handling */
uint8_t * cy_ble_hpscRdLongBuffPtr;
uint16_t cy_ble_hpscRdLongBuffLen;
uint16_t cy_ble_hpscCurrLen;

/* The pointer to a global BLE HPS server config structure */
const cy_stc_ble_hpss_config_t *cy_ble_hpssConfigPtr = NULL;

/* The pointer to a global BLE HPS client config structure */
const cy_stc_ble_hpsc_config_t *cy_ble_hpscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_HPSS_Init
***************************************************************************//**
*
*  This function initializes server of the HTTP Proxy service.
*
*  \param config: Configuration structure for the HPS.
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
cy_en_ble_api_result_t Cy_BLE_HPSS_Init(const cy_stc_ble_hpss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_hpssConfigPtr = config;

        /* Registers event handler for the HPS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_HPS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_HPSS_EventHandlerCallback = &Cy_BLE_HPSS_EventHandler;

        cy_ble_hpssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_Init
***************************************************************************//**
*
*  This function initializes client of the HTTP Proxy service.
*
*  \param config: Configuration structure for the HPS.
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
cy_en_ble_api_result_t Cy_BLE_HPSC_Init(const cy_stc_ble_hpsc_config_t *config)
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
        cy_ble_hpscConfigPtr = config;

        /* Registers event handler for the HPS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_HPS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_HPSC_EventHandlerCallback = &Cy_BLE_HPSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 hpsServIdx = cy_ble_hpscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + hpsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to HPS client structure */
                cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(hpscPtr, 0, sizeof(cy_stc_ble_hpsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + hpsServIdx].uuid =
                    CY_BLE_UUID_HTTP_PROXY_SERVICE;
            }

            cy_ble_hpscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HPS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for HPS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_HPSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_hps_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_HPS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_HPS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_HPSS_WriteEventHandler
***************************************************************************//**
*
*  Handles Write Request event for HTTP Proxy service.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
* \return
*  A return value of type cy_en_ble_gatt_err_code_t.
*   * CY_BLE_GATT_ERR_NONE - Write is successful.
*   * CY_BLE_GATT_ERR_REQUEST_NOT_SUPPORTED - The request is not supported.
*   * CY_BLE_GATT_ERR_INVALID_HANDLE - 'handleValuePair.attrHandle' is not valid.
*   * CY_BLE_GATT_ERR_WRITE_NOT_PERMITTED - The write operation is not permitted on
*                                          this attribute.
*   * CY_BLE_GATT_ERR_INVALID_OFFSET - The offset value is invalid.
*   * CY_BLE_GATT_ERR_UNLIKELY_ERROR - Some other error occurred.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HPSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    uint32_t event = (uint32_t)CY_BLE_EVT_HPSS_NOTIFICATION_DISABLED;
    cy_stc_ble_hps_char_value_t wrCharReqParam =
    {
        .connHandle = eventParam->connHandle
    };

    if(Cy_BLE_HPS_ApplCallback != NULL)
    {
        /* Client Characteristic Configuration descriptor Write Request */
        if(eventParam->handleValPair.attrHandle ==
           cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_STATUS_CODE].descrHandle[CY_BLE_HPS_CCCD])
        {
            wrCharReqParam.charIndex = CY_BLE_HPS_HTTP_STATUS_CODE;
            wrCharReqParam.value = NULL;

            if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
            {
                event = (uint32_t)CY_BLE_EVT_HPSS_NOTIFICATION_ENABLED;
            }
        }
        else
        {
            event = (uint32_t)CY_BLE_EVT_HPSS_WRITE_CHAR;
            wrCharReqParam.value = &eventParam->handleValPair.value;

            if(cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_URI].charHandle == eventParam->handleValPair.attrHandle)
            {
                wrCharReqParam.charIndex = CY_BLE_HPS_URI;
            }
            else if(cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_HEADERS].charHandle ==
                    eventParam->handleValPair.attrHandle)
            {
                wrCharReqParam.charIndex = CY_BLE_HPS_HTTP_HEADERS;
            }
            else if(cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_ENTITY_BODY].charHandle ==
                    eventParam->handleValPair.attrHandle)
            {
                wrCharReqParam.charIndex = CY_BLE_HPS_HTTP_ENTITY_BODY;
            }
            else if(cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_CP].charHandle ==
                    eventParam->handleValPair.attrHandle)
            {
                if(CY_BLE_IS_NOTIFICATION_ENABLED(eventParam->connHandle.attId,
                                                  cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_STATUS_CODE].
                                                   descrHandle[CY_BLE_HPS_CCCD]))
                {
                    wrCharReqParam.charIndex = CY_BLE_HPS_HTTP_CP;
                }
                else
                {
                    gattErr = CY_BLE_GATT_ERR_CCCD_IMPROPERLY_CONFIGURED;

                    /* Clear callback flag indicating that request was handled */
                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                }
            }
            else
            {
                /* Set charIndex  to CY_BLE_HPS_CHAR_COUNT as the requested handle doesn't
                 * match to any handles of HPS characteristics.
                 */
                wrCharReqParam.charIndex = CY_BLE_HPS_CHAR_COUNT;
            }
        }

        if(gattErr == CY_BLE_GATT_ERR_NONE)
        {
            if(wrCharReqParam.charIndex < CY_BLE_HPS_CHAR_COUNT)
            {
                cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

                dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                dbAttrValInfo.connHandle      = eventParam->connHandle;
                dbAttrValInfo.offset          = 0u;
                dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

                gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);

                if(gattErr == CY_BLE_GATT_ERR_NONE)
                {
                    if((event == (uint32_t)CY_BLE_EVT_HPSS_WRITE_CHAR) &&
                       (wrCharReqParam.value->len <=
                        (CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(eventParam->handleValPair.attrHandle))))
                    {
                        CY_BLE_GATT_DB_ATTR_SET_ATTR_GEN_LEN(eventParam->handleValPair.attrHandle,
                                                             wrCharReqParam.value->len);
                    }

                    /* Make sure that GATT error is set to "No error" */
                    wrCharReqParam.gattErrorCode = CY_BLE_GATT_ERR_NONE;
                    Cy_BLE_HPS_ApplCallback(event, &wrCharReqParam);
                    gattErr = wrCharReqParam.gattErrorCode;
                }

                /* Clear callback flag indicating that request was handled */
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            }
        }
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_HPSS_PrepareWriteRequestEventHandler
***************************************************************************//**
*
*  Handles the Write Request HTTP Proxy service.
*
*  \param eventParam: The pointer to the data that received with a prepare write
*                     request event for the HTTP Proxy service.
*
******************************************************************************/
static void Cy_BLE_HPSS_PrepareWriteRequestEventHandler(const cy_stc_ble_gatts_prep_write_req_param_t *eventParam)
{
    if(Cy_BLE_HPS_ApplCallback != NULL)
    {
        if((eventParam->baseAddr[eventParam->currentPrepWriteReqCount - 1u].handleValuePair.attrHandle ==
            cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_URI].charHandle) ||
           (eventParam->baseAddr[eventParam->currentPrepWriteReqCount - 1u].handleValuePair.attrHandle ==
            cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_HEADERS].charHandle) ||
           (eventParam->baseAddr[eventParam->currentPrepWriteReqCount - 1u].handleValuePair.attrHandle ==
            cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_ENTITY_BODY].charHandle))
        {
            if(cy_ble_hpssReqHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Send Prepare Write Response which identifies acknowledgment for
                 * long characteristic value write.
                 */

                cy_ble_hpssReqHandle =
                    eventParam->baseAddr[eventParam->currentPrepWriteReqCount - 1u].handleValuePair.attrHandle;
            }
            /* Indicate that request was handled */
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HPSS_ExecuteWriteRequestEventHandler
***************************************************************************//**
*
*  Handles the Execute Write Request event for the HTTP Proxy service.
*
*  \param eventParam: The pointer to the data that came with a Write Request
*                     for the HTTP Proxy service.
*
******************************************************************************/
static void Cy_BLE_HPSS_ExecuteWriteRequestEventHandler(cy_stc_ble_gatts_exec_write_req_t *eventParam)
{
    uint32_t locCount;
    uint16_t locLength = 0u;
    cy_stc_ble_gatt_value_t locCharValue;
    cy_stc_ble_hps_char_value_t wrCharReqParam = { .connHandle = eventParam->connHandle };

    if((Cy_BLE_HPS_ApplCallback != NULL) && (cy_ble_hpssReqHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        if((eventParam->baseAddr[0u].handleValuePair.attrHandle ==
            cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_URI].charHandle) ||
           (eventParam->baseAddr[0u].handleValuePair.attrHandle ==
            cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_HEADERS].charHandle) ||
           (eventParam->baseAddr[0u].handleValuePair.attrHandle ==
            cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_ENTITY_BODY].charHandle))
        {
            /* Check the execWriteFlag before execute or cancel write long operation */
            if(eventParam->execWriteFlag == CY_BLE_GATT_EXECUTE_WRITE_EXEC_FLAG)
            {
                /* Calculate total length */
                for(locCount = 0u; locCount < eventParam->prepWriteReqCount; locCount++)
                {
                    locLength += eventParam->baseAddr[locCount].handleValuePair.value.len;
                }

                if(CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(cy_ble_hpssReqHandle) >= locLength)
                {
                    /* Fill data and pass it to user */
                    if(eventParam->baseAddr[0u].handleValuePair.attrHandle == cy_ble_hpssConfigPtr->attrInfo->
                        charInfo[CY_BLE_HPS_URI].charHandle)
                    {
                        wrCharReqParam.charIndex = CY_BLE_HPS_URI;
                    }
                    else if(eventParam->baseAddr[0u].handleValuePair.attrHandle ==
                            cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_HEADERS].charHandle)
                    {
                        wrCharReqParam.charIndex = CY_BLE_HPS_HTTP_HEADERS;
                    }
                    else
                    {
                        wrCharReqParam.charIndex = CY_BLE_HPS_HTTP_ENTITY_BODY;
                    }


                    wrCharReqParam.gattErrorCode = CY_BLE_GATT_ERR_NONE;
                    locCharValue = eventParam->baseAddr[0u].handleValuePair.value;
                    wrCharReqParam.value = &locCharValue;
                    wrCharReqParam.value->len = locLength;

                    Cy_BLE_HPS_ApplCallback((uint32_t)CY_BLE_EVT_HPSS_WRITE_CHAR, &wrCharReqParam);

                    if(wrCharReqParam.gattErrorCode == CY_BLE_GATT_ERR_NONE)
                    {
                        CY_BLE_GATT_DB_ATTR_SET_ATTR_GEN_LEN(cy_ble_hpssReqHandle, locLength);
                    }
                }
                else
                {
                    wrCharReqParam.gattErrorCode = CY_BLE_GATT_ERR_INVALID_ATTRIBUTE_LEN;
                }

                /* Pass user error code to Stack */
                eventParam->gattErrorCode = (uint8_t)wrCharReqParam.gattErrorCode;
            }

            /* Indicate that request was handled */
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;

            /* Clear requested handle */
            cy_ble_hpssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HPSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a value for one of characteristic values of the HTTP Proxy
*  service. The characteristic is identified by charIndex.
*
*  \param charIndex: The index of the service characteristic of type
*                    \ref cy_en_ble_hps_char_index_t.
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
cy_en_ble_api_result_t Cy_BLE_HPSS_SetCharacteristicValue(cy_en_ble_hps_char_index_t charIndex,
                                                          uint16_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((attrValue != NULL) && (charIndex < CY_BLE_HPS_CHAR_COUNT))
    {
        /* Fill structure */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_hpssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        
        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            if(attrSize <= CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(dbAttrValInfo.handleValuePair.attrHandle))
            {
                CY_BLE_GATT_DB_ATTR_SET_ATTR_GEN_LEN(dbAttrValInfo.handleValuePair.attrHandle, attrSize);
            }

            /* Indicate success */
            apiResult = CY_BLE_SUCCESS;
        }
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HPSS_GetCharacteristicValue
***************************************************************************//**
*
*  Reads a characteristic value of the HTTP Proxy service, which is identified
*  by charIndex from the GATT database.
*
*  \param charIndex: The index of the service characteristic of type
*                    \ref cy_en_ble_hps_char_index_t.
*  \param attrSize:  The size of the HTTP Proxy service characteristic value attribute.
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
cy_en_ble_api_result_t Cy_BLE_HPSS_GetCharacteristicValue(cy_en_ble_hps_char_index_t charIndex,
                                                          uint16_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((attrValue != NULL) && (charIndex < CY_BLE_HPS_CHAR_COUNT))
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_hpssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        
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
* Function Name: Cy_BLE_HPSS_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sets the characteristic descriptor value of the specified characteristic.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hps_char_index_t.
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_hps_descr_index_t.
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  The pointer to the descriptor value data to be stored in the GATT
*                     database.
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
cy_en_ble_api_result_t Cy_BLE_HPSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hps_char_index_t charIndex,
                                                               cy_en_ble_hps_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex == CY_BLE_HPS_HTTP_STATUS_CODE) && (descrIndex == CY_BLE_HPS_CCCD) && (attrValue != NULL))
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_STATUS_CODE].
                                                   descrHandle[CY_BLE_HPS_CCCD];
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.connHandle                 = connHandle;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        
        /* Sets characteristic value to database */
        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_SUCCESS;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HPSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Reads a a characteristic descriptor of a specified characteristic of the
*  HTTP Proxy service from the GATT database.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hps_char_index_t.
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_hps_descr_index_t.
*  \param attrSize:   The size of the descriptor value.
*  \param attrValue:  The pointer to the location where characteristic descriptor value
*                     data should be stored.
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
cy_en_ble_api_result_t Cy_BLE_HPSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hps_char_index_t charIndex,
                                                               cy_en_ble_hps_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex == CY_BLE_HPS_HTTP_STATUS_CODE) && (descrIndex == CY_BLE_HPS_CCCD) && (attrValue != NULL))
    {
        if(cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_STATUS_CODE].descrHandle[CY_BLE_HPS_CCCD] !=
           CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_STATUS_CODE].
                                                       descrHandle[CY_BLE_HPS_CCCD];
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
* Function Name: Cy_BLE_HPSS_SendNotification
***************************************************************************//**
*
*  Sends a notification with a characteristic value of the HTTP Proxy
*  service, which is a value specified by charIndex, to the client's device.
*  On enabling notification successfully for a service characteristic it sends out a
*  'Handle Value Notification' which results in \ref CY_BLE_EVT_HPSC_NOTIFICATION event
*  at the GATT Client's end.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hps_char_index_t.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     sent to the client's device.
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
cy_en_ble_api_result_t Cy_BLE_HPSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_hps_char_index_t charIndex,
                                                    uint8_t attrSize,
                                                    uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if(charIndex < CY_BLE_HPS_CHAR_COUNT)
    {
        if(cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_STATUS_CODE].descrHandle[CY_BLE_HPS_CCCD] !=
           CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            /* Send notification if it is enabled and peer device is connected */
            if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
            {
                apiResult = CY_BLE_ERROR_INVALID_STATE;
            }

            else if(!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId,
                                                    cy_ble_hpssConfigPtr->attrInfo->
                                                     charInfo[CY_BLE_HPS_HTTP_STATUS_CODE].descrHandle[CY_BLE_HPS_CCCD]))
            {
                apiResult = CY_BLE_ERROR_NTF_DISABLED;
            }
            else
            {
                /* Fill all fields of Write Request structure ... */
                cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
                ntfReqParam.handleValPair.attrHandle = cy_ble_hpssConfigPtr->attrInfo->charInfo[CY_BLE_HPS_HTTP_STATUS_CODE].
                                                       charHandle;
                ntfReqParam.handleValPair.value.val  = attrValue;
                ntfReqParam.handleValPair.value.len  = attrSize;
                ntfReqParam.connHandle               = connHandle;

                /* Send notification to client using previously filled structure */
                apiResult = Cy_BLE_GATTS_Notification(&ntfReqParam);
            }
        }
        else
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_HPSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* HPS characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_hpscCharUuid[CY_BLE_HPS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_URI,
        CY_BLE_UUID_CHAR_HTTP_HEADERS,
        CY_BLE_UUID_CHAR_HTTP_ENTITY_BODY,
        CY_BLE_UUID_CHAR_HTTP_CP,
        CY_BLE_UUID_CHAR_HTTP_STATUS_CODE,
        CY_BLE_UUID_CHAR_HTTPS_SECURITY
    };
    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t hpsDiscIdx = cy_ble_hpscConfigPtr->serviceDiscIdx;
    uint32_t i;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == hpsDiscIdx))
    {
        /* Update last characteristic endHandle to declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        /* Search through all available characteristics */
        for(i = (uint32_t)CY_BLE_HPS_URI; (i < (uint32_t)CY_BLE_HPS_CHAR_COUNT); i++)
        {
            if(cy_ble_hpscCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                /* Get pointer (with offset) to HPS client structure with attribute handles */
                cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];

                if(hpscPtr->charInfo[i].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    hpscPtr->charInfo[i].valueHandle = discCharInfo->valueHandle;
                    hpscPtr->charInfo[i].properties = discCharInfo->properties;
                    lastEndHandle[discIdx] = &hpscPtr->charInfo[i].endHandle;
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
* Function Name: Cy_BLE_HPSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  Based on the descriptor UUID, an appropriate data structure is populated using
*  the data received as part of the callback.
*
*  \param discDescrInfo: The pointer to a descriptor information structure.
*
******************************************************************************/
static void Cy_BLE_HPSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t notSupportedDescr = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t hpsDiscIdx = cy_ble_hpscConfigPtr->serviceDiscIdx;
    uint32_t descIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == hpsDiscIdx)
    {
        switch(discDescrInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_CLIENT_CONFIG:
                descIdx = (uint32_t)CY_BLE_HPS_CCCD;
                break;

            default:
                /* Not supported descriptor */
                notSupportedDescr = 1u;
                break;
        }

        if(notSupportedDescr == 0u)
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            
            /* Get pointer (with offset) to HPS client structure with attribute handles */
            cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];
            
            if(hpscPtr->charInfo[charIdx].descrHandle[descIdx] == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                hpscPtr->charInfo[charIdx].descrHandle[descIdx] = discDescrInfo->descrHandle;
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
* Function Name: Cy_BLE_HPSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_HPSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t hpsDiscIdx = cy_ble_hpscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == hpsDiscIdx)
    {
        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_HPS_CHAR_COUNT) && (exitFlag == 0u))
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            
            /* Get pointer (with offset) to HPS client structure with attribute handles */
            cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];
            
            if((hpscPtr->charInfo[charIdx].endHandle - hpscPtr->charInfo[charIdx].valueHandle) != 0u)
            {
                /* Read characteristic range */
                charRangeInfo->range.startHandle = hpscPtr->charInfo[charIdx].valueHandle + 1u;
                charRangeInfo->range.endHandle = hpscPtr->charInfo[charIdx].endHandle;
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
* Function Name: Cy_BLE_HPSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles a notification event for the HTTP service.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_HPSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    if(Cy_BLE_HPS_ApplCallback != NULL)
    {
        uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
        
        /* Get pointer (with offset) to HPS client structure with attribute handles */
        cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];
        
        if((discIdx < cy_ble_configPtr->params->maxClientCount) &&
           (hpscPtr->charInfo[CY_BLE_HPS_HTTP_STATUS_CODE].valueHandle ==
            eventParam->handleValPair.attrHandle))
        {
            cy_stc_ble_hps_char_value_t notifValue;
            notifValue.connHandle = eventParam->connHandle;
            notifValue.charIndex  = CY_BLE_HPS_HTTP_STATUS_CODE;
            notifValue.value      = &eventParam->handleValPair.value;

            Cy_BLE_HPS_ApplCallback((uint32_t)CY_BLE_EVT_HPSC_NOTIFICATION, &notifValue);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles a Read Response event for the HTTP Proxy service.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_HPSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    cy_en_ble_hps_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
    
    /* Get pointer (with offset) to HPS client structure with attribute handles */
    cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];
    
    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_HPS_ApplCallback != NULL) &&
       (cy_ble_hpscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        for(locCharIndex = CY_BLE_HPS_URI; (locCharIndex < CY_BLE_HPS_CHAR_COUNT); locCharIndex++)
        {
            if(cy_ble_hpscReqHandle[discIdx] == hpscPtr->charInfo[locCharIndex].valueHandle)
            {
                break;
            }
        }

        if(locCharIndex < CY_BLE_HPS_CHAR_COUNT)
        {
            cy_stc_ble_hps_char_value_t locCharValue;
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.charIndex  = locCharIndex;
            locCharValue.value      = &eventParam->value;

            cy_ble_hpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_HPS_ApplCallback((uint32_t)CY_BLE_EVT_HPSC_READ_CHAR_RESPONSE, &locCharValue);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else if(cy_ble_hpscReqHandle[discIdx] == hpscPtr->charInfo[CY_BLE_HPS_HTTP_STATUS_CODE].
                 descrHandle[CY_BLE_HPS_CCCD])
        {
            cy_stc_ble_hps_descr_value_t locDescrValue;
            locDescrValue.connHandle = eventParam->connHandle;
            locDescrValue.charIndex  = CY_BLE_HPS_HTTP_STATUS_CODE;
            locDescrValue.descrIndex = CY_BLE_HPS_CCCD;
            locDescrValue.value      = &eventParam->value;

            cy_ble_hpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_HPS_ApplCallback((uint32_t)CY_BLE_EVT_HPSC_READ_DESCR_RESPONSE, &locDescrValue);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else
        {
            /* Do nothing */
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_ReadLongRespEventHandler
***************************************************************************//**
*
*  Handles a Read Long Response event for the HTTP Proxy service.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_HPSC_ReadLongRespEventHandler(const cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    cy_stc_ble_gattc_stop_cmd_param_t stopCmdParam = { .connHandle = eventParam->connHandle };
    cy_stc_ble_gatt_xchg_mtu_param_t mtuParam = { .connHandle = eventParam->connHandle };
    cy_en_ble_hps_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
    uint32_t isReqEnded = 1u;

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_HPS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to HPS client structure with attribute handles */
        cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];
       
        /* Check whether requested handle equals to any of HPS characteristic handles */
        if(cy_ble_hpscReqHandle[discIdx] == hpscPtr->charInfo[CY_BLE_HPS_URI].valueHandle)
        {
            locCharIndex = CY_BLE_HPS_URI;
        }
        else if(cy_ble_hpscReqHandle[discIdx] == hpscPtr->charInfo[CY_BLE_HPS_HTTP_HEADERS].valueHandle)
        {
            locCharIndex = CY_BLE_HPS_HTTP_HEADERS;
        }
        else if(cy_ble_hpscReqHandle[discIdx] == hpscPtr->charInfo[CY_BLE_HPS_HTTP_ENTITY_BODY].valueHandle)
        {
            locCharIndex = CY_BLE_HPS_HTTP_ENTITY_BODY;
        }
        else
        {
            locCharIndex = CY_BLE_HPS_CHAR_COUNT;
        }

        /* If match was found then proceed with the request handling */
        if((locCharIndex < CY_BLE_HPS_CHAR_COUNT) && (cy_ble_hpscReqHandle[discIdx] !=
                                                      CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
        {
            /* Update user buffer with received data */
            if(cy_ble_hpscCurrLen < cy_ble_hpscRdLongBuffLen)
            {
                (void)memcpy((void*)&cy_ble_hpscRdLongBuffPtr[cy_ble_hpscCurrLen], (void*)&eventParam->value.val[0],
                             (uint32_t)eventParam->value.len);

                cy_ble_hpscCurrLen += eventParam->value.len;
            }

            (void)Cy_BLE_GATT_GetMtuSize(&mtuParam);

            /* If the received data length is less than the MTU size, the Read Long
             * request is completed or the provided user's buffer is full.
             */
            if(((mtuParam.mtu - 1u) > eventParam->value.len))
            {
                cy_ble_hpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            }
            else if(cy_ble_hpscCurrLen == cy_ble_hpscRdLongBuffLen)
            {
                (void)Cy_BLE_GATTC_StopCmd(&stopCmdParam);
            }
            else
            {
                isReqEnded = 0u;
            }

            /* If the buffer is full, then stop processing any remaining read long
             * requests.
             */
            if(isReqEnded == 1u)
            {
                cy_stc_ble_hps_char_value_t rdCharValue = { .connHandle = eventParam->connHandle };
                cy_stc_ble_gatt_value_t rdValue;
                rdValue.val = cy_ble_hpscRdLongBuffPtr;
                rdValue.len = cy_ble_hpscCurrLen;

                rdCharValue.charIndex = locCharIndex;
                rdCharValue.value = &rdValue;

                Cy_BLE_HPS_ApplCallback((uint32_t)CY_BLE_EVT_HPSC_READ_CHAR_RESPONSE, &rdCharValue);
                cy_ble_hpscCurrLen = 0u;
            }

            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles a Write Response event for the HTTP Proxy service.
*
*  \param eventParam: The pointer to a data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_HPSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    cy_en_ble_hps_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_HPS_ApplCallback != NULL) &&
       (cy_ble_hpscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to HPS client structure with attribute handles */
        cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];
        
        for(locCharIndex = CY_BLE_HPS_URI; (locCharIndex < CY_BLE_HPS_CHAR_COUNT); locCharIndex++)
        {
            if(cy_ble_hpscReqHandle[discIdx] == hpscPtr->charInfo[locCharIndex].valueHandle)
            {
                break;
            }
        }

        if(locCharIndex < CY_BLE_HPS_CHAR_COUNT)
        {
            cy_stc_ble_hps_char_value_t locCharValue;
            locCharValue.connHandle = *eventParam;
            locCharValue.charIndex  = locCharIndex;
            locCharValue.value      = NULL;

            cy_ble_hpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_HPS_ApplCallback((uint32_t)CY_BLE_EVT_HPSC_WRITE_CHAR_RESPONSE, &locCharValue);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else if(cy_ble_hpscReqHandle[discIdx] ==
                hpscPtr->charInfo[CY_BLE_HPS_HTTP_STATUS_CODE].descrHandle[CY_BLE_HPS_CCCD])
        {
            cy_stc_ble_hps_descr_value_t locDescrValue;
            locDescrValue.connHandle = *eventParam;
            locDescrValue.charIndex  = CY_BLE_HPS_HTTP_STATUS_CODE;
            locDescrValue.descrIndex = CY_BLE_HPS_CCCD;
            locDescrValue.value      = NULL;

            cy_ble_hpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_HPS_ApplCallback((uint32_t)CY_BLE_EVT_HPSC_WRITE_DESCR_RESPONSE, &locDescrValue);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else
        {
            /* The requested handle doesn't belong to HPS */
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_ExecuteWriteResponseEventHandler
***************************************************************************//**
*
*  Handles a Execute Write Response event for the HTTP Proxy service.
*
*  \param eventParam: The pointer to a data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_HPSC_ExecuteWriteResponseEventHandler(const cy_stc_ble_gattc_exec_write_rsp_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
    cy_en_ble_hps_char_index_t locCharIndex;

    /* Check whether service handler was registered and request handle is valid. */
    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_HPS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to HPS client structure with attribute handles */
        cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];
            
        /* Check whether requested handle equals to any of HPS characteristic handles */
        if(cy_ble_hpscReqHandle[discIdx] == hpscPtr->charInfo[CY_BLE_HPS_URI].valueHandle)
        {
            locCharIndex = CY_BLE_HPS_URI;
        }
        else if(cy_ble_hpscReqHandle[discIdx] == hpscPtr->charInfo[CY_BLE_HPS_HTTP_HEADERS].valueHandle)
        {
            locCharIndex = CY_BLE_HPS_HTTP_HEADERS;
        }
        else if(cy_ble_hpscReqHandle[discIdx] == hpscPtr->charInfo[CY_BLE_HPS_HTTP_ENTITY_BODY].valueHandle)
        {
            locCharIndex = CY_BLE_HPS_HTTP_ENTITY_BODY;
        }
        else
        {
            locCharIndex = CY_BLE_HPS_CHAR_COUNT;
        }

        if(locCharIndex < CY_BLE_HPS_CHAR_COUNT)
        {
            cy_stc_ble_hps_char_value_t locCharValue;
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.charIndex  = locCharIndex;
            locCharValue.value      = NULL;

            cy_ble_hpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_HPS_ApplCallback((uint32_t)CY_BLE_EVT_HPSC_WRITE_CHAR_RESPONSE, &locCharValue);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles an Error Response event for the HTTP Proxy service.
*
*  \param eventParam - The pointer to a data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_HPSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        if(cy_ble_hpscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] == eventParam->errInfo.attrHandle)
        {
            cy_ble_hpscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result a Write Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the \ref CY_BLE_EVT_HPSS_WRITE_CHAR event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hps_char_index_t.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be
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
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the HPS service-specific callback is registered
*   with \ref Cy_BLE_HPS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HPSC_WRITE_CHAR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_hps_char_value_t.
*   .
*   Otherwise (if the HPS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - In case if the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*     requested attribute on the peer device, the details are provided with event
*     parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HPSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_hps_char_index_t charIndex,
                                                          uint16_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    
    /* Get pointer (with offset) to HPS client structure with attribute handles */
    cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HPS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(hpscPtr->charInfo[charIndex].valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle = hpscPtr->charInfo[charIndex].valueHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;

        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);
        /* Save handle to support service-specific Write Response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_hpscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_GetCharacteristicValue
***************************************************************************//**
*
*  Sends a request to the peer device to get a characteristic value, as
*  identified by its charIndex.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hps_char_index_t.
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the HPS service-specific callback is registered
*      with \ref Cy_BLE_HPS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HPSC_READ_CHAR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details 
*     (charIndex , value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_hps_char_value_t.
*   .
*   Otherwise (if an HPS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_HPSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_hps_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HPS client structure with attribute handles */
    cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];
        
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HPS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(hpscPtr->charInfo[charIndex].valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = hpscPtr->charInfo[charIndex].valueHandle;
        readReqParam.connHandle = connHandle;

        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_hpscReqHandle[discIdx] = hpscPtr->charInfo[charIndex].valueHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_SetLongCharacteristicValue
***************************************************************************//**
*
*  Sends a request to set a long characteristic value of the service, which is
*  a value identified by charIndex, to the server's device.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hps_char_index_t.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be
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
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the HPS service-specific callback is registered
*   with \ref Cy_BLE_HPS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HPSC_WRITE_CHAR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_hps_char_value_t.
*   .
*   Otherwise (if the HPS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_EXEC_WRITE_RSP - In case if the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*     requested attribute on the peer device, the details are provided with event
*     parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HPSC_SetLongCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                              cy_en_ble_hps_char_index_t charIndex,
                                                              uint16_t attrSize,
                                                              uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HPS client structure with attribute handles */
    cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];
    
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HPS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(hpscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gattc_prep_write_req_t writeReqParam;
        writeReqParam.handleValOffsetPair.handleValuePair.attrHandle = hpscPtr->charInfo[charIndex].valueHandle;
        writeReqParam.handleValOffsetPair.handleValuePair.value.val  = attrValue;
        writeReqParam.handleValOffsetPair.handleValuePair.value.len  = attrSize;
        writeReqParam.handleValOffsetPair.offset                     = 0u;
        writeReqParam.connHandle                                     = connHandle;
        
        /* ... and send request to server's device. */
        apiResult = Cy_BLE_GATTC_WriteLongCharacteristicValues(&writeReqParam);

        /* Save handle to support service-specific Write Response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_hpscReqHandle[discIdx] = writeReqParam.handleValOffsetPair.handleValuePair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_GetLongCharacteristicValue
***************************************************************************//**
*
*  This function is used to read a long characteristic value, which is a value
*  identified by charIndex, from the server.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hps_char_index_t.
*  \param attrSize:   The size of the buffer to store long characteristic value.
*  \param attrValue:  The pointer to the buffer where the read long characteristic
*                     value should be stored.
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the HPS service-specific callback is registered
*      with \ref Cy_BLE_HPS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HPSC_READ_CHAR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details 
*     (charIndex , value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_hps_char_value_t.
*   .
*   Otherwise (if an HPS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_HPSC_GetLongCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                              cy_en_ble_hps_char_index_t charIndex,
                                                              uint16_t attrSize,
                                                              uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HPS client structure with attribute handles */
    cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];
    
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HPS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(hpscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gattc_read_blob_req_t readLongData;
        readLongData.handleOffset.attrHandle = hpscPtr->charInfo[charIndex].valueHandle;
        readLongData.handleOffset.offset     = 0u;
        readLongData.connHandle              = connHandle;
        
        cy_ble_hpscRdLongBuffLen = attrSize;
        cy_ble_hpscRdLongBuffPtr = attrValue;

        apiResult = Cy_BLE_GATTC_ReadLongCharacteristicValues(&readLongData);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_hpscReqHandle[discIdx] = hpscPtr->charInfo[charIndex].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  This function is used to write the characteristic descriptor to the server,
*  which is identified by charIndex and descrIndex.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hps_char_index_t.
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_hps_descr_index_t.*  \param attrSize:   The size of the characteristic value attribute.
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
*   If the HPS service-specific callback is registered
*   with \ref Cy_BLE_HPS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HPSC_WRITE_DESCR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, descrIndex etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_hps_char_value_t.
*   .
*   Otherwise (if an HPS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure ( \ref cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HPSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hps_char_index_t charIndex,
                                                               cy_en_ble_hps_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HPS client structure with attribute handles */
    cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];
    
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HPS_CHAR_COUNT) || (descrIndex >= CY_BLE_HPS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(hpscPtr->charInfo[charIndex].descrHandle[descrIndex] == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
         writeReqParam.handleValPair.attrHandle = hpscPtr->charInfo[charIndex].descrHandle[descrIndex];
         writeReqParam.handleValPair.value.val  = attrValue;
         writeReqParam.handleValPair.value.len  = attrSize;
         writeReqParam.connHandle               = connHandle;

        /* ... and send request to server's device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

        /* Save handle to support service-specific Write Response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_hpscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to get the characteristic descriptor of the specified
*  characteristic of the service.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hps_char_index_t.
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_hps_descr_index_t.*
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
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the HPS service-specific callback is registered
*   with \ref Cy_BLE_HPS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HPSC_READ_DESCR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     descrIndex, value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_hps_descr_value_t.
*   .
*   Otherwise (if an HPS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_HPSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hps_char_index_t charIndex,
                                                               cy_en_ble_hps_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HPS client structure with attribute handles */
    cy_stc_ble_hpsc_t *hpscPtr = (cy_stc_ble_hpsc_t *)&cy_ble_hpscConfigPtr->attrInfo[discIdx];
    
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HPS_CHAR_COUNT) || (descrIndex >= CY_BLE_HPS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(hpscPtr->charInfo[charIndex].descrHandle[descrIndex] == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = hpscPtr->charInfo[charIndex].descrHandle[descrIndex];
        readReqParam.connHandle = connHandle;
     
        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_hpscReqHandle[discIdx] = hpscPtr->charInfo[charIndex].descrHandle[descrIndex];
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HPS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the HPS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HPS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_HPSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_HPSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_HPSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_HPSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_HPSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the HPS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HPSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_HPSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_PREP_WRITE_REQ:
            Cy_BLE_HPSS_PrepareWriteRequestEventHandler((cy_stc_ble_gatts_prep_write_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_EXEC_WRITE_REQ:
            Cy_BLE_HPSS_ExecuteWriteRequestEventHandler((cy_stc_ble_gatts_exec_write_req_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_HPSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the HPS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HPSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_HPSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_HPSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_HPSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_HPSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
            break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_HPSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_HPSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_BLOB_RSP:
                Cy_BLE_HPSC_ReadLongRespEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_EXEC_WRITE_RSP:
                Cy_BLE_HPSC_ExecuteWriteResponseEventHandler((cy_stc_ble_gattc_exec_write_rsp_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_HPSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
