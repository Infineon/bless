/***************************************************************************//**
* \file cy_ble_gls.c
* \version 3.60
*
* \brief
*  This file contains the source code for the Glucose service.
*
********************************************************************************
* \copyright
* Copyright 2017-2021, Cypress Semiconductor Corporation.  All rights reserved.
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

#define CY_BLE_GLS_IS_GMC_SUPPORTED \
    (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_glssConfigPtr->attrInfo->charInfo[CY_BLE_GLS_GLMC].charHandle)

#define CY_BLE_GLS_IS_PROCEDURE_IN_PROGRESS \
    (0u != (CY_BLE_GLS_FLAG_PROCESS & cy_ble_glsFlag))


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/
    
static cy_en_ble_gatt_err_code_t Cy_BLE_GLS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_GLSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_GLSC_EventHandler(uint32_t eventCode, void *eventParam);

static void Cy_BLE_GLSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_GLSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_GLSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_GLSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_GLSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_GLSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_GLSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam);
static void Cy_BLE_GLSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_GLSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_GLSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_GLS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_GLSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_GLSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response for server */
static cy_ble_gatt_db_attr_handle_t cy_ble_glssReqHandle;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_glscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE GLS server config structure */
const cy_stc_ble_glss_config_t *cy_ble_glssConfigPtr = NULL;

/* The pointer to a global BLE GLS client config structure */
const cy_stc_ble_glsc_config_t *cy_ble_glscConfigPtr = NULL;

uint8_t cy_ble_glsFlag;


/******************************************************************************
* Function Name: Cy_BLE_GLSS_Init
***************************************************************************//**
*
*  This function initializes server of the Glucose service.
*
*  \param config: Configuration structure for the GLS.
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
cy_en_ble_api_result_t Cy_BLE_GLSS_Init(const cy_stc_ble_glss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_glssConfigPtr = config;

        /* Registers event handler for the GLS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_GLS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_GLSS_EventHandlerCallback = &Cy_BLE_GLSS_EventHandler;

        cy_ble_glssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        cy_ble_glsFlag = 0u;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GLSC_Init
***************************************************************************//**
*
*  This function initializes client of the Glucose service.
*
*  \param config: Configuration structure for the GLS.
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
cy_en_ble_api_result_t Cy_BLE_GLSC_Init(const cy_stc_ble_glsc_config_t *config)
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
        cy_ble_glscConfigPtr = config;

        /* Registers event handler for the GLS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_GLS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_GLSC_EventHandlerCallback = &Cy_BLE_GLSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 glsServIdx = cy_ble_glscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + glsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to GLS client structure */
                cy_stc_ble_glsc_t *glscPtr = (cy_stc_ble_glsc_t *)&cy_ble_glscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(glscPtr, 0, sizeof(cy_stc_ble_glsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + glsServIdx].uuid =
                    CY_BLE_UUID_GLUCOSE_SERVICE;
            }

            cy_ble_glscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GLS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for GLS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_GLSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_gls_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_GLS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_GLS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_GLSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a characteristic value of the service, which is identified by charIndex.
*
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_gls_char_index_t.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param *attrValue: The pointer to the characteristic value data that should be
*                     stored in the GATT database.
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
cy_en_ble_api_result_t Cy_BLE_GLSS_SetCharacteristicValue(cy_en_ble_gls_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check the parameters */
    if(charIndex >= CY_BLE_GLS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_glssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Store characteristic value into GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_glssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_GLSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets a characteristic value of Glucose service. The value is
*  identified by charIndex.
*
*  \param charIndex:  The index of the service characteristic of 
*                     type \ref cy_en_ble_gls_char_index_t.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param *attrValue: Pointer to the location where Characteristic value data should
*                     be stored.
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
cy_en_ble_api_result_t Cy_BLE_GLSS_GetCharacteristicValue(cy_en_ble_gls_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check the parameters */
    if(charIndex >= CY_BLE_GLS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_glssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Get characteristic value from GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_glssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_GLSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of the specified characteristic of Glucose 
*  service.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type 
*                     \ref cy_en_ble_gls_char_index_t.
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_gls_descr_index_t.
*  \param attrSize:   The size of the descriptor value attribute.
*  \param *attrValue: Pointer to the location where the descriptor value
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
cy_en_ble_api_result_t Cy_BLE_GLSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_gls_char_index_t charIndex,
                                                               cy_en_ble_gls_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check the parameters */
    if((charIndex >= CY_BLE_GLS_CHAR_COUNT) || (descrIndex >= CY_BLE_GLS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_glssConfigPtr->attrInfo->charInfo[charIndex].cccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Get data from database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_glssConfigPtr->attrInfo->charInfo[charIndex].cccdHandle;
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
* Function Name: Cy_BLE_GLSS_SendNotification
***************************************************************************//**
*
*  Sends a notification of the specified characteristic to the client device,
*  as defined by the charIndex value.
*  On enabling notification successfully for a service characteristic it sends out a
*  'Handle Value Notification' which results in \ref CY_BLE_EVT_GLSC_NOTIFICATION
*  event at the GATT Client's end.
*
*  \param connHandle: The connection handle which consist of the device ID and ATT
*                     connection ID.
*  \param charIndex:  The index of the service characteristic of 
                      type \ref cy_en_ble_gls_char_index_t.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param *attrValue: Pointer to the Characteristic value data that should be sent
*                     to Client device.
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
cy_en_ble_api_result_t Cy_BLE_GLSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_gls_char_index_t charIndex,
                                                    uint8_t attrSize,
                                                    uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send Notification if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_GLS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_glssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if(!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_glssConfigPtr->attrInfo->charInfo[charIndex].cccdHandle))
    {
        apiResult = CY_BLE_ERROR_NTF_DISABLED;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
        ntfReqParam.handleValPair.attrHandle = cy_ble_glssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        ntfReqParam.handleValPair.value.val  = attrValue;
        ntfReqParam.handleValPair.value.len  = attrSize;
        ntfReqParam.connHandle               = connHandle;
        
        /* Send notification to client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Notification(&ntfReqParam);
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GLSS_SendIndication
***************************************************************************//**
*
*  Sends an indication of the specified characteristic to the client device, as
*  defined by the charIndex value.
*
*  On enabling indication successfully it sends out a 'Handle Value Indication' which
*  results in #CY_BLE_EVT_GLSC_INDICATION or #CY_BLE_EVT_GATTC_HANDLE_VALUE_IND (if
*  service-specific callback function is not registered) event at the GATT Client's end.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
                      type \ref cy_en_ble_gls_char_index_t.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param *attrValue: Pointer to the Characteristic value data that should be sent
*                     to Client device.
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
*   CY_BLE_ERROR_IND_DISABLED                | Indication is not enabled by the client.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the GLS service-specific callback is registered
*   with \ref Cy_BLE_GLS_RegisterAttrCallback():
*  * \ref CY_BLE_EVT_GLSS_INDICATION_CONFIRMED - In case if the indication is
*    successfully delivered to the peer device.
*  .
*   Otherwise (if the GLS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF - In case if the indication is
*    successfully delivered to the peer device.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GLSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_gls_char_index_t charIndex,
                                                  uint8_t attrSize,
                                                  uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send Indication if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_GLS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_glssConfigPtr->attrInfo->charInfo[charIndex].cccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if(!CY_BLE_IS_INDICATION_ENABLED(connHandle.attId, cy_ble_glssConfigPtr->attrInfo->charInfo[charIndex].cccdHandle))
    {
        apiResult = CY_BLE_ERROR_IND_DISABLED;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gatts_handle_value_ind_t indParam;
        indParam.handleValPair.attrHandle = cy_ble_glssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        indParam.handleValPair.value.val  = attrValue;
        indParam.handleValPair.value.len  = attrSize;
        indParam.connHandle               = connHandle;

        /* Send notification to client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Indication(&indParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            /* Save handle to support service-specific value confirmation response from client */
            cy_ble_glssReqHandle = indParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GLSS_ConfirmationEventHandler
***************************************************************************//**
*
*  Handles the Value Confirmation request event from the BLE Stack.
*
*  \param *eventParam: Pointer to a structure of type cy_stc_ble_conn_handle_t
*
******************************************************************************/
static void Cy_BLE_GLSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    if((Cy_BLE_GLS_ApplCallback != NULL) &&
       (cy_ble_glssConfigPtr->attrInfo->charInfo[CY_BLE_GLS_RACP].charHandle == cy_ble_glssReqHandle))
    {
        cy_stc_ble_gls_char_value_t locCharIndex;
        locCharIndex.connHandle = *eventParam;
        locCharIndex.charIndex  = CY_BLE_GLS_RACP;
        locCharIndex.value      = NULL;

        cy_ble_glssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        cy_ble_glsFlag &= (uint8_t) ~CY_BLE_GLS_FLAG_PROCESS;

        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        Cy_BLE_GLS_ApplCallback((uint32_t)CY_BLE_EVT_GLSS_INDICATION_CONFIRMED, &locCharIndex);
    }
}


/******************************************************************************
* Function Name: Cy_BLE_GLSS_WriteEventHandler
***************************************************************************//**
*
*  Handles the Write Request event.
*
*  \param *eventParam: The pointer to the data structure specified by the event.
*
* \return
*  cy_en_ble_gatt_err_code_t - A function result state if it succeeded
*  (CY_BLE_GATT_ERR_NONE) or failed with error codes:
*   * CY_BLE_GATTS_ERR_PROCEDURE_ALREADY_IN_PROGRESS
*   * CY_BLE_GATTS_ERR_CCCD_IMPROPERLY_CONFIGURED
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_GLSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    cy_stc_ble_gls_char_value_t locCharValue = { .connHandle = eventParam->connHandle };

    if(Cy_BLE_GLS_ApplCallback != NULL)
    {
        if(eventParam->handleValPair.attrHandle == cy_ble_glssConfigPtr->attrInfo->charInfo[CY_BLE_GLS_RACP].charHandle)
        {
            if(CY_BLE_GLS_IS_PROCEDURE_IN_PROGRESS
               && (CY_BLE_GLS_RACP_OPCODE_ABORT != eventParam->handleValPair.value.val[0u]))
            {
                gattErr = CY_BLE_GATTS_ERR_PROCEDURE_ALREADY_IN_PROGRESS;
            }
            else if(!CY_BLE_IS_INDICATION_ENABLED(eventParam->connHandle.attId,
                                                  cy_ble_glssConfigPtr->attrInfo->charInfo[CY_BLE_GLS_RACP].cccdHandle))
            {
                gattErr = CY_BLE_GATTS_ERR_CCCD_IMPROPERLY_CONFIGURED;
            }
            else
            {
                cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                dbAttrValInfo.connHandle      = eventParam->connHandle;
                dbAttrValInfo.offset          = 0u;
                dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

                locCharValue.charIndex = CY_BLE_GLS_RACP;
                locCharValue.value = &eventParam->handleValPair.value;

                gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
                if(gattErr == CY_BLE_GATT_ERR_NONE)
                {
                    Cy_BLE_GLS_ApplCallback((uint32_t)CY_BLE_EVT_GLSS_WRITE_CHAR, &locCharValue);
                    cy_ble_glsFlag |= CY_BLE_GLS_FLAG_PROCESS;
                }
            }

            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else
        {
            for(locCharValue.charIndex = CY_BLE_GLS_GLMT;
                locCharValue.charIndex < CY_BLE_GLS_CHAR_COUNT; locCharValue.charIndex++)
            {
                if(eventParam->handleValPair.attrHandle ==
                   cy_ble_glssConfigPtr->attrInfo->charInfo[locCharValue.charIndex].cccdHandle)
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
                        if(locCharValue.charIndex < CY_BLE_GLS_GLFT)
                        {
                            if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                            {
                                Cy_BLE_GLS_ApplCallback((uint32_t)CY_BLE_EVT_GLSS_NOTIFICATION_ENABLED, &locCharValue);
                            }
                            else
                            {
                                Cy_BLE_GLS_ApplCallback((uint32_t)CY_BLE_EVT_GLSS_NOTIFICATION_DISABLED, &locCharValue);
                            }
                        }
                        else
                        {
                            if(CY_BLE_IS_INDICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                            {
                                Cy_BLE_GLS_ApplCallback((uint32_t)CY_BLE_EVT_GLSS_INDICATION_ENABLED, &locCharValue);
                            }
                            else
                            {
                                Cy_BLE_GLS_ApplCallback((uint32_t)CY_BLE_EVT_GLSS_INDICATION_DISABLED, &locCharValue);
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
* Function Name: Cy_BLE_GLSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result a Write Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the #CY_BLE_EVT_GLSS_WRITE_CHAR event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
*                     type \ref cy_en_ble_gls_char_index_t.
*  \param attrSize: The size of the characteristic value attribute.
*  \param *attrValue: The pointer to the characteristic value data that should be
*               sent to the server device.
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
*   If the GLS service-specific callback is registered
*   with \ref Cy_BLE_GLS_RegisterAttrCallback():
*   * #CY_BLE_EVT_GLSC_WRITE_CHAR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_gls_char_value_t.
*   .
*   Otherwise (if the GLS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - In case if the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*     requested attribute on the peer device, the details are provided with event
*     parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GLSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_gls_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    
    /* Get pointer (with offset) to GLS client structure with attribute handles */
    cy_stc_ble_glsc_t *glscPtr = (cy_stc_ble_glsc_t *)&cy_ble_glscConfigPtr->attrInfo[discIdx];

    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_GLS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(glscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_WRITE & glscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle = glscPtr->charInfo[charIndex].valueHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;
            
        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_glscReqHandle[discIdx] = glscPtr->charInfo[charIndex].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GLSC_GetCharacteristicValue
***************************************************************************//**
*
*  This function is used to read the characteristic Value from a server
*  which is identified by charIndex.
*
*  Sends a request to the peer device to get a characteristic value, as
*  identified by its charIndex.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
*                     type \ref cy_en_ble_gls_char_index_t.
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the GLS service-specific callback is registered
*      with \ref Cy_BLE_GLS_RegisterAttrCallback():
*   * #CY_BLE_EVT_GLSC_READ_CHAR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details 
*     (charIndex , value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_gls_char_value_t.
*   .
*   Otherwise (if an GLS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_GLSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_gls_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    
    /* Get pointer (with offset) to GLS client structure with attribute handles */
    cy_stc_ble_glsc_t *glscPtr = (cy_stc_ble_glsc_t *)&cy_ble_glscConfigPtr->attrInfo[discIdx];

    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_GLS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(glscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_READ & glscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = glscPtr->charInfo[charIndex].valueHandle;
        readReqParam.connHandle = connHandle;
       
        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_glscReqHandle[discIdx] = glscPtr->charInfo[charIndex].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GLSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to the peer device to set the characteristic descriptor of the
*  specified characteristic of Glucose service.
*
*  Internally, Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*  * \ref CY_BLE_EVT_GLSS_INDICATION_ENABLED
*  * \ref CY_BLE_EVT_GLSS_INDICATION_DISABLED
*  * \ref CY_BLE_EVT_GLSS_NOTIFICATION_ENABLED
*  * \ref CY_BLE_EVT_GLSS_NOTIFICATION_DISABLED
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type 
*                     \ref cy_en_ble_gls_char_index_t.
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_gls_descr_index_t.
*  \param attrSize:   The size of the characteristic descriptor value attribute.
*  \param *attrValue: Pointer to the characteristic descriptor value data that should
*                     be sent to the server device.
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
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the GLS service-specific callback is registered
*   with \ref Cy_BLE_GLS_RegisterAttrCallback():
*   * #CY_BLE_EVT_GLSC_WRITE_DESCR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, descrIndex etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_gls_descr_value_t.
*   .
*   Otherwise (if an GLS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure ( \ref cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_GLSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_gls_char_index_t charIndex,
                                                               cy_en_ble_gls_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to GLS client structure with attribute handles */
    cy_stc_ble_glsc_t *glscPtr = (cy_stc_ble_glsc_t *)&cy_ble_glscConfigPtr->attrInfo[discIdx];

    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_GLS_CHAR_COUNT) || (descrIndex >= CY_BLE_GLS_DESCR_COUNT) ||
            (attrSize != CY_BLE_CCCD_LEN) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(glscPtr->charInfo[charIndex].cccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle = glscPtr->charInfo[charIndex].cccdHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = CY_BLE_CCCD_LEN;
        writeReqParam.connHandle               = connHandle;
        
        /* ... and send request to server device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            /* Save handle to support service-specific read response from device */
            cy_ble_glscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GLSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets the characteristic descriptor of the specified characteristic.
*  Sends a request to the peer device to get the characteristic descriptor of the
*  specified characteristic of Glucose service.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type 
*                     \ref cy_en_ble_gls_char_index_t.
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_gls_descr_index_t.
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular descriptor
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the GLS service-specific callback is registered
*   with \ref Cy_BLE_GLS_RegisterAttrCallback():
*   * #CY_BLE_EVT_GLSC_READ_DESCR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     descrIndex, value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_gls_descr_value_t.
*   .
*   Otherwise (if an GLS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_GLSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_gls_char_index_t charIndex,
                                                               cy_en_ble_gls_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to GLS client structure with attribute handles */
    cy_stc_ble_glsc_t *glscPtr = (cy_stc_ble_glsc_t *)&cy_ble_glscConfigPtr->attrInfo[discIdx];

    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_GLS_CHAR_COUNT) || (descrIndex >= CY_BLE_GLS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(glscPtr->charInfo[charIndex].cccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = glscPtr->charInfo[charIndex].cccdHandle;
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_glscReqHandle[discIdx] = glscPtr->charInfo[charIndex].cccdHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_GLSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_GLSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* Glucose service characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_glscCharUuid[CY_BLE_GLS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_GLUCOSE_MSRMNT,
        CY_BLE_UUID_CHAR_GL_MSRMT_CTX,
        CY_BLE_UUID_CHAR_GL_FEATURE,
        CY_BLE_UUID_CHAR_RACP
    };
    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t glsDiscIdx = cy_ble_glscConfigPtr->serviceDiscIdx;
    uint32_t i;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == glsDiscIdx))
    {
        /* Update last characteristic endHandle to declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        for(i = 0u; i < (uint32_t)CY_BLE_GLS_CHAR_COUNT; i++)
        {
            if(cy_ble_glscCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                /* Get pointer (with offset) to GLS client structure with attribute handles */
                cy_stc_ble_glsc_t *glscPtr = (cy_stc_ble_glsc_t *)&cy_ble_glscConfigPtr->attrInfo[discIdx];

                if(glscPtr->charInfo[i].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    glscPtr->charInfo[i].valueHandle = discCharInfo->valueHandle;
                    glscPtr->charInfo[i].properties = discCharInfo->properties;
                    lastEndHandle[discIdx] = &glscPtr->charInfo[i].endHandle;
                }
                else
                {
                    Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo);
                }
            }
        }

        /* Init characteristic endHandle to service endHandle.
         * Characteristic endHandle will be updated to the declaration
         * Handler of the following characteristic,
         * in the following characteristic discovery procedure. */
        if(lastEndHandle[discIdx] != NULL)
        {
            uint32_t discServiNum = cy_ble_configPtr->context->discServiCount;
            *lastEndHandle[discIdx] =
                cy_ble_configPtr->context->serverInfo[(discIdx * discServiNum) + glsDiscIdx].range.endHandle;
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_GLSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param *discDescrInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_GLSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t glsDiscIdx = cy_ble_glscConfigPtr->serviceDiscIdx;
    
    if(cy_ble_configPtr->context->discovery[discIdx].servCount == glsDiscIdx)
    {
        if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_CLIENT_CONFIG)
        {
            /* Get pointer (with offset) to GLS client structure with attribute handles */
            cy_stc_ble_glsc_t *glscPtr = (cy_stc_ble_glsc_t *)&cy_ble_glscConfigPtr->attrInfo[discIdx];

            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            if(glscPtr->charInfo[charIdx].cccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                glscPtr->charInfo[charIdx].cccdHandle = discDescrInfo->descrHandle;
            }
            else    /* Duplication of descriptor */
            {
                Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_DESCR_DUPLICATION, &discDescrInfo);
            }
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_GLSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_GLSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t glsDiscIdx = cy_ble_glscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == glsDiscIdx)
    {
        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_GLS_CHAR_COUNT) && (exitFlag == 0u))
        {
            /* Get pointer (with offset) to GLS client structure with attribute handles */
            cy_stc_ble_glsc_t *glscPtr = (cy_stc_ble_glsc_t *)&cy_ble_glscConfigPtr->attrInfo[discIdx];

            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            if((glscPtr->charInfo[charIdx].endHandle - glscPtr->charInfo[charIdx].valueHandle) != 0u)
            {
                /* Read characteristic range */
                charRangeInfo->range.startHandle = glscPtr->charInfo[charIdx].valueHandle + 1u;
                charRangeInfo->range.endHandle = glscPtr->charInfo[charIdx].endHandle;
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
* Function Name: Cy_BLE_GLSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles the Notification event.
*
*  \param *eventParam: The pointer to a data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_GLSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_GLS_ApplCallback != NULL))
    {
        cy_stc_ble_gls_char_value_t locCharValue = { .connHandle = eventParam->connHandle };
        
        /* Get pointer (with offset) to GLS client structure with attribute handles */
        cy_stc_ble_glsc_t *glscPtr = (cy_stc_ble_glsc_t *)&cy_ble_glscConfigPtr->attrInfo[discIdx];

        if(glscPtr->charInfo[CY_BLE_GLS_GLMT].valueHandle == eventParam->handleValPair.attrHandle)
        {
            locCharValue.charIndex = CY_BLE_GLS_GLMT;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else if(glscPtr->charInfo[CY_BLE_GLS_GLMC].valueHandle == eventParam->handleValPair.attrHandle)
        {
            locCharValue.charIndex = CY_BLE_GLS_GLMC;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else
        {
            /* Glucose service doesn't support any other notifications */
        }

        if((cy_ble_eventHandlerFlag & CY_BLE_CALLBACK) == 0u)
        {
            locCharValue.value = &eventParam->handleValPair.value;
            Cy_BLE_GLS_ApplCallback((uint32_t)CY_BLE_EVT_GLSC_NOTIFICATION, &locCharValue);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_GLSC_IndicationEventHandler
***************************************************************************//**
*
*  Handles the Indication event.
*
*  \param *eventParam:  the pointer to a data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_GLSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_GLS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to GLS client structure with attribute handles */
        cy_stc_ble_glsc_t *glscPtr = (cy_stc_ble_glsc_t *)&cy_ble_glscConfigPtr->attrInfo[discIdx];

        if(glscPtr->charInfo[CY_BLE_GLS_RACP].valueHandle == eventParam->handleValPair.attrHandle)
        {
            cy_stc_ble_gls_char_value_t locCharValue;
            locCharValue.charIndex  = CY_BLE_GLS_RACP;
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.value      = &eventParam->handleValPair.value;

            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_GLS_ApplCallback((uint32_t)CY_BLE_EVT_GLSC_INDICATION, &locCharValue);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_GLSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles the Read Response event.
*
*  \param *eventParam: The pointer to the data structure.
*
******************************************************************************/
static void Cy_BLE_GLSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_GLS_ApplCallback != NULL) &&
       (cy_ble_glscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to GLS client structure with attribute handles */
        cy_stc_ble_glsc_t *glscPtr = (cy_stc_ble_glsc_t *)&cy_ble_glscConfigPtr->attrInfo[discIdx];

        if(glscPtr->charInfo[CY_BLE_GLS_GLFT].valueHandle == cy_ble_glscReqHandle[discIdx])
        {
            cy_stc_ble_gls_char_value_t locCharValue;
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.charIndex  = CY_BLE_GLS_GLFT;
            locCharValue.value      = &eventParam->value;
            
            cy_ble_glscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_GLS_ApplCallback((uint32_t)CY_BLE_EVT_GLSC_READ_CHAR_RESPONSE, &locCharValue);
        }
        else
        {
            cy_en_ble_gls_char_index_t i;

            for(i = CY_BLE_GLS_GLMT; i < CY_BLE_GLS_CHAR_COUNT; i++)
            {
                if(glscPtr->charInfo[i].cccdHandle == cy_ble_glscReqHandle[discIdx])
                {
                    cy_stc_ble_gls_descr_value_t locDescrValue;
                    locDescrValue.connHandle = eventParam->connHandle;
                    locDescrValue.charIndex  = i;
                    locDescrValue.descrIndex = CY_BLE_GLS_CCCD;
                    locDescrValue.value      = &eventParam->value;
           
                    cy_ble_glscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                    Cy_BLE_GLS_ApplCallback((uint32_t)CY_BLE_EVT_GLSC_READ_DESCR_RESPONSE, &locDescrValue);
                    break;
                }
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_GLSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles the Write Response event.
*
*  \param *eventParam: The pointer to a data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_GLSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_GLS_ApplCallback != NULL) &&
       (cy_ble_glscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to GLS client structure with attribute handles */
        cy_stc_ble_glsc_t *glscPtr = (cy_stc_ble_glsc_t *)&cy_ble_glscConfigPtr->attrInfo[discIdx];

        if(glscPtr->charInfo[CY_BLE_GLS_RACP].valueHandle == cy_ble_glscReqHandle[discIdx])
        {
            cy_stc_ble_gls_char_value_t locCharIndex;
            locCharIndex.connHandle = *eventParam;
            locCharIndex.charIndex  = CY_BLE_GLS_RACP;
            locCharIndex.value      = NULL;
            
            cy_ble_glscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_GLS_ApplCallback((uint32_t)CY_BLE_EVT_GLSC_WRITE_CHAR_RESPONSE, &locCharIndex);
        }
        else
        {
            cy_en_ble_gls_char_index_t i;

            for(i = CY_BLE_GLS_GLMT; i < CY_BLE_GLS_CHAR_COUNT; i++)
            {
                if(glscPtr->charInfo[i].cccdHandle == cy_ble_glscReqHandle[discIdx])
                {
                    cy_stc_ble_gls_descr_value_t locDescIndex;
                    locDescIndex.connHandle = *eventParam;
                    locDescIndex.charIndex  = i;
                    locDescIndex.descrIndex = CY_BLE_GLS_CCCD;
                    locDescIndex.value      = NULL;
                    
                    cy_ble_glscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                    Cy_BLE_GLS_ApplCallback((uint32_t)CY_BLE_EVT_GLSC_WRITE_DESCR_RESPONSE, &locDescIndex);
                    break;
                }
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_GLSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles the Error Response event.
*
*  \param *eventParam: Pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_GLSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
        if(eventParam->errInfo.attrHandle == cy_ble_glscReqHandle[discIdx])
        {
            cy_ble_glscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}



/******************************************************************************
* Function Name: Cy_BLE_GLS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the Glucose service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_GLS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_GLSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_GLSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_GLSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_GLSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_GLSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the Glucose service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_GLSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            (void)Cy_BLE_GLSS_Init(cy_ble_glssConfigPtr);
            break;
            
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_GLSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            Cy_BLE_GLSS_ConfirmationEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_GLSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the Glucose service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_GLSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_GLSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_GLSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_GLSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_GLSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_GLSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_IND:
                Cy_BLE_GLSC_IndicationEventHandler((cy_stc_ble_gattc_handle_value_ind_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_GLSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_GLSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
