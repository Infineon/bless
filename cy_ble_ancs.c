/***************************************************************************//**
* \file cy_ble_ancs.c
* \version 3.40
*
* \brief
*  This file contains the source code for
*  the Apple Notification Center service.
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
* Global Variables
*******************************************************************************/
    
/* Apple Notification Center service UUID */
const cy_stc_ble_uuid128_t cy_ble_ancscServUuid =
{
    { 0xD0u, 0x00u, 0x2Du, 0x12u, 0x1Eu, 0x4Bu, 0x0Fu, 0xA4u, 0x99u, 0x4Eu, 0xCEu, 0xB5u, 0x31u, 0xF4u, 0x05u, 0x79u }
};

/* Apple Notification Center service characteristics UUIDs */
const cy_stc_ble_uuid128_t cy_ble_ancscCharUuid[CY_BLE_ANCS_CHAR_COUNT] =
{
    /* Notification Source characteristic UUID */
    { { 0xBDu, 0x1Du, 0xA2u, 0x99u, 0xE6u, 0x25u, 0x58u, 0x8Cu, 0xD9u, 0x42u, 0x01u, 0x63u, 0x0Du, 0x12u, 0xBFu, 0x9Fu } },
    /* Control Point characteristic UUID */
    { { 0xD9u, 0xD9u, 0xAAu, 0xFDu, 0xBDu, 0x9Bu, 0x21u, 0x98u, 0xA8u, 0x49u, 0xE1u, 0x45u, 0xF3u, 0xD8u, 0xD1u, 0x69u } },
    /* Data Source characteristic UUID */
    { { 0xFBu, 0x7Bu, 0x7Cu, 0xCEu, 0x6Au, 0xB3u, 0x44u, 0xBEu, 0xB5u, 0x4Bu, 0xD6u, 0x24u, 0xE9u, 0xC6u, 0xEAu, 0x22u } }
};

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_ANCS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_ANCSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_ANCSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_ancscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE ANCS server config structure */
const cy_stc_ble_ancss_config_t *cy_ble_ancssConfigPtr = NULL;

/* The pointer to a global BLE ANCS client config structure */
const cy_stc_ble_ancsc_config_t *cy_ble_ancscConfigPtr = NULL;

    
/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_ANCS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_ANCSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_ANCSC_EventHandler(uint32_t eventCode, void *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_ANCSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_ANCSC_DiscoverServiceEventHandler(const cy_stc_ble_disc_srv_info_t *discServInfo);
static void Cy_BLE_ANCSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_ANCSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_ANCSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_ANCSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_ANCSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_ANCSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_ANCSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/******************************************************************************
* Function Name: Cy_BLE_ANCSS_Init
***************************************************************************//**
*
*  This function initializes server of the Apple Notification Center service.
*
*  \param config: Configuration structure for the ANCS.
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
cy_en_ble_api_result_t Cy_BLE_ANCSS_Init(const cy_stc_ble_ancss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_ancssConfigPtr = config;

        /* Registers event handler for the ANCS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_ANCS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_ANCSS_EventHandlerCallback = &Cy_BLE_ANCSS_EventHandler;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSC_Init
***************************************************************************//**
*
*  This function initializes the client of the Apple Notification Center services.
*
*  \param config: Configuration structure for the ANCS.
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
cy_en_ble_api_result_t Cy_BLE_ANCSC_Init(const cy_stc_ble_ancsc_config_t *config)
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
        cy_ble_ancscConfigPtr = config;

        /* Registers event handler for the ANCS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_ANCS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_ANCSC_EventHandlerCallback = &Cy_BLE_ANCSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 ancsServIdx = cy_ble_ancscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + ancsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to ANCS client structure */
                cy_stc_ble_ancsc_t *ancscPtr = (cy_stc_ble_ancsc_t *)&cy_ble_ancscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(ancscPtr, 0, sizeof(cy_stc_ble_ancsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + ancsServIdx].uuid = 0x0000u;
            }

            cy_ble_ancscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANCS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for ANCS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_ANCSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_ancs_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_ANCS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_ANCS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets the value of the characteristic identified by charIndex.
*
*  \param charIndex: The index of the service characteristic.
*  \param attrSize:  The size of the characteristic value attribute.
*  \param attrValue: The pointer to the characteristic value data that should be
*                    stored to the GATT database.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE| An optional characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_ANCSS_SetCharacteristicValue(cy_en_ble_ancs_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check the parameters */
    if(charIndex >= CY_BLE_ANCS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE == cy_ble_ancssConfigPtr->attrInfo->charInfo[charIndex].charHandle)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Store the characteristic value into the GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_ancssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        
        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
        else
        {
            apiResult = CY_BLE_SUCCESS;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets the value of the characteristic, as identified by charIndex.
*
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue: The pointer to the location where characteristic
*              value data should be stored.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | A characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_ANCSS_GetCharacteristicValue(cy_en_ble_ancs_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check the parameters */
    if(charIndex >= CY_BLE_ANCS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE == cy_ble_ancssConfigPtr->attrInfo->charInfo[charIndex].charHandle)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Get characteristic value from GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_ancssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        
        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
        else
        {
            apiResult = CY_BLE_SUCCESS;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of the specified characteristic.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the characteristic.
*  \param descrIndex: The index of the descriptor.
*  \param attrSize:   The size of the descriptor value attribute.
*  \param attrValue:  The pointer to the location where characteristic
*                      descriptor value data should be stored.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | A descriptor is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_ANCSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_ancs_char_index_t charIndex,
                                                                cy_en_ble_ancs_descr_index_t descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check the parameters */
    if((charIndex >= CY_BLE_ANCS_CHAR_COUNT) || (descrIndex >= CY_BLE_ANCS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_ancssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[CY_BLE_ANCS_CCCD] ==
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Get data from the database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_ancssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex];
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.connHandle                 = connHandle;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
        else
        {
            apiResult = CY_BLE_SUCCESS;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSS_SendNotification
***************************************************************************//**
*
*  Sends a notification of the specified characteristic value, as identified by
*  the charIndex.
*  On enabling notification successfully for a service characteristic, this function sends out a
*  Handle Value notification that results in a #CY_BLE_EVT_ANCSC_NOTIFICATION event
*  at the GATT client's end.
*
*  \param connHandle: The connection handle that consists of the device ID and ATT
*                     connection ID.
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should
*                     be sent to the client device.
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
*   CY_BLE_ERROR_NTF_DISABLED                | Notification is not enabled by the client.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_ANCSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                     cy_en_ble_ancs_char_index_t charIndex,
                                                     uint8_t attrSize,
                                                     uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send notification if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_ANCS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_ancssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[CY_BLE_ANCS_CCCD] ==
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else if(!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_ancssConfigPtr->attrInfo->charInfo[charIndex].
                                             descrHandle[CY_BLE_ANCS_CCCD]))
    {
        apiResult = CY_BLE_ERROR_NTF_DISABLED;
    }
    else
    {
        cy_stc_ble_gatts_handle_value_ntf_t ntfParam;
        /* Fill all the fields of the Write Request structure ... */
        ntfParam.handleValPair.attrHandle = cy_ble_ancssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        ntfParam.handleValPair.value.val  = attrValue;
        ntfParam.handleValPair.value.len  = attrSize;
        ntfParam.connHandle               = connHandle;
       
        /* Send notification to the client using a previously filled structure */
        apiResult = Cy_BLE_GATTS_Notification(&ntfParam);
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSS_WriteEventHandler
***************************************************************************//**
*
*  Handles the Write Request event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
* \return
*  \ref cy_en_ble_gatt_err_code_t - A function result state if it succeeded
*  (CY_BLE_GATT_ERR_NONE) or failed with error codes:
*   * CY_BLE_GATTS_ERR_PROCEDURE_ALREADY_IN_PROGRESS;
*   * CY_BLE_GATTS_ERR_CCCD_IMPROPERLY_CONFIGURED.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_ANCSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(Cy_BLE_ANCS_ApplCallback != NULL)
    {
        cy_stc_ble_ancs_char_value_t locCharValue;
        locCharValue.charIndex = CY_BLE_ANCS_CHAR_COUNT;

        if(eventParam->handleValPair.attrHandle ==
           cy_ble_ancssConfigPtr->attrInfo->charInfo[CY_BLE_ANCS_NS].descrHandle[CY_BLE_ANCS_CCCD])
        {
            locCharValue.charIndex = CY_BLE_ANCS_NS;
        }
        else if(eventParam->handleValPair.attrHandle ==
                cy_ble_ancssConfigPtr->attrInfo->charInfo[CY_BLE_ANCS_DS].descrHandle[CY_BLE_ANCS_CCCD])
        {
            locCharValue.charIndex = CY_BLE_ANCS_DS;
        }
        else
        {
            /* Leave locCharValue.charIndex = CY_BLE_ANCS_CHAR_COUNT */
        }

        if(locCharValue.charIndex != CY_BLE_ANCS_CHAR_COUNT)
        {
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.handleValuePair = eventParam->handleValPair;
            dbAttrValInfo.connHandle      = eventParam->connHandle;
            dbAttrValInfo.offset          = 0u;
            dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;
            
            gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);

            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.value = NULL;
            if(gattErr == CY_BLE_GATT_ERR_NONE)
            {
                uint32_t eventCode;

                if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                {
                    eventCode = (uint32_t)CY_BLE_EVT_ANCSS_NOTIFICATION_ENABLED;
                }
                else
                {
                    eventCode = (uint32_t)CY_BLE_EVT_ANCSS_NOTIFICATION_DISABLED;
                }

                Cy_BLE_ANCS_ApplCallback(eventCode, &locCharValue);
            }
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else
        {
            if(eventParam->handleValPair.attrHandle == cy_ble_ancssConfigPtr->attrInfo->charInfo[CY_BLE_ANCS_CP].charHandle)
            {
                cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                dbAttrValInfo.connHandle      = eventParam->connHandle;
                dbAttrValInfo.offset          = 0u;
                dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;
                
                gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
                locCharValue.charIndex = CY_BLE_ANCS_CP;
                locCharValue.value = &eventParam->handleValPair.value;

                if(gattErr == CY_BLE_GATT_ERR_NONE)
                {
                    CY_BLE_GATT_DB_ATTR_SET_ATTR_GEN_LEN(cy_ble_ancssConfigPtr->attrInfo->charInfo[locCharValue.charIndex].
                                                          charHandle, locCharValue.value->len);

                    Cy_BLE_ANCS_ApplCallback((uint32_t)CY_BLE_EVT_ANCSS_WRITE_CHAR, &locCharValue);

                    if((locCharValue.gattErrorCode == CY_BLE_GATT_ERR_ANCS_UNKNOWN_COMMAND) ||
                       (locCharValue.gattErrorCode == CY_BLE_GATT_ERR_ANCS_INVALID_COMMAND) ||
                       (locCharValue.gattErrorCode == CY_BLE_GATT_ERR_ANCS_INVALID_PARAMETER) ||
                       (locCharValue.gattErrorCode == CY_BLE_GATT_ERR_ANCS_ACTION_FAILED))
                    {
                        gattErr = locCharValue.gattErrorCode;
                    }
                }

                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            }
        }
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (identified by
*  charIndex) value attribute in the server. As a result, a Write Request is
*  sent to the GATT server and on successful execution of the request on the
*  server's side, a #CY_BLE_EVT_ANCSS_WRITE_CHAR event is generated.
*  On successful request execution on the server's side, a Write Response is
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*   If execution is successful (return value = #CY_BLE_SUCCESS),
*   these events can appear: \n
*   If an ANCS service-specific callback is 
*    registered with \ref Cy_BLE_ANCS_RegisterAttrCallback():
*   * #CY_BLE_EVT_ANCSC_WRITE_CHAR_RESPONSE - If the requested attribute is
*     successfully written on the peer device, the details (charIndex, etc.)
*     are provided with an event  parameter structure 
*     of type \ref cy_stc_ble_ancs_char_value_t.
*   .
*   Otherwise (if an ANCS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is successfully 
*     written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the requested 
*     attribute on the peer device, the details are provided with an event
*     parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_ANCSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_ancs_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to ANCS client structure with attribute handles */
    cy_stc_ble_ancsc_t *ancscPtr = (cy_stc_ble_ancsc_t *)&cy_ble_ancscConfigPtr->attrInfo[discIdx];

    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_ANCS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(ancscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_WRITE & ancscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;  
        writeReqParam.handleValPair.attrHandle = ancscPtr->charInfo[charIndex].valueHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;
        
        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_ancscReqHandle[discIdx] = ancscPtr->charInfo[charIndex].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  This function is used to write the characteristic value to the server
*  identified by its charIndex.
*
*  Internally, a Write Request is sent to the GATT server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*   * #CY_BLE_EVT_ANCSS_NOTIFICATION_ENABLED
*   * #CY_BLE_EVT_ANCSS_NOTIFICATION_DISABLED
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic.
*  \param descrIndex: The index of the service characteristic descriptor.
*  \param attrSize:   The size of the characteristic descriptor value attribute.
*  \param attrValue:  The pointer to the characteristic descriptor value data that
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
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*   If execution is successful (return value = #CY_BLE_SUCCESS),
*   these events can appear: \n
*   If an ANCS service-specific callback is registered
*   with \ref Cy_BLE_ANCS_RegisterAttrCallback():
*   * #CY_BLE_EVT_ANCSC_WRITE_DESCR_RESPONSE - If the requested attribute is
*     successfully written on the peer device, the details (charIndex, 
*     descrIndex etc.) are provided with an event parameter structure
*     of type #cy_stc_ble_ancs_descr_value_t.
*   
*   Otherwise (if an ANCS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure ( \ref cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_ANCSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_ancs_char_index_t charIndex,
                                                                cy_en_ble_ancs_descr_index_t descrIndex,
                                                                uint8_t
                                                                attrSize,
                                                                uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    
    /* Get pointer (with offset) to ANCS client structure with attribute handles */
    cy_stc_ble_ancsc_t *ancscPtr = (cy_stc_ble_ancsc_t *)&cy_ble_ancscConfigPtr->attrInfo[discIdx];

    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_ANCS_CHAR_COUNT) || (descrIndex >= CY_BLE_ANCS_DESCR_COUNT) ||
            (attrSize != CY_BLE_CCCD_LEN)  || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(ancscPtr->charInfo[charIndex].descrHandle[descrIndex] == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Fill all fields of the Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle = ancscPtr->charInfo[charIndex].descrHandle[descrIndex];
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = CY_BLE_CCCD_LEN;
        writeReqParam.connHandle               = connHandle;
      
        /* ... and send request to the server device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            /* Save the handle to support service-specific read response from the device */
            cy_ble_ancscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets the characteristic descriptor of the specified characteristic.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic.
*  \param descrIndex: The index of the service characteristic descriptor.
*
* \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | This operation is not permitted on the specified attribute.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular descriptor.
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*   If execution is successful (return value = #CY_BLE_SUCCESS),
*   these following events can appear: \n
*   If an ANCS service-specific callback is registered
*   with \ref Cy_BLE_ANCS_RegisterAttrCallback():
*   * #CY_BLE_EVT_ANCSC_READ_DESCR_RESPONSE - If the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     descrIndex, value, etc.) are provided with an event parameter structure
*     of type #cy_stc_ble_ancs_descr_value_t.
*   
*   Otherwise (if an ANCS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_ANCSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_ancs_char_index_t charIndex,
                                                                cy_en_ble_ancs_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    
    /* Get pointer (with offset) to ANCS client structure with attribute handles */
    cy_stc_ble_ancsc_t *ancscPtr = (cy_stc_ble_ancsc_t *)&cy_ble_ancscConfigPtr->attrInfo[discIdx];

    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_ANCS_CHAR_COUNT) || (descrIndex >= CY_BLE_ANCS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(ancscPtr->charInfo[charIndex].descrHandle[descrIndex] == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = ancscPtr->charInfo[charIndex].descrHandle[descrIndex];
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_ancscReqHandle[discIdx] = ancscPtr->charInfo[charIndex].descrHandle[descrIndex];
        }
    }

    return(apiResult);
}

/******************************************************************************
* Function Name: Cy_BLE_ANCSC_DiscoverServiceEventHandler
***************************************************************************//**
*
*  This function is called on receiving a Read-By-Group Response event or
*  Read response with 128-bit service uuid.
*
*  \param discServInfo: The pointer to the service information structure.
*
******************************************************************************/
static void Cy_BLE_ANCSC_DiscoverServiceEventHandler(const cy_stc_ble_disc_srv_info_t *discServInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discServInfo->connHandle);
    uint32_t discServiNum = cy_ble_configPtr->context->discServiCount;
    uint32_t ancsDiscIdx = cy_ble_ancscConfigPtr->serviceDiscIdx;
    
    if(memcmp(&cy_ble_ancscServUuid, &discServInfo->srvcInfo->uuid.uuid128, CY_BLE_GATT_128_BIT_UUID_SIZE) == 0u)
    {
        
        if(cy_ble_configPtr->context->serverInfo[(discIdx * discServiNum) + ancsDiscIdx].range.startHandle == 
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            cy_ble_configPtr->context->serverInfo[(discIdx * discServiNum) + ancsDiscIdx].range = 
                discServInfo->srvcInfo->range;
            cy_ble_configPtr->context->discovery[discIdx].servCount++;
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}

/******************************************************************************
* Function Name: Cy_BLE_ANCSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, the appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to the characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_ANCSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    cy_en_ble_ancs_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t discServiNum = cy_ble_configPtr->context->discServiCount;
    uint32_t ancsDiscIdx = cy_ble_ancscConfigPtr->serviceDiscIdx;
    
    /* Get pointer (with offset) to ANCS client structure with attribute handles */
    cy_stc_ble_ancsc_t *ancscPtr = (cy_stc_ble_ancsc_t *)&cy_ble_ancscConfigPtr->attrInfo[discIdx];

    if((discCharInfo->uuidFormat == CY_BLE_GATT_128_BIT_UUID_FORMAT) &&
       (discCharInfo->valueHandle >= 
            cy_ble_configPtr->context->serverInfo[(discIdx * discServiNum) + ancsDiscIdx].range.startHandle) &&
       (discCharInfo->valueHandle <= 
            cy_ble_configPtr->context->serverInfo[(discIdx * discServiNum) + ancsDiscIdx].range.endHandle))
    {
        /* Update the last characteristic endHandle to the declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        for(locCharIndex = CY_BLE_ANCS_NS; locCharIndex < CY_BLE_ANCS_CHAR_COUNT; locCharIndex++)
        {
            if(memcmp(&cy_ble_ancscCharUuid[locCharIndex], &discCharInfo->uuid.uuid128,
                      CY_BLE_GATT_128_BIT_UUID_SIZE) == 0)
            {
                if(ancscPtr->charInfo[locCharIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    ancscPtr->charInfo[locCharIndex].valueHandle = discCharInfo->valueHandle;
                    ancscPtr->charInfo[locCharIndex].properties = discCharInfo->properties;
                    lastEndHandle[discIdx] = &ancscPtr->charInfo[locCharIndex].endHandle;
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
            *lastEndHandle[discIdx] = 
                cy_ble_configPtr->context->serverInfo[(discIdx * discServiNum) + ancsDiscIdx].range.endHandle;
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, the appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to the characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_ANCSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t notSupportedDescr = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t ancsDiscIdx = cy_ble_ancscConfigPtr->serviceDiscIdx;
    uint32_t descIdx;
    
    if(cy_ble_configPtr->context->discovery[discIdx].servCount == ancsDiscIdx)
    {
        switch(discDescrInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_CLIENT_CONFIG:
                descIdx = (uint32_t)CY_BLE_ANCS_CCCD;
                break;

            default:
                /* Not supported descriptor */
                notSupportedDescr = 1u;
                break;
        }

        if(notSupportedDescr == 0u)
        {
            /* Get pointer (with offset) to ANCS client structure with attribute handles */
            cy_stc_ble_ancsc_t *ancscPtr = (cy_stc_ble_ancsc_t *)&cy_ble_ancscConfigPtr->attrInfo[discIdx];

            if(ancscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] ==
               CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                ancscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] =
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
* Function Name: Cy_BLE_ANCSC_GetCharRange
***************************************************************************//**
*
*  Returns a possible range of the current characteristic descriptor via
*  input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_ANCSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t ancsDiscIdx = cy_ble_ancscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == ancsDiscIdx)
    {
        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_ANCS_CHAR_COUNT) && (exitFlag == 0u))
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
           
            /* Get pointer (with offset) to ANCS client structure with attribute handles */
            cy_stc_ble_ancsc_t *ancscPtr = (cy_stc_ble_ancsc_t *)&cy_ble_ancscConfigPtr->attrInfo[discIdx];

            if((ancscPtr->charInfo[charIdx].endHandle -
                ancscPtr->charInfo[charIdx].valueHandle) != 0u)
            {
                /* Read characteristic range */
                charRangeInfo->range.startHandle = ancscPtr->charInfo[charIdx].valueHandle + 1u;
                charRangeInfo->range.endHandle = ancscPtr->charInfo[charIdx].endHandle;
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
* Function Name: Cy_BLE_ANCSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles a notification event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_ANCSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_ANCS_ApplCallback != NULL))
    {
        cy_stc_ble_ancs_char_value_t locCharValue;
        /* Get pointer (with offset) to ANCS client structure with attribute handles */
        cy_stc_ble_ancsc_t *ancscPtr = (cy_stc_ble_ancsc_t *)&cy_ble_ancscConfigPtr->attrInfo[discIdx];

        if(ancscPtr->charInfo[CY_BLE_ANCS_NS].valueHandle == eventParam->handleValPair.attrHandle)
        {
            locCharValue.charIndex = CY_BLE_ANCS_NS;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else if(ancscPtr->charInfo[CY_BLE_ANCS_DS].valueHandle == eventParam->handleValPair.attrHandle)
        {
            locCharValue.charIndex = CY_BLE_ANCS_DS;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else
        {
            /* Apple Notification Center service doesn't support any other notification */
        }

        if((cy_ble_eventHandlerFlag & CY_BLE_CALLBACK) == 0u)
        {
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.value = &eventParam->handleValPair.value;
            Cy_BLE_ANCS_ApplCallback((uint32_t)CY_BLE_EVT_ANCSC_NOTIFICATION, &locCharValue);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles a read response event.
*
*  \param eventParam: The pointer to the data structure.
*
******************************************************************************/
static void Cy_BLE_ANCSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_ANCS_ApplCallback != NULL) &&
       (cy_ble_ancscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        cy_stc_ble_ancs_descr_value_t locDescrValue;
        
        /* Get pointer (with offset) to ANCS client structure with attribute handles */
        cy_stc_ble_ancsc_t *ancscPtr = (cy_stc_ble_ancsc_t *)&cy_ble_ancscConfigPtr->attrInfo[discIdx];

        if(ancscPtr->charInfo[CY_BLE_ANCS_NS].descrHandle[CY_BLE_ANCS_CCCD] == cy_ble_ancscReqHandle[discIdx])
        {
            locDescrValue.charIndex = CY_BLE_ANCS_NS;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else if(ancscPtr->charInfo[CY_BLE_ANCS_DS].descrHandle[CY_BLE_ANCS_CCCD] ==
                cy_ble_ancscReqHandle[discIdx])
        {
            locDescrValue.charIndex = CY_BLE_ANCS_DS;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else
        {
        }

        if((cy_ble_eventHandlerFlag & CY_BLE_CALLBACK) == 0u)
        {
            locDescrValue.connHandle = eventParam->connHandle;
            locDescrValue.value = &eventParam->value;

            Cy_BLE_ANCS_ApplCallback((uint32_t)CY_BLE_EVT_ANCSC_READ_DESCR_RESPONSE, &locDescrValue);
        }

        cy_ble_ancscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles a Write Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_ANCSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_ANCS_ApplCallback != NULL) &&
       (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_ancscReqHandle[discIdx]))
    {
        /* Get pointer (with offset) to ANCS client structure with attribute handles */
        cy_stc_ble_ancsc_t *ancscPtr = (cy_stc_ble_ancsc_t *)&cy_ble_ancscConfigPtr->attrInfo[discIdx];

    
        if(ancscPtr->charInfo[CY_BLE_ANCS_CP].valueHandle == cy_ble_ancscReqHandle[discIdx])
        {
            cy_stc_ble_ancs_char_value_t locCharIndex =
            {
                .connHandle = *eventParam,
                .charIndex  = CY_BLE_ANCS_CP,
                .value      = NULL
            };
            cy_ble_ancscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_ANCS_ApplCallback((uint32_t)CY_BLE_EVT_ANCSC_WRITE_CHAR_RESPONSE, &locCharIndex);
        }
        else
        {
            cy_stc_ble_ancs_descr_value_t locDescIndex;

            if(ancscPtr->charInfo[CY_BLE_ANCS_NS].descrHandle[CY_BLE_ANCS_CCCD] ==
               cy_ble_ancscReqHandle[discIdx])
            {
                locDescIndex.charIndex = CY_BLE_ANCS_NS;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            }
            else if(ancscPtr->charInfo[CY_BLE_ANCS_DS].descrHandle[CY_BLE_ANCS_CCCD] ==
                    cy_ble_ancscReqHandle[discIdx])
            {
                locDescIndex.charIndex = CY_BLE_ANCS_DS;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            }
            else
            {
                /* Apple Notification Center service doesn't support any other notification */
            }

            if((cy_ble_eventHandlerFlag & CY_BLE_CALLBACK) == 0u)
            {
                locDescIndex.connHandle = *eventParam;
                locDescIndex.descrIndex = CY_BLE_ANCS_CCCD;
                locDescIndex.value = NULL;

                cy_ble_ancscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                Cy_BLE_ANCS_ApplCallback((uint32_t)CY_BLE_EVT_ANCSC_WRITE_DESCR_RESPONSE, &locDescIndex);
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles an Error Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_ANCSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
        if(eventParam->errInfo.attrHandle == cy_ble_ancscReqHandle[discIdx])
        {
            /* Get pointer (with offset) to ANCS client structure with attribute handles */
            cy_stc_ble_ancsc_t *ancscPtr = (cy_stc_ble_ancsc_t *)&cy_ble_ancscConfigPtr->attrInfo[discIdx];

            if((Cy_BLE_ANCS_ApplCallback != NULL) &&
               (cy_ble_ancscReqHandle[discIdx] == ancscPtr->charInfo[CY_BLE_ANCS_CP].valueHandle))
            {
                cy_stc_ble_ancs_char_value_t locGattError =
                {
                    .connHandle    = eventParam->connHandle,
                    .charIndex     = CY_BLE_ANCS_CP,
                    .value         = NULL,
                    .gattErrorCode = eventParam->errInfo.errorCode
                };

                Cy_BLE_ANCS_ApplCallback((uint32_t)CY_BLE_EVT_ANCSC_ERROR_RESPONSE, &locGattError);
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            }

            cy_ble_ancscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_ANCS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the Apple Notification Center
*  service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type \ref cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_ANCS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_ANCSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_ANCSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_ANCSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_ANCSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the ANCS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type \ref cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_ANCSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_ANCSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
        break;

        default:
        break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_ANCSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the ANCS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type \ref cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_ANCSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
             /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_SERVICE:
                Cy_BLE_ANCSC_DiscoverServiceEventHandler((cy_stc_ble_disc_srv_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_ANCSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_ANCSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

                case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_ANCSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_ANCSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_ANCSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_ANCSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_ANCSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
