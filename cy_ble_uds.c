/***************************************************************************//**
* \file cy_ble_uds.c
* \version 3.20
*
* \brief
*  This file contains the source code for the User Data service.
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

#define CY_BLE_UDS_IS_PROCEDURE_IN_PROGRESS    (0u != (CY_BLE_UDS_FLAG_PROCESS & cy_ble_udsFlag))
#define CY_BLE_UDS_LONG_CHAR_NUM               (4u)


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_UDS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_UDSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_UDSC_EventHandler(uint32_t eventCode, void *eventParam);

static void Cy_BLE_UDSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_UDSS_ReadRequestEventHandler(cy_stc_ble_gatts_char_val_read_req_t *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_UDSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);
static void Cy_BLE_UDSS_PrepareWriteRequestEventHandler(cy_stc_ble_gatts_prep_write_req_param_t *eventParam);
static void Cy_BLE_UDSS_ExecuteWriteRequestEventHandler(cy_stc_ble_gatts_exec_write_req_t *eventParam);

static void Cy_BLE_UDSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_UDSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_UDSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_UDSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_UDSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam);
static void Cy_BLE_UDSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_UDSC_ReadLongRespEventHandler(const cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_UDSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_UDSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);
static void Cy_BLE_UDSC_ExecuteWriteResponseEventHandler(const cy_stc_ble_gattc_exec_write_rsp_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_UDS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_UDSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_UDSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response for server */
static cy_ble_gatt_db_attr_handle_t cy_ble_udssReqHandle;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_udscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE UDS server config structure */
const cy_stc_ble_udss_config_t *cy_ble_udssConfigPtr = NULL;

/* The pointer to a global BLE UDS client config structure */
const cy_stc_ble_udsc_config_t *cy_ble_udscConfigPtr = NULL;

uint8_t cy_ble_udsFlag;

/* Read Long Descriptors variables */
static uint8_t * cy_ble_udscRdLongBuffPtr[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
static uint16_t cy_ble_udscRdLongBuffLen[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
static uint8_t cy_ble_udscCurrLen[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };


/******************************************************************************
* Function Name: Cy_BLE_UDSS_Init
***************************************************************************//**
*
*  This function initializes server of the User Data service.
*
*  \param config: Configuration structure for the UDS.
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
cy_en_ble_api_result_t Cy_BLE_UDSS_Init(const cy_stc_ble_udss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_udssConfigPtr = config;

        /* Registers event handler for the UDS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_UDS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_UDSS_EventHandlerCallback = &Cy_BLE_UDSS_EventHandler;

        cy_ble_udssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        cy_ble_udsFlag = 0u;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_Init
***************************************************************************//**
*
*  This function initializes the client of the User Data service.
*
*  \param config: Configuration structure for the UDS.
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
cy_en_ble_api_result_t Cy_BLE_UDSC_Init(const cy_stc_ble_udsc_config_t *config)
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
        cy_ble_udscConfigPtr = config;

        /* Registers event handler for the UDS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_UDS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_UDSC_EventHandlerCallback = &Cy_BLE_UDSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 udsServIdx = cy_ble_udscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + udsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to UDS client structure */
                cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(udscPtr, 0, sizeof(cy_stc_ble_udsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + udsServIdx].uuid =
                    CY_BLE_UUID_USER_DATA_SERVICE;
            }

            cy_ble_udscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_UDS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for UDS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback;
*         (e.g. #CY_BLE_EVT_UDSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_uds_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_UDS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_UDS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_UDSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets the value of the characteristic, as identified by charIndex.
*
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     stored to the GATT database.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The function completed successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | An optional characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_UDSS_SetCharacteristicValue(cy_en_ble_uds_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check parameters */
    if(charIndex >= CY_BLE_UDS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_udssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Store characteristic value into GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_udssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_UDSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets the value of the characteristic, as identified by charIndex.
*
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the location where characteristic
*                     value data should be stored.
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
cy_en_ble_api_result_t Cy_BLE_UDSS_GetCharacteristicValue(cy_en_ble_uds_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check parameters */
    if(charIndex >= CY_BLE_UDS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_udssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Get characteristic value from GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_udssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_UDSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of the specified characteristic.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the characteristic.
*  \param descrIndex: The index of the descriptor.
*  \param attrSize:   The size of the descriptor value attribute.
*  \param attrValue:  The pointer to the location where characteristic descriptor value
*                     data should be stored.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | A characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_UDSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_uds_char_index_t charIndex,
                                                               cy_en_ble_uds_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check parameters */
    if((charIndex >= CY_BLE_UDS_CHAR_COUNT) || (descrIndex >= CY_BLE_UDS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_udssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[CY_BLE_UDS_CCCD] ==
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Get data from database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_udssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex];
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
* Function Name: Cy_BLE_UDSS_SendNotification
***************************************************************************//**
*
*  Sends a notification of the specified characteristic value, as identified by
*  the charIndex.
*
*  On enabling notification successfully for a service characteristic, sends out a
*  'Handle Value Notification', which results in #CY_BLE_EVT_UDSC_NOTIFICATION event
*  at the GATT Client's end.
*
*  \param connHandle: The connection handle which consist of the device ID and ATT
*                     connection ID.
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data.
*                     that should be sent to the client device.
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
cy_en_ble_api_result_t Cy_BLE_UDSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_uds_char_index_t charIndex,
                                                    uint8_t attrSize,
                                                    uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send Notification if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_UDS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_udssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[CY_BLE_UDS_CCCD] ==
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if(!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId,
                                            cy_ble_udssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[CY_BLE_UDS_CCCD]))
    {
        apiResult = CY_BLE_ERROR_NTF_DISABLED;
    }
    else
    {
        cy_stc_ble_gatts_handle_value_ntf_t ntfParam;
        
        /* Fill all fields of Write Request structure ... */
        ntfParam.handleValPair.attrHandle = cy_ble_udssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        ntfParam.handleValPair.value.val  = attrValue;
        ntfParam.handleValPair.value.len  = attrSize;
        ntfParam.connHandle               = connHandle;

        /* Send notification to client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Notification(&ntfParam);
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_UDSS_SendIndication
***************************************************************************//**
*
*  Sends an indication of the specified characteristic value, as identified by
*  the charIndex.
*
*  On enabling indication successfully, sends out a 'Handle Value Indication', which
*  results in #CY_BLE_EVT_UDSC_INDICATION or #CY_BLE_EVT_GATTC_HANDLE_VALUE_IND (if
*  service-specific callback function is not registered) event at the GATT Client's end.
*
*  \param connHandle: The connection handle which consist of the device ID and ATT
*                     connection ID.
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be sent
*                     to the client device.
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
*   CY_BLE_ERROR_IND_DISABLED                | Indication is not enabled by the client.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the UDS service-specific callback is registered
*      (with \ref Cy_BLE_UDS_RegisterAttrCallback):
*  * #CY_BLE_EVT_UDSS_INDICATION_CONFIRMED - Whether the indication is
*                                successfully delivered to the peer device.
*  .
*   Otherwise (if the UDS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF - Whether the indication is
*                                successfully delivered to the peer device.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_UDSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_uds_char_index_t charIndex,
                                                  uint8_t attrSize,
                                                  uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send Indication if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_UDS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_udssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[CY_BLE_UDS_CCCD] ==
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if(!CY_BLE_IS_INDICATION_ENABLED(connHandle.attId,
                                          cy_ble_udssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[CY_BLE_UDS_CCCD]))
    {
        apiResult = CY_BLE_ERROR_IND_DISABLED;
    }
    else
    {
        cy_stc_ble_gatts_handle_value_ind_t indParam;
        
        /* Fill all fields of Write Request structure ... */
        indParam.handleValPair.attrHandle = cy_ble_udssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        indParam.handleValPair.value.val  = attrValue;
        indParam.handleValPair.value.len  = attrSize;
        indParam.connHandle               = connHandle;

        /* Send notification to client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Indication(&indParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            /* Save handle to support service-specific value confirmation response from client */
            cy_ble_udssReqHandle = indParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_UDSS_ConfirmationEventHandler
***************************************************************************//**
*
*  Handles the Value Confirmation request event from the BLE Stack.
*
*  \param eventParam: The pointer to a structure of type cy_stc_ble_conn_handle_t.
*
******************************************************************************/
static void Cy_BLE_UDSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    if((Cy_BLE_UDS_ApplCallback != NULL)
       && (cy_ble_udssConfigPtr->attrInfo->charInfo[CY_BLE_UDS_UCP].charHandle == cy_ble_udssReqHandle))
    {
        cy_stc_ble_uds_char_value_t locCharIndex;
        
        locCharIndex.connHandle = *eventParam;
        locCharIndex.charIndex  = CY_BLE_UDS_UCP;
        locCharIndex.value      = NULL;

        cy_ble_udssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        cy_ble_udsFlag &= (uint8_t) ~CY_BLE_UDS_FLAG_PROCESS;

        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        Cy_BLE_UDS_ApplCallback((uint32_t)CY_BLE_EVT_UDSS_INDICATION_CONFIRMED, &locCharIndex);
    }
}


/******************************************************************************
* Function Name: Cy_BLE_UDSS_ReadRequestEventHandler
***************************************************************************//**
*
*  Handles the Read Request event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_UDSS_ReadRequestEventHandler(cy_stc_ble_gatts_char_val_read_req_t *eventParam)
{
    if(Cy_BLE_UDS_ApplCallback != NULL)
    {
        cy_stc_ble_uds_char_value_t locCharValue;
        
        locCharValue.connHandle    = eventParam->connHandle;
        locCharValue.gattErrorCode = CY_BLE_GATT_ERR_NONE;
        locCharValue.value         = NULL;

        for(locCharValue.charIndex = CY_BLE_UDS_FNM;
            locCharValue.charIndex < CY_BLE_UDS_CHAR_COUNT;
            locCharValue.charIndex++)
        {
            if((locCharValue.charIndex != CY_BLE_UDS_UCP) &&
               (eventParam->attrHandle == cy_ble_udssConfigPtr->attrInfo->charInfo[locCharValue.charIndex].charHandle))
            {
                Cy_BLE_UDS_ApplCallback((uint32_t)CY_BLE_EVT_UDSS_READ_CHAR, &locCharValue);

                if(locCharValue.gattErrorCode == CY_BLE_GATT_ERR_USER_DATA_ACCESS_NOT_PERMITTED)
                {
                    eventParam->gattErrorCode = CY_BLE_GATT_ERR_USER_DATA_ACCESS_NOT_PERMITTED;
                }

                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                break;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_UDSS_WriteEventHandler
***************************************************************************//**
*
*  Handles the Write Request event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
* \return
*  cy_en_ble_gatt_err_code_t - A function result state if it succeeded
*  (CY_BLE_GATT_ERR_NONE) or failed with error codes:
*   * CY_BLE_GATTS_ERR_PROCEDURE_ALREADY_IN_PROGRESS
*   * CY_BLE_GATTS_ERR_CCCD_IMPROPERLY_CONFIGURED
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_UDSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(Cy_BLE_UDS_ApplCallback != NULL)
    {
        cy_stc_ble_uds_char_value_t locCharValue;
        locCharValue.connHandle = eventParam->connHandle;
        locCharValue.gattErrorCode = CY_BLE_GATT_ERR_NONE;

        if((eventParam->handleValPair.attrHandle ==
            cy_ble_udssConfigPtr->attrInfo->charInfo[CY_BLE_UDS_DCI].descrHandle[CY_BLE_UDS_CCCD]) ||
           (eventParam->handleValPair.attrHandle ==
            cy_ble_udssConfigPtr->attrInfo->charInfo[CY_BLE_UDS_UCP].descrHandle[CY_BLE_UDS_CCCD]))
        {
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
            
            dbAttrValInfo.handleValuePair = eventParam->handleValPair;
            dbAttrValInfo.connHandle      = eventParam->connHandle;
            dbAttrValInfo.offset          = 0u;
            dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

            gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
            locCharValue.value = NULL;
            if(gattErr == CY_BLE_GATT_ERR_NONE)
            {
                uint32_t eventCode;

                if(eventParam->handleValPair.attrHandle == cy_ble_udssConfigPtr->attrInfo->charInfo[CY_BLE_UDS_DCI].
                    descrHandle[CY_BLE_UDS_CCCD])
                {
                    locCharValue.charIndex = CY_BLE_UDS_DCI;

                    if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                    {
                        eventCode = (uint32_t)CY_BLE_EVT_UDSS_NOTIFICATION_ENABLED;
                    }
                    else
                    {
                        eventCode = (uint32_t)CY_BLE_EVT_UDSS_NOTIFICATION_DISABLED;
                    }
                }
                else
                {
                    locCharValue.charIndex = CY_BLE_UDS_UCP;

                    if(CY_BLE_IS_INDICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                    {
                        eventCode = (uint32_t)CY_BLE_EVT_UDSS_INDICATION_ENABLED;
                    }
                    else
                    {
                        eventCode = (uint32_t)CY_BLE_EVT_UDSS_INDICATION_DISABLED;
                    }
                }

                Cy_BLE_UDS_ApplCallback(eventCode, &locCharValue);
            }
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else
        {
            for(locCharValue.charIndex = CY_BLE_UDS_FNM;
                locCharValue.charIndex < CY_BLE_UDS_CHAR_COUNT;
                locCharValue.charIndex++)
            {
                if(eventParam->handleValPair.attrHandle ==
                   cy_ble_udssConfigPtr->attrInfo->charInfo[locCharValue.charIndex].charHandle)
                {
                    gattErr = Cy_BLE_GATT_DbCheckPermission(eventParam->handleValPair.attrHandle,
                                                            &eventParam->connHandle, CY_BLE_GATT_DB_WRITE |
                                                            CY_BLE_GATT_DB_PEER_INITIATED);

                    if((gattErr == CY_BLE_GATT_ERR_NONE) && (locCharValue.charIndex == CY_BLE_UDS_UCP))
                    {
                        if(CY_BLE_UDS_IS_PROCEDURE_IN_PROGRESS)
                        {
                            gattErr = CY_BLE_GATT_ERR_PROCEDURE_ALREADY_IN_PROGRESS;
                        }
                        else if(!CY_BLE_IS_INDICATION_ENABLED(eventParam->connHandle.attId,
                                                              cy_ble_udssConfigPtr->attrInfo->charInfo[CY_BLE_UDS_UCP].
                                                               descrHandle[CY_BLE_UDS_CCCD]))
                        {
                            gattErr = CY_BLE_GATT_ERR_CCCD_IMPROPERLY_CONFIGURED;
                        }
                        else
                        {
                        }
                    }

                    if(gattErr == CY_BLE_GATT_ERR_NONE)
                    {
                        locCharValue.value = &eventParam->handleValPair.value;

                        if(!CY_BLE_GATT_DB_ATTR_CHECK_PRPTY(eventParam->handleValPair.attrHandle,
                                                            CY_BLE_GATT_DB_CH_PROP_WRITE))
                        {
                            gattErr = CY_BLE_GATT_ERR_WRITE_NOT_PERMITTED;
                        }
                        else if(eventParam->handleValPair.value.len >
                                CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(eventParam->handleValPair.attrHandle))
                        {
                            gattErr = CY_BLE_GATT_ERR_INVALID_ATTRIBUTE_LEN;
                        }
                        else
                        {
                            Cy_BLE_UDS_ApplCallback((uint32_t)CY_BLE_EVT_UDSS_WRITE_CHAR, &locCharValue);

                            if(locCharValue.gattErrorCode == CY_BLE_GATT_ERR_USER_DATA_ACCESS_NOT_PERMITTED)
                            {
                                gattErr = CY_BLE_GATT_ERR_USER_DATA_ACCESS_NOT_PERMITTED;
                            }
                            else
                            {
                                cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                                
                                dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                                dbAttrValInfo.connHandle      = eventParam->connHandle;
                                dbAttrValInfo.offset          = 0u;
                                dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

                                gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);

                                if(gattErr == CY_BLE_GATT_ERR_NONE)
                                {
                                    CY_BLE_GATT_DB_ATTR_SET_ATTR_GEN_LEN(cy_ble_udssConfigPtr->attrInfo->
                                                                          charInfo[locCharValue.charIndex].charHandle,
                                                                         locCharValue.value->len);

                                    if(locCharValue.charIndex == CY_BLE_UDS_UCP)
                                    {
                                        cy_ble_udsFlag |= CY_BLE_UDS_FLAG_PROCESS;
                                    }
                                }
                            }
                        }
                    }
                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                    break;
                }
            }
        }
    }

    return(gattErr);
}

/******************************************************************************
* Function Name: Cy_BLE_UDSS_PrepareWriteRequestEventHandler 
***************************************************************************//**
*
*  Handles the Prepare Write Request event.
*
*  \param eventParam: The pointer to the data that comes with a Prepare
*                     Write Request.
*
******************************************************************************/
static void Cy_BLE_UDSS_PrepareWriteRequestEventHandler(cy_stc_ble_gatts_prep_write_req_param_t *eventParam)
{
    cy_en_ble_uds_char_index_t locCharIndex;
    cy_stc_ble_gatt_xchg_mtu_param_t mtuParam = { .connHandle = eventParam->connHandle };

    (void)Cy_BLE_GATT_GetMtuSize(&mtuParam);

    for(locCharIndex = CY_BLE_UDS_FNM; locCharIndex <= CY_BLE_UDS_EML; locCharIndex++)
    {
        if(eventParam->baseAddr[eventParam->currentPrepWriteReqCount - 1u].handleValuePair.attrHandle ==
           cy_ble_udssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle)
        {
            if(CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(cy_ble_udssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle) <=
               (mtuParam.mtu - CY_BLE_GATT_WRITE_HEADER_LEN))
            {
                eventParam->gattErrorCode = (uint8_t)CY_BLE_GATT_ERR_ATTRIBUTE_NOT_LONG;
            }

            /* Indicate that request was handled */
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            break;
        }
    }
}

/******************************************************************************
* Function Name: Cy_BLE_UDSS_ExecuteWriteRequestEventHandler
***************************************************************************//**
*
*  Handles the Execute Write Request event.
*
*  \param eventParam: The pointer to the data that came with a Write Request.
*
******************************************************************************/
static void Cy_BLE_UDSS_ExecuteWriteRequestEventHandler(cy_stc_ble_gatts_exec_write_req_t *eventParam)
{
    uint32_t locCount;
    cy_stc_ble_uds_char_value_t locCharValue[CY_BLE_UDS_LONG_CHAR_COUNT];
    cy_stc_ble_gatt_value_t locGattValue[CY_BLE_UDS_LONG_CHAR_COUNT];
    cy_en_ble_uds_char_index_t locCharIndex;

    for(locCharIndex = CY_BLE_UDS_FNM; locCharIndex <= CY_BLE_UDS_EML; locCharIndex++)
    {
        locGattValue[locCharIndex].len = 0u;
        locGattValue[locCharIndex].val = NULL;
    }

    for(locCount = 0u; locCount < eventParam->prepWriteReqCount; locCount++)
    {
        for(locCharIndex = CY_BLE_UDS_FNM; locCharIndex <= CY_BLE_UDS_EML; locCharIndex++)
        {
            if(eventParam->baseAddr[locCount].handleValuePair.attrHandle ==
               cy_ble_udssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle)
            {
                locGattValue[locCharIndex].len = eventParam->baseAddr[locCount].offset +
                                                 eventParam->baseAddr[locCount].handleValuePair.value.len;

                if(locGattValue[locCharIndex].val == NULL)
                {
                    locGattValue[locCharIndex].val = eventParam->baseAddr[locCount].handleValuePair.value.val;
                }
                else if(eventParam->baseAddr[locCount].offset == 0u)
                {
                    /* Case when client wants to rewrite value from beginning */
                    locGattValue[locCharIndex].val = eventParam->baseAddr[locCount].handleValuePair.value.val;
                }
                else
                {
                    /* Do nothing */
                }
            }
        }
    }

    for(locCharIndex = CY_BLE_UDS_FNM; locCharIndex <= CY_BLE_UDS_EML; locCharIndex++)
    {
        if((Cy_BLE_UDS_ApplCallback != NULL) && (locGattValue[locCharIndex].len != 0u) &&
           (locGattValue[locCharIndex].len <=
            CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(cy_ble_udssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle)))
        {
            /* Check the execWriteFlag before execute or cancel write long operation */
            if(eventParam->execWriteFlag == CY_BLE_GATT_EXECUTE_WRITE_EXEC_FLAG)
            {
                locCharValue[locCharIndex].gattErrorCode = CY_BLE_GATT_ERR_NONE;
                locCharValue[locCharIndex].connHandle = eventParam->connHandle;
                locCharValue[locCharIndex].charIndex = locCharIndex;
                locCharValue[locCharIndex].value = &locGattValue[locCharIndex];

                Cy_BLE_UDS_ApplCallback((uint32_t)CY_BLE_EVT_UDSS_WRITE_CHAR, &locCharValue[locCharIndex]);

                if(locCharValue[locCharIndex].gattErrorCode == CY_BLE_GATT_ERR_USER_DATA_ACCESS_NOT_PERMITTED)
                {
                    eventParam->gattErrorCode = (uint8_t)CY_BLE_GATT_ERR_USER_DATA_ACCESS_NOT_PERMITTED;
                }
                else
                {
                    CY_BLE_GATT_DB_ATTR_SET_ATTR_GEN_LEN(cy_ble_udssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle,
                                                         locGattValue[locCharIndex].len);
                }
            }

            /* Indicate that request was handled */
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result, a Write Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the #CY_BLE_EVT_UDSS_WRITE_CHAR event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue: The pointer to the characteristic value data that should be
*                     sent to the server device.
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
*  In the case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the UDS service-specific callback is registered
*      (with \ref Cy_BLE_UDS_RegisterAttrCallback):
*  * #CY_BLE_EVT_UDSC_WRITE_CHAR_RESPONSE - If the requested attribute is
*                                successfully written on the peer device,
*                                the details (charIndex, etc.) are
*                                provided with an event parameter structure
*                                of type cy_stc_ble_uds_char_value_t.
*  
*   Otherwise (if the UDS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*                                successfully written on the peer device.
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - If there is some trouble with the
*                                requested attribute on the peer device,
*                                the details are provided with an event parameter
*                                structure (cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_UDSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_uds_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    cy_stc_ble_gatt_xchg_mtu_param_t mtuParam; 
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    
    mtuParam.connHandle = connHandle;
    
    /* Get pointer (with offset) to UDS client structure with attribute handles */
    cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];

    (void)Cy_BLE_GATT_GetMtuSize(&mtuParam);

    /* Check parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_UDS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(udscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_WRITE & udscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else if((mtuParam.mtu - 3u) < attrSize)
    {
        cy_stc_ble_gattc_prep_write_req_t prepWriteReqParam;
        
        prepWriteReqParam.handleValOffsetPair.handleValuePair.attrHandle = udscPtr->charInfo[charIndex].valueHandle;
        prepWriteReqParam.handleValOffsetPair.offset                     = 0u;
        prepWriteReqParam.handleValOffsetPair.handleValuePair.value.val  = attrValue;
        prepWriteReqParam.handleValOffsetPair.handleValuePair.value.len  = attrSize;
        prepWriteReqParam.connHandle                                     = connHandle;
        
        apiResult = Cy_BLE_GATTC_WriteLongCharacteristicValues(&prepWriteReqParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_udscReqHandle[discIdx] = udscPtr->charInfo[charIndex].valueHandle;
        }
    }
    else
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;
        
        writeReqParam.handleValPair.attrHandle = udscPtr->charInfo[charIndex].valueHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;

        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_udscReqHandle[discIdx] = udscPtr->charInfo[charIndex].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_GetCharacteristicValue
***************************************************************************//**
*
*  This function is used to read the characteristic Value from a server,
*  as identified by its charIndex. As a result, a Read Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the #CY_BLE_EVT_UDSS_READ_CHAR event is generated.
*  On successful request execution on the server side, the Read Response is
*  sent to the client.
*
*  The Read Response only contains the characteristic Value that is less than or
*  equal to (MTU - 1) octets in length. If the characteristic Value is greater
*  than (MTU - 1) octets in length, the Read Long Characteristic Value procedure
*  may be used if the rest of the characteristic Value is required.
*
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The Read Request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameters failed.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the UDS service-specific callback is registered
*      (with \ref Cy_BLE_UDS_RegisterAttrCallback):
*  * #CY_BLE_EVT_UDSC_READ_CHAR_RESPONSE - If the requested attribute is
*                                successfully read on the peer device,
*                                the details (charIndex , value, etc.) are
*                                provided with an event parameter structure
*                                of type cy_stc_ble_uds_char_value_t.
*  .
*   Otherwise (if the UDS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*                                successfully read on the peer device,
*                                the details (handle, value, etc.) are
*                                provided with an event parameter
*                                structure (cy_stc_ble_gattc_read_rsp_param_t).
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - If there is some trouble with the
*                                requested attribute on the peer device,
*                                the details are provided with an event parameter
*                                structure (cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_UDSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_uds_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to UDS client structure with attribute handles */
    cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];

    /* Check parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_UDS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(udscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_READ & udscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = udscPtr->charInfo[charIndex].valueHandle;
        readReqParam.connHandle = connHandle;

        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);
    }

    if(apiResult == CY_BLE_SUCCESS)
    {
        cy_ble_udscReqHandle[discIdx] = udscPtr->charInfo[charIndex].valueHandle;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_GetLongCharacteristicValue
***************************************************************************//**
*
*  Sends a request to read a long characteristic.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the buffer where the read long characteristic
*                     descriptor value should be stored.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The Read Request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameters failed.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*
* \events
*  In the case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the UDS service-specific callback is registered
*      (with \ref Cy_BLE_UDS_RegisterAttrCallback):
*  * #CY_BLE_EVT_UDSC_READ_CHAR_RESPONSE - If the requested attribute is
*                                successfully read on the peer device,
*                                the details (charIndex , value, etc.) are
*                                provided with an event parameter structure
*                                of type cy_stc_ble_uds_char_value_t.
*  .
*   Otherwise (if the UDS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_READ_BLOB_RSP - If the requested attribute is
*                                successfully read on the peer device,
*                                the details (handle, value, etc.) are
*                                provided with an event parameter
*                                structure (cy_stc_ble_gattc_read_rsp_param_t).
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - If there is some trouble with the
*                                requested attribute on the peer device,
*                                the details are provided with an event parameter
*                                structure (cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_UDSC_GetLongCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                              cy_en_ble_uds_char_index_t charIndex,
                                                              uint16_t attrSize,
                                                              uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to UDS client structure with attribute handles */
    cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];

    /* Check parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(((charIndex > CY_BLE_UDS_EML) && (charIndex != CY_BLE_UDS_LNG)) || (attrSize == 0u) || 
            (attrValue == NULL) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(udscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_READ & udscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_read_blob_req_t readBlobReqParam;
        
        readBlobReqParam.handleOffset.attrHandle = udscPtr->charInfo[charIndex].valueHandle;
        readBlobReqParam.handleOffset.offset     = 0u;
        readBlobReqParam.connHandle              = connHandle;

        apiResult = Cy_BLE_GATTC_ReadLongCharacteristicValues(&readBlobReqParam);
    }

    if(apiResult == CY_BLE_SUCCESS)
    {
        cy_ble_udscReqHandle[discIdx] = udscPtr->charInfo[charIndex].valueHandle;
        cy_ble_udscRdLongBuffLen[discIdx] = attrSize;
        cy_ble_udscRdLongBuffPtr[discIdx] = attrValue;
        cy_ble_udscCurrLen[discIdx] = 0u;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  This function is used to write the characteristic Value to the server,
*  as identified by its charIndex.
*
*  Internally, Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*  * #CY_BLE_EVT_UDSS_INDICATION_ENABLED
*  * #CY_BLE_EVT_UDSS_INDICATION_DISABLED
*  * #CY_BLE_EVT_UDSS_NOTIFICATION_ENABLED
*  * #CY_BLE_EVT_UDSS_NOTIFICATION_DISABLED
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic.
*  \param descrIndex: The index of the service characteristic descriptor.
*  \param attrSize:   The size of the characteristic descriptor value attribute.
*  \param attrValue: The pointer to the characteristic descriptor value data that
*                      should be sent to the server device.
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_OPERATION           | This operation is not permitted on the specified attribute.
*
* \events
*  In the case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the UDS service-specific callback is registered
*      (with \ref Cy_BLE_UDS_RegisterAttrCallback):
*  * #CY_BLE_EVT_UDSC_WRITE_DESCR_RESPONSE - If the requested attribute is
*                                successfully written on the peer device,
*                                the details (charIndex, descrIndex, etc.) are
*                                provided with an event parameter structure
*                                of type cy_stc_ble_uds_descr_value_t.
*  .
*   Otherwise (if the UDS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*                                successfully written on the peer device.
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - If there is some trouble with the
*                                requested attribute on the peer device,
*                                the details are provided with an event parameter
*                                structure (cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_UDSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_uds_char_index_t charIndex,
                                                               cy_en_ble_uds_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to UDS client structure with attribute handles */
    cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];

    /* Check parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_UDS_CHAR_COUNT) || (descrIndex >= CY_BLE_UDS_DESCR_COUNT) ||
            (attrSize != CY_BLE_CCCD_LEN) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(udscPtr->charInfo[charIndex].descrHandle[descrIndex] == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;
            
        /* Fill all fields of Write Request structure ... */
        writeReqParam.handleValPair.attrHandle = udscPtr->charInfo[charIndex].descrHandle[descrIndex];
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = CY_BLE_CCCD_LEN;
        writeReqParam.connHandle               = connHandle;

        /* ... and send request to server device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            /* Save handle to support service-specific read response from device */
            cy_ble_udscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets the characteristic descriptor of the specified characteristic.
*
*  \param connHandle: The connection handle
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic.
*  \param descrIndex: The index of the service characteristic descriptor.
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular descriptor.
*   CY_BLE_ERROR_INVALID_OPERATION           | This operation is not permitted on the specified attribute.
*
* \events
*  In the case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*  If the UDS service-specific callback is registered
*      (with \ref Cy_BLE_UDS_RegisterAttrCallback):
*  * #CY_BLE_EVT_UDSC_READ_DESCR_RESPONSE - If the requested attribute is
*                                successfully read on the peer device,
*                                the details (charIndex, descrIndex, value, etc.)
*                                are provided with an event parameter structure
*                                of type cy_stc_ble_uds_descr_value_t.
*  .
*  Otherwise (if the UDS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*                                successfully read on the peer device,
*                                the details (handle, value, etc.) are
*                                provided with an event parameter
*                                structure (cy_stc_ble_gattc_read_rsp_param_t).
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - If there is some trouble with the
*                                requested attribute on the peer device,
*                                the details are provided with an event parameter
*                                structure (cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_UDSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_uds_char_index_t charIndex,
                                                               cy_en_ble_uds_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to UDS client structure with attribute handles */
    cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];

    /* Check parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_UDS_CHAR_COUNT) || (descrIndex >= CY_BLE_UDS_DESCR_COUNT) ||
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(udscPtr->charInfo[charIndex].descrHandle[descrIndex] == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = udscPtr->charInfo[charIndex].descrHandle[descrIndex];
        readReqParam.connHandle = connHandle;

        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_udscReqHandle[discIdx] = udscPtr->charInfo[charIndex].descrHandle[descrIndex];
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_UDSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* User Data service characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_udscCharUuid[CY_BLE_UDS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_FIRST_NAME,
        CY_BLE_UUID_CHAR_LAST_NAME,
        CY_BLE_UUID_CHAR_EMAIL_ADDRESS,
        CY_BLE_UUID_CHAR_AGE,
        CY_BLE_UUID_CHAR_DATE_OF_BIRTH,
        CY_BLE_UUID_CHAR_GENDER,
        CY_BLE_UUID_CHAR_WEIGHT,
        CY_BLE_UUID_CHAR_HEIGHT,
        CY_BLE_UUID_CHAR_VO2_MAX,
        CY_BLE_UUID_CHAR_HEART_RATE_MAX,
        CY_BLE_UUID_CHAR_RESTING_HEART_RATE,
        CY_BLE_UUID_CHAR_MRH,
        CY_BLE_UUID_CHAR_AEROBIC_THRESHOLD,
        CY_BLE_UUID_CHAR_ANAEROBIC_THRESHOLD,
        CY_BLE_UUID_CHAR_STP,
        CY_BLE_UUID_CHAR_DATE_OF_THRESHOLD_ASSESSMENT,
        CY_BLE_UUID_CHAR_WAIST_CIRCUMFERENCE,
        CY_BLE_UUID_CHAR_HIP_CIRCUNFERENCE,
        CY_BLE_UUID_CHAR_FBL,
        CY_BLE_UUID_CHAR_FBU,
        CY_BLE_UUID_CHAR_AEL,
        CY_BLE_UUID_CHAR_AEU,
        CY_BLE_UUID_CHAR_ANL,
        CY_BLE_UUID_CHAR_ANU,
        CY_BLE_UUID_CHAR_FIVE_ZONE_HEART_RATE_LIMITS,
        CY_BLE_UUID_CHAR_THREE_ZONE_HEART_RATE_LIMITS,
        CY_BLE_UUID_CHAR_TWO_ZONE_HEART_RATE_LIMIT,
        CY_BLE_UUID_CHAR_DATABASE_CHANGE_INCREMENT,
        CY_BLE_UUID_CHAR_USER_INDEX,
        CY_BLE_UUID_CHAR_USER_CONTROL_POINT,
        CY_BLE_UUID_CHAR_LANGUAGE
    };
    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t udsDiscIdx = cy_ble_udscConfigPtr->serviceDiscIdx;
    uint32_t i;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == udsDiscIdx))
    {
        /* Get pointer (with offset) to UDS client structure with attribute handles */
        cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];

        /* Update last characteristic endHandle to declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        for(i = 0u; i < (uint32_t)CY_BLE_UDS_CHAR_COUNT; i++)
        {
            if(cy_ble_udscCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                if(udscPtr->charInfo[i].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    udscPtr->charInfo[i].valueHandle = discCharInfo->valueHandle;
                    udscPtr->charInfo[i].properties = discCharInfo->properties;
                    lastEndHandle[discIdx] = &udscPtr->charInfo[i].endHandle;
                }
                else
                {
                    Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo->uuid);
                }
            }
        }

        /* Init characteristic endHandle to service endHandle.
         * Characteristic endHandle will be updated to the declaration
         * Handler of the following characteristic,
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
* Function Name: Cy_BLE_UDSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to the characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_UDSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t notSupportedDescr = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t udsDiscIdx = cy_ble_udscConfigPtr->serviceDiscIdx;
    uint32_t descIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == udsDiscIdx)
    {
        /* Get pointer (with offset) to UDS client structure with attribute handles */
        cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];

        switch(discDescrInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_CLIENT_CONFIG:
                descIdx = (uint32_t)CY_BLE_UDS_CCCD;
                break;

            default:
                /* Not supported descriptor */
                notSupportedDescr = 1u;
                break;
        }

        if(notSupportedDescr == 0u)
        {
            if(udscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] ==
               CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                udscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] =
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
* Function Name: Cy_BLE_UDSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_UDSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t udsDiscIdx = cy_ble_udscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == udsDiscIdx)
    {
        /* Get pointer (with offset) to UDS client structure with attribute handles */
        cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];

        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_UDS_CHAR_COUNT) && (exitFlag == 0u))
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            if((udscPtr->charInfo[charIdx].endHandle - udscPtr->charInfo[charIdx].valueHandle) != 0u)
            {
                /* Read characteristic range */
                charRangeInfo->range.startHandle = udscPtr->charInfo[charIdx].valueHandle + 1u;
                charRangeInfo->range.endHandle = udscPtr->charInfo[charIdx].endHandle;
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
* Function Name: Cy_BLE_UDSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles the Notification event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_UDSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
    
    /* Get pointer (with offset) to UDS client structure with attribute handles */
    cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_UDS_ApplCallback != NULL) &&
       (udscPtr->charInfo[CY_BLE_UDS_DCI].valueHandle == eventParam->handleValPair.attrHandle))
    {
        cy_stc_ble_uds_char_value_t locCharValue;
        
        locCharValue.connHandle = eventParam->connHandle;
        locCharValue.charIndex  = CY_BLE_UDS_DCI;
        locCharValue.value      = &eventParam->handleValPair.value;

        Cy_BLE_UDS_ApplCallback((uint32_t)CY_BLE_EVT_UDSC_NOTIFICATION, &locCharValue);
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_IndicationEventHandler
***************************************************************************//**
*
*  Handles the Indication event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_UDSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
    
    /* Get pointer (with offset) to UDS client structure with attribute handles */
    cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_UDS_ApplCallback != NULL))
    {
        if(udscPtr->charInfo[CY_BLE_UDS_UCP].valueHandle == eventParam->handleValPair.attrHandle)
        {
            cy_stc_ble_uds_char_value_t locCharValue;
            
            locCharValue.charIndex  = CY_BLE_UDS_UCP;
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.value      = &eventParam->handleValPair.value;
            
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_UDS_ApplCallback((uint32_t)CY_BLE_EVT_UDSC_INDICATION, &locCharValue);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles the Read Response event.
*
*  \param eventParam - The pointer to the data structure.
*
******************************************************************************/
static void Cy_BLE_UDSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (cy_ble_udscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        if(Cy_BLE_UDS_ApplCallback != NULL)
        {
            /* Get pointer (with offset) to UDS client structure with attribute handles */
            cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];

            cy_en_ble_uds_char_index_t i;

            for(i = CY_BLE_UDS_FNM; i < CY_BLE_UDS_CHAR_COUNT; i++)
            {
                if(udscPtr->charInfo[i].valueHandle == cy_ble_udscReqHandle[discIdx])
                {
                    cy_stc_ble_uds_char_value_t locCharValue;
                    
                    locCharValue.connHandle = eventParam->connHandle;
                    locCharValue.charIndex  = i;
                    locCharValue.value      = &eventParam->value;

                    Cy_BLE_UDS_ApplCallback((uint32_t)CY_BLE_EVT_UDSC_READ_CHAR_RESPONSE, &locCharValue);
                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                    break;
                }
            }

            if((cy_ble_eventHandlerFlag & CY_BLE_CALLBACK) != 0u)
            {
                for(i = CY_BLE_UDS_FNM; i < CY_BLE_UDS_CHAR_COUNT; i++)
                {
                    if(udscPtr->charInfo[i].descrHandle[CY_BLE_UDS_CCCD] == cy_ble_udscReqHandle[discIdx])
                    {
                        cy_stc_ble_uds_descr_value_t locDescrValue;
                        
                        locDescrValue.connHandle = eventParam->connHandle;
                        locDescrValue.charIndex  = i;
                        locDescrValue.descrIndex = CY_BLE_UDS_CCCD;
                        locDescrValue.value      = &eventParam->value;

                        Cy_BLE_UDS_ApplCallback((uint32_t)CY_BLE_EVT_UDSC_READ_DESCR_RESPONSE, &locDescrValue);
                        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                        break;
                    }
                }
            }
        }

        cy_ble_udscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_ReadLongRespEventHandler
***************************************************************************//**
*
*  Handles a Read Long Response event.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
*
******************************************************************************/
static void Cy_BLE_UDSC_ReadLongRespEventHandler(const cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    cy_stc_ble_gattc_stop_cmd_param_t stopCmdParam = { .connHandle = eventParam->connHandle };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_UDS_ApplCallback != NULL) &&
       (cy_ble_udscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to UDS client structure with attribute handles */
        cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];
        cy_stc_ble_uds_char_value_t locCharValue;
        cy_stc_ble_gatt_value_t locGattValue;
        uint32_t longCharIndex;
        cy_en_ble_uds_char_index_t longCharSet[CY_BLE_UDS_LONG_CHAR_NUM] ={ CY_BLE_UDS_FNM, CY_BLE_UDS_LNM,
                                                                            CY_BLE_UDS_EML, CY_BLE_UDS_LNG };
        locCharValue.connHandle = eventParam->connHandle;

        /* Go trough all long characteristics */
        for(longCharIndex = 0u; longCharIndex < CY_BLE_UDS_LONG_CHAR_NUM; longCharIndex++)
        {
            if(udscPtr->charInfo[longCharSet[longCharIndex]].valueHandle == cy_ble_udscReqHandle[discIdx])
            {
                uint32_t j;
                locCharValue.charIndex = longCharSet[longCharIndex];

                /* Update user buffer with received data */
                for(j = 0u; j < eventParam->value.len; j++)
                {
                    if(cy_ble_udscCurrLen[discIdx] < cy_ble_udscRdLongBuffLen[discIdx])
                    {
                        cy_ble_udscRdLongBuffPtr[discIdx][cy_ble_udscCurrLen[discIdx]] = eventParam->value.val[j];
                        cy_ble_udscCurrLen[discIdx]++;
                    }
                    else
                    {
                        (void)Cy_BLE_GATTC_StopCmd(&stopCmdParam);
                        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                        break;
                    }
                }

                if((cy_ble_eventHandlerFlag & CY_BLE_CALLBACK) != 0u)
                {
                    cy_stc_ble_gatt_xchg_mtu_param_t mtuParam =
                    {
                        .connHandle = eventParam->connHandle
                    };

                    (void)Cy_BLE_GATT_GetMtuSize(&mtuParam);

                    /* If received data length is less than MTU size, Read Long
                     * request is completed or provided user's buffer is full.
                     */
                    if(((mtuParam.mtu - 1u) > eventParam->value.len))
                    {
                        locGattValue.val = cy_ble_udscRdLongBuffPtr[discIdx];
                        locGattValue.len = cy_ble_udscCurrLen[discIdx];
                        locCharValue.value = &locGattValue;
                        Cy_BLE_UDS_ApplCallback((uint32_t)CY_BLE_EVT_UDSC_READ_CHAR_RESPONSE, &locCharValue);
                        cy_ble_udscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                    }

                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                }

                break;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles the Write Response event.
*
*  \param eventParam: The pointer to a data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_UDSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_UDS_ApplCallback != NULL) &&
       (cy_ble_udscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to UDS client structure with attribute handles */
        cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];

        if(udscPtr->charInfo[CY_BLE_UDS_UCP].valueHandle == cy_ble_udscReqHandle[discIdx])
        {
            cy_stc_ble_uds_char_value_t locCharIndex;
            
            locCharIndex.connHandle = *eventParam;
            locCharIndex.charIndex  = CY_BLE_UDS_UCP;
            locCharIndex.value      = NULL;

            cy_ble_udscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_UDS_ApplCallback((uint32_t)CY_BLE_EVT_UDSC_WRITE_CHAR_RESPONSE, &locCharIndex);
        }
        else
        {
            cy_en_ble_uds_char_index_t i;

            for(i = CY_BLE_UDS_FNM; i < CY_BLE_UDS_CHAR_COUNT; i++)
            {
                if(udscPtr->charInfo[i].descrHandle[CY_BLE_UDS_CCCD] == cy_ble_udscReqHandle[discIdx])
                {
                    cy_stc_ble_uds_descr_value_t locDescIndex;
                    
                    locDescIndex.connHandle = *eventParam;
                    locDescIndex.charIndex  = i;
                    locDescIndex.descrIndex = CY_BLE_UDS_CCCD;
                    locDescIndex.value      = NULL;

                    cy_ble_udscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                    Cy_BLE_UDS_ApplCallback((uint32_t)CY_BLE_EVT_UDSC_WRITE_DESCR_RESPONSE, &locDescIndex);
                    break;
                }
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_ExecuteWriteResponseEventHandler
***************************************************************************//**
*
*  Handles a Execute Write Response event for the User Data service.
*
*  \param eventParam: The pointer to a data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_UDSC_ExecuteWriteResponseEventHandler(const cy_stc_ble_gattc_exec_write_rsp_t *eventParam)
{
    uint32_t i;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_UDS_ApplCallback != NULL) &&
       (cy_ble_udscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to UDS client structure with attribute handles */
        cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];
    
        for(i = 0u; i < ((uint8_t)CY_BLE_UDS_CHAR_COUNT); i++)
        {
            if(udscPtr->charInfo[i].valueHandle == cy_ble_udscReqHandle[discIdx])
            {
                cy_stc_ble_uds_char_value_t locCharVal;
                
                locCharVal.connHandle = eventParam->connHandle;
                locCharVal.charIndex  = (cy_en_ble_uds_char_index_t)i;
                locCharVal.value      = NULL;

                cy_ble_udscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_UDS_ApplCallback((uint32_t)CY_BLE_EVT_UDSC_WRITE_CHAR_RESPONSE, &locCharVal);
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles the Error Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
*
******************************************************************************/
static void Cy_BLE_UDSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
        if(cy_ble_udscReqHandle[discIdx] == eventParam->errInfo.attrHandle)
        {
            if((eventParam->errInfo.opCode == CY_BLE_GATT_READ_BLOB_REQ) &&
               (eventParam->errInfo.errorCode == CY_BLE_GATT_ERR_ATTRIBUTE_NOT_LONG))
            {
                cy_stc_ble_gattc_read_req_t readReqParam;
                
                readReqParam.attrHandle = eventParam->errInfo.attrHandle;
                readReqParam.connHandle = eventParam->connHandle;

                (void)Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            }
            else
            {
                if(Cy_BLE_UDS_ApplCallback != NULL)
                {
                    /* Get pointer (with offset) to UDS client structure with attribute handles */
                    cy_stc_ble_udsc_t *udscPtr = (cy_stc_ble_udsc_t *)&cy_ble_udscConfigPtr->attrInfo[discIdx];

                    cy_stc_ble_uds_char_value_t locGattError;
                    
                    locGattError.gattErrorCode = eventParam->errInfo.errorCode;
                    locGattError.connHandle    = eventParam->connHandle;
                    locGattError.value         = NULL;

                    for(locGattError.charIndex = CY_BLE_UDS_FNM; locGattError.charIndex < CY_BLE_UDS_CHAR_COUNT;
                        locGattError.charIndex++)
                    {
                        if(udscPtr->charInfo[locGattError.charIndex].valueHandle ==
                           eventParam->errInfo.attrHandle)
                        {
                            Cy_BLE_UDS_ApplCallback((uint32_t)CY_BLE_EVT_UDSC_ERROR_RESPONSE, &locGattError);
                            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                            break;
                        }
                    }
                }

                cy_ble_udscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_UDS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the UDS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_UDS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_UDSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_UDSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_UDSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_UDSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_UDSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the UDS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_UDSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            (void)Cy_BLE_UDSS_Init(cy_ble_udssConfigPtr);
            break;

        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_UDSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            Cy_BLE_UDSS_ConfirmationEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_PREP_WRITE_REQ:
            Cy_BLE_UDSS_PrepareWriteRequestEventHandler((cy_stc_ble_gatts_prep_write_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_EXEC_WRITE_REQ:
            Cy_BLE_UDSS_ExecuteWriteRequestEventHandler((cy_stc_ble_gatts_exec_write_req_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_READ_CHAR_VAL_ACCESS_REQ:
            Cy_BLE_UDSS_ReadRequestEventHandler((cy_stc_ble_gatts_char_val_read_req_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_UDSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the UDS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_UDSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
           /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_UDSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_UDSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_UDSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_UDSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_UDSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_IND:
                Cy_BLE_UDSC_IndicationEventHandler((cy_stc_ble_gattc_handle_value_ind_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_UDSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_UDSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_BLOB_RSP:
                Cy_BLE_UDSC_ReadLongRespEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_EXEC_WRITE_RSP:
                Cy_BLE_UDSC_ExecuteWriteResponseEventHandler((cy_stc_ble_gattc_exec_write_rsp_t*)eventParam);
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
