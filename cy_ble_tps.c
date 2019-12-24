/***************************************************************************//**
* \file cy_ble_tps.c
* \version 3.30
*
* \brief
*  This file contains the source code for the Tx Power service.
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

static cy_en_ble_gatt_err_code_t Cy_BLE_TPS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_TPSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_TPSC_EventHandler(uint32_t eventCode, void *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_TPSS_WriteEventHandler(const cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_TPSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_TPSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_TPSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_TPSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_TPSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_TPSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_TPSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_TPS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_TPSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_TPSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_tpscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE TPS server config structure */
const cy_stc_ble_tpss_config_t *cy_ble_tpssConfigPtr = NULL;

/* The pointer to a global BLE TPS client config structure */
const cy_stc_ble_tpsc_config_t *cy_ble_tpscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_TPSS_Init
***************************************************************************//**
*
*  This function initializes server of the Tx Power service.
*
*  \param config: Configuration structure for the TPS.
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
cy_en_ble_api_result_t Cy_BLE_TPSS_Init(const cy_stc_ble_tpss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_tpssConfigPtr = config;

        /* Registers event handler for the TPS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_TPS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_TPSS_EventHandlerCallback = &Cy_BLE_TPSS_EventHandler;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_Init
***************************************************************************//**
*
*  This function initializes client of the Tx Power service.
*
*  \param config: Configuration structure for the TPS.
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
cy_en_ble_api_result_t Cy_BLE_TPSC_Init(const cy_stc_ble_tpsc_config_t *config)
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
        cy_ble_tpscConfigPtr = config;

        /* Registers event handler for the TPS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_TPS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_TPSC_EventHandlerCallback = &Cy_BLE_TPSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 tpsServIdx = cy_ble_tpscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + tpsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to TPS client structure */
                cy_stc_ble_tpsc_t *tpscPtr = (cy_stc_ble_tpsc_t *)&cy_ble_tpscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(tpscPtr, 0, sizeof(cy_stc_ble_tpsc_t));

                /* Initialize uuid */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + tpsServIdx].uuid =
                    CY_BLE_UUID_TX_POWER_SERVICE;
            }

            cy_ble_tpscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_TPS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for TPS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback;
*         (e.g. #CY_BLE_EVT_TPSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_tps_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_TPS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_TPS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_TPSS_WriteEventHandler
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
static cy_en_ble_gatt_err_code_t Cy_BLE_TPSS_WriteEventHandler(const cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    uint32_t event = (uint32_t)CY_BLE_EVT_TPSS_NOTIFICATION_DISABLED;


    /* Check whether event code defines Write Request */
    if(NULL != Cy_BLE_TPS_ApplCallback)
    {
        /* Client Characteristic Configuration descriptor Write Request */
        if(eventParam->handleValPair.attrHandle == cy_ble_tpssConfigPtr->attrInfo->txPowerLevelCccdHandle)
        {
            /* Verify that optional notification property is enabled for Tx Power Level characteristic */
            if(CY_BLE_IS_NOTIFICATION_SUPPORTED(cy_ble_tpssConfigPtr->attrInfo->txPowerLevelCharHandle))
            {
                cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

                dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                dbAttrValInfo.connHandle      = eventParam->connHandle;
                dbAttrValInfo.offset          = 0u;
                dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;
                gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);

                if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                {
                    event = (uint32_t)CY_BLE_EVT_TPSS_NOTIFICATION_ENABLED;
                }

                if(gattErr == CY_BLE_GATT_ERR_NONE)
                {
                    cy_stc_ble_tps_char_value_t wrReqParam;
                    wrReqParam.connHandle = eventParam->connHandle;
                    wrReqParam.charIndex  = CY_BLE_TPS_TX_POWER_LEVEL;
                    wrReqParam.value      = NULL;

                    Cy_BLE_TPS_ApplCallback(event, (void*)&wrReqParam);
                }
            }
            else
            {
                gattErr = CY_BLE_GATT_ERR_REQUEST_NOT_SUPPORTED;
            }
            /* Indicate that request was handled */
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_TPSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets characteristic value of the Tx Power service, which is identified by
*  charIndex.
*
*  \param charIndex: The index of the service characteristic.
*
*  \param attrSize:  The size of the characteristic value attribute.
*
*  \param attrValue: The pointer to the characteristic value data that should be
*                    stored in the GATT database.
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                        | Description
*   ------------                       | -----------
*   CY_BLE_SUCCESS                     | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER     | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_TPSS_SetCharacteristicValue(cy_en_ble_tps_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          int8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if(charIndex < CY_BLE_TPS_CHAR_COUNT)
    {
        /* Set Tx Power Level characteristic value to GATT database. Need to handle return type difference of
         * Cy_BLE_GATTS_WriteAttributeValueCCCD() and cy_ble_tpssSetCharacteristicValue(). */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_tpssConfigPtr->attrInfo->txPowerLevelCharHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = (uint8_t*)attrValue;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

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
* Function Name: Cy_BLE_TPSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets characteristic value of the Tx Power service, which is identified by
*  charIndex.
*
*  \param charIndex: The index of the Tx Power characteristic.
*
*  \param attrSize:  The size of the Tx Power characteristic value attribute.
*
*  \param attrValue: The pointer to the location where Tx Power characteristic
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
cy_en_ble_api_result_t Cy_BLE_TPSS_GetCharacteristicValue(cy_en_ble_tps_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          int8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((attrValue != NULL) && (charIndex == CY_BLE_TPS_TX_POWER_LEVEL) && (attrSize == CY_BLE_TPS_TX_POWER_LEVEL_SIZE))
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_tpssConfigPtr->attrInfo->txPowerLevelCharHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = (uint8_t*)attrValue;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        dbAttrValInfo.offset                     = 0u;
        
        /* Get Tx Power Level characteristic value from GATT database */
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
* Function Name: Cy_BLE_TPSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets characteristic descriptor of specified characteristic of the Tx Power
*  service.
*
*  \param connHandle: The connection handle.
*
*  \param charIndex:  The index of the characteristic.
*
*  \param descrIndex: The index of the descriptor.
*
*  \param attrSize:   The size of the characteristic value attribute.
*
*  \param attrValue:  The pointer to the location where characteristic descriptor value
*                     data should be stored.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE| Optional descriptor is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_TPSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_tps_char_index_t charIndex,
                                                               cy_en_ble_tps_char_descriptors_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex == CY_BLE_TPS_TX_POWER_LEVEL) && (descrIndex == CY_BLE_TPS_CCCD))
    {
        if(CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_tpssConfigPtr->attrInfo->txPowerLevelCccdHandle)
        {
            /* Set Client Characteristic Configuration descriptor of Tx Power Level
             *  characteristic to GATT database.
             *  Need to handle return type difference of Cy_BLE_GATTS_WriteAttributeValueCCCD() and
             *  cy_ble_TpssGetCharacteristicDescriptor(). */

            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_tpssConfigPtr->attrInfo->txPowerLevelCccdHandle;
            dbAttrValInfo.handleValuePair.value.len  = attrSize;
            dbAttrValInfo.handleValuePair.value.val  = (uint8_t*)attrValue;
            dbAttrValInfo.connHandle                 = connHandle;
            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
            dbAttrValInfo.offset                     = 0u;

            if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
            {
                /* Indicate success */
                apiResult = CY_BLE_SUCCESS;
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
* Function Name: Cy_BLE_TPSS_SendNotification
***************************************************************************//**
*
*  Sends a notification with the characteristic value, as specified by charIndex,
*  to the client device.
*
*  On enabling notification successfully for a service characteristic it sends out a
*  'Handle Value Notification', which results in #CY_BLE_EVT_TPSC_NOTIFICATION event
*  at the GATT Client's end.
*
*  \param connHandle: The connection handle.
*
*  \param charIndex:  The index of the service characteristic.
*
*  \param attrSize:   The size of the characteristic value attribute.
*
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     sent to the client's device.
*
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
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_TPSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_tps_char_index_t charIndex,
                                                    uint8_t attrSize,
                                                    int8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((attrValue != NULL) && (charIndex == CY_BLE_TPS_TX_POWER_LEVEL) && (attrSize == CY_BLE_TPS_TX_POWER_LEVEL_SIZE))
    {
        if(CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_tpssConfigPtr->attrInfo->txPowerLevelCccdHandle))
        {
            /* Set Tx Power Level characteristic value to GATT database.
             *  Need to handle return type difference of Cy_BLE_GATTS_WriteAttributeValueCCCD() and
             *  cy_ble_tpssSetCharacteristicValue(). */
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_tpssConfigPtr->attrInfo->txPowerLevelCharHandle;
            dbAttrValInfo.handleValuePair.value.val  = (uint8_t*)attrValue;
            dbAttrValInfo.handleValuePair.value.len  = attrSize;
            dbAttrValInfo.connHandle                 = connHandle;
            dbAttrValInfo.offset                     = 0u;
            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

            if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
            {
                if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
                {
                    apiResult = CY_BLE_ERROR_INVALID_STATE;
                }
                else
                {
                    /* Fill all fields of Write Request structure ... */
                    cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
                    ntfReqParam.handleValPair.attrHandle = dbAttrValInfo.handleValuePair.attrHandle;
                    ntfReqParam.handleValPair.value.val  = dbAttrValInfo.handleValuePair.value.val;
                    ntfReqParam.handleValPair.value.len  = dbAttrValInfo.handleValuePair.value.len;
                    ntfReqParam.connHandle               = connHandle;
                    /* ... and send notification to client using previously filled structure */
                    apiResult = Cy_BLE_GATTS_Notification(&ntfReqParam);
                }
            }
        }
        else
        {
            apiResult = CY_BLE_ERROR_NTF_DISABLED;
        }
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP event.
*  Based on the service UUID, an appropriate data structure is populated using the
*  data received as part of the callback.
*
*  \param discCharInfo: The pointer to the characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_TPSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t tpsDiscIdx = cy_ble_tpscConfigPtr->serviceDiscIdx;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == tpsDiscIdx))
    {
        /* Get pointer (with offset) to TPS client structure with attribute handles */
        cy_stc_ble_tpsc_t *tpscPtr = (cy_stc_ble_tpsc_t *)&cy_ble_tpscConfigPtr->attrInfo[discIdx];

        if(discCharInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_TX_POWER_LEVEL)
        {
            /* Save Tx Power Level Characteristic handle */
            Cy_BLE_CheckStoreCharHandle(tpscPtr->txPowerLevelChar);
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  This event is generated when a server successfully sends the data for
*  #CY_BLE_EVT_GATTC_FIND_INFO_REQ. Based on the service UUID, an appropriate data
*  structure is populated to the service with a service callback.
*
*  \param  discDescrInfo: The pointer to descriptor information structure.
*
******************************************************************************/
static void Cy_BLE_TPSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t tpsDiscIdx = cy_ble_tpscConfigPtr->serviceDiscIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == tpsDiscIdx)
    {
        /* Get pointer (with offset) to TPS client structure with attribute handles */
        cy_stc_ble_tpsc_t *tpscPtr = (cy_stc_ble_tpsc_t *)&cy_ble_tpscConfigPtr->attrInfo[discIdx];

        if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_CLIENT_CONFIG)
        {
            Cy_BLE_CheckStoreCharDescrHandle(tpscPtr->txPowerLevelCccdHandle);
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_TPSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t tpsDiscIdx = cy_ble_tpscConfigPtr->serviceDiscIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == tpsDiscIdx)
    {
        /* Get pointer (with offset) to TPS client structure with attribute handles */
        cy_stc_ble_tpsc_t *tpscPtr = (cy_stc_ble_tpsc_t *)&cy_ble_tpscConfigPtr->attrInfo[discIdx];

        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        if(cy_ble_configPtr->context->discovery[discIdx].charCount == 0u)
        {
            uint32 servCnt = cy_ble_configPtr->context->discServiCount;

            /* One descriptor is available per characteristic */
            charRangeInfo->range.startHandle = tpscPtr->txPowerLevelChar.valueHandle + 1u;
            charRangeInfo->range.endHandle =
                cy_ble_configPtr->context->serverInfo[(discIdx * servCnt) + tpsDiscIdx].range.endHandle;
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles the Notification event for the Tx Power service.
*
*  \param *eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_TPSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_TPS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to TPS client structure with attribute handles */
        cy_stc_ble_tpsc_t *tpscPtr = (cy_stc_ble_tpsc_t *)&cy_ble_tpscConfigPtr->attrInfo[discIdx];

        if(tpscPtr->txPowerLevelChar.valueHandle == eventParam->handleValPair.attrHandle)
        {
            cy_stc_ble_tps_char_value_t ntfParam;
            ntfParam.connHandle = eventParam->connHandle;
            ntfParam.charIndex  = CY_BLE_TPS_TX_POWER_LEVEL;
            ntfParam.value      = &eventParam->handleValPair.value;

            Cy_BLE_TPS_ApplCallback((uint32_t)CY_BLE_EVT_TPSC_NOTIFICATION, (void*)&ntfParam);
            cy_ble_eventHandlerFlag &= (uint8_t)(~CY_BLE_CALLBACK);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles the Read Response event.
*
*  \param *eventParam: The pointer to the data that came with a read response
*                      for the Tx Power service.
*
******************************************************************************/
static void Cy_BLE_TPSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_TPS_ApplCallback != NULL) &&
       (cy_ble_tpscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {

        /* Get pointer (with offset) to TPS client structure with attribute handles */
        cy_stc_ble_tpsc_t *tpscPtr = (cy_stc_ble_tpsc_t *)&cy_ble_tpscConfigPtr->attrInfo[discIdx];

        if(tpscPtr->txPowerLevelChar.valueHandle == cy_ble_tpscReqHandle[discIdx])
        {
            /* Fill Tx Power service read response parameter structure for Tx Power Level
             * Characteristic. */
            cy_stc_ble_tps_char_value_t rdRspParam;
            rdRspParam.connHandle = eventParam->connHandle;
            rdRspParam.charIndex  = CY_BLE_TPS_TX_POWER_LEVEL;
            rdRspParam.value      = &eventParam->value;

            cy_ble_tpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_TPS_ApplCallback((uint32_t)CY_BLE_EVT_TPSC_READ_CHAR_RESPONSE, &rdRspParam);
        }
        else if(tpscPtr->txPowerLevelCccdHandle == cy_ble_tpscReqHandle[discIdx])
        {
            /* Fill Tx Power service read response parameter structure for Tx Power Level
             * Client Characteristic Configuration Descriptor. */
            cy_stc_ble_tps_descr_value_t rdRspParam;
            rdRspParam.connHandle = eventParam->connHandle;
            rdRspParam.charIndex  = CY_BLE_TPS_TX_POWER_LEVEL;
            rdRspParam.descrIndex = CY_BLE_TPS_CCCD;
            rdRspParam.value      = &eventParam->value;

            cy_ble_tpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_TPS_ApplCallback((uint32_t)CY_BLE_EVT_TPSC_READ_DESCR_RESPONSE, &rdRspParam);
        }
        else
        {
            /* No TPS characteristic was requested for read */
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles the Write Response event.
*
*  \param cy_stc_ble_conn_handle_t *eventParam: The pointer to a cy_stc_ble_conn_handle_t data structure.
*
*
******************************************************************************/
static void Cy_BLE_TPSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    /* Get pointer (with offset) to TPS client structure with attribute handles */
    cy_stc_ble_tpsc_t *tpscPtr = (cy_stc_ble_tpsc_t *)&cy_ble_tpscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_TPS_ApplCallback != NULL) &&
       (tpscPtr->txPowerLevelCccdHandle == cy_ble_tpscReqHandle[discIdx]) &&
       (cy_ble_tpscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        cy_stc_ble_tps_descr_value_t wrRspParam;
        wrRspParam.connHandle = *eventParam;
        wrRspParam.charIndex  = CY_BLE_TPS_TX_POWER_LEVEL;
        wrRspParam.descrIndex = CY_BLE_TPS_CCCD;
        wrRspParam.value      = NULL;

        cy_ble_tpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        Cy_BLE_TPS_ApplCallback((uint32_t)CY_BLE_EVT_TPSC_WRITE_DESCR_RESPONSE, &wrRspParam);
    }
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_GetCharacteristicValueHandle
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
cy_ble_gatt_db_attr_handle_t Cy_BLE_TPSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_tps_char_index_t charIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

    if( (Cy_BLE_GetDiscoveryIdx(connHandle) < cy_ble_configPtr->params->maxClientCount) &&
            (charIndex < CY_BLE_TPS_CHAR_COUNT))
    {
        /* Get pointer (with offset) to TPS client structure with attribute handles */
        cy_stc_ble_tpsc_t *tpscPtr =
                (cy_stc_ble_tpsc_t *)&cy_ble_tpscConfigPtr->attrInfo[Cy_BLE_GetDiscoveryIdx(connHandle)];

        returnHandle = tpscPtr->txPowerLevelChar.valueHandle;
    }
    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_GetCharacteristicDescriptorHandle
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
cy_ble_gatt_db_attr_handle_t Cy_BLE_TPSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_tps_char_index_t charIndex,
                                                                            cy_en_ble_tps_char_descriptors_t descrIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

    if((Cy_BLE_GetDiscoveryIdx(connHandle) < cy_ble_configPtr->params->maxClientCount) &&
           (charIndex < CY_BLE_TPS_CHAR_COUNT) && (descrIndex < CY_BLE_TPS_DESCR_COUNT))
    {
        /* Get pointer (with offset) to TPS client structure with attribute handles */
        cy_stc_ble_tpsc_t *tpscPtr =
                (cy_stc_ble_tpsc_t *)&cy_ble_tpscConfigPtr->attrInfo[Cy_BLE_GetDiscoveryIdx(connHandle)];

        returnHandle = tpscPtr->txPowerLevelCccdHandle;
    }

    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles the Error Response event for the Tx Power service.
*
*  \param *eventParam: The pointer to the cy_stc_ble_gatt_err_param_t structure.
*
******************************************************************************/
static void Cy_BLE_TPSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
        if(eventParam->errInfo.attrHandle != cy_ble_tpscReqHandle[discIdx])
        {
            cy_ble_tpscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_GetCharacteristicValue
***************************************************************************//**
*
*  Gets the characteristic value of the Tx Power service, which is
*  identified by charIndex.
*
*  This function call can result in generation of the following events based on
*  the response from the server device:
*  * #CY_BLE_EVT_TPSC_READ_CHAR_RESPONSE
*  * #CY_BLE_EVT_GATTC_ERROR_RSP
*
*  \param connHandle: The connection handle.
*
*  \param charIndex: The index of the characteristic.
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
*  If the TPS service-specific callback is registered
*      (with \ref Cy_BLE_TPS_RegisterAttrCallback):
*  * \ref CY_BLE_EVT_TPSC_READ_CHAR_RESPONSE - if the requested attribute is
*    successfully read on the peer device, the details (charIndex,
*    value, etc.) are provided with an event parameter structure
*    of type \ref cy_stc_ble_tps_char_value_t.
*  .
*  Otherwise (if a TPS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_TPSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_tps_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    if((charIndex == CY_BLE_TPS_TX_POWER_LEVEL) && (discIdx < cy_ble_configPtr->params->maxClientCount))
    {
        /* Get pointer (with offset) to TPS client structure with attribute handles */
        cy_stc_ble_tpsc_t *tpscPtr = (cy_stc_ble_tpsc_t *)&cy_ble_tpscConfigPtr->attrInfo[discIdx];

        /* Send request to write Tx Power Level characteristic value */
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = tpscPtr->txPowerLevelChar.valueHandle;
            readReqParam.connHandle = connHandle;

        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_tpscReqHandle[discIdx] = tpscPtr->txPowerLevelChar.valueHandle;
        }
    }
    else
    {
        /* apiResult equals CY_BLE_ERROR_INVALID_PARAMETER */
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sets a characteristic descriptor value of the Tx Power service.
*
*  Internally, Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*  * #CY_BLE_EVT_TPSS_NOTIFICATION_ENABLED
*  * #CY_BLE_EVT_TPSS_NOTIFICATION_DISABLED
*
*  \param connHandle: The connection handle.
*
*  \param charIndex:  The index of the Characteristic
*
*  \param descrIndex: The index of the TX Power service characteristic descriptor.
*
*  \param attrSize:   The size of the characteristic descriptor attribute.
*
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
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
*
* \events
*  If execution is successful (return value = #CY_BLE_SUCCESS),
*  these events can appear: \n
*   If a TPS service-specific callback is registered
*      with \ref Cy_BLE_TPS_RegisterAttrCallback():
*   * #CY_BLE_EVT_TPSC_WRITE_DESCR_RESPONSE - if the requested attribute is
*     successfully written on the peer device, the details (charIndex,
*     descrIndex etc.) are provided with an event parameter structure
*     of type \ref cy_stc_ble_tps_descr_value_t.
*   .
*   Otherwise (if a TPS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is successfully
*     written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the requested
*     attribute on the peer device, the details are provided with an event
*     parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_TPSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_tps_char_index_t charIndex,
                                                               cy_en_ble_tps_char_descriptors_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (attrValue != NULL) &&
       (charIndex == CY_BLE_TPS_TX_POWER_LEVEL) && (descrIndex == CY_BLE_TPS_CCCD) &&
       (attrSize == CY_BLE_CCCD_LEN))
    {
        /* Get pointer (with offset) to TPS client structure with attribute handles */
        cy_stc_ble_tpsc_t *tpscPtr = (cy_stc_ble_tpsc_t *)&cy_ble_tpscConfigPtr->attrInfo[discIdx];

        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle = tpscPtr->txPowerLevelCccdHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;

        /* ... and send request to server device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_tpscReqHandle[discIdx] = tpscPtr->txPowerLevelCccdHandle;
        }
    }
    else
    {
        /* apiResult equals CY_BLE_ERROR_INVALID_PARAMETER */
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of the Tx Power service.
*
*  This function call can result in generation of the following events based on
*  the response from the server device:
*  * #CY_BLE_EVT_TPSC_READ_DESCR_RESPONSE
*  * #CY_BLE_EVT_GATTC_ERROR_RSP
*
*  \param connHandle: The connection handle.
*
*  \param charIndex:  The index of the characteristic.
*
*  \param descrIndex: The index of the characteristic descriptor.
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
*
*  \events
*  If execution is successful (return value = \ref CY_BLE_SUCCESS),
*  these events can appear: \n
*  If a TPS service-specific callback is registered
*  (with \ref Cy_BLE_TPS_RegisterAttrCallback):
*  * \ref CY_BLE_EVT_TPSC_READ_DESCR_RESPONSE - if the requested attribute is
*    successfully read on the peer device, the details (charIndex,
*    descrIndex, value, etc.) are provided with an event parameter structure
*    of type \ref cy_stc_ble_tps_descr_value_t.
*  .
*  Otherwise (if a TPS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTC_READ_RSP - if the requested attribute is
*    successfully read on the peer device, the details (handle,
*    value, etc.) are provided with the event parameters structure
*    \ref cy_stc_ble_gattc_read_rsp_param_t.
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*    requested attribute on the peer device, the details are provided
*    with the event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_TPSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_tps_char_index_t charIndex,
                                                               cy_en_ble_tps_char_descriptors_t descrIndex)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((discIdx < cy_ble_configPtr->params->maxClientCount) && (charIndex == CY_BLE_TPS_TX_POWER_LEVEL) &&
            (descrIndex == CY_BLE_TPS_CCCD))
    {
        /* Get pointer (with offset) to TPS client structure with attribute handles */
        cy_stc_ble_tpsc_t *tpscPtr = (cy_stc_ble_tpsc_t *)&cy_ble_tpscConfigPtr->attrInfo[discIdx];

        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = tpscPtr->txPowerLevelCccdHandle;
        readReqParam.connHandle = connHandle;

        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_tpscReqHandle[discIdx] = tpscPtr->txPowerLevelCccdHandle;
        }
    }
    else
    {
        /* apiResult equals CY_BLE_ERROR_INVALID_PARAMETER */
    }

    /* Return status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_TPS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the TPS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_TPS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_TPSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_TPSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_TPSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_TPSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_TPSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the TPS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_TPSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_TPSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_TPSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the TPS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_TPSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_TPSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_TPSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_TPSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_TPSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_TPSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_TPSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_TPSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
