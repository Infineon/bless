/***************************************************************************//**
* \file cy_ble_wpts.c
* \version 3.30
*
* \brief
*  Contains the source code for Wireless Power Transfer service.
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

static cy_en_ble_gatt_err_code_t Cy_BLE_WPTS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_WPTSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_WPTSC_EventHandler(uint32_t eventCode, void *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_WPTSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);
static void Cy_BLE_WPTSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam);

static void Cy_BLE_WPTSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_WPTSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_WPTSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_WPTSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_WPTSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam);
static void Cy_BLE_WPTSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_WPTSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_WPTSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_WPTS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_WPTSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_WPTSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response for server */
static cy_ble_gatt_db_attr_handle_t cy_ble_wptssReqHandle;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_wptscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE WPTS server config structure */
const cy_stc_ble_wptss_config_t *cy_ble_wptssConfigPtr = NULL;

/* The pointer to a global BLE WPTS client config structure */
const cy_stc_ble_wptsc_config_t *cy_ble_wptscConfigPtr = NULL;

/* WPTS characteristics 128-bit UUIDs */
const cy_stc_ble_uuid128_t cy_ble_wptscCharUuid128[CY_BLE_WPTS_CHAR_COUNT] =
{
    /* PRU Control Characteristic*/
    { { 0x67u, 0x9Au, 0x0Cu, 0x20u, 0x00u, 0x08u, 0x96u, 0x9Eu, 0xE2u, 0x11u, 0x46u, 0xA1u, 0x70u, 0xE6u, 0x55u, 0x64u } },
    /* PTU Static Parameter Characteristic*/
    { { 0x68u, 0x9Au, 0x0Cu, 0x20u, 0x00u, 0x08u, 0x96u, 0x9Eu, 0xE2u, 0x11u, 0x46u, 0xA1u, 0x70u, 0xE6u, 0x55u, 0x64u } },
    /* PRU Alert Characteristic*/
    { { 0x69u, 0x9Au, 0x0Cu, 0x20u, 0x00u, 0x08u, 0x96u, 0x9Eu, 0xE2u, 0x11u, 0x46u, 0xA1u, 0x70u, 0xE6u, 0x55u, 0x64u } },
    /* PRU Static Parameter Characteristic*/
    { { 0x6Au, 0x9Au, 0x0Cu, 0x20u, 0x00u, 0x08u, 0x96u, 0x9Eu, 0xE2u, 0x11u, 0x46u, 0xA1u, 0x70u, 0xE6u, 0x55u, 0x64u } },
    /* PRU Dynamic Parameter Characteristic*/
    { { 0x6Bu, 0x9Au, 0x0Cu, 0x20u, 0x00u, 0x08u, 0x96u, 0x9Eu, 0xE2u, 0x11u, 0x46u, 0xA1u, 0x70u, 0xE6u, 0x55u, 0x64u } }
};


/******************************************************************************
* Function Name: Cy_BLE_WPTSS_Init
***************************************************************************//**
*
*  This function initializes server for the Wireless Power Transfer service.
*
*  \param config: Configuration structure for the WPTS.
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
cy_en_ble_api_result_t Cy_BLE_WPTSS_Init(const cy_stc_ble_wptss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_wptssConfigPtr = config;

        /* Registers event handler for the WPTS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_WPTS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_WPTSS_EventHandlerCallback = &Cy_BLE_WPTSS_EventHandler;

        cy_ble_wptssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_Init
***************************************************************************//**
*
*  This function initializes client for the Wireless Power Transfer service.
*
*  \param config: Configuration structure for the WPTS.
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
cy_en_ble_api_result_t Cy_BLE_WPTSC_Init(const cy_stc_ble_wptsc_config_t *config)
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
        cy_ble_wptscConfigPtr = config;

        /* Registers event handler for the WPTS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_WPTS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_WPTSC_EventHandlerCallback = &Cy_BLE_WPTSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 wptsServIdx = cy_ble_wptscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + wptsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to WPTS client structure */
                cy_stc_ble_wptsc_t *wptscPtr = (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(wptscPtr, 0, sizeof(cy_stc_ble_wptsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + wptsServIdx].uuid =
                    CY_BLE_UUID_WIRELESS_POWER_TRANSFER_SERVICE;
            }

            cy_ble_wptscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for WPTS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback;
*         (e.g. #CY_BLE_EVT_WPTSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_wpts_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_WPTS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_WPTS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a characteristic value of the Wireless Power Transfer service in the
*  local GATT database. The characteristic is identified by charIndex.
*
*  \param charIndex: The index of a service characteristic of type
*                     cy_en_ble_wpts_char_index_t.
*  \param attrSize:  The size of the characteristic value attribute.
*  \param attrValue: The pointer to the characteristic value data that should be
*                     stored to the GATT database.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The characteristic value was written successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameters failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WPTSS_SetCharacteristicValue(cy_en_ble_wpts_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((attrValue != NULL) && (charIndex < CY_BLE_WPTS_CHAR_COUNT))
    {
        /* Fill the structure */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_wptssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
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
* Function Name: Cy_BLE_WPTSS_GetCharacteristicValue
***************************************************************************//**
*
*  Reads a characteristic value of the Wireless Power Transfer service, which
*  is identified by charIndex from the GATT database.
*
*  \param charIndex:       The index of a service characteristic of type
*                          cy_en_ble_wpts_char_index_t.
*  \param attrSize:        The size of the characteristic value attribute.
*  \param attrValue:       The pointer to the location where characteristic value data
*                          should be stored.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The characteristic value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameters failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WPTSS_GetCharacteristicValue(cy_en_ble_wpts_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((attrValue != NULL) && (charIndex < CY_BLE_WPTS_CHAR_COUNT))
    {
        /* Fill the structure */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_wptssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_WPTSS_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sets the characteristic descriptor of the specified characteristic.
*
*  \param connHandle:      The connection handle.
*  \param charIndex:       The index of a service characteristic of type
*                   cy_en_ble_wpts_char_index_t.
*  \param descrIndex:      The index of a service characteristic descriptor of type
*                   cy_en_ble_wpts_descr_index_t.
*  \param attrSize:        The size of the characteristic descriptor attribute.
*  \param attrValue:      The pointer to the descriptor value data that should
*                   be stored to the GATT database.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The characteristic value was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameters failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WPTSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_wpts_char_index_t charIndex,
                                                                cy_en_ble_wpts_descr_index_t descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex < CY_BLE_WPTS_CHAR_COUNT) && (descrIndex < CY_BLE_WPTS_DESCR_COUNT) && (attrValue != NULL))
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        

        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_wptssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex];
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.connHandle                 = connHandle;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

        /* Set characteristic value to database */
        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            /* Indicate success */
            apiResult = CY_BLE_SUCCESS;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Reads a characteristic descriptor of a specified characteristic of the
*  Wireless Power Transfer service from the GATT database.
*
*  \param connHandle:      The connection handle.
*  \param charIndex:       The index of a service characteristic of type
*                          cy_en_ble_wpts_char_index_t.
*  \param descrIndex:      The index of a service characteristic descriptor of type
*                          cy_en_ble_wpts_descr_index_t.
*  \param attrSize:        The size of the characteristic descriptor attribute.
*  \param attrValue:       The pointer to the location where characteristic descriptor
*                          value data should be stored.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameters failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WPTSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_wpts_char_index_t charIndex,
                                                                cy_en_ble_wpts_descr_index_t descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex < CY_BLE_WPTS_CHAR_COUNT) && (descrIndex < CY_BLE_WPTS_DESCR_COUNT) && (attrValue != NULL))
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_wptssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex];
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.connHandle                 = connHandle;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

        /* Get descriptor value from GATT database */
        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            /* Indicate success */
            apiResult = CY_BLE_SUCCESS;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSS_SendNotification
***************************************************************************//**
*
*  Sends notification with a characteristic value of the WPTS, which is a value
*  specified by charIndex, to the client device.
*
*  On enabling notification successfully for a service characteristic, sends out a
*  'Handle Value Notification', which results in #CY_BLE_EVT_WPTSC_NOTIFICATION event
*  at the GATT Client's end.
*
*  \param connHandle:      The connection handle
*  \param charIndex:       The index of a service characteristic of type
*                          cy_en_ble_wpts_char_index_t.
*  \param attrSize:        The size of the characteristic value attribute.
*  \param attrValue:       The pointer to the characteristic value data that should be
*                          sent to the client device.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | This operation is not permitted.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Optional characteristic is absent.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_NTF_DISABLED                | Notification is not enabled by the client.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WPTSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                     cy_en_ble_wpts_char_index_t charIndex,
                                                     uint8_t attrSize,
                                                     uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send Notification if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_WPTS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_wptssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((!CY_BLE_IS_NOTIFICATION_SUPPORTED(cy_ble_wptssConfigPtr->attrInfo->charInfo[charIndex].charHandle)) ||
            (!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_wptssConfigPtr->attrInfo->charInfo[charIndex].
                                              descrHandle[CY_BLE_WPTS_CCCD])))
    {
        apiResult = CY_BLE_ERROR_NTF_DISABLED;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
        
        ntfReqParam.handleValPair.attrHandle = cy_ble_wptssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        ntfReqParam.handleValPair.value.val  = attrValue;
        ntfReqParam.handleValPair.value.len  = attrSize;
        ntfReqParam.connHandle               = connHandle;
        
        /* Send notification to the client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Notification(&ntfReqParam);
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSS_SendIndication
***************************************************************************//**
*
*  Sends an indication with a characteristic value of the Wireless Power Transfer
*  service, which is a value specified by charIndex, to the client device.
*
*  On enabling indication successfully, sends out a 'Handle Value Indication', which
*  results in #CY_BLE_EVT_WPTSC_INDICATION or #CY_BLE_EVT_GATTC_HANDLE_VALUE_IND (if
*  service-specific callback function is not registered) event at the GATT Client's end.
*
*  \param connHandle:      The connection handle
*  \param charIndex:       The index of a service characteristic of type
*                   cy_en_ble_wpts_char_index_t.
*  \param attrSize:        The size of the characteristic value attribute.
*  \param attrValue:      The pointer to the characteristic value data that should be
*                   sent to the client device.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | This operation is not permitted.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Optional characteristic is absent.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_IND_DISABLED                | Indication is not enabled by the client.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the WPTS service-specific callback is registered
*      (with \ref Cy_BLE_WPTS_RegisterAttrCallback):
*  * #CY_BLE_EVT_WPTSS_INDICATION_CONFIRMED - If the indication is
*                                successfully delivered to the peer device.
*  .
*   Otherwise (if the WPTS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF - If the indication is
*                                successfully delivered to the peer device.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WPTSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                   cy_en_ble_wpts_char_index_t charIndex,
                                                   uint8_t attrSize,
                                                   uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send Notification if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_WPTS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_wptssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((!CY_BLE_IS_INDICATION_SUPPORTED(cy_ble_wptssConfigPtr->attrInfo->charInfo[charIndex].charHandle)) ||
            (!CY_BLE_IS_INDICATION_ENABLED(connHandle.attId, cy_ble_wptssConfigPtr->attrInfo->charInfo[charIndex].
                                            descrHandle[CY_BLE_WPTS_CCCD])))
    {
        apiResult = CY_BLE_ERROR_IND_DISABLED;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gatts_handle_value_ind_t indReqParam;
        
        indReqParam.handleValPair.attrHandle = cy_ble_wptssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        indReqParam.handleValPair.value.val  = attrValue;
        indReqParam.handleValPair.value.len  = attrSize;
        indReqParam.connHandle               = connHandle;
        
        /* Send indication to client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Indication(&indReqParam);
        /* Save handle to support service-specific value confirmation response from Client */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_wptssReqHandle = indReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSS_WriteEventHandler
***************************************************************************//**
*
*  Handles Write Request event for Wireless Power Transfer service.
*
*  \param void *eventParam: The pointer to the data structure specified by the event.
*
* \return
*  A return value of type cy_en_ble_gatt_err_code_t.
*
*   Error Codes                           | Description
*   ------------                          | -----------
*   CY_BLE_GATT_ERR_NONE                  | Write is successful.
*   CY_BLE_GATT_ERR_REQUEST_NOT_SUPPORTED | Request is not supported.
*   CY_BLE_GATT_ERR_INVALID_HANDLE        | 'handleValuePair.attrHandle' is not valid.
*   CY_BLE_GATT_ERR_WRITE_NOT_PERMITTED   | Write operation is not permitted on this attribute.
*   CY_BLE_GATT_ERR_INVALID_OFFSET        | Offset value is invalid.
*   CY_BLE_GATT_ERR_UNLIKELY_ERROR        | Some other error occurred.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_WPTSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_wpts_char_index_t locCharIndex;
    cy_stc_ble_wpts_char_value_t locCharValue = { .connHandle = eventParam->connHandle };
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    uint32_t locReqHandle = 0u;

    if(Cy_BLE_WPTS_ApplCallback != NULL)
    {
        for(locCharIndex = CY_BLE_WPTS_PRU_CONTROL; (locCharIndex < CY_BLE_WPTS_CHAR_COUNT) && (locReqHandle == 0u);
            locCharIndex++)
        {
            if((eventParam->handleValPair.attrHandle ==
                cy_ble_wptssConfigPtr->attrInfo->charInfo[locCharIndex].descrHandle[CY_BLE_WPTS_CCCD])
               || (eventParam->handleValPair.attrHandle ==
                   cy_ble_wptssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle))
            {
                /* Store value to database */
                cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                
                dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                dbAttrValInfo.connHandle      = eventParam->connHandle;
                dbAttrValInfo.offset          = 0u;
                dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;
                
                locCharValue.charIndex = locCharIndex;
                locCharValue.value = &eventParam->handleValPair.value;
                gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
                if(gattErr == CY_BLE_GATT_ERR_NONE)
                {
                    /* Characteristic value Write Request */
                    if(eventParam->handleValPair.attrHandle == cy_ble_wptssConfigPtr->attrInfo->charInfo[locCharIndex].
                        charHandle)
                    {
                        Cy_BLE_WPTS_ApplCallback((uint32_t)CY_BLE_EVT_WPTSS_WRITE_CHAR, &locCharValue);
                    }
                    else /* Client Characteristic Configuration descriptor Write Request */
                    {
                        /* Check characteristic properties for Notification */
                        if(CY_BLE_IS_NOTIFICATION_SUPPORTED(cy_ble_wptssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle))
                        {
                            uint32_t eventCode;
                            if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                            {
                                eventCode = (uint32_t)CY_BLE_EVT_WPTSS_NOTIFICATION_ENABLED;
                            }
                            else
                            {
                                eventCode = (uint32_t)CY_BLE_EVT_WPTSS_NOTIFICATION_DISABLED;
                            }
                            Cy_BLE_WPTS_ApplCallback(eventCode, &locCharValue);
                        }
                        /* Check characteristic properties for Indication */
                        if(CY_BLE_IS_INDICATION_SUPPORTED(cy_ble_wptssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle))
                        {
                            uint32_t eventCode;
                            if(CY_BLE_IS_INDICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                            {
                                eventCode = (uint32_t)CY_BLE_EVT_WPTSS_INDICATION_ENABLED;
                            }
                            else
                            {
                                eventCode = (uint32_t)CY_BLE_EVT_WPTSS_INDICATION_DISABLED;
                            }
                            Cy_BLE_WPTS_ApplCallback(eventCode, &locCharValue);
                        }
                    }
                }
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                locReqHandle = 1u;
            }
        }
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSS_ConfirmationEventHandler
***************************************************************************//**
*
*  Handles a Value Confirmation request event from the BLE Stack.
*
*  *eventParam - The pointer to a structure of type cy_stc_ble_conn_handle_t.
*
******************************************************************************/
static void Cy_BLE_WPTSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    cy_en_ble_wpts_char_index_t locCharIndex;

    uint32_t locReqHandle = 0u;

    if((Cy_BLE_WPTS_ApplCallback != NULL) && (cy_ble_wptssReqHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        for(locCharIndex = CY_BLE_WPTS_PRU_CONTROL; (locCharIndex < CY_BLE_WPTS_CHAR_COUNT) && (locReqHandle == 0u);
            locCharIndex++)
        {
            if(cy_ble_wptssReqHandle == cy_ble_wptssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle)
            {
                /* Fill in event data and inform application about successfully confirmed indication. */
                cy_stc_ble_wpts_char_value_t locCharValue;
                
                locCharValue.connHandle = *eventParam;
                locCharValue.charIndex  = locCharIndex;
                locCharValue.value      = NULL;
                
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                cy_ble_wptssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_WPTS_ApplCallback((uint32_t)CY_BLE_EVT_WPTSS_INDICATION_CONFIRMED, &locCharValue);
                locReqHandle = 1u;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_Discovery
***************************************************************************//**
*
*  This function discovers the PRU's WPT service and characteristics using the
*  GATT Primary service Handle, received through the WPT service Data
*  within the PRU advertisement payload, together with the handle offsets defined
*  A4WP specification.
*
*  The PTU may perform service discovery using the Cy_BLE_GATTC_StartDiscovery().
*  This function may be used in response to service Changed indication or to
*  discover services other than the WPT service supported by the PRU.
*
*  \param servHandle: GATT Primary service Handle of the WPT service.
*  \param connHandle: BLE peer device connection handle.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                           | Description
*   ------------                          | -----------
*   CY_BLE_SUCCESS                        | The request was handled successfully.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED | All client instances are used. 
*    
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WPTSC_Discovery(cy_ble_gatt_db_attr_handle_t servHandle,
                                              cy_stc_ble_conn_handle_t connHandle)
{
    uint32_t discIdx;
    cy_en_ble_api_result_t apiResult;
    
    apiResult = Cy_BLE_GATTC_AddConnHandle(connHandle);
    
    if(apiResult == CY_BLE_SUCCESS)
    {
        discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

        /* Get pointer (with offset) to WPS client structure with attribute handles */
        cy_stc_ble_wptsc_t *wptscPtr = (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[discIdx];

        wptscPtr->serviceHandle = servHandle;
        wptscPtr->charInfo[CY_BLE_WPTS_PRU_CONTROL].valueHandle =
            servHandle + CY_BLE_WPTS_PRU_CONTROL_CHAR_VALUE_OFFSET;
        wptscPtr->charInfo[CY_BLE_WPTS_PTU_STATIC_PAR].valueHandle =
            servHandle + CY_BLE_WPTS_PTU_STATIC_PAR_CHAR_VALUE_OFFSET;
        wptscPtr->charInfo[CY_BLE_WPTS_PRU_ALERT].valueHandle =
            servHandle + CY_BLE_WPTS_PRU_ALERT_PAR_CHAR_VALUE_OFFSET;
        wptscPtr->charInfo[CY_BLE_WPTS_PRU_ALERT].descrHandle[CY_BLE_WPTS_CCCD] =
            servHandle + CY_BLE_WPTS_PRU_ALERT_PAR_CHAR_CCCD_OFFSET;
        wptscPtr->charInfo[CY_BLE_WPTS_PRU_STATIC_PAR].valueHandle =
            servHandle + CY_BLE_WPTS_PRU_STATIC_PAR_CHAR_VALUE_OFFSET;
        wptscPtr->charInfo[CY_BLE_WPTS_PRU_DYNAMIC_PAR].valueHandle =
            servHandle + CY_BLE_WPTS_PRU_DYNAMIC_PAR_CHAR_VALUE_OFFSET;

        Cy_BLE_SetConnectionState(connHandle, CY_BLE_CONN_STATE_CLIENT_DISCOVERED);
    }
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_WPTSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t wptsDiscIdx = cy_ble_wptscConfigPtr->serviceDiscIdx;

    /* WPTS service has only characteristics with 128-bit UUIDs */
    if((discCharInfo->uuidFormat == CY_BLE_GATT_128_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == wptsDiscIdx))
    {
        /* Update last characteristic endHandle to declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        /* Compare common part of characteristics 128-bit UUID (except first byte) */
        if(memcmp(&cy_ble_wptscCharUuid128[CY_BLE_WPTS_PRU_CONTROL].value[1u],
                  &discCharInfo->uuid.uuid128.value[1u], CY_BLE_GATT_128_BIT_UUID_SIZE - 1u) == 0)
        {
            /* Get pointer (with offset) to WPS client structure with attribute handles */
            cy_stc_ble_wptsc_t *wptscPtr = (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[discIdx];

            /* Check the range of the first byte to be between the first and last characteristic */
            if((discCharInfo->uuid.uuid128.value[0u] >= cy_ble_wptscCharUuid128[CY_BLE_WPTS_PRU_CONTROL].value[0u]) &&
               (discCharInfo->uuid.uuid128.value[0u] <= cy_ble_wptscCharUuid128[CY_BLE_WPTS_CHAR_COUNT - 1u].value[0u]))
            {
                uint32_t locCharIndex;
                locCharIndex = (uint32_t)discCharInfo->uuid.uuid128.value[0u] -
                               cy_ble_wptscCharUuid128[CY_BLE_WPTS_PRU_CONTROL].value[0u];
                /* Verify that characteristic handler is not assigned already */
                if(wptscPtr->charInfo[locCharIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    wptscPtr->charInfo[locCharIndex].valueHandle = discCharInfo->valueHandle;
                    wptscPtr->charInfo[locCharIndex].properties = discCharInfo->properties;
                    lastEndHandle[discIdx] = &wptscPtr->charInfo[locCharIndex].endHandle;
                }
                else
                {
                    Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo);
                }
            }
        }

        /* Initialize characteristic endHandle to service endHandle. Characteristic endHandle
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
* Function Name: Cy_BLE_WPTSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  Based on the descriptor UUID, an appropriate data structure is populated using
*  the data received as part of the callback.
*
*  \param discDescrInfo: The pointer to a descriptor information structure.
*
******************************************************************************/
static void Cy_BLE_WPTSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t notSupportedDescr = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t wptsDiscIdx = cy_ble_wptscConfigPtr->serviceDiscIdx;
    uint32_t descIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == wptsDiscIdx)
    {
        /* Get pointer (with offset) to WPS client structure with attribute handles */
        cy_stc_ble_wptsc_t *wptscPtr = (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[discIdx];

        switch(discDescrInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_CLIENT_CONFIG:
                descIdx = (uint32_t)CY_BLE_WPTS_CCCD;
                break;

            default:
                /* Not supported descriptor */
                notSupportedDescr = 1u;
                break;
        }

        if(notSupportedDescr == 0u)
        {
            if(wptscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] ==
               CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                wptscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] =
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
* Function Name: Cy_BLE_WPTSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_WPTSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t wptsDiscIdx = cy_ble_wptscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == wptsDiscIdx)
    {    
        /* Get pointer (with offset) to WPS client structure with attribute handles */
        cy_stc_ble_wptsc_t *wptscPtr = (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[discIdx];

        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_WPTS_CHAR_COUNT) && (exitFlag == 0u))
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            if((wptscPtr->charInfo[charIdx].endHandle -
                wptscPtr->charInfo[charIdx].valueHandle) != 0u)
            {
                /* Read characteristic range */
                charRangeInfo->range.startHandle = wptscPtr->charInfo[charIdx].valueHandle + 1u;
                charRangeInfo->range.endHandle = wptscPtr->charInfo[charIdx].endHandle;
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
* Function Name: Cy_BLE_WPTSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles notification event for Wireless Power Transfer service.
*
*  \param eventParam:  The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_WPTSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    cy_en_ble_wpts_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_WPTS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to WPS client structure with attribute handles */
        cy_stc_ble_wptsc_t *wptscPtr = (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[discIdx];

        for(locCharIndex = CY_BLE_WPTS_PRU_CONTROL; locCharIndex < CY_BLE_WPTS_CHAR_COUNT; locCharIndex++)
        {
            if(wptscPtr->charInfo[locCharIndex].valueHandle == eventParam->handleValPair.attrHandle)
            {
                cy_stc_ble_wpts_char_value_t notifValue;
                
                notifValue.connHandle = eventParam->connHandle;
                notifValue.charIndex  = locCharIndex;
                notifValue.value      = &eventParam->handleValPair.value;

                Cy_BLE_WPTS_ApplCallback((uint32_t)CY_BLE_EVT_WPTSC_NOTIFICATION, &notifValue);
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                break;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_IndicationEventHandler
***************************************************************************//**
*
*  Handles an indication event for Wireless Power Transfer service.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_WPTSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam)
{
    cy_en_ble_wpts_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_WPTS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to WPS client structure with attribute handles */
        cy_stc_ble_wptsc_t *wptscPtr = (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[discIdx];

        for(locCharIndex = CY_BLE_WPTS_PRU_CONTROL; locCharIndex < CY_BLE_WPTS_CHAR_COUNT; locCharIndex++)
        {
            if(wptscPtr->charInfo[locCharIndex].valueHandle == eventParam->handleValPair.attrHandle)
            {
                cy_stc_ble_wpts_char_value_t indicationValue;
                
                indicationValue.connHandle = eventParam->connHandle;
                indicationValue.charIndex  = locCharIndex;
                indicationValue.value      = &eventParam->handleValPair.value;
                
                Cy_BLE_WPTS_ApplCallback((uint32_t)CY_BLE_EVT_WPTSC_INDICATION, &indicationValue);
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                break;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles a Read Response event for the Wireless Power Transfer service.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_WPTSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t locReqHandle = 0u;
    cy_en_ble_wpts_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_WPTS_ApplCallback != NULL) &&
       (cy_ble_wptscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to WPS client structure with attribute handles */
        cy_stc_ble_wptsc_t *wptscPtr = (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[discIdx];

        for(locCharIndex = CY_BLE_WPTS_PRU_CONTROL; (locCharIndex < CY_BLE_WPTS_CHAR_COUNT) && (locReqHandle == 0u);
            locCharIndex++)
        {
            if(cy_ble_wptscReqHandle[discIdx] == wptscPtr->charInfo[locCharIndex].valueHandle)
            {
                cy_stc_ble_wpts_char_value_t locCharValue;
                
                locCharValue.connHandle = eventParam->connHandle;
                locCharValue.charIndex  = locCharIndex;
                locCharValue.value      = &eventParam->value;
                
                cy_ble_wptscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_WPTS_ApplCallback((uint32_t)CY_BLE_EVT_WPTSC_READ_CHAR_RESPONSE, &locCharValue);
                locReqHandle = 1u;
            }
            else if(cy_ble_wptscReqHandle[discIdx] == wptscPtr->charInfo[locCharIndex].
                     descrHandle[CY_BLE_WPTS_CCCD])
            {
                cy_stc_ble_wpts_descr_value_t locDescrValue;
                
                locDescrValue.connHandle = eventParam->connHandle;
                locDescrValue.charIndex  = locCharIndex;
                locDescrValue.descrIndex = CY_BLE_WPTS_CCCD;
                locDescrValue.value      = &eventParam->value;
                
                cy_ble_wptscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_WPTS_ApplCallback((uint32_t)CY_BLE_EVT_WPTSC_READ_DESCR_RESPONSE, &locDescrValue);
                locReqHandle = 1u;
            }
            else /* No more descriptors available */
            {
            }
        }
        if(locReqHandle != 0u)
        {
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles a Write Response event for the Wireless Power Transfer service.
*
*  \param eventParam: The pointer to a data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_WPTSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t locReqHandle = 0u;
    cy_en_ble_wpts_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    /* Check whether service handler was registered and request handle is valid */
    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_WPTS_ApplCallback != NULL) &&
       (cy_ble_wptscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to WPS client structure with attribute handles */
        cy_stc_ble_wptsc_t *wptscPtr = (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[discIdx];

        for(locCharIndex = CY_BLE_WPTS_PRU_CONTROL; (locCharIndex < CY_BLE_WPTS_CHAR_COUNT) && (locReqHandle == 0u);
            locCharIndex++)
        {
            if(cy_ble_wptscReqHandle[discIdx] == wptscPtr->charInfo[locCharIndex].valueHandle)
            {
                cy_stc_ble_wpts_char_value_t locCharValue;
                
                locCharValue.connHandle = *eventParam;
                locCharValue.charIndex  = locCharIndex;
                locCharValue.value      = NULL;
                
                cy_ble_wptscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_WPTS_ApplCallback((uint32_t)CY_BLE_EVT_WPTSC_WRITE_CHAR_RESPONSE, &locCharValue);
                locReqHandle = 1u;
            }
            else if(cy_ble_wptscReqHandle[discIdx] == wptscPtr->charInfo[locCharIndex].
                     descrHandle[CY_BLE_WPTS_CCCD])
            {
                cy_stc_ble_wpts_descr_value_t locDescrValue;
                
                locDescrValue.connHandle = *eventParam;
                locDescrValue.charIndex  = locCharIndex;
                locDescrValue.descrIndex = CY_BLE_WPTS_CCCD;
                locDescrValue.value      = NULL;
                
                cy_ble_wptscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_WPTS_ApplCallback((uint32_t)CY_BLE_EVT_WPTSC_WRITE_DESCR_RESPONSE, &locDescrValue);
                locReqHandle = 1u;
            }
            else /* No more descriptors available */
            {
            }
        }
        if(locReqHandle != 0u)
        {
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles an Error Response event for the Wireless Power Transfer service.
*
*  \param eventParam: The pointer to a data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_WPTSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
        if(eventParam->errInfo.attrHandle == cy_ble_wptscReqHandle[discIdx])
        {
            cy_ble_wptscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result, a Write Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the #CY_BLE_EVT_WPTSS_WRITE_CHAR event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle:      The connection handle.
*  \param charIndex:       The index of a service characteristic of type
*                   cy_en_ble_wpts_char_index_t.
*  \param attrSize:        The size of the characteristic value attribute.
*  \param attrValue:      The pointer to the characteristic value data that
*                   should be send to the server device.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameters failed.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the WPTS service-specific callback is registered
*      (with \ref Cy_BLE_WPTS_RegisterAttrCallback):
*  * #CY_BLE_EVT_WPTSC_WRITE_CHAR_RESPONSE - If the requested attribute is
*                                successfully written on the peer device,
*                                the details (charIndex, etc.) are
*                                provided with event parameter structure
*                                of type cy_stc_ble_wpts_char_value_t.
*  .
*   Otherwise (if the WPTS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*                                successfully written on the peer device.
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*                                requested attribute on the peer device,
*                                the details are provided with event parameters
*                                structure (cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WPTSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_wpts_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to WPS client structure with attribute handles */
    cy_stc_ble_wptsc_t *wptscPtr = (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_WPTS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(wptscPtr->charInfo[charIndex].valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;
        
        writeReqParam.handleValPair.attrHandle = wptscPtr->charInfo[charIndex].valueHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;
        
        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);
        /* Save handle to support service-specific Write Response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_wptscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_GetCharacteristicValue
***************************************************************************//**
*
*  This function is used to read a characteristic value, which is a value
*  identified by charIndex, from the server.
*
*  \param connHandle:      The connection handle.
*  \param charIndex:       The index of a service characteristic of type
*                   cy_en_ble_wpts_char_index_t.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameters failed.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the WPTS service-specific callback is registered
*      (with \ref Cy_BLE_WPTS_RegisterAttrCallback):
*  * #CY_BLE_EVT_WPTSC_READ_CHAR_RESPONSE - If the requested attribute is
*                                successfully read on the peer device,
*                                the details (charIndex , value, etc.) are
*                                provided with event parameter structure
*                                of type cy_stc_ble_wpts_char_value_t.
*  .
*   Otherwise (if the WPTS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*                                successfully read on the peer device,
*                                the details (handle, value, etc.) are
*                                provided with event parameters
*                                structure (cy_stc_ble_gattc_read_rsp_param_t).
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*                                requested attribute on the peer device,
*                                the details are provided with event parameters
*                                structure (cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WPTSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_wpts_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to WPS client structure with attribute handles */
    cy_stc_ble_wptsc_t *wptscPtr = (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_WPTS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(wptscPtr->charInfo[charIndex].valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = wptscPtr->charInfo[charIndex].valueHandle;
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_wptscReqHandle[discIdx] = wptscPtr->charInfo[charIndex].valueHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  This function is used to write the characteristic descriptor to the server,
*  which is identified by charIndex and descrIndex.
*
*  Internally, Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*  * #CY_BLE_EVT_WPTSS_NOTIFICATION_ENABLED
*  * #CY_BLE_EVT_WPTSS_NOTIFICATION_DISABLED
*  * #CY_BLE_EVT_WPTSS_INDICATION_ENABLED
*  * #CY_BLE_EVT_WPTSS_INDICATION_DISABLED
*
*  \param connHandle:      The connection handle.
*  \param charIndex:       The index of a service characteristic of type
*                   cy_en_ble_wpts_char_index_t.
*  \param descrIndex:      The index of a service characteristic descriptor of type
*                   cy_en_ble_wpts_descr_index_t.
*  \param attrSize:        The size of the characteristic value attribute.
*  \param attrValue:      The pointer to the characteristic descriptor value data that
*                   should be sent to the server device.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameters failed.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is not permitted on the specified attribute.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the WPTS service-specific callback is registered
*      (with \ref Cy_BLE_WPTS_RegisterAttrCallback):
*  * #CY_BLE_EVT_WPTSC_WRITE_DESCR_RESPONSE - If the requested attribute is
*                                successfully written on the peer device,
*                                the details (charIndex, descrIndex etc.) are
*                                provided with event parameter structure
*                                of type cy_stc_ble_wpts_descr_value_t.
*  .
*   Otherwise (if the WPTS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*                                successfully written on the peer device.
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*                                requested attribute on the peer device,
*                                the details are provided with event parameters
*                                structure (cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WPTSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_wpts_char_index_t charIndex,
                                                                cy_en_ble_wpts_descr_index_t descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_WPTS_CHAR_COUNT) || (descrIndex >= CY_BLE_WPTS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Get pointer (with offset) to WPS client structure with attribute handles */
        cy_stc_ble_wptsc_t *wptscPtr = (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[discIdx];

        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        
        writeReqParam.handleValPair.attrHandle = wptscPtr->charInfo[charIndex].descrHandle[descrIndex];
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;
        
        /* ... and send request to server's device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_wptscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to get the characteristic descriptor of the specified
*  characteristic of the service.
*
*  \param connHandle:      The connection handle.
*  \param charIndex:       The index of a service characteristic of type
*                   cy_en_ble_wpts_char_index_t.
*  \param descrIndex:      The index of a service characteristic descriptor of type
*                   cy_en_ble_wpts_descr_index_t.
*
* \return
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameters failed.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is not permitted on the specified attribute.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*  If the WPTS service-specific callback is registered
*      (with \ref Cy_BLE_WPTS_RegisterAttrCallback):
*  * #CY_BLE_EVT_WPTSC_READ_DESCR_RESPONSE - If the requested attribute is
*                                successfully read on the peer device,
*                                the details (charIndex, descrIndex, value, etc.)
*                                are provided with event parameter structure
*                                of type cy_stc_ble_wpts_descr_value_t.
*  .
*  Otherwise (if the WPTS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*                                successfully read on the peer device,
*                                the details (handle, value, etc.) are
*                                provided with event parameters
*                                structure (cy_stc_ble_gattc_read_rsp_param_t).
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*                                requested attribute on the peer device,
*                                the details are provided with event parameters
*                                structure (cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_WPTSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_wpts_char_index_t charIndex,
                                                                cy_en_ble_wpts_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_WPTS_CHAR_COUNT) || (descrIndex >= CY_BLE_WPTS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Get pointer (with offset) to WPS client structure with attribute handles */
        cy_stc_ble_wptsc_t *wptscPtr = (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[discIdx];

        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = wptscPtr->charInfo[charIndex].descrHandle[descrIndex];
        readReqParam.connHandle = connHandle;
       
        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_wptscReqHandle[discIdx] = wptscPtr->charInfo[charIndex].descrHandle[descrIndex];
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_GetCharacteristicValueHandle
***************************************************************************//**
*
*  Returns the discovered peer device characteristic value handle.
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of a service characteristic.
*
*  \return
*   Returns characteristic value handle of type cy_ble_gatt_db_attr_handle_t.
*   * CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE: when a peer device doesn't have an optional characteristic.
*
******************************************************************************/
cy_ble_gatt_db_attr_handle_t Cy_BLE_WPTSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_wpts_char_index_t charIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

    if( (Cy_BLE_GetDiscoveryIdx(connHandle) < cy_ble_configPtr->params->maxClientCount) && 
            (charIndex < CY_BLE_WPTS_CHAR_COUNT) )
    {
        /* Get pointer (with offset) to WPTS client structure with attribute handles */
        cy_stc_ble_wptsc_t *wptscPtr =
                (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[Cy_BLE_GetDiscoveryIdx(connHandle)];

        returnHandle = wptscPtr->charInfo[charIndex].valueHandle;
    }
    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_GetCharacteristicDescriptorHandle
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
cy_ble_gatt_db_attr_handle_t Cy_BLE_WPTSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_wpts_char_index_t charIndex,
                                                                            cy_en_ble_wpts_descr_index_t descrIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

    if((Cy_BLE_GetDiscoveryIdx(connHandle) < cy_ble_configPtr->params->maxClientCount) &&
           (charIndex < CY_BLE_WPTS_CHAR_COUNT) && (descrIndex < CY_BLE_WPTS_DESCR_COUNT))
    {
        /* Get pointer (with offset) to WPTS client structure with attribute handles */
        cy_stc_ble_wptsc_t *wptscPtr =
                (cy_stc_ble_wptsc_t *)&cy_ble_wptscConfigPtr->attrInfo[Cy_BLE_GetDiscoveryIdx(connHandle)];

        returnHandle = wptscPtr->charInfo[charIndex].descrHandle[descrIndex];
    }

    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the WPTS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_WPTS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_WPTSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_WPTSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_WPTSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_WPTSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the WPTS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_WPTSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_WPTSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            Cy_BLE_WPTSS_ConfirmationEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_WPTSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the WPTS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_WPTSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
              /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_WPTSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_WPTSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_WPTSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                if((discIdx < cy_ble_configPtr->params->maxClientCount) && (cy_ble_configPtr->context->discovery[discIdx].autoDiscoveryFlag == 0u) &&
                   (((cy_stc_ble_gatt_err_param_t*)eventParam)->errInfo.errorCode !=
                    CY_BLE_GATT_ERR_ATTRIBUTE_NOT_FOUND))
                {
                    Cy_BLE_WPTSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_WPTSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_IND:
                Cy_BLE_WPTSC_IndicationEventHandler((cy_stc_ble_gattc_handle_value_ind_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_WPTSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_WPTSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
