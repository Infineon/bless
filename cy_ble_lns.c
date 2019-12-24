/***************************************************************************//**
* \file cy_ble_lns.c
* \version 3.30
*
* \brief
*  This file contains the source code for Location and Navigation service.
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

#define CY_BLE_LNS_IS_PQ_SUPPORTED \
    (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_lnssConfigPtr->attrInfo->charInfo[CY_BLE_LNS_PQ].charHandle)

#define CY_BLE_LNS_IS_CP_SUPPORTED \
    (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_lnssConfigPtr->attrInfo->charInfo[CY_BLE_LNS_CP].charHandle)

#define CY_BLE_LNS_IS_NV_SUPPORTED \
    (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_lnssConfigPtr->attrInfo->charInfo[CY_BLE_LNS_NV].charHandle)

#define CY_BLE_LNS_IS_PROCEDURE_IN_PROGRESS \
    (0u != (CY_BLE_LNS_FLAG_PROCESS & cy_ble_lnsFlag))

    
/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_LNS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_LNSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_LNSC_EventHandler(uint32_t eventCode, void *eventParam);

static void Cy_BLE_LNSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_LNSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_LNSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_LNSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t * discDescrInfo);
static void Cy_BLE_LNSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_LNSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_LNSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam);
static void Cy_BLE_LNSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_LNSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_LNSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_LNS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_LNSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_LNSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response for server */
static cy_ble_gatt_db_attr_handle_t cy_ble_lnssReqHandle;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_lnscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE LNS server config structure */
const cy_stc_ble_lnss_config_t *cy_ble_lnssConfigPtr = NULL;

/* The pointer to a global BLE LNS client config structure */
const cy_stc_ble_lnsc_config_t *cy_ble_lnscConfigPtr = NULL;

uint8_t cy_ble_lnsFlag;


/******************************************************************************
* Function Name: Cy_BLE_LNSS_Init
***************************************************************************//**
*
*  This function initializes server of the Location and Navigation service.
*
*  \param config: Configuration structure for the LNS.
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
cy_en_ble_api_result_t Cy_BLE_LNSS_Init(const cy_stc_ble_lnss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_lnssConfigPtr = config;

        /* Registers event handler for the LNS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_LNS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_LNSS_EventHandlerCallback = &Cy_BLE_LNSS_EventHandler;

        cy_ble_lnssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        cy_ble_lnsFlag = 0u;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_LNSC_Init
***************************************************************************//**
*
*  This function initializes client of the Location and Navigation service.
*
*  \param config: Configuration structure for the LNS.
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
cy_en_ble_api_result_t Cy_BLE_LNSC_Init(const cy_stc_ble_lnsc_config_t *config)
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
        cy_ble_lnscConfigPtr = config;

        /* Registers event handler for the LNS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_LNS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_LNSC_EventHandlerCallback = &Cy_BLE_LNSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 lnsServIdx = cy_ble_lnscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + lnsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to LNS client structure */
                cy_stc_ble_lnsc_t *lnscPtr = (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(lnscPtr, 0, sizeof(cy_stc_ble_lnsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + lnsServIdx].uuid =
                    CY_BLE_UUID_LOCATION_AND_NAVIGATION_SERVICE;
            }

            cy_ble_lnscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_LNS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for LNS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_LNSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_lns_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_LNS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_LNS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_LNSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets the value of the characteristic, as identified by charIndex.
*
*  \param charIndex: The index of the service characteristic.
*  \param attrSize: The size of the characteristic value attribute.
*  \param attrValue: The pointer to the characteristic value data that should be
*                   stored to the GATT database.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Optional characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_LNSS_SetCharacteristicValue(cy_en_ble_lns_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check the parameters */
    if(charIndex >= CY_BLE_LNS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_lnssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        /* Store characteristic value into GATT database */

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_lnssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_LNSS_GetCharacteristicValue
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
*   CY_BLE_SUCCESS                           | Characteristic value was read successfully
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_LNSS_GetCharacteristicValue(cy_en_ble_lns_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check the parameters */
    if(charIndex >= CY_BLE_LNS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_lnssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        /* Get characteristic value from GATT database */

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_lnssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_LNSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of the specified characteristic.
*
*  \param connHandle: The connection handle which consist of the device ID and ATT
*                       connection ID.
*  \param charIndex:  The index of the characteristic.
*  \param descrIndex: The index of the descriptor.
*  \param attrSize:   The size of the descriptor value attribute.
*  \param attrValue: The pointer to the location where characteristic descriptor value
*                       data should be stored.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | characteristic descriptor value was read successfully
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_LNSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_lns_char_index_t charIndex,
                                                               cy_en_ble_lns_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check the parameters */
    if((charIndex >= CY_BLE_LNS_CHAR_COUNT) || (descrIndex >= CY_BLE_LNS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(((charIndex == CY_BLE_LNS_CP) && (!CY_BLE_LNS_IS_CP_SUPPORTED)) ||
            ((charIndex == CY_BLE_LNS_NV) && (!CY_BLE_LNS_IS_NV_SUPPORTED)))
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        /* Get data from database */
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_lnssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex];
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
* Function Name: Cy_BLE_LNSS_SendNotification
***************************************************************************//**
*
*  Sends a notification of the specified characteristic value, as identified by
*  the charIndex.
*
*  On enabling notification successfully for a service characteristic it sends out a
*  'Handle Value Notification' which results in #CY_BLE_EVT_LNSC_NOTIFICATION event
*  at the GATT Client's end.
*
*  \param connHandle: The connection handle which consist of the device ID and ATT
*              connection ID.
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue: The pointer to the characteristic value data that should be sent
*              to the client device.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Optional characteristic is absent.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_NTF_DISABLED                | Notification is not enabled by the client.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_LNSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_lns_char_index_t charIndex,
                                                    uint8_t attrSize,
                                                    uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send Notification if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_LNS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_lnssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[CY_BLE_LNS_CCCD] ==
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if(!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_lnssConfigPtr->attrInfo->charInfo[charIndex].
                                             descrHandle[CY_BLE_LNS_CCCD]))
    {
        apiResult = CY_BLE_ERROR_NTF_DISABLED;
    }
    else
    {
        cy_stc_ble_gatts_handle_value_ntf_t ntfParam;
        
        /* Fill all fields of Write Request structure ... */
        ntfParam.handleValPair.attrHandle = cy_ble_lnssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        ntfParam.handleValPair.value.val  = attrValue;
        ntfParam.handleValPair.value.len  = attrSize;
        ntfParam.connHandle               = connHandle;

        /* Send notification to client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Notification(&ntfParam);
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_LNSS_SendIndication
***************************************************************************//**
*
*  Sends an indication of the specified characteristic value, as identified by
*  the charIndex.
*
*  On enabling indication successfully it sends out a 'Handle Value Indication' which
*  results in #CY_BLE_EVT_LNSC_INDICATION or #CY_BLE_EVT_GATTC_HANDLE_VALUE_IND (if
*  service-specific callback function is not registered) event at the GATT Client's end.
*
*  \param connHandle: The connection handle which consist of the device ID and ATT
*              connection ID.
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue: The pointer to the characteristic value data that should be sent
*              to the client device.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Optional characteristic is absent.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_IND_DISABLED                | Indication is not enabled by the client.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the LNS service-specific callback is registered
*      (with \ref Cy_BLE_LNS_RegisterAttrCallback):
*  * #CY_BLE_EVT_LNSS_INDICATION_CONFIRMED - In case if the indication is
*                                successfully delivered to the peer device.
*  .
*   Otherwise (if the LNS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF - In case if the indication is
*                                successfully delivered to the peer device.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_LNSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_lns_char_index_t charIndex,
                                                  uint8_t attrSize,
                                                  uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send Indication if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_LNS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_lnssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[CY_BLE_LNS_CCCD] ==
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if(!CY_BLE_IS_INDICATION_ENABLED(connHandle.attId, cy_ble_lnssConfigPtr->attrInfo->charInfo[charIndex].
                                           descrHandle[CY_BLE_LNS_CCCD]))
    {
        apiResult = CY_BLE_ERROR_IND_DISABLED;
    }
    else
    {
        cy_stc_ble_gatts_handle_value_ind_t indParam;
        
        /* Fill all fields of Write Request structure ... */
        indParam.handleValPair.attrHandle = cy_ble_lnssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        indParam.handleValPair.value.val  = attrValue;
        indParam.handleValPair.value.len  = attrSize;
        indParam.connHandle               = connHandle;

        /* Send notification to client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Indication(&indParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            /* Save handle to support service-specific value confirmation response from client */
            cy_ble_lnssReqHandle = indParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_LNSS_ConfirmationEventHandler
***************************************************************************//**
*
*  Handles the Value Confirmation request event from the BLE Stack.
*
*  \param eventParam: Pointer to a structure of type cy_stc_ble_conn_handle_t
*
******************************************************************************/
static void Cy_BLE_LNSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    if((Cy_BLE_LNS_ApplCallback != NULL) && (cy_ble_lnssConfigPtr->attrInfo->charInfo[CY_BLE_LNS_CP].charHandle ==
                                             cy_ble_lnssReqHandle))
    {
        cy_stc_ble_lns_char_value_t locCharIndex;
        
        locCharIndex.connHandle = *eventParam;

        cy_ble_lnssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        cy_ble_lnsFlag &= (uint8_t) ~CY_BLE_LNS_FLAG_PROCESS;

        locCharIndex.charIndex = CY_BLE_LNS_CP;
        locCharIndex.value = NULL;

        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        Cy_BLE_LNS_ApplCallback((uint32_t)CY_BLE_EVT_LNSS_INDICATION_CONFIRMED, &locCharIndex);
    }
}


/******************************************************************************
* Function Name: Cy_BLE_LNSS_WriteEventHandler
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
static cy_en_ble_gatt_err_code_t Cy_BLE_LNSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    cy_stc_ble_lns_char_value_t locCharValue;

    if(Cy_BLE_LNS_ApplCallback != NULL)
    {
        locCharValue.connHandle = eventParam->connHandle;

        if(eventParam->handleValPair.attrHandle == cy_ble_lnssConfigPtr->attrInfo->charInfo[CY_BLE_LNS_CP].charHandle)
        {
            if(CY_BLE_LNS_IS_PROCEDURE_IN_PROGRESS)
            {
                gattErr = CY_BLE_GATT_ERR_PROCEDURE_ALREADY_IN_PROGRESS;
            }
            else if(!CY_BLE_IS_INDICATION_ENABLED(eventParam->connHandle.attId,
                                                  cy_ble_lnssConfigPtr->attrInfo->charInfo[CY_BLE_LNS_CP].
                                                   descrHandle[CY_BLE_LNS_CCCD]))
            {
                gattErr = CY_BLE_GATT_ERR_CCCD_IMPROPERLY_CONFIGURED;
            }
            else
            {
                cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                
                dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                dbAttrValInfo.connHandle      = eventParam->connHandle;
                dbAttrValInfo.offset          = 0u;
                dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

                gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);

                locCharValue.charIndex = CY_BLE_LNS_CP;
                locCharValue.value = &eventParam->handleValPair.value;
                if(gattErr == CY_BLE_GATT_ERR_NONE)
                {
                    Cy_BLE_LNS_ApplCallback((uint32_t)CY_BLE_EVT_LNSS_WRITE_CHAR, &locCharValue);
                    cy_ble_lnsFlag |= CY_BLE_LNS_FLAG_PROCESS;
                }
            }

            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else
        {
            for(locCharValue.charIndex = CY_BLE_LNS_LS; locCharValue.charIndex < CY_BLE_LNS_CHAR_COUNT;
                locCharValue.charIndex++)
            {
                if( (locCharValue.charIndex != CY_BLE_LNS_PQ) && /* CY_BLE_LNS_PQ does not have CCCD */
                    (eventParam->handleValPair.attrHandle == cy_ble_lnssConfigPtr->attrInfo->charInfo[locCharValue.charIndex].
                    descrHandle[CY_BLE_LNS_CCCD]) )
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

                        if(locCharValue.charIndex == CY_BLE_LNS_CP)
                        {
                            if(CY_BLE_IS_INDICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                            {
                                eventCode = (uint32_t)CY_BLE_EVT_LNSS_INDICATION_ENABLED;
                            }
                            else
                            {
                                eventCode = (uint32_t)CY_BLE_EVT_LNSS_INDICATION_DISABLED;
                            }
                        }
                        else
                        {
                            if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                            {
                                eventCode = (uint32_t)CY_BLE_EVT_LNSS_NOTIFICATION_ENABLED;
                            }
                            else
                            {
                                eventCode = (uint32_t)CY_BLE_EVT_LNSS_NOTIFICATION_DISABLED;
                            }
                        }

                        Cy_BLE_LNS_ApplCallback(eventCode, &locCharValue);
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
* Function Name: Cy_BLE_LNSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result a Write Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the #CY_BLE_EVT_LNSS_WRITE_CHAR event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle: The connection handle.
*  \param charIndex: The index of the service characteristic.
*  \param attrSize: The size of the characteristic value attribute.
*  \param attrValue: The pointer to the characteristic value data that should be
*              sent to the server device.
*
*   \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the LNS service-specific callback is registered
*      (with \ref Cy_BLE_LNS_RegisterAttrCallback):
*  * #CY_BLE_EVT_LNSC_WRITE_CHAR_RESPONSE - In case if the requested attribute is
*                                successfully written on the peer device,
*                                the details (charIndex, etc.) are
*                                provided with event parameter structure
*                                of type cy_stc_ble_lns_char_value_t.
*  .
*   Otherwise (if the LNS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_WRITE_RSP - In case if the requested attribute is
*                                successfully written on the peer device.
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*                                requested attribute on the peer device,
*                                the details are provided with event parameters
*                                structure (cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_LNSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_lns_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    
    /* Get pointer (with offset) to LNS client structure with attribute handles */
    cy_stc_ble_lnsc_t *lnscPtr = (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[discIdx];
    
    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_LNS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(lnscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_WRITE & lnscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;
        
        writeReqParam.handleValPair.attrHandle = lnscPtr->charInfo[charIndex].valueHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;

        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_lnscReqHandle[discIdx] = lnscPtr->charInfo[charIndex].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_LNSC_GetCharacteristicValue
***************************************************************************//**
*
*  This function is used to read the characteristic Value from a server,
*  as identified by its charIndex
*
*  The Read Response returns the characteristic Value in the Attribute Value
*  parameter.
*
*  The Read Response only contains the characteristic Value that is less than or
*  equal to (MTU - 1) octets in length. If the characteristic Value is greater
*  than (MTU - 1) octets in length, the Read Long Characteristic Value procedure
*  may be used if the rest of the characteristic Value is required.
*
*  \param connHandle: The connection handle.
*  \param charIndex: The index of the service characteristic.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the LNS service-specific callback is registered
*      (with \ref Cy_BLE_LNS_RegisterAttrCallback):
*  * #CY_BLE_EVT_LNSC_READ_CHAR_RESPONSE - In case if the requested attribute is
*                                successfully read on the peer device,
*                                the details (charIndex , value, etc.) are
*                                provided with event parameter structure
*                                of type cy_stc_ble_lns_char_value_t.
*  .
*   Otherwise (if the LNS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_READ_RSP - In case if the requested attribute is
*                                successfully read on the peer device,
*                                the details (handle, value, etc.) are
*                                provided with event parameters
*                                structure (cy_stc_ble_gattc_read_rsp_param_t).
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*                                requested attribute on the peer device,
*                                the details are provided with event parameters
*                                structure (cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_LNSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_lns_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to LNS client structure with attribute handles */
    cy_stc_ble_lnsc_t *lnscPtr = (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[discIdx];

    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_LNS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(lnscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_READ & lnscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = lnscPtr->charInfo[charIndex].valueHandle;
        readReqParam.connHandle = connHandle;

        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_lnscReqHandle[discIdx] = lnscPtr->charInfo[charIndex].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_LNSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  This function is used to write the characteristic Value to the server,
*  as identified by its charIndex.
*
*  Internally, Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*  * #CY_BLE_EVT_LNSS_INDICATION_ENABLED
*  * #CY_BLE_EVT_LNSS_INDICATION_DISABLED
*  * #CY_BLE_EVT_LNSS_NOTIFICATION_ENABLED
*  * #CY_BLE_EVT_LNSS_NOTIFICATION_DISABLED
*
*  \param connHandle: The connection handle.
*  \param charIndex: The index of the service characteristic.
*  \param descrIndex: The index of the service characteristic descriptor.
*  \param attrSize: The size of the characteristic descriptor value attribute.
*  \param attrValue: The pointer to the characteristic descriptor value data that
*               should be sent to the server device.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | This operation is not permitted on the specified attribute.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the LNS service-specific callback is registered
*      (with \ref Cy_BLE_LNS_RegisterAttrCallback):
*  * #CY_BLE_EVT_LNSC_WRITE_DESCR_RESPONSE - In case if the requested attribute is
*                                successfully written on the peer device,
*                                the details (charIndex, descrIndex etc.) are
*                                provided with event parameter structure
*                                of type cy_stc_ble_lns_descr_value_t.
*  .
*   Otherwise (if the LNS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_WRITE_RSP - In case if the requested attribute is
*                                successfully written on the peer device.
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*                                requested attribute on the peer device,
*                                the details are provided with event parameters
*                                structure (cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_LNSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_lns_char_index_t charIndex,
                                                               cy_en_ble_lns_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to LNS client structure with attribute handles */
    cy_stc_ble_lnsc_t *lnscPtr = (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[discIdx];
    
    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_LNS_CHAR_COUNT) || (descrIndex >= CY_BLE_LNS_DESCR_COUNT) ||
            (attrSize != CY_BLE_CCCD_LEN) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(lnscPtr->charInfo[charIndex].descrHandle[descrIndex] == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;
        
        /* Fill all fields of Write Request structure ... */
        writeReqParam.handleValPair.attrHandle = lnscPtr->charInfo[charIndex].descrHandle[descrIndex];
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = CY_BLE_CCCD_LEN;
        writeReqParam.connHandle               = connHandle;
        
        /* ... and send request to server device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            /* Save handle to support service-specific read response from device */
            cy_ble_lnscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_LNSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets the characteristic descriptor of the specified characteristic.
*
*  \param connHandle: The connection handle.
*  \param charIndex: The index of the service characteristic.
*  \param descrIndex: The index of the service characteristic descriptor.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | This operation is not permitted on the specified attribute.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular descriptor.
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*  If the LNS service-specific callback is registered
*      (with \ref Cy_BLE_LNS_RegisterAttrCallback):
*  * #CY_BLE_EVT_LNSC_READ_DESCR_RESPONSE - In case if the requested attribute is
*                                successfully read on the peer device,
*                                the details (charIndex, descrIndex, value, etc.)
*                                are provided with event parameter structure
*                                of type cy_stc_ble_lns_descr_value_t.
*  .
*  Otherwise (if the LNS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTC_READ_RSP - In case if the requested attribute is
*                                successfully read on the peer device,
*                                the details (handle, value, etc.) are
*                                provided with event parameters
*                                structure (cy_stc_ble_gattc_read_rsp_param_t).
*  * #CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*                                requested attribute on the peer device,
*                                the details are provided with event parameters
*                                structure (cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_LNSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_lns_char_index_t charIndex,
                                                               cy_en_ble_lns_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to LNS client structure with attribute handles */
    cy_stc_ble_lnsc_t *lnscPtr = (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[discIdx];

    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_LNS_CHAR_COUNT) || (descrIndex >= CY_BLE_LNS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(lnscPtr->charInfo[charIndex].descrHandle[descrIndex] == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = lnscPtr->charInfo[charIndex].descrHandle[descrIndex];
        readReqParam.connHandle = connHandle;

        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_lnscReqHandle[discIdx] = lnscPtr->charInfo[charIndex].descrHandle[descrIndex];
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_LNSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_LNSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* Location and Navigation service characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_lnscCharUuid[CY_BLE_LNS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_LN_FEATURE,
        CY_BLE_UUID_CHAR_LOCATION_AND_SPEED,
        CY_BLE_UUID_CHAR_POSITION_QUALITY,
        CY_BLE_UUID_CHAR_LN_CONTROL_POINT,
        CY_BLE_UUID_CHAR_NAVIGATION
    };
    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t lnsDiscIdx = cy_ble_lnscConfigPtr->serviceDiscIdx;
    uint32_t i;

    /* Get pointer (with offset) to LNS client structure with attribute handles */
    cy_stc_ble_lnsc_t *lnscPtr = (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[discIdx];

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == lnsDiscIdx))
    {
        /* Update last characteristic endHandle to declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        for(i = 0u; i < (uint32_t)CY_BLE_LNS_CHAR_COUNT; i++)
        {
            if(cy_ble_lnscCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                if(lnscPtr->charInfo[i].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    lnscPtr->charInfo[i].valueHandle = discCharInfo->valueHandle;
                    lnscPtr->charInfo[i].properties = discCharInfo->properties;
                    lastEndHandle[discIdx] = &lnscPtr->charInfo[i].endHandle;
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
* Function Name: Cy_BLE_LNSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to the characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_LNSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t * discDescrInfo)
{
    uint32_t notSupportedDescr = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t lnsDiscIdx = cy_ble_lnscConfigPtr->serviceDiscIdx;
    uint32_t descIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == lnsDiscIdx)
    {
        /* Get pointer (with offset) to LNS client structure with attribute handles */
        cy_stc_ble_lnsc_t *lnscPtr = (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[discIdx];

    
        switch(discDescrInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_CLIENT_CONFIG:
                descIdx = (uint32_t)CY_BLE_LNS_CCCD;
                break;

            default:
                /* Not supported descriptor */
                notSupportedDescr = 1u;
                break;
        }

        if(notSupportedDescr == 0u)
        {
            if(lnscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] ==
               CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                lnscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] =
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
* Function Name: Cy_BLE_LNSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_LNSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t lnsDiscIdx = cy_ble_lnscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == lnsDiscIdx)
    {
        /* Get pointer (with offset) to LNS client structure with attribute handles */
        cy_stc_ble_lnsc_t *lnscPtr = (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[discIdx];
    
        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_LNS_CHAR_COUNT) && (exitFlag == 0u))
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            if((lnscPtr->charInfo[charIdx].endHandle - lnscPtr->charInfo[charIdx].valueHandle) != 0u)
            {
                /* Read characteristic range */
                charRangeInfo->range.startHandle = lnscPtr->charInfo[charIdx].valueHandle + 1u;
                charRangeInfo->range.endHandle = lnscPtr->charInfo[charIdx].endHandle;
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
* Function Name: Cy_BLE_LNSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles the Notification event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_LNSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_LNS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to LNS client structure with attribute handles */
        cy_stc_ble_lnsc_t *lnscPtr = (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[discIdx];

        cy_stc_ble_lns_char_value_t locCharValue = { .connHandle = eventParam->connHandle };

        if(lnscPtr->charInfo[CY_BLE_LNS_LS].valueHandle == eventParam->handleValPair.attrHandle)
        {
            locCharValue.charIndex = CY_BLE_LNS_LS;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else if(lnscPtr->charInfo[CY_BLE_LNS_NV].valueHandle == eventParam->handleValPair.attrHandle)
        {
            locCharValue.charIndex = CY_BLE_LNS_NV;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
        else
        {
            /* Location and Navigation service doesn't support any other notifications */
        }

        if((cy_ble_eventHandlerFlag & CY_BLE_CALLBACK) == 0u)
        {
            locCharValue.value = &eventParam->handleValPair.value;
            Cy_BLE_LNS_ApplCallback((uint32_t)CY_BLE_EVT_LNSC_NOTIFICATION, &locCharValue);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_LNSC_IndicationEventHandler
***************************************************************************//**
*
*  Handles the Indication event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_LNSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_LNS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to LNS client structure with attribute handles */
        cy_stc_ble_lnsc_t *lnscPtr = (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[discIdx];

        if(lnscPtr->charInfo[CY_BLE_LNS_CP].valueHandle == eventParam->handleValPair.attrHandle)
        {
            cy_stc_ble_lns_char_value_t locCharValue;
            
            locCharValue.charIndex  = CY_BLE_LNS_CP;
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.value      = &eventParam->handleValPair.value;
            
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_LNS_ApplCallback((uint32_t)CY_BLE_EVT_LNSC_INDICATION, &locCharValue);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_LNSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles the Read Response event.
*
*  \param eventParam:  the pointer to the data structure.
*
******************************************************************************/
static void Cy_BLE_LNSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_LNS_ApplCallback != NULL) &&
       (cy_ble_lnscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        cy_en_ble_lns_char_index_t i;
    
        /* Get pointer (with offset) to LNS client structure with attribute handles */
        cy_stc_ble_lnsc_t *lnscPtr = (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[discIdx];

        for(i = CY_BLE_LNS_FT; i < CY_BLE_LNS_CHAR_COUNT; i++)
        {
            if(lnscPtr->charInfo[i].valueHandle == cy_ble_lnscReqHandle[discIdx])
            {
                cy_stc_ble_lns_char_value_t locCharValue;
                
                locCharValue.connHandle = eventParam->connHandle;
                locCharValue.charIndex  = i;
                locCharValue.value      = &eventParam->value;

                cy_ble_lnscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                Cy_BLE_LNS_ApplCallback((uint32_t)CY_BLE_EVT_LNSC_READ_CHAR_RESPONSE, &locCharValue);
            }
        }

        if(0u != (cy_ble_eventHandlerFlag | CY_BLE_CALLBACK))
        {
            for(i = CY_BLE_LNS_FT; i < CY_BLE_LNS_CHAR_COUNT; i++)
            {
                if(lnscPtr->charInfo[i].descrHandle[CY_BLE_LNS_CCCD] == cy_ble_lnscReqHandle[discIdx])
                {
                    cy_stc_ble_lns_descr_value_t locDescrValue;
                    
                    locDescrValue.connHandle = eventParam->connHandle;
                    locDescrValue.charIndex  = i;
                    locDescrValue.descrIndex = CY_BLE_LNS_CCCD;
                    locDescrValue.value      = &eventParam->value;

                    cy_ble_lnscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                    Cy_BLE_LNS_ApplCallback((uint32_t)CY_BLE_EVT_LNSC_READ_DESCR_RESPONSE, &locDescrValue);
                    break;
                }
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_LNSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles the Write Response event.
*
*  \param eventParam: The pointer to a data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_LNSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_LNS_ApplCallback != NULL) &&
       (cy_ble_lnscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to LNS client structure with attribute handles */
        cy_stc_ble_lnsc_t *lnscPtr = (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[discIdx];

        if(lnscPtr->charInfo[CY_BLE_LNS_CP].valueHandle == cy_ble_lnscReqHandle[discIdx])
        {
            cy_stc_ble_lns_char_value_t locCharIndex;
            
            locCharIndex.connHandle = *eventParam;
            locCharIndex.charIndex  = CY_BLE_LNS_CP;
            locCharIndex.value      = NULL;

            cy_ble_lnscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_LNS_ApplCallback((uint32_t)CY_BLE_EVT_LNSC_WRITE_CHAR_RESPONSE, &locCharIndex);
        }
        else
        {
            cy_en_ble_lns_char_index_t i;

            for(i = CY_BLE_LNS_FT; i < CY_BLE_LNS_CHAR_COUNT; i++)
            {
                if(lnscPtr->charInfo[i].descrHandle[CY_BLE_LNS_CCCD] == cy_ble_lnscReqHandle[discIdx])
                {
                    cy_stc_ble_lns_descr_value_t locDescIndex;
                    
                    locDescIndex.connHandle = *eventParam;
                    locDescIndex.charIndex  = i;
                    locDescIndex.descrIndex = CY_BLE_LNS_CCCD;
                    locDescIndex.value      = NULL;

                    cy_ble_lnscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                    Cy_BLE_LNS_ApplCallback((uint32_t)CY_BLE_EVT_LNSC_WRITE_DESCR_RESPONSE, &locDescIndex);
                    break;
                }
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_LNSC_GetCharacteristicValueHandle
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
cy_ble_gatt_db_attr_handle_t Cy_BLE_LNSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_lns_char_index_t charIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

    if((Cy_BLE_GetDiscoveryIdx(connHandle) < cy_ble_configPtr->params->maxClientCount) && 
           (charIndex < CY_BLE_LNS_CHAR_COUNT))
    {
        /* Get pointer (with offset) to LNS client structure with attribute handles */
        cy_stc_ble_lnsc_t *lnscPtr =
                (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[Cy_BLE_GetDiscoveryIdx(connHandle)];

        returnHandle = lnscPtr->charInfo[charIndex].valueHandle;
    }
    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_LNSC_GetCharacteristicDescriptorHandle
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
cy_ble_gatt_db_attr_handle_t Cy_BLE_LNSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_lns_char_index_t charIndex,
                                                                            cy_en_ble_lns_descr_index_t descrIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;

    if((Cy_BLE_GetDiscoveryIdx(connHandle) < cy_ble_configPtr->params->maxClientCount) && 
           (charIndex < CY_BLE_LNS_CHAR_COUNT) && (descrIndex < CY_BLE_LNS_DESCR_COUNT))
    {
        /* Get pointer (with offset) to LNS client structure with attribute handles */
        cy_stc_ble_lnsc_t *lnscPtr =
                (cy_stc_ble_lnsc_t *)&cy_ble_lnscConfigPtr->attrInfo[Cy_BLE_GetDiscoveryIdx(connHandle)];

        returnHandle = lnscPtr->charInfo[charIndex].descrHandle[descrIndex];
    }

    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_LNSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles the Error Response event.
*
*  \param eventParam: Pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_LNSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        if(cy_ble_lnscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] == eventParam->errInfo.attrHandle)
        {
            cy_ble_lnscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_LNS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the LNS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_LNS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_LNSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_LNSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_LNSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_LNSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_LNSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the LNS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_LNSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATT_DISCONNECT_IND:
            (void)Cy_BLE_LNSS_Init(cy_ble_lnssConfigPtr);
            break;

        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_LNSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            if(eventParam != NULL)
            {
                Cy_BLE_LNSS_ConfirmationEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
            }
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_LNSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the LNS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_LNSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_LNSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_LNSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_LNSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_LNSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_LNSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_IND:
                Cy_BLE_LNSC_IndicationEventHandler((cy_stc_ble_gattc_handle_value_ind_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_LNSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_LNSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
