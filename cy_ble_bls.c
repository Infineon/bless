/***************************************************************************//**
* \file cy_ble_bls.c
* \version 3.30
*
* \brief
*  This file contains the source code for the Blood Pressure service.
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
* Internal Defines / Macro Functions
*******************************************************************************/

#define CY_BLE_BLS_IS_ICP_SUPPORTED \
    (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_blssConfigPtr->attrInfo->charInfo[CY_BLE_BLS_ICP].charHandle)

    
/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_BLS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_BLSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_BLSC_EventHandler(uint32_t eventCode, void *eventParam);

static void Cy_BLE_BLSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_BLSS_WriteEventHandler(const cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_BLSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_BLSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_BLSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_BLSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_BLSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam);
static void Cy_BLE_BLSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_BLSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_BLSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_BLS_EventHandler(uint32_t eventCode, void *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_BLS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_BLSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_BLSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response for server */
static cy_ble_gatt_db_attr_handle_t cy_ble_blssReqHandle;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_blscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE BLS server config structure */
const cy_stc_ble_blss_config_t *cy_ble_blssConfigPtr = NULL;

/* The pointer to a global BLE BLS client config structure */
const cy_stc_ble_blsc_config_t *cy_ble_blscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_BLSS_Init
***************************************************************************//**
*
*  This function initializes server of the Blood Pressure service.
*
*  \param config: Configuration structure for the BLS.
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
cy_en_ble_api_result_t Cy_BLE_BLSS_Init(const cy_stc_ble_blss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_blssConfigPtr = config;

        /* Registers event handler for the BLS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_BLS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_BLSS_EventHandlerCallback = &Cy_BLE_BLSS_EventHandler;
        
        cy_ble_blssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BLSC_Init
***************************************************************************//**
*
*  This function initializes client of the Blood Pressure service.
*
*  \param config: Configuration structure for the BLS.
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
cy_en_ble_api_result_t Cy_BLE_BLSC_Init(const cy_stc_ble_blsc_config_t *config)
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
        cy_ble_blscConfigPtr = config;

        /* Registers event handler for the BLS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_BLS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_BLSC_EventHandlerCallback = &Cy_BLE_BLSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 blsServIdx = cy_ble_blscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + blsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to BLS client structure */
                cy_stc_ble_blsc_t *blscPtr = (cy_stc_ble_blsc_t *)&cy_ble_blscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(blscPtr, 0, sizeof(cy_stc_ble_blsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + blsServIdx].uuid =
                    CY_BLE_UUID_BLOOD_PRESSURE_SERVICE;
            }

            cy_ble_blscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BLS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for BLS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_BLSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_bls_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_BLS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_BLS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_BLSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a value of the characteristic which is identified by charIndex.
*
*  \param charIndex: The index of the service characteristic of type
*                    \ref cy_en_ble_bls_char_index_t. The valid values are,
*                       * \ref CY_BLE_BLS_BPM
*                       * \ref CY_BLE_BLS_ICP
*                       * \ref CY_BLE_BLS_BPF
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE| An optional characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BLSS_SetCharacteristicValue(cy_en_ble_bls_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if(charIndex >= CY_BLE_BLS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_blssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Store the characteristic value into the GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_blssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_BLSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets a characteristic value of the Blood Pressure service identified by
*  charIndex.
*
*  \param charIndex: The index of the service characteristic of type
*                    \ref cy_en_ble_bls_char_index_t. The valid values are,
*                       * \ref CY_BLE_BLS_BPM
*                       * \ref CY_BLE_BLS_ICP
*                       * \ref CY_BLE_BLS_BPF
*  \param attrSize:  The size of the characteristic value attribute.
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | An optional characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BLSS_GetCharacteristicValue(cy_en_ble_bls_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    /* Check the parameters */
    if(charIndex >= CY_BLE_BLS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_blssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Get the characteristic value from the GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_blssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_BLSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of a specified characteristic of the Blood
*  Pressure service from the local GATT database.
*
*  \param connHandle: The connection handle
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_bls_char_index_t. The valid values are,
*                       * \ref CY_BLE_BLS_BPM
*                       * \ref CY_BLE_BLS_ICP
*                       * \ref CY_BLE_BLS_BPF
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_bls_descr_index_t. The valid value is,
*                       * #CY_BLE_BLS_CCCD
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  The pointer to the location where the characteristic descriptor
*                     value data should be stored.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.

*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | An optional descriptor is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BLSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bls_char_index_t charIndex,
                                                               cy_en_ble_bls_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    /* Check the parameters */
    if((charIndex >= CY_BLE_BLS_CHAR_COUNT) || (descrIndex >= CY_BLE_BLS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_blssConfigPtr->attrInfo->charInfo[charIndex].cccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Get data from the database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_blssConfigPtr->attrInfo->charInfo[charIndex].cccdHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.connHandle                 = connHandle;
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
* Function Name: Cy_BLE_BLSS_SendNotification
***************************************************************************//**
*
*  Sends a notification with the characteristic value, as specified by its 
*  charIndex, to the client device.
*  On enabling notification successfully for a service characteristic it sends out a
*  'Handle Value Notification' which results in \ref CY_BLE_EVT_BLSC_NOTIFICATION event
*  at the GATT Client's end.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_bls_char_index_t. The valid values are,
*                       * \ref CY_BLE_BLS_BPM
*                       * \ref CY_BLE_BLS_ICP
*                       * \ref CY_BLE_BLS_BPF
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     sent to the client device.
*
* \return
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
cy_en_ble_api_result_t Cy_BLE_BLSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_bls_char_index_t charIndex,
                                                    uint8_t attrSize,
                                                    uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send notification if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_BLS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_blssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if(!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_blssConfigPtr->attrInfo->charInfo[charIndex].cccdHandle))
    {
        apiResult = CY_BLE_ERROR_NTF_DISABLED;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gatts_handle_value_ntf_t ntfParam;
        
        ntfParam.handleValPair.attrHandle = cy_ble_blssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        ntfParam.handleValPair.value.val  = attrValue;
        ntfParam.handleValPair.value.len  = attrSize;
        ntfParam.connHandle               = connHandle;
        
        /* Send notification to the client using a previously filled structure */
        apiResult = Cy_BLE_GATTS_Notification(&ntfParam);
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BLSS_SendIndication
***************************************************************************//**
*
*  Sends an indication of the specified characteristic to the client device.
*
*  On enabling indication successfully it sends out a Handle Value Indication which
*  results in a \ref CY_BLE_EVT_BLSC_INDICATION or \ref CY_BLE_EVT_GATTC_HANDLE_VALUE_IND 
*  (if the service-specific callback function is not registered) event at the 
*  GATT Client's end.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_bls_char_index_t. The valid values are,
*                       * \ref CY_BLE_BLS_BPM
*                       * \ref CY_BLE_BLS_ICP
*                       * \ref CY_BLE_BLS_BPF
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     sent to the client device.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_IND_DISABLED                | Indication is not enabled by the client.
*
*  \events
*  If execution is successful(return value = \ref CY_BLE_SUCCESS),
*  these events can appear: \n
*  If a BLS service-specific callback is registered
*  (with \ref Cy_BLE_BLS_RegisterAttrCallback):
*  * \ref CY_BLE_EVT_BLSS_INDICATION_CONFIRMED - if the indication is
*    successfully delivered to the peer device.
*  .
*  Otherwise (if a BLS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF - if the indication is
*    successfully delivered to the peer device.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BLSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_bls_char_index_t charIndex,
                                                  uint8_t attrSize,
                                                  uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send indication if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_BLS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(charIndex != CY_BLE_BLS_BPM)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else if(!CY_BLE_IS_INDICATION_ENABLED(connHandle.attId, cy_ble_blssConfigPtr->attrInfo->charInfo[charIndex].cccdHandle))
    {
        apiResult = CY_BLE_ERROR_IND_DISABLED;
    }
    else
    {
        /* Fill all fields of the Write Request structure ... */
        cy_stc_ble_gatts_handle_value_ind_t indParam;
        
        indParam.handleValPair.attrHandle = cy_ble_blssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        indParam.handleValPair.value.val  = attrValue;
        indParam.handleValPair.value.len  = attrSize;
        indParam.connHandle               = connHandle;

        /* Send indication to the client using a previously filled structure */
        apiResult = Cy_BLE_GATTS_Indication(&indParam);

        /* Save the handle to support a service-specific value confirmation response from the client */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_blssReqHandle = indParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BLSS_ConfirmationEventHandler
***************************************************************************//**
*
*  Handles a Value Confirmation request event from the BLE Stack.
*
*  \param eventParam - The pointer to a structure of type cy_stc_ble_conn_handle_t.
*
******************************************************************************/
static void Cy_BLE_BLSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    if((Cy_BLE_BLS_ApplCallback != NULL) &&
       (cy_ble_blssConfigPtr->attrInfo->charInfo[CY_BLE_BLS_BPM].charHandle == cy_ble_blssReqHandle))
    {
        cy_stc_ble_bls_char_value_t locCharIndex;
        
        locCharIndex.connHandle = *eventParam;
        locCharIndex.charIndex  = CY_BLE_BLS_BPM;
        locCharIndex.value      = NULL;
        
        cy_ble_blssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        Cy_BLE_BLS_ApplCallback((uint32_t)CY_BLE_EVT_BLSS_INDICATION_CONFIRMED, &locCharIndex);
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BLSS_WriteEventHandler
***************************************************************************//**
*
*  Handles a Write Request event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
*  \return
*   \ref cy_en_ble_gatt_err_code_t - The function result state if it succeeded
*   (CY_BLE_GATT_ERR_NONE) or GATT error codes returned by 
*    Cy_BLE_GATTS_WriteAttributeValueCCCD().
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BLSS_WriteEventHandler(const cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(Cy_BLE_BLS_ApplCallback != NULL)
    {
        cy_stc_ble_bls_char_value_t locCharIndex;
        locCharIndex.connHandle = eventParam->connHandle;
        locCharIndex.value = NULL;

        for(locCharIndex.charIndex = CY_BLE_BLS_BPM; locCharIndex.charIndex < CY_BLE_BLS_BPF; locCharIndex.charIndex++)
        {
            if(eventParam->handleValPair.attrHandle == cy_ble_blssConfigPtr->attrInfo->charInfo[locCharIndex.charIndex].
                cccdHandle)
            {
                cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                
                dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                dbAttrValInfo.connHandle      = eventParam->connHandle;
                dbAttrValInfo.offset          = 0u;
                dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;
                
                gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);

                if(gattErr == CY_BLE_GATT_ERR_NONE)
                {
                    uint32_t eventCode;

                    if(locCharIndex.charIndex == CY_BLE_BLS_ICP)
                    {
                        if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                        {
                            eventCode = (uint32_t)CY_BLE_EVT_BLSS_NOTIFICATION_ENABLED;
                        }
                        else
                        {
                            eventCode = (uint32_t)CY_BLE_EVT_BLSS_NOTIFICATION_DISABLED;
                        }
                    }
                    else
                    {
                        if(CY_BLE_IS_INDICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                        {
                            eventCode = (uint32_t)CY_BLE_EVT_BLSS_INDICATION_ENABLED;
                        }
                        else
                        {
                            eventCode = (uint32_t)CY_BLE_EVT_BLSS_INDICATION_DISABLED;
                        }
                    }
                    Cy_BLE_BLS_ApplCallback(eventCode, &locCharIndex);
                }

                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                break;
            }
        }
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_BLSC_GetCharacteristicValue
***************************************************************************//**
*
*  This function is used to read the characteristic value from the server
*  identified by charIndex.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_bls_char_index_t. The valid values are,
*                       * \ref CY_BLE_BLS_BPM
*                       * \ref CY_BLE_BLS_ICP
*                       * \ref CY_BLE_BLS_BPF
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*
* \events
*  If execution is successful(return value = \ref CY_BLE_SUCCESS)
*  these events can appear: \n
*  If a BLS service-specific callback is registered
*  (with \ref Cy_BLE_BLS_RegisterAttrCallback):
*  * \ref CY_BLE_EVT_BLSC_READ_CHAR_RESPONSE - if the requested attribute is
*    successfully read on the peer device, the details (charIndex , 
*    value, etc.) are provided with an event parameter structure
*    of type \ref cy_stc_ble_bls_char_value_t.
*  .
*  Otherwise (if a BLS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTC_READ_RSP - if the requested attribute is
*    successfully read on the peer device, the details (handle, 
*    value, etc.) are provided with the event parameters
*    structure \ref cy_stc_ble_gattc_read_rsp_param_t.
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*    requested attribute on the peer device, the details are provided 
*    with the event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BLSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_bls_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    
    /* Get pointer (with offset) to BLS client structure with attribute handles */
    cy_stc_ble_blsc_t *blscPtr = (cy_stc_ble_blsc_t *)&cy_ble_blscConfigPtr->attrInfo[discIdx];
    
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_BLS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(blscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_READ & blscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = blscPtr->charInfo[charIndex].valueHandle;
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_blscReqHandle[discIdx] = blscPtr->charInfo[charIndex].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BLSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to set the characteristic descriptor of the specified Blood Pressure
*  service characteristic to the server device.
*
*  Internally, a Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*  * \ref CY_BLE_EVT_BLSS_INDICATION_ENABLED
*  * \ref CY_BLE_EVT_BLSS_INDICATION_DISABLED
*  * \ref CY_BLE_EVT_BLSS_NOTIFICATION_ENABLED
*  * \ref CY_BLE_EVT_BLSS_NOTIFICATION_DISABLED
*
*  \param connHandle: The BLE peer device connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_bls_char_index_t. The valid values are,
*                       * \ref CY_BLE_BLS_BPM
*                       * \ref CY_BLE_BLS_ICP
*                       * \ref CY_BLE_BLS_BPF
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_bls_descr_index_t. The valid value is,
*                       * #CY_BLE_BLS_CCCD
*  \param attrSize:   The size of the characteristic descriptor value attribute.
*  \param attrValue:  The pointer to the characteristic descriptor value data that should
*                     be sent to the server device.
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
*  If execution is successful(return value = \ref CY_BLE_SUCCESS),
*  the following events can appear: \n
*  If a BLS service-specific callback is registered
*  with \ref Cy_BLE_BLS_RegisterAttrCallback():
*  * \ref CY_BLE_EVT_BLSC_WRITE_DESCR_RESPONSE - if the requested attribute is
*    successfully written on the peer device, the details (charIndex, 
*    descrIndex etc.) are provided with an event parameter structure
*    of type \ref cy_stc_ble_bls_descr_value_t.
*  .
*   Otherwise (if the BLS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTC_WRITE_RSP - if the requested attribute is
*    successfully written on the peer device.
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*    requested attribute on the peer device, the details are provided 
*    with the event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BLSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bls_char_index_t charIndex,
                                                               cy_en_ble_bls_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to BLS client structure with attribute handles */
    cy_stc_ble_blsc_t *blscPtr = (cy_stc_ble_blsc_t *)&cy_ble_blscConfigPtr->attrInfo[discIdx];
    
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_BLS_CHAR_COUNT) || (descrIndex >= CY_BLE_BLS_DESCR_COUNT)
            || (attrSize != CY_BLE_CCCD_LEN) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(blscPtr->charInfo[charIndex].cccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Fill all fields of a Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        
        writeReqParam.handleValPair.attrHandle = blscPtr->charInfo[charIndex].cccdHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = CY_BLE_CCCD_LEN;
        writeReqParam.connHandle               = connHandle;
        
        /* ... and send a request to the server device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

        /* Save the handle to support a service-specific Read response from the device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_blscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BLSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to get the characteristic descriptor of the specified Blood Pressure
*  service characteristic from the server device. This function call can result
*  in the generation of the following events based on the response from the server
*  device:
*  * \ref CY_BLE_EVT_BLSC_READ_DESCR_RESPONSE
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP
*
*  \param connHandle: The BLE peer device connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_bls_char_index_t. The valid values are,
*                       * \ref CY_BLE_BLS_BPM
*                       * \ref CY_BLE_BLS_ICP
*                       * \ref CY_BLE_BLS_BPF
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_bls_descr_index_t. The valid value is,
*                       * #CY_BLE_BLS_CCCD*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | This operation is not permitted on the specified attribute.
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular descriptor.
*
*  \events
*  If execution is successful (return value = \ref CY_BLE_SUCCESS),
*  these events can appear: \n
*  If a BLS service-specific callback is registered
*  with \ref Cy_BLE_BLS_RegisterAttrCallback():
*  * \ref CY_BLE_EVT_BLSC_READ_DESCR_RESPONSE - if the requested attribute is
*    successfully read on the peer device, the details (charIndex, descrIndex, 
*    value, etc.) are provided with an event parameter structure
*    of type \ref cy_stc_ble_bls_descr_value_t.
*  .
*  Otherwise (if a BLS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTC_READ_RSP - if the requested attribute is
*    successfully read on the peer device, the details (handle, 
*    value, etc.) are provided with the event parameters
*    structure \ref cy_stc_ble_gattc_read_rsp_param_t.
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*    requested attribute on the peer device, the details are provided 
*    with the event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BLSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bls_char_index_t charIndex,
                                                               cy_en_ble_bls_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to BLS client structure with attribute handles */
    cy_stc_ble_blsc_t *blscPtr = (cy_stc_ble_blsc_t *)&cy_ble_blscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_BLS_CHAR_COUNT) || (descrIndex >= CY_BLE_BLS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(blscPtr->charInfo[charIndex].cccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = blscPtr->charInfo[charIndex].cccdHandle;
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_blscReqHandle[discIdx] = blscPtr->charInfo[charIndex].cccdHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BLSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, the appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo:  The pointer to the characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_BLSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* Blood Pressure service characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_blscCharUuid[CY_BLE_BLS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_BP_MSRMT,
        CY_BLE_UUID_CHAR_INTRMDT_CUFF_PRSR,
        CY_BLE_UUID_CHAR_BP_FEATURE
    };
    
    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t blsDiscIdx = cy_ble_blscConfigPtr->serviceDiscIdx;
    uint32_t i;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == blsDiscIdx))
    {
        /* Get pointer (with offset) to BLS client structure with attribute handles */
        cy_stc_ble_blsc_t *blscPtr = (cy_stc_ble_blsc_t *)&cy_ble_blscConfigPtr->attrInfo[discIdx];

        /* Update the last characteristic endHandle to the declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        for(i = 0u; i < (uint32_t)CY_BLE_BLS_CHAR_COUNT; i++)
        {
            if(cy_ble_blscCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                if(blscPtr->charInfo[i].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    blscPtr->charInfo[i].valueHandle = discCharInfo->valueHandle;
                    blscPtr->charInfo[i].properties = discCharInfo->properties;
                    lastEndHandle[discIdx] = &blscPtr->charInfo[i].endHandle;
                }
                else
                {
                    Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo);
                }
            }
        }

        /* Initialize the characteristic endHandle to the service endHandle.
         * The characteristic endHandle will be updated to the declaration
         * handle of the following characteristic,
         * in the following characteristic discovery procedure. */
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
* Function Name: Cy_BLE_BLSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, the appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo:  The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_BLSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t * discDescrInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t blsDiscIdx = cy_ble_blscConfigPtr->serviceDiscIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == blsDiscIdx)
    {
        if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_CLIENT_CONFIG)
        {
            /* Get pointer (with offset) to BLS client structure with attribute handles */
            cy_stc_ble_blsc_t *blscPtr = (cy_stc_ble_blsc_t *)&cy_ble_blscConfigPtr->attrInfo[discIdx];
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            
            if(blscPtr->charInfo[charIdx].cccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                blscPtr->charInfo[charIdx].cccdHandle = discDescrInfo->descrHandle;
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
* Function Name: Cy_BLE_BLSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_BLSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t blsDiscIdx = cy_ble_blscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == blsDiscIdx)
    {
        /* Get pointer (with offset) to BLS client structure with attribute handles */
        cy_stc_ble_blsc_t *blscPtr = (cy_stc_ble_blsc_t *)&cy_ble_blscConfigPtr->attrInfo[discIdx];

        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_BLS_CHAR_COUNT) && (exitFlag == 0u))
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            if((blscPtr->charInfo[charIdx].endHandle - blscPtr->charInfo[charIdx].valueHandle) != 0u)
            {
                /* Read characteristic range */
                charRangeInfo->range.startHandle = blscPtr->charInfo[charIdx].valueHandle + 1u;
                charRangeInfo->range.endHandle = blscPtr->charInfo[charIdx].endHandle;
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
* Function Name: Cy_BLE_BLSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles a Notification event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BLSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
    /* Get pointer (with offset) to BLS client structure with attribute handles */
    cy_stc_ble_blsc_t *blscPtr = (cy_stc_ble_blsc_t *)&cy_ble_blscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_BLS_ApplCallback != NULL) &&
       (blscPtr->charInfo[CY_BLE_BLS_ICP].valueHandle == eventParam->handleValPair.attrHandle))
    {
        cy_stc_ble_bls_char_value_t locCharValue;
        
        locCharValue.connHandle = eventParam->connHandle;
        locCharValue.charIndex  = CY_BLE_BLS_ICP;
        locCharValue.value      = &eventParam->handleValPair.value;
        
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        Cy_BLE_BLS_ApplCallback((uint32_t)CY_BLE_EVT_BLSC_NOTIFICATION, &locCharValue);
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BLSC_IndicationEventHandler
***************************************************************************//**
*
*  Handles an Indication event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BLSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
    
    /* Get pointer (with offset) to BLS client structure with attribute handles */
    cy_stc_ble_blsc_t *blscPtr = (cy_stc_ble_blsc_t *)&cy_ble_blscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_BLS_ApplCallback != NULL) &&
       (blscPtr->charInfo[CY_BLE_BLS_BPM].valueHandle == eventParam->handleValPair.attrHandle))
    {
        cy_stc_ble_bls_char_value_t locCharValue;
        
        locCharValue.charIndex  = CY_BLE_BLS_BPM;
        locCharValue.connHandle = eventParam->connHandle;
        locCharValue.value      = &eventParam->handleValPair.value;
        
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        Cy_BLE_BLS_ApplCallback((uint32_t)CY_BLE_EVT_BLSC_INDICATION, &locCharValue);
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BLSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles a Read Response event.
*
*  \param cy_stc_ble_gattc_read_rsp_param_t *eventParam: The pointer to the data structure.
*
******************************************************************************/
static void Cy_BLE_BLSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_BLS_ApplCallback != NULL) &&
       (cy_ble_blscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to BLS client structure with attribute handles */
        cy_stc_ble_blsc_t *blscPtr = (cy_stc_ble_blsc_t *)&cy_ble_blscConfigPtr->attrInfo[discIdx];

        if(blscPtr->charInfo[CY_BLE_BLS_BPF].valueHandle == cy_ble_blscReqHandle[discIdx])
        {
            cy_stc_ble_bls_char_value_t locCharValue; 
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.charIndex  = CY_BLE_BLS_BPF;
            locCharValue.value      = &eventParam->value;

            cy_ble_blscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_BLS_ApplCallback((uint32_t)CY_BLE_EVT_BLSC_READ_CHAR_RESPONSE, &locCharValue);
        }
        else
        {
            cy_en_ble_bls_char_index_t i;

            for(i = CY_BLE_BLS_BPM; i < CY_BLE_BLS_BPF; i++)
            {
                if(blscPtr->charInfo[i].cccdHandle == cy_ble_blscReqHandle[discIdx])
                {
                    cy_stc_ble_bls_descr_value_t locDescrValue;
                    
                    locDescrValue.connHandle = eventParam->connHandle;
                    locDescrValue.charIndex  = i;
                    locDescrValue.descrIndex = CY_BLE_BLS_CCCD;
                    locDescrValue.value      = &eventParam->value;
                    
                    cy_ble_blscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                    Cy_BLE_BLS_ApplCallback((uint32_t)CY_BLE_EVT_BLSC_READ_DESCR_RESPONSE, &locDescrValue);
                    break;
                }
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BLSC_GetCharacteristicValueHandle
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
cy_ble_gatt_db_attr_handle_t Cy_BLE_BLSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_bls_char_index_t charIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

    if( (Cy_BLE_GetDiscoveryIdx(connHandle) < cy_ble_configPtr->params->maxClientCount) &&
            (charIndex < CY_BLE_BLS_CHAR_COUNT))
    {
        /* Get pointer (with offset) to BLS client structure with attribute handles */
        cy_stc_ble_blsc_t *blscPtr =
                (cy_stc_ble_blsc_t *)&cy_ble_blscConfigPtr->attrInfo[Cy_BLE_GetDiscoveryIdx(connHandle)];

        returnHandle = blscPtr->charInfo[charIndex].valueHandle;
    }
    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_BLSC_GetCharacteristicDescriptorHandle
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
cy_ble_gatt_db_attr_handle_t Cy_BLE_BLSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_bls_char_index_t charIndex,
                                                                            cy_en_ble_bls_descr_index_t descrIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

    if((Cy_BLE_GetDiscoveryIdx(connHandle) < cy_ble_configPtr->params->maxClientCount) &&
           (charIndex < CY_BLE_BLS_CHAR_COUNT) && (descrIndex < CY_BLE_BLS_DESCR_COUNT))
    {
        /* Get pointer (with offset) to BLS client structure with attribute handles */
        cy_stc_ble_blsc_t *blscPtr =
                (cy_stc_ble_blsc_t *)&cy_ble_blscConfigPtr->attrInfo[Cy_BLE_GetDiscoveryIdx(connHandle)];

        returnHandle = blscPtr->charInfo[charIndex].cccdHandle;
    }

    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_BLSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles a Write Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BLSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_BLS_ApplCallback != NULL) &&
       (cy_ble_blscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {   
        /* Get pointer (with offset) to BLS client structure with attribute handles */
        cy_stc_ble_blsc_t *blscPtr = (cy_stc_ble_blsc_t *)&cy_ble_blscConfigPtr->attrInfo[discIdx];
        cy_en_ble_bls_char_index_t i;
        
        for(i = CY_BLE_BLS_BPM; i < CY_BLE_BLS_BPF; i++)
        {
            if(blscPtr->charInfo[i].cccdHandle == cy_ble_blscReqHandle[discIdx])
            {
                cy_stc_ble_bls_descr_value_t locDescIndex;
                
                locDescIndex.connHandle = *eventParam;
                locDescIndex.charIndex  = i;
                locDescIndex.descrIndex = CY_BLE_BLS_CCCD;
                locDescIndex.value      = NULL;
                
                cy_ble_blscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                Cy_BLE_BLS_ApplCallback((uint32_t)CY_BLE_EVT_BLSC_WRITE_DESCR_RESPONSE, &locDescIndex);
                
                break;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BLSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles an Error Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BLSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        if(cy_ble_blscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] == eventParam->errInfo.attrHandle)
        {
            cy_ble_blscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BLS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the BLS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BLS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_BLSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_BLSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_BLSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_BLSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_BLSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the BLS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BLSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_BLSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            Cy_BLE_BLSS_ConfirmationEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_BLSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the BLS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BLSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
             /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_BLSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_BLSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_BLSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_BLSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_BLSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_IND:
                Cy_BLE_BLSC_IndicationEventHandler((cy_stc_ble_gattc_handle_value_ind_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_BLSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_BLSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
