/***************************************************************************//**
* \file cy_ble_bas.c
* \version 3.60
*
* \brief
*  Contains the source code for the Battery service.
*
********************************************************************************
* \copyright
* Copyright 2017-2021, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "cy_ble_event_handler.h"

#if CY_BLE_LIB_HOST_CORE


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_BAS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_BASC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_BASS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_bascReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE BAS server config structure */
const cy_stc_ble_bass_config_t *cy_ble_bassConfigPtr = NULL;

/* The pointer to a global BLE BAS client config structure */
const cy_stc_ble_basc_config_t *cy_ble_bascConfigPtr = NULL;


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_BAS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_BASS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_BASC_EventHandler(uint32_t eventCode, void *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_BASS_WriteEventHandler(const cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_BASC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_BASC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_BASC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_BASC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_BASC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_BASC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_BASC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/******************************************************************************
* Function Name: Cy_BLE_BASS_Init
***************************************************************************//**
*
*  This function initializes the server of the Battery service.
*
*  \param config: Configuration structure for the BAS.
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
cy_en_ble_api_result_t Cy_BLE_BASS_Init(const cy_stc_ble_bass_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_bassConfigPtr = config;

        /* Registers event handler for the BAS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_BAS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_BASS_EventHandlerCallback = &Cy_BLE_BASS_EventHandler;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BASC_Init
***************************************************************************//**
*
*  This function initializes client of the Battery service.
*
*  \param config: Configuration structure for the BAS.
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
cy_en_ble_api_result_t Cy_BLE_BASC_Init(const cy_stc_ble_basc_config_t *config)
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
        cy_ble_bascConfigPtr = config;

        /* Registers event handler for the BAS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_BAS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_BASC_EventHandlerCallback = &Cy_BLE_BASC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32_t locServIdx;

            for(locServIdx = 0u; locServIdx < (uint32_t)cy_ble_bascConfigPtr->serviceCount; locServIdx++)
            {
                uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
                uint32 basServIdx = cy_ble_bascConfigPtr->serviceDiscIdx + locServIdx;


                if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + basServIdx].range.startHandle ==
                    CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    /* Get pointer to BAS client structure */
                    cy_stc_ble_basc_t *bascPtr = (cy_stc_ble_basc_t *)&cy_ble_bascConfigPtr->
                                                  attrInfo[(idx * cy_ble_bascConfigPtr->serviceCount) + locServIdx];

                    /* Clean client structure */
                    (void)memset(bascPtr, 0, sizeof(cy_stc_ble_basc_t));

                    /* Initialize uuid  */
                    cy_ble_configPtr->context->serverInfo[(idx * servCnt) + basServIdx].uuid = CY_BLE_UUID_BAS_SERVICE;
                }
            }
            cy_ble_bascReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BAS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for BAS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_BASS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_bas_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_BAS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_BAS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_BASS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a characteristic value of the service, which is a value
*  identified by charIndex, to the local database.
*
*  \param serviceIndex: The index of the service instance.
*
*  \param charIndex:    The index of the service characteristic of type
*                       #cy_en_ble_bas_char_index_t.
*
*  \param attrSize:     The size of the characteristic value attribute.
*                       A battery level characteristic has a 1-byte length.
*
*  \param attrValue:    The pointer to the characteristic value data that
*                       should be stored to the GATT database.
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
cy_en_ble_api_result_t Cy_BLE_BASS_SetCharacteristicValue(uint8_t serviceIndex,
                                                          cy_en_ble_bas_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((serviceIndex >= cy_ble_bassConfigPtr->serviceCount) || (charIndex >= CY_BLE_BAS_CHAR_COUNT) ||
       (attrSize != CY_BLE_BAS_BATTERY_LEVEL_LEN) || (*attrValue > CY_BLE_BAS_MAX_BATTERY_LEVEL_VALUE))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Store data in the database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_bassConfigPtr->attrInfo[serviceIndex].batteryLevelHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BASS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets a characteristic value of the Battery service, identified by charIndex.
*
*  \param serviceIndex: The index of the service instance; e.g. if two Battery
*                       services are supported in your design, then the first
*                       service will be identified by serviceIndex of 0 and the
*                       second service by serviceIndex of 1.
*
*  \param charIndex:    The index of a service characteristic of type
*                       \ref cy_en_ble_bas_char_index_t.
*
*  \param attrSize:     The size of the characteristic value attribute. A
*                       battery-level characteristic has a 1-byte length.
*
*  \param attrValue:    The pointer to the location where characteristic value
*                       data should be stored.
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
cy_en_ble_api_result_t Cy_BLE_BASS_GetCharacteristicValue(uint8_t serviceIndex,
                                                          cy_en_ble_bas_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((serviceIndex >= cy_ble_bassConfigPtr->serviceCount) || (charIndex >= CY_BLE_BAS_CHAR_COUNT) ||
       (attrSize != CY_BLE_BAS_BATTERY_LEVEL_LEN))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Read characteristic value from database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_bassConfigPtr->attrInfo[serviceIndex].batteryLevelHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BASS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of a specified characteristic of the
*  Battery service from the local GATT database.
*
*  \param connHandle:   The BLE peer device connection handle.
*
*  \param serviceIndex: The index of the service instance; e.g. if two Battery
*                       services are supported in your design, then the first
*                       service will be identified by serviceIndex of 0 and the
*                       second service by serviceIndex of 1.
*
*  \param charIndex:    The index of a service characteristic of type
*                       \ref cy_en_ble_bas_char_index_t.
*
*  \param descrIndex:   The index of a service characteristic descriptor of type
*                       \ref cy_en_ble_bas_descr_index_t.
*
*  \param attrSize:     The size of the characteristic descriptor attribute.
*
*  \param attrValue:    The pointer to the location where the characteristic
*                       descriptor value data should be stored.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BASS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               uint8_t serviceIndex,
                                                               cy_en_ble_bas_char_index_t charIndex,
                                                               cy_en_ble_bas_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((serviceIndex >= cy_ble_bassConfigPtr->serviceCount) || (charIndex >= CY_BLE_BAS_CHAR_COUNT) ||
       (descrIndex >= CY_BLE_BAS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Read value from database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.handleValuePair.value.len = attrSize;
        dbAttrValInfo.handleValuePair.value.val = attrValue;
        dbAttrValInfo.connHandle                = connHandle;
        dbAttrValInfo.offset                    = 0u;
        dbAttrValInfo.flags                     = CY_BLE_GATT_DB_LOCALLY_INITIATED;

        /* Get data from the database */
        if(descrIndex == CY_BLE_BAS_BATTERY_LEVEL_CCCD)
        {
            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_bassConfigPtr->attrInfo[serviceIndex].cccdHandle;
        }
        else
        {
            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_bassConfigPtr->attrInfo[serviceIndex].cpfdHandle;
        }

        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BASS_WriteEventHandler
***************************************************************************//**
*
*  Handles a Write Request event for the Battery service.
*
*  \param *eventParam: The pointer to the data structure specified by the event.
*
*  \return
*   A return value of type \ref cy_en_ble_gatt_err_code_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_GATT_ERR_NONE                     | A Write Request is handled successfully.
*   CY_BLE_GATT_ERR_REQUEST_NOT_SUPPORTED    | Notification isn't supported.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BASS_WriteEventHandler(const cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    cy_stc_ble_bas_char_value_t locCharIndex;
    uint8_t locServIndex = 0u;

    if(Cy_BLE_BAS_ApplCallback != NULL)
    {
        do
        {
            /* Client Characteristic Configuration descriptor Write Request */
            if(eventParam->handleValPair.attrHandle == cy_ble_bassConfigPtr->attrInfo[locServIndex].cccdHandle)
            {
                /* Verify that the optional notification property is enabled for the Battery Level characteristic */
                if(CY_BLE_IS_NOTIFICATION_SUPPORTED(cy_ble_bassConfigPtr->attrInfo[locServIndex].batteryLevelHandle))
                {
                    /* Fill GATT database attribute value parameters */
                    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

                    dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                    dbAttrValInfo.connHandle      = eventParam->connHandle;
                    dbAttrValInfo.offset          = 0u;
                    dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

                    gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
                    if(gattErr == CY_BLE_GATT_ERR_NONE)
                    {
                        locCharIndex.connHandle   = eventParam->connHandle;
                        locCharIndex.serviceIndex = locServIndex;
                        locCharIndex.charIndex    = CY_BLE_BAS_BATTERY_LEVEL;
                        locCharIndex.value        = NULL;

                        if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                        {
                            Cy_BLE_BAS_ApplCallback((uint32_t)CY_BLE_EVT_BASS_NOTIFICATION_ENABLED, &locCharIndex);
                        }
                        else
                        {
                            Cy_BLE_BAS_ApplCallback((uint32_t)CY_BLE_EVT_BASS_NOTIFICATION_DISABLED, &locCharIndex);
                        }
                    }
                }
                else
                {
                    gattErr = CY_BLE_GATT_ERR_REQUEST_NOT_SUPPORTED;
                }
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                break;
            }
            locServIndex++;
        }
        while(locServIndex < cy_ble_bassConfigPtr->serviceCount);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_BASS_SendNotification
***************************************************************************//**
*
*  This function updates the value of the Battery Level characteristic in the
*  GATT database. If the client has configured a notification on the Battery
*  Level characteristic, the function additionally sends this value using a
*  GATT Notification message.
*
*  On enabling notification successfully for a service characteristic, this function sends out
*  a Handle Value notification that results in a \ref CY_BLE_EVT_BASC_NOTIFICATION
*  event at the GATT client's end.
*
*  \param connHandle:   The BLE peer device connection handle
*
*  \param serviceIndex: The index of the service instance; e.g. if two Battery
*                       services are supported in your design, then the first
*                       service will be identified by serviceIndex of 0 and the
*                       second service by serviceIndex of 1.
*
*  \param charIndex:    The index of a service characteristic of type
*                       \ref cy_en_ble_bas_char_index_t.
*
*  \param attrSize:     The size of the characteristic value attribute. A Battery
*                       Level characteristic has a 1-byte length.
*
*  \param attrValue:    The pointer to the characteristic value data that should
*                       be sent to the client device.
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
cy_en_ble_api_result_t Cy_BLE_BASS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    uint8_t serviceIndex,
                                                    cy_en_ble_bas_char_index_t charIndex,
                                                    uint8_t attrSize,
                                                    uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Store new data in the database */
    apiResult = Cy_BLE_BASS_SetCharacteristicValue(serviceIndex, charIndex, attrSize, attrValue);

    if(apiResult == CY_BLE_SUCCESS)
    {
        /* Send notification if it is enabled and connected */
        if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
        {
            apiResult = CY_BLE_ERROR_INVALID_STATE;
        }
        else if((cy_ble_bassConfigPtr->attrInfo[serviceIndex].cccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE) ||
                (!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_bassConfigPtr->attrInfo[serviceIndex].cccdHandle)))
        {
            apiResult = CY_BLE_ERROR_NTF_DISABLED;
        }
        else
        {
            /* Fill all fields of the Write Request structure ... */
            cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
            ntfReqParam.handleValPair.attrHandle = cy_ble_bassConfigPtr->attrInfo[serviceIndex].batteryLevelHandle;
            ntfReqParam.handleValPair.value.val  = attrValue;
            ntfReqParam.handleValPair.value.len  = attrSize;
            ntfReqParam.connHandle               = connHandle;

            /* ... and send notification to client device. */
            apiResult = Cy_BLE_GATTS_Notification(&ntfReqParam);
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BASC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, the appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_BASC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t basDiscIdx = cy_ble_bascConfigPtr->serviceDiscIdx;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (discCharInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_BATTERY_LEVEL) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount >= basDiscIdx) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount <= ((basDiscIdx + cy_ble_bascConfigPtr->serviceCount) - 1u)))
    {
        /* The index of the service instance */
        uint32_t basServIdx = cy_ble_configPtr->context->discovery[discIdx].servCount -
                                cy_ble_bascConfigPtr->serviceDiscIdx;

        /* Get pointer (with offset) to BAS client structure with attribute handles */
        cy_stc_ble_basc_t *bascPtr = (cy_stc_ble_basc_t *)&cy_ble_bascConfigPtr->
                                     attrInfo[(discIdx * cy_ble_bascConfigPtr->serviceCount) + basServIdx];

        /* Check and store characteristic handle */
        Cy_BLE_CheckStoreCharHandle(bascPtr->batteryLevel);

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BASC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  Based on the descriptor UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discDescrInfo: The pointer to a descriptor information structure.
*
******************************************************************************/
static void Cy_BLE_BASC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t basDiscIdx = cy_ble_bascConfigPtr->serviceDiscIdx;

    if((cy_ble_configPtr->context->discovery[discIdx].servCount >= basDiscIdx) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount <= ((basDiscIdx + cy_ble_bascConfigPtr->serviceCount) - 1u)))
    {
        /* The index of the service instance */
        uint32_t basServIdx = cy_ble_configPtr->context->discovery[discIdx].servCount -
                                cy_ble_bascConfigPtr->serviceDiscIdx;

        /* Get pointer (with offset) to BAS client structure with attribute handles */
        cy_stc_ble_basc_t *bascPtr = (cy_stc_ble_basc_t *)&cy_ble_bascConfigPtr->
                                         attrInfo[(discIdx * cy_ble_bascConfigPtr->serviceCount) + basServIdx];


        if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_CLIENT_CONFIG)
        {
            Cy_BLE_CheckStoreCharDescrHandle(bascPtr->cccdHandle);
        }
        else if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_FORMAT)
        {
            Cy_BLE_CheckStoreCharDescrHandle(bascPtr->cpfdHandle);
        }
        else if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_REPORT_REFERENCE)
        {
            Cy_BLE_CheckStoreCharDescrHandle(bascPtr->rrdHandle);
        }
        else    /* BAS doesn't support other descriptors */
        {
            /* Empty else */
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BASC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_BASC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t basDiscIdx = cy_ble_bascConfigPtr->serviceDiscIdx;

    if((cy_ble_configPtr->context->discovery[discIdx].servCount >= basDiscIdx) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount <= ((basDiscIdx + cy_ble_bascConfigPtr->serviceCount) - 1u)))
    {
        uint32_t locServCount = cy_ble_configPtr->context->discovery[discIdx].servCount;
        uint32_t discServiNum = cy_ble_configPtr->context->discServiCount;

        /* The index of the service instance */
        uint32_t basServIdx = locServCount - basDiscIdx;

        /* Get pointer (with offset) to BAS client structure with attribute handles */
        cy_stc_ble_basc_t *bascPtr = (cy_stc_ble_basc_t *)&cy_ble_bascConfigPtr->
                                     attrInfo[(discIdx * cy_ble_bascConfigPtr->serviceCount) + basServIdx];

        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        if((cy_ble_configPtr->context->discovery[discIdx].charCount == 0u) &&
           (cy_ble_configPtr->context->serverInfo[(discIdx * discServiNum) + locServCount].range.endHandle !=
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
        {
            charRangeInfo->range.startHandle = bascPtr->batteryLevel.valueHandle + 1u;
            charRangeInfo->range.endHandle =
                cy_ble_configPtr->context->serverInfo[(discIdx * discServiNum) + locServCount].range.endHandle;
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}

/******************************************************************************
* Function Name: Cy_BLE_BASC_GetCharacteristicValue
***************************************************************************//**
*
*  This function is used to read the characteristic value from the server
*  identified by charIndex.
*
*  This function call can result in generation of the following events based on
*  a response from the server device:
*  * \ref CY_BLE_EVT_BASC_READ_CHAR_RESPONSE
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP
*
*  \param connHandle:   The BLE peer device connection handle.
*
*  \param serviceIndex: The index of the service instance; e.g. if two Battery
*                       services are supported in your design, then the first
*                       service will be identified by serviceIndex of 0 and the
*                       second service by serviceIndex of 1.
*
*  \param charIndex:    The index of a service characteristic of type
*                       \ref cy_en_ble_bas_char_index_t.
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
*   If execution is successful (return value = \ref CY_BLE_SUCCESS),
*   these events can appear: \n
*   If the BAS service-specific callback is registered
*      (with \ref Cy_BLE_BAS_RegisterAttrCallback):
*   * \ref CY_BLE_EVT_BASC_READ_CHAR_RESPONSE - If the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     value, etc.) are provided with an event parameter structure
*     of type \ref cy_stc_ble_bas_char_value_t.
*   
*   Otherwise (if a BAS service-specific callback is not registered):
*   * \ref CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*     successfully read on the peer device, the details (handle,
*     value, etc.) are provided with the event parameters structure
*     \ref cy_stc_ble_gattc_read_rsp_param_t.
*
*   * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided
*     with the event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BASC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          uint8_t serviceIndex,
                                                          cy_en_ble_bas_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to BAS client structure with attribute handles */
    cy_stc_ble_basc_t *bascPtr = (cy_stc_ble_basc_t *)&cy_ble_bascConfigPtr->
                                 attrInfo[(discIdx * cy_ble_bascConfigPtr->serviceCount) + serviceIndex];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((serviceIndex >= (uint32_t)cy_ble_bascConfigPtr->serviceCount) || (charIndex > CY_BLE_BAS_BATTERY_LEVEL) ||
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(bascPtr->batteryLevel.valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        /* Fill Read Request parameter */
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = bascPtr->batteryLevel.valueHandle;
        readReqParam.connHandle = connHandle;

        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

        /* ... and send a request to the server device. */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_bascReqHandle[discIdx] = bascPtr->batteryLevel.valueHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BASC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to set a characteristic descriptor of the specified Battery service
*  characteristic on the server device.
*
*  Internally, a Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*  * \ref CY_BLE_EVT_BASS_NOTIFICATION_ENABLED
*  * \ref CY_BLE_EVT_BASS_NOTIFICATION_DISABLED
*
*  \param connHandle:   The BLE peer device connection handle.
*
*  \param serviceIndex: The index of the service instance; e.g. if two Battery services
*                       are supported in your design, then the first service will be
*                       identified by serviceIndex of 0 and the second service by
*                       serviceIndex of 1.
*
*  \param charIndex:    The index of a service characteristic of type
*                       \ref cy_en_ble_bas_char_index_t.
*
*  \param descrIndex:   The index of a service characteristic descriptor of type
*                       \ref cy_en_ble_bas_descr_index_t.
*
*  \param attrSize:     The size of the characteristic descriptor attribute.
*
*  \param attrValue:    The pointer to the characteristic descriptor value data that should
*                       be sent to the server device.
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
*   If a BAS service-specific callback is registered
*      with \ref Cy_BLE_BAS_RegisterAttrCallback():
*   * #CY_BLE_EVT_BASC_WRITE_DESCR_RESPONSE - If the requested attribute is
*     successfully written on the peer device, the details (charIndex,
*     descrIndex, etc.) are provided with an event parameter structure
*     of type \ref cy_stc_ble_bas_descr_value_t.
*   
*   Otherwise (if a BAS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is successfully
*     written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the requested
*     attribute on the peer device, the details are provided with an event
*     parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BASC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               uint8_t serviceIndex,
                                                               cy_en_ble_bas_char_index_t charIndex,
                                                               cy_en_ble_bas_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to BAS client structure with attribute handles */
    cy_stc_ble_basc_t *bascPtr = (cy_stc_ble_basc_t *)&cy_ble_bascConfigPtr->
                                 attrInfo[(discIdx * cy_ble_bascConfigPtr->serviceCount) + serviceIndex];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((serviceIndex >= (uint32_t)cy_ble_bascConfigPtr->serviceCount) || (charIndex > CY_BLE_BAS_BATTERY_LEVEL) ||
            (descrIndex >= CY_BLE_BAS_DESCR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(descrIndex != CY_BLE_BAS_BATTERY_LEVEL_CCCD)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        /* Fill all the fields of the Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle = bascPtr->cccdHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;

        /* ... and send a request to the server device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

        /* Save the handle to support service-specific read response from the device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_bascReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BASC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to get a characteristic descriptor of the specified Battery service
*  characteristic from the server device. This function call can result in
*  generation of the following events based on a response from the server
*  device:
*  * \ref CY_BLE_EVT_BASC_READ_DESCR_RESPONSE
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP

*  \param connHandle:   The BLE peer device connection handle.
*
*  \param serviceIndex: The index of the service instance; e.g. if two Battery services
*                       are supported in your design, then the first service will be
*                       identified by serviceIndex of 0 and the second service by
*                       serviceIndex of 1.
*
*  \param charIndex:    The index of a service characteristic of type
*                       \ref cy_en_ble_bas_char_index_t.
*
*  \param descrIndex:   The index of a service characteristic descriptor of type
*                       \ref cy_en_ble_bas_descr_index_t.
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
*   If execution is successful (return value = \ref CY_BLE_SUCCESS),
*   these events can appear: \n
*   If a BAS service-specific callback is registered
*   (with \ref Cy_BLE_BAS_RegisterAttrCallback):
*   * \ref CY_BLE_EVT_BASC_READ_DESCR_RESPONSE - If the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     descrIndex, value, etc.) are provided with an event parameter structure
*     of type \ref cy_stc_ble_bas_descr_value_t.
*   .
*   Otherwise (if a BAS service-specific callback is not registered):
*   * \ref CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*     successfully read on the peer device, the details (handle,
*     value, etc.) are provided with the event parameters structure
*     \ref cy_stc_ble_gattc_read_rsp_param_t.
*   * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided
*     with the event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BASC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               uint8_t serviceIndex,
                                                               cy_en_ble_bas_char_index_t charIndex,
                                                               cy_en_ble_bas_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gattc_read_req_t readReqParam;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to BAS client structure with attribute handles */
    cy_stc_ble_basc_t *bascPtr = (cy_stc_ble_basc_t *)&cy_ble_bascConfigPtr->
                                 attrInfo[(discIdx * cy_ble_bascConfigPtr->serviceCount) + serviceIndex];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((serviceIndex >= cy_ble_bascConfigPtr->serviceCount) || (charIndex > CY_BLE_BAS_BATTERY_LEVEL) ||
            (descrIndex >= CY_BLE_BAS_DESCR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        if(descrIndex == CY_BLE_BAS_BATTERY_LEVEL_CCCD)
        {
            readReqParam.attrHandle = bascPtr->cccdHandle;
        }
        else /* CY_BLE_BAS_BATTERY_LEVEL_CPFD */
        {
            readReqParam.attrHandle = bascPtr->cpfdHandle;
        }
        readReqParam.connHandle = connHandle;

        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

        /* Save the handle to support a service-specific read response from the device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_bascReqHandle[discIdx] = readReqParam.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BASC_GetCharacteristicValueHandle
***************************************************************************//**
*
*  Returns the discovered peer device characteristic value handle.
*
*  \param connHandle:   The connection handle.
*  \param serviceIndex: The index of the service instance.
*  \param charIndex:    The index of a service characteristic.
*
* \return
*  Returns characteristic value handle of type cy_ble_gatt_db_attr_handle_t.
*  * CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE: when a peer device doesn't have
*                                           an optional characteristic
*
******************************************************************************/
cy_ble_gatt_db_attr_handle_t Cy_BLE_BASC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                      uint8_t serviceIndex,
                                                                      cy_en_ble_bas_char_index_t charIndex)
{
    cy_ble_gatt_db_attr_handle_t locAttrHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    uint32_t basServiceCount = cy_ble_bascConfigPtr->serviceCount;

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (charIndex < CY_BLE_BAS_CHAR_COUNT) &&
       (serviceIndex < basServiceCount))
    {
        /* Get pointer (with offset) to BAS client structure with attribute handles */
        cy_stc_ble_basc_t *bascPtr =
                (cy_stc_ble_basc_t *)&cy_ble_bascConfigPtr->attrInfo[(discIdx * basServiceCount) + serviceIndex];

        /* Get attribute handles */
        locAttrHandle = bascPtr->batteryLevel.valueHandle;
    }

    return (locAttrHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_BASC_GetCharacteristicDescriptorHandle
***************************************************************************//**
*
*  Returns the discovered peer device characteristic descriptor handle.
*
*  \param connHandle:   The connection handle.
*  \param serviceIndex: The index of the service instance.
*  \param charIndex:    The index of a service characteristic.
*  \param descrIndex:   The index of the descriptor.
*
* \return
*  Returns characteristic value handle of type cy_ble_gatt_db_attr_handle_t.
*  * CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE: when a peer device doesn't have an optional descriptor
*
******************************************************************************/
cy_ble_gatt_db_attr_handle_t Cy_BLE_BASC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                           uint8_t serviceIndex,
                                                                           cy_en_ble_bas_char_index_t charIndex,
                                                                           cy_en_ble_bas_descr_index_t descrIndex)
{
    cy_ble_gatt_db_attr_handle_t locAttrHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    uint32_t basServiceCount = cy_ble_bascConfigPtr->serviceCount;

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (charIndex < CY_BLE_BAS_CHAR_COUNT) &&
       (serviceIndex < basServiceCount) && (descrIndex < CY_BLE_BAS_DESCR_COUNT))
    {
        /* Get pointer (with offset) to BAS client structure with attribute handles */
        cy_stc_ble_basc_t *bascPtr =
                (cy_stc_ble_basc_t *)&cy_ble_bascConfigPtr->attrInfo[(discIdx * basServiceCount) + serviceIndex];

        /* Get attribute handles */
        locAttrHandle = (descrIndex == CY_BLE_BAS_BATTERY_LEVEL_CCCD) ?  bascPtr->cccdHandle : bascPtr->cpfdHandle;
    }

    return (locAttrHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_BASC_NotificationEventHandler
***************************************************************************//**
*
*  Handles a Notification event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BASC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
    uint32_t i;

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_BAS_ApplCallback != NULL))
    {
        for(i = 0u; i < cy_ble_bascConfigPtr->serviceCount; i++)
        {
            /* Get pointer (with offset) to BAS client structure with attribute handles */
            cy_stc_ble_basc_t *bascPtr = (cy_stc_ble_basc_t *)&cy_ble_bascConfigPtr->
                                         attrInfo[(discIdx * cy_ble_bascConfigPtr->serviceCount) + i];

            if(bascPtr->batteryLevel.valueHandle == eventParam->handleValPair.attrHandle)
            {
                cy_stc_ble_bas_char_value_t locCharValue;
                locCharValue.serviceIndex = (uint8_t)i;
                locCharValue.connHandle   = eventParam->connHandle;
                locCharValue.charIndex    = CY_BLE_BAS_BATTERY_LEVEL;
                locCharValue.value        = &eventParam->handleValPair.value;

                Cy_BLE_BAS_ApplCallback((uint32_t)CY_BLE_EVT_BASC_NOTIFICATION, &locCharValue);
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                break;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BASC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles a Read Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BASC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
    uint32_t locReqHandle = 0u;
    uint8_t locServIndex;

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_BAS_ApplCallback != NULL) &&
       (cy_ble_bascReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        for(locServIndex = 0u; (locServIndex < cy_ble_bascConfigPtr->serviceCount) && (locReqHandle == 0u); locServIndex++)
        {
            /* Get pointer (with offset) to BAS client structure with attribute handles */
            cy_stc_ble_basc_t *bascPtr = (cy_stc_ble_basc_t *)&cy_ble_bascConfigPtr->
                                         attrInfo[(discIdx * cy_ble_bascConfigPtr->serviceCount) + locServIndex];

            if(bascPtr->batteryLevel.valueHandle == cy_ble_bascReqHandle[discIdx])
            {
                cy_stc_ble_bas_char_value_t batteryLevelValue;
                batteryLevelValue.connHandle   = eventParam->connHandle;
                batteryLevelValue.serviceIndex = locServIndex;
                batteryLevelValue.charIndex    = CY_BLE_BAS_BATTERY_LEVEL;
                batteryLevelValue.value        = &eventParam->value;

                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                cy_ble_bascReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_BAS_ApplCallback((uint32_t)CY_BLE_EVT_BASC_READ_CHAR_RESPONSE, &batteryLevelValue);
                locReqHandle = 1u;
            }
            else if((bascPtr->cccdHandle == cy_ble_bascReqHandle[discIdx]) ||
                    (bascPtr->cpfdHandle == cy_ble_bascReqHandle[discIdx]))
            {
                cy_stc_ble_bas_descr_value_t locDescrValue;
                locDescrValue.connHandle   = eventParam->connHandle;
                locDescrValue.serviceIndex = locServIndex;
                locDescrValue.charIndex    = CY_BLE_BAS_BATTERY_LEVEL;
                locDescrValue.descrIndex   = ((bascPtr->cccdHandle == cy_ble_bascReqHandle[discIdx]) ?
                                                CY_BLE_BAS_BATTERY_LEVEL_CCCD : CY_BLE_BAS_BATTERY_LEVEL_CPFD);
                locDescrValue.value        = &eventParam->value;

                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                cy_ble_bascReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_BAS_ApplCallback((uint32_t)CY_BLE_EVT_BASC_READ_DESCR_RESPONSE, &locDescrValue);
                locReqHandle = 1u;
            }
            else /* Unsupported event code */
            {
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BASC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles a Write Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BASC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);
    uint8_t locServIndex;

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_BAS_ApplCallback != NULL) &&
       (cy_ble_bascReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        for(locServIndex = 0u; locServIndex < cy_ble_bascConfigPtr->serviceCount; locServIndex++)
        {
            /* Get pointer (with offset) to BAS client structure with attribute handles */
            cy_stc_ble_basc_t *bascPtr = (cy_stc_ble_basc_t *)&cy_ble_bascConfigPtr->
                                         attrInfo[(discIdx * cy_ble_bascConfigPtr->serviceCount) + locServIndex];

            if(bascPtr->cccdHandle == cy_ble_bascReqHandle[discIdx])
            {
                cy_stc_ble_bas_descr_value_t locDescIndex;
                locDescIndex.connHandle   = *eventParam;
                locDescIndex.serviceIndex = locServIndex;
                locDescIndex.charIndex    = CY_BLE_BAS_BATTERY_LEVEL;
                locDescIndex.descrIndex   = CY_BLE_BAS_BATTERY_LEVEL_CCCD;
                locDescIndex.value        = NULL;

                cy_ble_bascReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                Cy_BLE_BAS_ApplCallback((uint32_t)CY_BLE_EVT_BASC_WRITE_DESCR_RESPONSE, &locDescIndex);
                break;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BASC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles an Error Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BASC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        if(cy_ble_bascReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] == eventParam->errInfo.attrHandle)
        {
            cy_ble_bascReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BAS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the BAS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BAS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_BASS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_BASS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_BASC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_BASC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_BASS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the BAS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BASS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_BASS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_BASC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the BAS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BASC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_BASC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_BASC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_BASC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_BASC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_BASC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_BASC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_BASC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
                break;

            default:
                break;
        }
    }

    return(gattErr);
}

#endif /* CY_BLE_LIB_HOST_CORE */



/* [] END OF FILE */
