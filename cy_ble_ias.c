/***************************************************************************//**
* \file cy_ble_ias.c
* \version 3.50
*
* \brief
*  This file contains the source code for the Immediate Alert service.
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

static cy_en_ble_gatt_err_code_t Cy_BLE_IAS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_IASS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_IASC_EventHandler(uint32_t eventCode, void *eventParam);

static void Cy_BLE_IASS_DisconnectEventHandler(void);
static void Cy_BLE_IASS_WriteCmdEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_IASC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_IASC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_IAS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_IASC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_IASS_EventHandlerCallback = NULL;

/* The pointer to a global BLE IAS server config structure */
const cy_stc_ble_iass_config_t *cy_ble_iassConfigPtr = NULL;

/* The pointer to a global BLE IAS client config structure */
const cy_stc_ble_iasc_config_t *cy_ble_iascConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_IASS_Init
***************************************************************************//**
*
*  This function initializes server of the Immediate Alert service.
*
*  \param config: Configuration structure for the IAS.
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
cy_en_ble_api_result_t Cy_BLE_IASS_Init(const cy_stc_ble_iass_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_iassConfigPtr = config;

        /* Registers event handler for the IAS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_IAS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_IASS_EventHandlerCallback = &Cy_BLE_IASS_EventHandler;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_IASC_Init
***************************************************************************//**
*
*  This function initializes client of the Immediate Alert service.
*
*  \param config: Configuration structure for the IAS.
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
cy_en_ble_api_result_t Cy_BLE_IASC_Init(const cy_stc_ble_iasc_config_t *config)
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
        cy_ble_iascConfigPtr = config;

        /* Registers event handler for the IAS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_IAS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_IASC_EventHandlerCallback = &Cy_BLE_IASC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 iasServIdx = cy_ble_iascConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + iasServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to IAS client structure */
                cy_stc_ble_iasc_t *iascPtr = (cy_stc_ble_iasc_t *)&cy_ble_iascConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(iascPtr, 0, sizeof(cy_stc_ble_iasc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + iasServIdx].uuid =
                    CY_BLE_UUID_IMMEDIATE_ALERT_SERVICE;
            }
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_IAS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for IAS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_IASS_WRITE_CHAR_CMD).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_ias_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_IAS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_IAS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_IASS_DisconnectEventHandler
***************************************************************************//**
*
*  Handles the Disconnection Indication event for the Immediate Alert service.
*
******************************************************************************/
static void Cy_BLE_IASS_DisconnectEventHandler(void)
{
    uint8_t tmpAlertLevel = CY_BLE_NO_ALERT;

    /* Set alert level to "No Alert" per IAS spec */
    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

    dbAttrValInfo.connHandle.bdHandle        = 0u;
    dbAttrValInfo.connHandle.attId           = 0u;
    dbAttrValInfo.handleValuePair.attrHandle = cy_ble_iassConfigPtr->attrInfo->alertLevelCharHandle;
    dbAttrValInfo.handleValuePair.value.len  = CY_BLE_IAS_ALERT_LEVEL_SIZE;
    dbAttrValInfo.handleValuePair.value.val  = &tmpAlertLevel;
    dbAttrValInfo.offset                     = 0u;
    dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

    (void)Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
}


/******************************************************************************
* Function Name: Cy_BLE_IASS_WriteCmdEventHandler
***************************************************************************//**
*
*  Handles the Write Without Response Request event for the Immediate Alert
*  service.
*
*  \param eventParam: The pointer to a data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_IASS_WriteCmdEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    if(Cy_BLE_IAS_ApplCallback != NULL)
    {
        /* Check whether attribute handle is handle of Alert Level characteristic of
         * Immediate Alert service. */
        if((cy_ble_iassConfigPtr->attrInfo->alertLevelCharHandle == eventParam->handleValPair.attrHandle) &&
           (eventParam->handleValPair.value.len == CY_BLE_IAS_ALERT_LEVEL_SIZE) &&
           (eventParam->handleValPair.value.val[0u] <= CY_BLE_HIGH_ALERT))
        {
            /* Input parameters validation passed, so save Alert Level */
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.handleValuePair = eventParam->handleValPair;
            dbAttrValInfo.connHandle      = eventParam->connHandle;
            dbAttrValInfo.offset          = 0u;
            dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

            if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
            {
                cy_stc_ble_ias_char_value_t wrCmdParam;
                wrCmdParam.connHandle = eventParam->connHandle;
                wrCmdParam.charIndex  = CY_BLE_IAS_ALERT_LEVEL;
                wrCmdParam.value      = &eventParam->handleValPair.value;

                /* Send callback to user if no error occurred while writing Alert Level */
                Cy_BLE_IAS_ApplCallback((uint32_t)CY_BLE_EVT_IASS_WRITE_CHAR_CMD, (void*)&wrCmdParam);
                cy_ble_eventHandlerFlag &= (uint8_t)(~CY_BLE_CALLBACK);
            }
        }
        /* As this handler handles Write Without Response request the Error Response
         * can't be sent for the client. The erroneous value will be with
         * CY_BLE_EVT_GATTS_WRITE_CMD_REQ event. User will decide how to handle it. */
    }
}


/******************************************************************************
* Function Name: Cy_BLE_IASS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets the Alert Level characteristic value of the service, which is identified
*  by charIndex.
*
*  \param charIndex: The index of the Alert Level characteristic.
*
*  \param attrSize:  The size of the Alert Level characteristic value attribute.
*
*  \param attrValue: The pointer to the location where the Alert Level characteristic
*                    value data should be stored.
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
cy_en_ble_api_result_t Cy_BLE_IASS_GetCharacteristicValue(cy_en_ble_ias_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((attrValue != NULL) && (charIndex == CY_BLE_IAS_ALERT_LEVEL) && (attrSize == CY_BLE_IAS_ALERT_LEVEL_SIZE))
    {
        /* Get Alert Level characteristic value from GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_iassConfigPtr->attrInfo->alertLevelCharHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        dbAttrValInfo.offset                     = 0u;
        
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
* Function Name: Cy_BLE_IASC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP event.
*  Based on the service UUID, an appropriate data structure is populated using
*  the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_IASC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t iasDiscIdx = cy_ble_iascConfigPtr->serviceDiscIdx;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == iasDiscIdx))
    {
        /* Get pointer (with offset) to IAS client structure with attribute handles */
        cy_stc_ble_iasc_t *iascPtr = (cy_stc_ble_iasc_t *)&cy_ble_iascConfigPtr->attrInfo[discIdx];


        if(discCharInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_ALERT_LEVEL)
        {
            /* Save Alert Level Characteristic handle */
            Cy_BLE_CheckStoreCharHandle(iascPtr->alertLevelChar);
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_IASC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_IASC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t iasDiscIdx = cy_ble_iascConfigPtr->serviceDiscIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == iasDiscIdx)
    {
        /* IAS does not have any descriptions, return CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE to skip */
        charRangeInfo->range.startHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        charRangeInfo->range.endHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_IASC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result a Write Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the #CY_BLE_EVT_IASS_WRITE_CHAR_CMD event is generated.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the Alert Level service characteristic.
*  \param attrSize:   The size of the Alert Level characteristic value attribute.
*  \param attrValue:  The pointer to the Alert Level characteristic value data that
*                     should be sent to the server device.
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
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_IASC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_ias_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to IAS client structure with attribute handles */
    cy_stc_ble_iasc_t *iascPtr = (cy_stc_ble_iasc_t *)&cy_ble_iascConfigPtr->attrInfo[discIdx];


    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((discIdx < cy_ble_configPtr->params->maxClientCount) && (attrValue != NULL) &&
            (charIndex == CY_BLE_IAS_ALERT_LEVEL) && (attrSize == CY_BLE_IAS_ALERT_LEVEL_SIZE) &&
            (*attrValue <= CY_BLE_HIGH_ALERT) &&
            (iascPtr->alertLevelChar.valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Fill all fields of write command request structure ... */
        cy_stc_ble_gattc_write_cmd_req_t wrCmdReq;
        wrCmdReq.handleValPair.attrHandle = iascPtr->alertLevelChar.valueHandle;
        wrCmdReq.handleValPair.value.val  = attrValue;
        wrCmdReq.handleValPair.value.len  = attrSize;
        wrCmdReq.connHandle               = connHandle;

        /* ... and send request to write Alert Level characteristic value */
        apiResult = Cy_BLE_GATTC_WriteWithoutResponse(&wrCmdReq);
    }
    else
    {
        /* apiResult equals CY_BLE_ERROR_INVALID_PARAMETER */
    }

    /* Return status */
    return(apiResult);
}

/******************************************************************************
* Function Name: Cy_BLE_IAS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the IAS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_IAS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_IASS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_IASS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_IASC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_IASC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_IASS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the IAS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_IASS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_CMD_REQ:
            Cy_BLE_IASS_WriteCmdEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            Cy_BLE_IASS_DisconnectEventHandler();
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_IASC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the IAS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_IASC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_IASC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_IASC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
