/***************************************************************************//**
* \file cy_ble_hrs.c
* \version 3.30
*
* \brief
*  This file contains the source code for the Heart Rate service.
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

#define CY_BLE_HRS_IS_BSL_SUPPORTED \
    (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_hrssConfigPtr->attrInfo->charHandle[CY_BLE_HRS_BSL])

    
/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_HRS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_HRSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_HRSC_EventHandler(uint32_t eventCode, void *eventParam);
 
static cy_en_ble_gatt_err_code_t Cy_BLE_HRSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_HRSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_HRSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_HRSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_HRSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_HRSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_HRSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_HRSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);
    

/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_HRS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_HRSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_HRSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_hrscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE HRS server config structure */
const cy_stc_ble_hrss_config_t *cy_ble_hrssConfigPtr = NULL;

/* The pointer to a global BLE HRS client config structure */
const cy_stc_ble_hrsc_config_t *cy_ble_hrscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_HRSS_Init
***************************************************************************//**
*
*  This function initializes server of the Heart Rate service.
*
*  \param config: Configuration structure for the HRS.
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
cy_en_ble_api_result_t Cy_BLE_HRSS_Init(const cy_stc_ble_hrss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_hrssConfigPtr = config;

        /* Registers event handler for the HRS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_HRS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_HRSS_EventHandlerCallback = &Cy_BLE_HRSS_EventHandler;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HRSC_Init
***************************************************************************//**
*
*  This function initializes client of the Heart Rate service.
*
*  \param config: Configuration structure for the HRS.
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
cy_en_ble_api_result_t Cy_BLE_HRSC_Init(const cy_stc_ble_hrsc_config_t *config)
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
        cy_ble_hrscConfigPtr = config;

        /* Registers event handler for the HRS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_HRS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_HRSC_EventHandlerCallback = &Cy_BLE_HRSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 hrsServIdx = cy_ble_hrscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + hrsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to HRS client structure */
                cy_stc_ble_hrsc_t *hrscPtr = (cy_stc_ble_hrsc_t *)&cy_ble_hrscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(hrscPtr, 0, sizeof(cy_stc_ble_hrsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + hrsServIdx].uuid =
                    CY_BLE_UUID_HEART_RATE_SERVICE;
            }

            cy_ble_hrscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HRS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for HRS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_HRSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_hrs_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_HRS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_HRS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_HRSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a characteristic value of Heart Rate service, which is a value
*  identified by charIndex, to the local database.
*
*  \param charIndex: The index of the service characteristic of type
*                    \ref cy_en_ble_hrs_char_index_t.
*  \param attrSize:  The size of the characteristic value attribute.
*  \param attrValue: The pointer to the characteristic value data that should be
*                    stored in the GATT database.
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
cy_en_ble_api_result_t Cy_BLE_HRSS_SetCharacteristicValue(cy_en_ble_hrs_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if(charIndex >= CY_BLE_HRS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if((charIndex == CY_BLE_HRS_BSL) && (!CY_BLE_HRS_IS_BSL_SUPPORTED))
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Store characteristic value into GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_hrssConfigPtr->attrInfo->charHandle[charIndex];
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
* Function Name: Cy_BLE_HRSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets a characteristic value of Heart Rate service. The value is
*  identified by charIndex.
*
*  \param charIndex: The index of the service characteristic of type
*                    \ref cy_en_ble_hrs_char_index_t.
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Optional characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HRSS_GetCharacteristicValue(cy_en_ble_hrs_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if(charIndex >= CY_BLE_HRS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if((charIndex == CY_BLE_HRS_BSL) && (!CY_BLE_HRS_IS_BSL_SUPPORTED))
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Get characteristic value from GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_hrssConfigPtr->attrInfo->charHandle[charIndex];
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_READ | CY_BLE_GATT_DB_LOCALLY_INITIATED;
        dbAttrValInfo.offset                     = 0u;

        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HRSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets the local characteristic descriptor of the specified Heart Rate
*  service characteristic.
*
*  \param connHandle: The connection handle
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hrs_char_index_t.
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_hrs_descr_index_t.
*  \param attrSize:   The size of the descriptor value attribute. The Heart Rate
*                     Measurement characteristic client configuration descriptor has
*                     2 bytes length.
*  \param attrValue:  The pointer to the location where characteristic descriptor
*                     value data should be stored.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | Optional descriptor is absent.
*
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HRSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hrs_char_index_t charIndex,
                                                               cy_en_ble_hrs_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    if((charIndex >= CY_BLE_HRS_CHAR_COUNT) || (descrIndex >= CY_BLE_HRS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if((charIndex != CY_BLE_HRS_HRM) || (descrIndex != CY_BLE_HRS_HRM_CCCD))
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Get data from database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_hrssConfigPtr->attrInfo->hrmCccdHandle;
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.connHandle                 = connHandle;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_READ | CY_BLE_GATT_DB_LOCALLY_INITIATED;
        dbAttrValInfo.offset                     = 0u;
        
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
* Function Name: Cy_BLE_HRSS_SendNotification
***************************************************************************//**
*
*  Sends notification of a specified Heart Rate service characteristic value
*  to the client device. No response is expected.
*
*  On enabling notification successfully for a service characteristic it sends out a
*  'Handle Value Notification' which results in \ref CY_BLE_EVT_HRSC_NOTIFICATION event
*  at the GATT Client's end.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hrs_char_index_t.
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
*   CY_BLE_ERROR_NTF_DISABLED                | Notification is not enabled by the client.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HRSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_hrs_char_index_t charIndex,
                                                    uint8_t attrSize,
                                                    uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send Notification if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_HRS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if((cy_ble_hrssConfigPtr->attrInfo->hrmCccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            || (!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_hrssConfigPtr->attrInfo->hrmCccdHandle)))
    {
        apiResult = CY_BLE_ERROR_NTF_DISABLED;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
        ntfReqParam.handleValPair.attrHandle = cy_ble_hrssConfigPtr->attrInfo->charHandle[charIndex];
        ntfReqParam.handleValPair.value.val  = attrValue;
        ntfReqParam.handleValPair.value.len  = attrSize;
        ntfReqParam.connHandle               = connHandle;
        
        /* Send notification to client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Notification(&ntfReqParam);
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HRSS_WriteEventHandler
***************************************************************************//**
*
*  Handles the Heart Rate Measurement Client Configuration Characteristic
*  descriptor Write event or Control Point Characteristic Write event.
*
*  \param void *eventParam: The pointer to the data structure specified by the event.
*
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HRSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    uint32_t eventCode = 0u;
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    cy_stc_ble_hrs_char_value_t locCharIndex;

    locCharIndex.connHandle = eventParam->connHandle;
    locCharIndex.value = NULL;

    if(eventParam->handleValPair.attrHandle == cy_ble_hrssConfigPtr->attrInfo->hrmCccdHandle)
    {
        locCharIndex.charIndex = CY_BLE_HRS_HRM;
        /* Heart Rate Measurement characteristic descriptor Write Request */
        if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
        {
            eventCode = (uint32_t)CY_BLE_EVT_HRSS_NOTIFICATION_ENABLED;
        }
        else
        {
            eventCode = (uint32_t)CY_BLE_EVT_HRSS_NOTIFICATION_DISABLED;
        }
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
    else if(eventParam->handleValPair.attrHandle == cy_ble_hrssConfigPtr->attrInfo->charHandle[CY_BLE_HRS_CPT])
    {
        locCharIndex.charIndex = CY_BLE_HRS_CPT;
        /* Heart Rate Control Point characteristic Write Request */
        if(CY_BLE_HRS_RESET_ENERGY_EXPENDED == eventParam->handleValPair.value.val[0u])
        {
            eventCode = (uint32_t)CY_BLE_EVT_HRSS_ENERGY_EXPENDED_RESET;
        }
        else
        {
            gattErr = CY_BLE_GATT_ERR_HEART_RATE_CONTROL_POINT_NOT_SUPPORTED;
        }

        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
    else
    {
        /* Heart Rate service doesn't support any other Write Requests */
    }

    if(0u != eventCode)
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.handleValuePair = eventParam->handleValPair;
        dbAttrValInfo.connHandle      = eventParam->connHandle;
        dbAttrValInfo.offset          = 0u;
        dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;
        
        gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
        if(gattErr == CY_BLE_GATT_ERR_NONE)
        {
            if(Cy_BLE_HRS_ApplCallback != NULL)
            {
                Cy_BLE_HRS_ApplCallback(eventCode, &locCharIndex);
            }
            else
            {
                Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTS_WRITE_REQ, eventParam);
            }
        }
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_HRSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic value attribute
*  (identified by charIndex) to the server. The Write Response just confirms
*  the operation success.
*
*  This function call can result in generation of the following events based on
*  the response from the server device:
*    * \ref CY_BLE_EVT_HRSC_WRITE_CHAR_RESPONSE
*    * \ref CY_BLE_EVT_GATTC_ERROR_RSP
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hrs_char_index_t.
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
*   If the HRS service-specific callback is registered
*   with \ref Cy_BLE_HRS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HRSC_WRITE_CHAR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_hrs_char_value_t.
*   .
*   Otherwise (if the HRS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - In case if the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*     requested attribute on the peer device, the details are provided with event
*     parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HRSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_hrs_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HRS client structure with attribute handles */
    cy_stc_ble_hrsc_t *hrscPtr = (cy_stc_ble_hrsc_t *)&cy_ble_hrscConfigPtr->attrInfo[discIdx];
    
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HRS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount) ||
            ((charIndex == CY_BLE_HRS_CPT) && (attrSize > CY_BLE_HRS_CPT_CHAR_LEN)))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(hrscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_WRITE & hrscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle = hrscPtr->charInfo[charIndex].valueHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;
        
        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_hrscReqHandle[discIdx] = hrscPtr->charInfo[charIndex].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HRSC_GetCharacteristicValue
***************************************************************************//**
*
*  Sends a request to the peer device to get a characteristic value, as
*  identified by its charIndex.
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
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hrs_char_index_t.
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
*   If the HRS service-specific callback is registered
*      with \ref Cy_BLE_HRS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HRSC_READ_CHAR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details 
*     (charIndex , value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_hrs_char_value_t.
*   .
*   Otherwise (if an HRS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_HRSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_hrs_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HRS client structure with attribute handles */
    cy_stc_ble_hrsc_t *hrscPtr = (cy_stc_ble_hrsc_t *)&cy_ble_hrscConfigPtr->attrInfo[discIdx];
    
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HRS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(hrscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_READ & hrscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = hrscPtr->charInfo[charIndex].valueHandle;
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_hrscReqHandle[discIdx] = hrscPtr->charInfo[charIndex].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HRSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  This function is used to write the characteristic Value to the server, which
*  is identified by charIndex.
*
*  This function call can result in generation of the following events based on
*  the response from the server device:
*  * \ref CY_BLE_EVT_HRSC_WRITE_DESCR_RESPONSE
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP
*
*  One of the following events are received by the peer device, on invoking
*  this function:
*  * \ref CY_BLE_EVT_HRSS_NOTIFICATION_ENABLED
*  * \ref CY_BLE_EVT_HRSS_NOTIFICATION_DISABLED
*  * \ref CY_BLE_EVT_HRSS_ENERGY_EXPENDED_RESET
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hrs_char_index_t.
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_hrs_descr_index_t.*  \param attrSize: The size of the characteristic descriptor value attribute.
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the HRS service-specific callback is registered
*   with \ref Cy_BLE_HRS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HRSC_WRITE_DESCR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, descrIndex etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_hrs_descr_value_t.
*   .
*   Otherwise (if an HRS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure ( \ref cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HRSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hrs_char_index_t charIndex,
                                                               cy_en_ble_hrs_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HRS client structure with attribute handles */
    cy_stc_ble_hrsc_t *hrscPtr = (cy_stc_ble_hrsc_t *)&cy_ble_hrscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HRS_CHAR_COUNT) || (descrIndex >= CY_BLE_HRS_DESCR_COUNT) ||
            (attrSize != CY_BLE_CCCD_LEN) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(charIndex != CY_BLE_HRS_HRM)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else if(hrscPtr->hrmCccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle = hrscPtr->hrmCccdHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = CY_BLE_CCCD_LEN;
        writeReqParam.connHandle               = connHandle;

        /* ... and send request to server device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_hrscReqHandle[discIdx] = hrscPtr->hrmCccdHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HRSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of a specified characteristic of the service.
*
*  This function call can result in generation of the following events based on
*  the response from the server device:
*  * \ref CY_BLE_EVT_HRSC_READ_DESCR_RESPONSE
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_hrs_char_index_t.
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_hrs_descr_index_t.*
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have
*                                               the particular descriptor
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the HRS service-specific callback is registered
*   with \ref Cy_BLE_HRS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HRSC_READ_DESCR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     descrIndex, value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_hrs_descr_value_t.
*   .
*   Otherwise (if an HRS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_HRSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hrs_char_index_t charIndex,
                                                               cy_en_ble_hrs_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HRS client structure with attribute handles */
    cy_stc_ble_hrsc_t *hrscPtr = (cy_stc_ble_hrsc_t *)&cy_ble_hrscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HRS_CHAR_COUNT) || (descrIndex >= CY_BLE_HRS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(charIndex != CY_BLE_HRS_HRM)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else if(hrscPtr->hrmCccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = hrscPtr->hrmCccdHandle;
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_hrscReqHandle[discIdx] = hrscPtr->hrmCccdHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HRSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
*
******************************************************************************/
static void Cy_BLE_HRSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* Heart Rate service characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_hrscCharUuid[CY_BLE_HRS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_HR_MSRMT,
        CY_BLE_UUID_CHAR_BODY_SENSOR_LOC,
        CY_BLE_UUID_CHAR_HR_CNTRL_POINT
    };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t hrsDiscIdx = cy_ble_hrscConfigPtr->serviceDiscIdx;
    uint32_t i;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == hrsDiscIdx))
    {
        for(i = 0u; i < (uint32_t)CY_BLE_HRS_CHAR_COUNT; i++)
        {
            if(cy_ble_hrscCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                /* Get pointer (with offset) to HRS client structure with attribute handles */
                cy_stc_ble_hrsc_t *hrscPtr = (cy_stc_ble_hrsc_t *)&cy_ble_hrscConfigPtr->attrInfo[discIdx];

                if(hrscPtr->charInfo[i].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    hrscPtr->charInfo[i].valueHandle = discCharInfo->valueHandle;
                    hrscPtr->charInfo[i].properties = discCharInfo->properties;
                }
                else
                {
                    Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo);
                }
            }
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HRSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_HRSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t hrsDiscIdx = cy_ble_hrscConfigPtr->serviceDiscIdx;
    
    if(cy_ble_configPtr->context->discovery[discIdx].servCount == hrsDiscIdx)
    {
        if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_CLIENT_CONFIG)
        {
            /* Get pointer (with offset) to HRS client structure with attribute handles */
            cy_stc_ble_hrsc_t *hrscPtr = (cy_stc_ble_hrsc_t *)&cy_ble_hrscConfigPtr->attrInfo[discIdx];

            if(hrscPtr->hrmCccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                hrscPtr->hrmCccdHandle = discDescrInfo->descrHandle;
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
* Function Name: Cy_BLE_HRSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_HRSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t hrsDiscIdx = cy_ble_hrscConfigPtr->serviceDiscIdx;
    
    if(cy_ble_configPtr->context->discovery[discIdx].servCount == hrsDiscIdx)
    {
        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        if(cy_ble_configPtr->context->discovery[discIdx].charCount == 0u)
        {
            /* Get pointer (with offset) to HRS client structure with attribute handles */
            cy_stc_ble_hrsc_t *hrscPtr = (cy_stc_ble_hrsc_t *)&cy_ble_hrscConfigPtr->attrInfo[discIdx];

            uint32_t locServCount = cy_ble_configPtr->context->discovery[discIdx].servCount;
            uint32_t discServiNum = cy_ble_configPtr->context->discServiCount;

            /* Read characteristic range */
            charRangeInfo->range.startHandle = hrscPtr->charInfo[CY_BLE_HRS_HRM].valueHandle + 1u;
            charRangeInfo->range.endHandle = cy_ble_configPtr->context->
                                             serverInfo[(discIdx * discServiNum) + locServCount].range.endHandle;
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HRSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles the Notification event.
*
*  \param eventParam: The pointer to a data structure specified by the event.
*
*
******************************************************************************/
static void Cy_BLE_HRSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    /* Get pointer (with offset) to HRS client structure with attribute handles */
    cy_stc_ble_hrsc_t *hrscPtr = (cy_stc_ble_hrsc_t *)&cy_ble_hrscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) &&
       (hrscPtr->charInfo[CY_BLE_HRS_HRM].valueHandle == eventParam->handleValPair.attrHandle))
    {
        if(Cy_BLE_HRS_ApplCallback != NULL)
        {
            cy_stc_ble_hrs_char_value_t locCharValue;
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.charIndex  = CY_BLE_HRS_HRM;
            locCharValue.value      = &eventParam->handleValPair.value;

            Cy_BLE_HRS_ApplCallback((uint32_t)CY_BLE_EVT_HRSC_NOTIFICATION, &locCharValue);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HRSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles the Read Response event.
*
*  \param cy_stc_ble_gattc_read_rsp_param_t *eventParam: The pointer to the data structure.
*
*
******************************************************************************/
static void Cy_BLE_HRSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_HRS_ApplCallback != NULL) &&
       (cy_ble_hrscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to HRS client structure with attribute handles */
        cy_stc_ble_hrsc_t *hrscPtr = (cy_stc_ble_hrsc_t *)&cy_ble_hrscConfigPtr->attrInfo[discIdx];

        if(hrscPtr->charInfo[CY_BLE_HRS_BSL].valueHandle == cy_ble_hrscReqHandle[discIdx])
        {
            cy_stc_ble_hrs_char_value_t locCharValue;
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.charIndex  = CY_BLE_HRS_BSL;
            locCharValue.value      = &eventParam->value;
            
            cy_ble_hrscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_HRS_ApplCallback((uint32_t)CY_BLE_EVT_HRSC_READ_CHAR_RESPONSE, &locCharValue);
        }
        else if(hrscPtr->hrmCccdHandle == cy_ble_hrscReqHandle[discIdx])
        {
            cy_stc_ble_hrs_descr_value_t locDescrValue;
            locDescrValue.connHandle = eventParam->connHandle;
            locDescrValue.charIndex  = CY_BLE_HRS_HRM;
            locDescrValue.descrIndex = CY_BLE_HRS_HRM_CCCD;
            locDescrValue.value      = &eventParam->value;
            
            cy_ble_hrscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_HRS_ApplCallback((uint32_t)CY_BLE_EVT_HRSC_READ_DESCR_RESPONSE, &locDescrValue);
        }
        else
        {
            /* No any more read response handles */
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HRSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles the Write Response event.
*
*  \param eventParam: The pointer to a data structure specified by the event.
*
*
******************************************************************************/
static void Cy_BLE_HRSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_HRS_ApplCallback != NULL) &&
       (cy_ble_hrscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to HRS client structure with attribute handles */
        cy_stc_ble_hrsc_t *hrscPtr = (cy_stc_ble_hrsc_t *)&cy_ble_hrscConfigPtr->attrInfo[discIdx];

        if(hrscPtr->charInfo[CY_BLE_HRS_CPT].valueHandle == cy_ble_hrscReqHandle[discIdx])
        {
            cy_stc_ble_hrs_char_value_t locCharIndex;
            locCharIndex.connHandle = *eventParam;
            locCharIndex.charIndex  = CY_BLE_HRS_CPT;
            locCharIndex.value      = NULL;
            
            cy_ble_hrscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_HRS_ApplCallback((uint32_t)CY_BLE_EVT_HRSC_WRITE_CHAR_RESPONSE, &locCharIndex);
        }
        if(hrscPtr->hrmCccdHandle == cy_ble_hrscReqHandle[discIdx])
        {
            cy_stc_ble_hrs_descr_value_t locDescIndex;
            locDescIndex.connHandle = *eventParam;
            locDescIndex.charIndex  = CY_BLE_HRS_HRM;
            locDescIndex.descrIndex = CY_BLE_HRS_HRM_CCCD;
            locDescIndex.value      = NULL;
            
            cy_ble_hrscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_HRS_ApplCallback((uint32_t)CY_BLE_EVT_HRSC_WRITE_DESCR_RESPONSE, &locDescIndex);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HRSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles the Error Response event.
*
*  \param eventParam: Pointer to the data structure specified by the event.
*
*
******************************************************************************/
static void Cy_BLE_HRSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        if(cy_ble_hrscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] == eventParam->errInfo.attrHandle)
        {
            cy_ble_hrscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HRS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the Heart Rate service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HRS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_HRSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_HRSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_HRSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_HRSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_HRSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the Heart Rate service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HRSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_HRSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;
        
        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_HRSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the Heart Rate service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HRSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_HRSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_HRSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_HRSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_HRSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_HRSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;


            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_HRSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_HRSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
