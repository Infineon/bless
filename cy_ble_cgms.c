/***************************************************************************//**
* \file cy_ble_cgms.c
* \version 3.20
*
* \brief
*  This file contains the source code for the Continuous Glucose Monitoring
*  service.
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

#define CY_BLE_CGMS_IS_PROCEDURE_IN_PROGRESS \
             (0u != (CY_BLE_CGMS_FLAG_PROCESS & cy_ble_cgmsFlag))

/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_CGMS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_CGMSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_CGMSC_EventHandler(uint32_t eventCode, void *eventParam);

static void Cy_BLE_CGMSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_CGMSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_CGMSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_CGMSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_CGMSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_CGMSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_CGMSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam);
static void Cy_BLE_CGMSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_CGMSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_CGMSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_CGMS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_CGMSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_CGMSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response for server */
static cy_ble_gatt_db_attr_handle_t cy_ble_cgmssReqHandle;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_cgmscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE CGMS server config structure */
const cy_stc_ble_cgmss_config_t *cy_ble_cgmssConfigPtr = NULL;

/* The pointer to a global BLE CGMS client config structure */
const cy_stc_ble_cgmsc_config_t *cy_ble_cgmscConfigPtr = NULL;

/* Internal CGMS flags */
uint8_t cy_ble_cgmsFlag;


/******************************************************************************
* Function Name: Cy_BLE_CGMSS_Init
***************************************************************************//**
*
*  This function initializes server of the Continuous Glucose Monitoring service.
*
*  \param config: Configuration structure for the CGMS.
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
cy_en_ble_api_result_t Cy_BLE_CGMSS_Init(const cy_stc_ble_cgmss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_cgmssConfigPtr = config;

        /* Registers event handler for the CGMS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_CGMS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_CGMSS_EventHandlerCallback = &Cy_BLE_CGMSS_EventHandler;

        cy_ble_cgmssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        cy_ble_cgmsFlag = 0u;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSC_Init
***************************************************************************//**
*
*  This function initializes client of the Continuous Glucose Monitoring service.
*
*  \param config: Configuration structure for the CGMS.
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
cy_en_ble_api_result_t Cy_BLE_CGMSC_Init(const cy_stc_ble_cgmsc_config_t *config)
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
        cy_ble_cgmscConfigPtr = config;

        /* Registers event handler for the CGMS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_CGMS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_CGMSC_EventHandlerCallback = &Cy_BLE_CGMSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 cgmsServIdx = cy_ble_cgmscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + cgmsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to CGMS client structure */
                cy_stc_ble_cgmsc_t *cgmscPtr = (cy_stc_ble_cgmsc_t *)&cy_ble_cgmscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(cgmscPtr, 0, sizeof(cy_stc_ble_cgmsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + cgmsServIdx].uuid =
                    CY_BLE_UUID_CGM_SERVICE;
            }

            cy_ble_cgmscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CGMS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for CGMS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_CGMSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_cgms_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_CGMS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_CGMS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a characteristic value of Continuous Glucose Monitoring service, which 
*  is a value identified by charIndex, to the local database.
    
*  \param charIndex: The index of the service characteristic of type
*                    \ref cy_en_ble_cgms_char_index_t. The valid values are
*                      * \ref CY_BLE_CGMS_CGMT
*                      * \ref CY_BLE_CGMS_CGFT
*                      * \ref CY_BLE_CGMS_CGST
*                      * \ref CY_BLE_CGMS_SSTM
*                      * \ref CY_BLE_CGMS_SRTM
*                      * \ref CY_BLE_CGMS_RACP
*                      * \ref CY_BLE_CGMS_SOCP
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
cy_en_ble_api_result_t Cy_BLE_CGMSS_SetCharacteristicValue(cy_en_ble_cgms_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check parameters */
    if(charIndex >= CY_BLE_CGMS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_cgmssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Store characteristic value into GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_cgmssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_CGMSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets a characteristic value of Continuous Glucose Monitoring service.
*  The value is identified by charIndex.
*    
*  \param charIndex: The index of the service characteristic of type
*                    \ref cy_en_ble_cgms_char_index_t. The valid values are
*                      * \ref CY_BLE_CGMS_CGMT
*                      * \ref CY_BLE_CGMS_CGFT
*                      * \ref CY_BLE_CGMS_CGST
*                      * \ref CY_BLE_CGMS_SSTM
*                      * \ref CY_BLE_CGMS_SRTM
*                      * \ref CY_BLE_CGMS_RACP
*                      * \ref CY_BLE_CGMS_SOCP
*  \param attrSize:  The size of the characteristic value attribute.
*  \param attrValue: The pointer to the location where Characteristic value data should
*                    be stored.
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
cy_en_ble_api_result_t Cy_BLE_CGMSS_GetCharacteristicValue(cy_en_ble_cgms_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check the parameters */
    if(charIndex >= CY_BLE_CGMS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_cgmssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Get characteristic value from GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_cgmssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_CGMSS_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sets a characteristic descriptor of a specified characteristic of the
*  Continuous Glucose Monitoring service. The value is identified by charIndex.
*    
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cgms_char_index_t. The valid values are
*                       * \ref CY_BLE_CGMS_CGMT
*                       * \ref CY_BLE_CGMS_CGFT
*                       * \ref CY_BLE_CGMS_CGST
*                       * \ref CY_BLE_CGMS_SSTM
*                       * \ref CY_BLE_CGMS_SRTM
*                       * \ref CY_BLE_CGMS_RACP
*                       * \ref CY_BLE_CGMS_SOCP
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_cgms_descr_index_t. The valid value is,
*                       * \ref CY_BLE_CGMS_CCCD
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  The pointer to the descriptor value data that should
*                     be stored to the GATT database.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CGMSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_cgms_char_index_t charIndex,
                                                                cy_en_ble_cgms_descr_index_t descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((charIndex >= CY_BLE_CGMS_CHAR_COUNT) || (descrIndex >= CY_BLE_CGMS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_cgmssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex];
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.connHandle                 = connHandle;
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
* Function Name: Cy_BLE_CGMSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of a specified characteristic of the
*  Continuous Glucose Monitoring service. The value is identified by charIndex.
*    
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cgms_char_index_t. The valid values are
*                       * \ref CY_BLE_CGMS_CGMT
*                       * \ref CY_BLE_CGMS_CGFT
*                       * \ref CY_BLE_CGMS_CGST
*                       * \ref CY_BLE_CGMS_SSTM
*                       * \ref CY_BLE_CGMS_SRTM
*                       * \ref CY_BLE_CGMS_RACP
*                       * \ref CY_BLE_CGMS_SOCP
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_cgms_descr_index_t. The valid value is,
*                       * \ref CY_BLE_CGMS_CCCD
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  The pointer to the location where characteristic descriptor
*                     value data should be stored.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CGMSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_cgms_char_index_t charIndex,
                                                                cy_en_ble_cgms_descr_index_t descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((charIndex >= CY_BLE_CGMS_CHAR_COUNT) || (descrIndex >= CY_BLE_CGMS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_cgmssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex];
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
* Function Name: Cy_BLE_CGMSS_SendNotification
***************************************************************************//**
*
*  Sends a notification with the characteristic value, as specified by its 
*  charIndex, to the client device.
*  On enabling notification successfully for a service characteristic it sends out a
*  'Handle Value Notification' which results in \ref CY_BLE_EVT_CGMSC_NOTIFICATION event
*  at the GATT Client's end.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cgms_char_index_t. The valid values are
*                       * \ref CY_BLE_CGMS_CGMT
*                       * \ref CY_BLE_CGMS_CGFT
*                       * \ref CY_BLE_CGMS_CGST
*                       * \ref CY_BLE_CGMS_SSTM
*                       * \ref CY_BLE_CGMS_SRTM
*                       * \ref CY_BLE_CGMS_RACP
*                       * \ref CY_BLE_CGMS_SOCP
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the Characteristic value data that should be sent
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
*   CY_BLE_ERROR_NTF_DISABLED                | Notification is not enabled by the client.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CGMSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                     cy_en_ble_cgms_char_index_t charIndex,
                                                     uint8_t attrSize,
                                                     uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send notification if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_CGMS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_cgmssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if(!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_cgmssConfigPtr->attrInfo->charInfo[charIndex].
                                             descrHandle[CY_BLE_CGMS_CCCD]))
    {
        apiResult = CY_BLE_ERROR_NTF_DISABLED;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gatts_handle_value_ntf_t ntfParam;
        
        ntfParam.handleValPair.attrHandle = cy_ble_cgmssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        ntfParam.handleValPair.value.val  = attrValue;
        ntfParam.handleValPair.value.len  = attrSize;
        ntfParam.connHandle               = connHandle;
        
        /* Send notification to client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Notification(&ntfParam);
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSS_SendIndication
***************************************************************************//**
*
*  Sends a indication with the characteristic value, as specified by its 
*  charIndex, to the client device.
*  On enabling indication successfully for a service characteristic it sends out a 
*  'Handle Value Indication' which results in \ref CY_BLE_EVT_CGMSC_INDICATION 
*  or \ref CY_BLE_EVT_GATTC_HANDLE_VALUE_IND (if service-specific callback 
*  function is not registered) event at the GATT Client's end.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cgms_char_index_t. The valid values are
*                       * \ref CY_BLE_CGMS_CGMT
*                       * \ref CY_BLE_CGMS_CGFT
*                       * \ref CY_BLE_CGMS_CGST
*                       * \ref CY_BLE_CGMS_SSTM
*                       * \ref CY_BLE_CGMS_SRTM
*                       * \ref CY_BLE_CGMS_RACP
*                       * \ref CY_BLE_CGMS_SOCP
*  \param attrSize:   The size of the characteristic value attribute.
*  \param attrValue:  The pointer to the Characteristic value data that should be sent
*                     to Client device.
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
*  If the CGMS service-specific callback is registered
*  with \ref Cy_BLE_CGMS_RegisterAttrCallback():
*  * \ref CY_BLE_EVT_CGMSS_INDICATION_CONFIRMED - In case if the indication is
*    successfully delivered to the peer device.
*  .
*  Otherwise (if the CGMS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF - In case if the indication is
*    successfully delivered to the peer device.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CGMSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                   cy_en_ble_cgms_char_index_t charIndex,
                                                   uint8_t attrSize,
                                                   uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send indication if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(charIndex >= CY_BLE_CGMS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_cgmssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[CY_BLE_CGMS_CCCD] ==
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if(!CY_BLE_IS_INDICATION_ENABLED(connHandle.attId, cy_ble_cgmssConfigPtr->attrInfo->charInfo[charIndex].
                                           descrHandle[CY_BLE_CGMS_CCCD]))
    {
        apiResult = CY_BLE_ERROR_IND_DISABLED;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gatts_handle_value_ind_t indParam;
        
        indParam.handleValPair.attrHandle = cy_ble_cgmssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        indParam.handleValPair.value.val  = attrValue;
        indParam.handleValPair.value.len  = attrSize;
        indParam.connHandle               = connHandle;
        
        /* Send notification to client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Indication(&indParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            /* Save handle to support service-specific value confirmation response from client */
            cy_ble_cgmssReqHandle = indParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSS_ConfirmationEventHandler
***************************************************************************//**
*
*  Handles a Value Confirmation request event from the BLE Stack.
*
*  \param eventParam: The pointer to a structure of type cy_stc_ble_conn_handle_t
*
******************************************************************************/
static void Cy_BLE_CGMSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    if((Cy_BLE_CGMS_ApplCallback != NULL)
       && (cy_ble_cgmssConfigPtr->attrInfo->charInfo[CY_BLE_CGMS_RACP].charHandle == cy_ble_cgmssReqHandle))
    {
        cy_stc_ble_cgms_char_value_t locCharIndex;

        for(locCharIndex.charIndex = CY_BLE_CGMS_RACP;
            locCharIndex.charIndex < CY_BLE_CGMS_CHAR_COUNT; locCharIndex.charIndex++)
        {
            if(cy_ble_cgmssConfigPtr->attrInfo->charInfo[locCharIndex.charIndex].charHandle == cy_ble_cgmssReqHandle)
            {
                cy_ble_cgmssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                locCharIndex.connHandle = *eventParam;
                locCharIndex.value = NULL;

                if(locCharIndex.charIndex == CY_BLE_CGMS_RACP)
                {
                    cy_ble_cgmsFlag &= (uint8_t) ~CY_BLE_CGMS_FLAG_PROCESS;
                }

                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                Cy_BLE_CGMS_ApplCallback((uint32_t)CY_BLE_EVT_CGMSS_INDICATION_CONFIRMED, &locCharIndex);
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSS_WriteEventHandler
***************************************************************************//**
*
*  Handles a Write Request event. Calls the registered CGMS application
*  callback function.
*
* Note: Writing the attribute into GATT DB (if needed) is on the user's responsibility,
*  after the characteristic content check in the registered CGMS application
*  callback function.
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
static cy_en_ble_gatt_err_code_t Cy_BLE_CGMSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_stc_ble_cgms_char_value_t locCharValue;
    
    locCharValue.gattErrorCode = CY_BLE_GATT_ERR_NONE;
    locCharValue.connHandle    = eventParam->connHandle;
    bool exitFlag = false;

    if(Cy_BLE_CGMS_ApplCallback != NULL)
    {
        for(locCharValue.charIndex = CY_BLE_CGMS_CGMT;
            (locCharValue.charIndex < CY_BLE_CGMS_CHAR_COUNT) && (exitFlag == false); locCharValue.charIndex++)
        {
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
            
            dbAttrValInfo.handleValuePair = eventParam->handleValPair;
            dbAttrValInfo.connHandle      = eventParam->connHandle;
            dbAttrValInfo.offset          = 0u;
            dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;
            
            if(eventParam->handleValPair.attrHandle ==
               cy_ble_cgmssConfigPtr->attrInfo->charInfo[locCharValue.charIndex].charHandle)
            {
                locCharValue.value = &eventParam->handleValPair.value;

                if(locCharValue.charIndex == CY_BLE_CGMS_RACP)
                {
                    if(CY_BLE_CGMS_IS_PROCEDURE_IN_PROGRESS)
                    {
                        if(eventParam->handleValPair.value.val[0u] == CY_BLE_CGMS_RACP_OPCODE_ABORT)
                        {
                            cy_ble_cgmsFlag &= (uint8_t) ~CY_BLE_CGMS_FLAG_PROCESS;
                        }
                        else
                        {
                            locCharValue.gattErrorCode = CY_BLE_GATT_ERR_PROCEDURE_ALREADY_IN_PROGRESS;
                        }
                    }
                    else if(!CY_BLE_IS_INDICATION_ENABLED(eventParam->connHandle.attId,
                                                          cy_ble_cgmssConfigPtr->attrInfo->
                                                           charInfo[CY_BLE_CGMS_RACP].descrHandle[CY_BLE_CGMS_CCCD]))
                    {
                        locCharValue.gattErrorCode = CY_BLE_GATT_ERR_CCCD_IMPROPERLY_CONFIGURED;
                    }
                    else
                    {
                        cy_ble_cgmsFlag |= CY_BLE_CGMS_FLAG_PROCESS;
                    }
                }
                else
                {
                    /* CY_BLE_CGMS_SOCP or CY_BLE_CGMS_SSTM */
                }

                if(locCharValue.gattErrorCode == CY_BLE_GATT_ERR_NONE)
                {
                    Cy_BLE_CGMS_ApplCallback((uint32_t)CY_BLE_EVT_CGMSS_WRITE_CHAR, &locCharValue);
                }

                if(locCharValue.gattErrorCode == CY_BLE_GATT_ERR_NONE)
                {
                    locCharValue.gattErrorCode = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
                }

                /* Clear callback flag indicating that request was handled */
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                exitFlag = false;
            }
            else if(eventParam->handleValPair.attrHandle ==
                    cy_ble_cgmssConfigPtr->attrInfo->charInfo[locCharValue.charIndex].descrHandle[CY_BLE_CGMS_CCCD])
            {
                locCharValue.value = NULL;
                locCharValue.gattErrorCode = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
                if(locCharValue.gattErrorCode == CY_BLE_GATT_ERR_NONE)
                {
                    uint32_t eventCode;

                    if(locCharValue.charIndex == CY_BLE_CGMS_CGMT)
                    {
                        if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                        {
                            eventCode = (uint32_t)CY_BLE_EVT_CGMSS_NOTIFICATION_ENABLED;
                        }
                        else
                        {
                            eventCode = (uint32_t)CY_BLE_EVT_CGMSS_NOTIFICATION_DISABLED;
                        }
                    }
                    else
                    {
                        if(CY_BLE_IS_INDICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                        {
                            eventCode = (uint32_t)CY_BLE_EVT_CGMSS_INDICATION_ENABLED;
                        }
                        else
                        {
                            eventCode = (uint32_t)CY_BLE_EVT_CGMSS_INDICATION_DISABLED;
                        }
                    }

                    Cy_BLE_CGMS_ApplCallback(eventCode, &locCharValue);
                }

                /* Clear callback flag indicating that request was handled */
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                exitFlag = false;
            }
            else
            {
                /* Nothing else */
            }
        }
    }

    return(locCharValue.gattErrorCode);
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result a Write Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the \ref CY_BLE_EVT_CGMSS_WRITE_CHAR event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cgms_char_index_t. The valid values are
*                       * \ref CY_BLE_CGMS_CGMT
*                       * \ref CY_BLE_CGMS_CGFT
*                       * \ref CY_BLE_CGMS_CGST
*                       * \ref CY_BLE_CGMS_SSTM
*                       * \ref CY_BLE_CGMS_SRTM
*                       * \ref CY_BLE_CGMS_RACP
*                       * \ref CY_BLE_CGMS_SOCP
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
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the CGMS service-specific callback is registered
*      with \ref Cy_BLE_CGMS_RegisterAttrCallback():
*   * \ref CY_BLE_EVT_CGMSC_WRITE_CHAR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex , value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_cgms_char_value_t.
*   .
*   Otherwise (if an CGMS service-specific callback is not registered):
*   * \ref CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*     successfully read on the peer device, the details (handle, value, etc.) are
*     provided with an event parameter structure 
*     \ref cy_stc_ble_gattc_read_rsp_param_t.
*
*   * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CGMSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_cgms_char_index_t charIndex,
                                                           uint8_t attrSize,
                                                           uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    
    /* Get pointer (with offset) to CGMS client structure with attribute handles */
    cy_stc_ble_cgmsc_t *cgmscPtr = (cy_stc_ble_cgmsc_t *)&cy_ble_cgmscConfigPtr->attrInfo[discIdx];

    /* Check parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_CGMS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cgmscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_WRITE & cgmscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;
        
        writeReqParam.handleValPair.attrHandle = cgmscPtr->charInfo[charIndex].valueHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;
        
        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_cgmscReqHandle[discIdx] = cgmscPtr->charInfo[charIndex].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSC_GetCharacteristicValue
***************************************************************************//**
*
*  This function is used to read the characteristic value from a server
*  identified by charIndex.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cgms_char_index_t. The valid values are
*                       * \ref CY_BLE_CGMS_CGMT
*                       * \ref CY_BLE_CGMS_CGFT
*                       * \ref CY_BLE_CGMS_CGST
*                       * \ref CY_BLE_CGMS_SSTM
*                       * \ref CY_BLE_CGMS_SRTM
*                       * \ref CY_BLE_CGMS_RACP
*                       * \ref CY_BLE_CGMS_SOCP
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the CGMS service-specific callback is registered
*      with \ref Cy_BLE_CGMS_RegisterAttrCallback():
*   * \ref CY_BLE_EVT_CGMSC_READ_CHAR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details 
*     (charIndex , value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_cgms_char_value_t.
*   .
*   Otherwise (if an CGMS service-specific callback is not registered):
*   * \ref CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*     successfully read on the peer device, the details (handle, value, etc.) are
*     provided with an event parameter structure 
*     \ref cy_stc_ble_gattc_read_rsp_param_t.
*
*   * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CGMSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_cgms_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to CGMS client structure with attribute handles */
    cy_stc_ble_cgmsc_t *cgmscPtr = (cy_stc_ble_cgmsc_t *)&cy_ble_cgmscConfigPtr->attrInfo[discIdx];

    /* Check parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_CGMS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cgmscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_READ & cgmscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = cgmscPtr->charInfo[charIndex].valueHandle;
        readReqParam.connHandle = connHandle;
        

        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_cgmscReqHandle[discIdx] = cgmscPtr->charInfo[charIndex].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to the peer device to set the characteristic descriptor of the
*  specified characteristic of Continuous Glucose Monitoring service.
*
*  Internally, Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*    * \ref CY_BLE_EVT_CGMSS_INDICATION_ENABLED
*    * \ref CY_BLE_EVT_CGMSS_INDICATION_DISABLED
*    * \ref CY_BLE_EVT_CGMSS_NOTIFICATION_ENABLED
*    * \ref CY_BLE_EVT_CGMSS_NOTIFICATION_DISABLED
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cgms_char_index_t. The valid values are
*                       * \ref CY_BLE_CGMS_CGMT
*                       * \ref CY_BLE_CGMS_CGFT
*                       * \ref CY_BLE_CGMS_CGST
*                       * \ref CY_BLE_CGMS_SSTM
*                       * \ref CY_BLE_CGMS_SRTM
*                       * \ref CY_BLE_CGMS_RACP
*                       * \ref CY_BLE_CGMS_SOCP
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_cgms_descr_index_t. The valid value is,
*                       * \ref CY_BLE_CGMS_CCCD
*  \param attrSize:   The size of the characteristic descriptor value attribute.
*  \param attrValue:  The pointer to the characteristic descriptor value data that should
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
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the CGMS service-specific callback is registered
*      with \ref Cy_BLE_CGMS_RegisterAttrCallback():
*   * \ref CY_BLE_EVT_CGMSC_WRITE_DESCR_RESPONSE - In case if the requested attribute is
*     successfully written on the peer device, the details (charIndex, descr 
*     index etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_cgms_descr_value_t.
*   .
*   Otherwise (if an CGMS service-specific callback is not registered):
*   * \ref CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*
*   * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure ( \ref cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CGMSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_cgms_char_index_t charIndex,
                                                                cy_en_ble_cgms_descr_index_t descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to CGMS client structure with attribute handles */
    cy_stc_ble_cgmsc_t *cgmscPtr = (cy_stc_ble_cgmsc_t *)&cy_ble_cgmscConfigPtr->attrInfo[discIdx];

    /* Check parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_CGMS_CHAR_COUNT) || (descrIndex >= CY_BLE_CGMS_DESCR_COUNT)
            || (attrSize != CY_BLE_CCCD_LEN) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cgmscPtr->charInfo[charIndex].descrHandle[CY_BLE_CGMS_CCCD] ==
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        
        writeReqParam.handleValPair.attrHandle = cgmscPtr->charInfo[charIndex].descrHandle[CY_BLE_CGMS_CCCD];
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = CY_BLE_CCCD_LEN;
        writeReqParam.connHandle               = connHandle;
        
        /* ... and send request to server device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            /* Save handle to support service-specific read response from device */
            cy_ble_cgmscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to the peer device to get the characteristic descriptor of the
*  specified characteristic of Continuous Glucose Monitoring  service.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of type
*                     \ref cy_en_ble_cgms_char_index_t. The valid values are
*                       * \ref CY_BLE_CGMS_CGMT
*                       * \ref CY_BLE_CGMS_CGFT
*                       * \ref CY_BLE_CGMS_CGST
*                       * \ref CY_BLE_CGMS_SSTM
*                       * \ref CY_BLE_CGMS_SRTM
*                       * \ref CY_BLE_CGMS_RACP
*                       * \ref CY_BLE_CGMS_SOCP
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_cgms_descr_index_t. The valid value is,
*                       * \ref CY_BLE_CGMS_CCCD
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*  If the CGMS service-specific callback is registered
*      (with \ref Cy_BLE_CGMS_RegisterAttrCallback):
*  * \ref CY_BLE_EVT_CGMSC_READ_DESCR_RESPONSE - In case if the requested attribute is
*    successfully read on the peer device, the details (charIndex, descrIndex, 
*    value, etc.) are provided with event parameter structure
*    of type cy_stc_ble_cgms_descr_value_t.
*  .
*  Otherwise (if an CGMS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*    successfully read on the peer device, the details (handle, value, etc.) are
*    provided with an event parameter structure 
*    \ref cy_stc_ble_gattc_read_rsp_param_t.
*
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*    requested attribute on the peer device, the details are provided with 
*    an event parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_CGMSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_cgms_char_index_t charIndex,
                                                                cy_en_ble_cgms_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to CGMS client structure with attribute handles */
    cy_stc_ble_cgmsc_t *cgmscPtr = (cy_stc_ble_cgmsc_t *)&cy_ble_cgmscConfigPtr->attrInfo[discIdx];

    /* Check parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_CGMS_CHAR_COUNT) || (descrIndex >= CY_BLE_CGMS_DESCR_COUNT) ||
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cgmscPtr->charInfo[charIndex].descrHandle[CY_BLE_CGMS_CCCD] ==
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = cgmscPtr->charInfo[charIndex].descrHandle[CY_BLE_CGMS_CCCD];
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_cgmscReqHandle[discIdx] = cgmscPtr->charInfo[charIndex].descrHandle[CY_BLE_CGMS_CCCD];
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_CGMSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* CGM service characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_cgmscCharUuid[CY_BLE_CGMS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_CGM_MEASUREMENT,
        CY_BLE_UUID_CHAR_CGM_FEATURE,
        CY_BLE_UUID_CHAR_CGM_STATUS,
        CY_BLE_UUID_CHAR_CGM_SESSION_START_TIME,
        CY_BLE_UUID_CHAR_CGM_SESSION_RUN_TIME,
        CY_BLE_UUID_CHAR_RACP,
        CY_BLE_UUID_CHAR_CGM_SOCP
    };
    
    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t cgmsDiscIdx = cy_ble_cgmscConfigPtr->serviceDiscIdx;
    uint32_t i;

    /* Get pointer (with offset) to CGMS client structure with attribute handles */
    cy_stc_ble_cgmsc_t *cgmscPtr = (cy_stc_ble_cgmsc_t *)&cy_ble_cgmscConfigPtr->attrInfo[discIdx];

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == cgmsDiscIdx))
    {
        /* Update last characteristic endHandle to declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        for(i = 0u; i < (uint32_t)CY_BLE_CGMS_CHAR_COUNT; i++)
        {
            if(cy_ble_cgmscCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                if(cgmscPtr->charInfo[i].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    cgmscPtr->charInfo[i].valueHandle = discCharInfo->valueHandle;
                    cgmscPtr->charInfo[i].properties = discCharInfo->properties;
                    lastEndHandle[discIdx] = &cgmscPtr->charInfo[i].endHandle;
                }
                else
                {
                    Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo);
                }
            }
        }

        /* Init characteristic endHandle to service endHandle.
         * Characteristic endHandle will be updated to declaration
         * Handler of following characteristic,
         * in following characteristic discovery procedure. */
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
* Function Name: Cy_BLE_CGMSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_CGMSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t notSupportedDescr = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t cgmsDiscIdx = cy_ble_cgmscConfigPtr->serviceDiscIdx;
    uint32_t descIdx;

    /* Get pointer (with offset) to CGMS client structure with attribute handles */
    cy_stc_ble_cgmsc_t *cgmscPtr = (cy_stc_ble_cgmsc_t *)&cy_ble_cgmscConfigPtr->attrInfo[discIdx];

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == cgmsDiscIdx)
    {
        switch(discDescrInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_CLIENT_CONFIG:
                descIdx = (uint32_t)CY_BLE_CGMS_CCCD;
                break;

            default:
                /* Not supported descriptor */
                notSupportedDescr = 1u;
                break;
        }

        if(notSupportedDescr == 0u)
        {
            if(cgmscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] ==
               CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                cgmscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] =
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
* Function Name: Cy_BLE_CGMSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_CGMSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t cgmsDiscIdx = cy_ble_cgmscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    /* Get pointer (with offset) to CGMS client structure with attribute handles */
    cy_stc_ble_cgmsc_t *cgmscPtr = (cy_stc_ble_cgmsc_t *)&cy_ble_cgmscConfigPtr->attrInfo[discIdx];

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == cgmsDiscIdx)
    {
        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_CGMS_CHAR_COUNT) && (exitFlag == 0u))
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            if((cgmscPtr->charInfo[charIdx].endHandle -
                cgmscPtr->charInfo[charIdx].valueHandle) != 0u)
            {
                /* Read characteristic range */
                charRangeInfo->range.startHandle = cgmscPtr->charInfo[charIdx].valueHandle + 1u;
                charRangeInfo->range.endHandle = cgmscPtr->charInfo[charIdx].endHandle;
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
* Function Name: Cy_BLE_CGMSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles a Notification event.
*
*  \param eventParam: The pointer to a data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_CGMSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    /* Get pointer (with offset) to CGMS client structure with attribute handles */
    cy_stc_ble_cgmsc_t *cgmscPtr = (cy_stc_ble_cgmsc_t *)&cy_ble_cgmscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_CGMS_ApplCallback != NULL))
    {
        if(cgmscPtr->charInfo[CY_BLE_CGMS_CGMT].valueHandle == eventParam->handleValPair.attrHandle)
        {
            cy_stc_ble_cgms_char_value_t locCharValue;
            
            locCharValue.charIndex  = CY_BLE_CGMS_CGMT;
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.value      = &eventParam->handleValPair.value;
            
            Cy_BLE_CGMS_ApplCallback((uint32_t)CY_BLE_EVT_CGMSC_NOTIFICATION, &locCharValue);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSC_IndicationEventHandler
***************************************************************************//**
*
*  Handles an Indication event.
*
*  \param eventParam:  The pointer to a data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_CGMSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    /* Get pointer (with offset) to CGMS client structure with attribute handles */
    cy_stc_ble_cgmsc_t *cgmscPtr = (cy_stc_ble_cgmsc_t *)&cy_ble_cgmscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_CGMS_ApplCallback != NULL))
    {
        cy_stc_ble_cgms_char_value_t locCharValue;
        locCharValue.charIndex = CY_BLE_CGMS_RACP;

        while((cgmscPtr->charInfo[locCharValue.charIndex].valueHandle !=
               eventParam->handleValPair.attrHandle) && (locCharValue.charIndex < CY_BLE_CGMS_CHAR_COUNT))
        {
            locCharValue.charIndex++;
        }

        if(locCharValue.charIndex < CY_BLE_CGMS_CHAR_COUNT)
        {
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.value = &eventParam->handleValPair.value;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_CGMS_ApplCallback((uint32_t)CY_BLE_EVT_CGMSC_INDICATION, &locCharValue);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles a Read Response event.
*
*  \param eventParam: The pointer to the data structure.
*
******************************************************************************/
static void Cy_BLE_CGMSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    /* Get pointer (with offset) to CGMS client structure with attribute handles */
    cy_stc_ble_cgmsc_t *cgmscPtr = (cy_stc_ble_cgmsc_t *)&cy_ble_cgmscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_CGMS_ApplCallback != NULL) &&
       (cy_ble_cgmscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        cy_en_ble_cgms_char_index_t charIdx;
        uint8_t fFlag = 0u;

        for(charIdx = CY_BLE_CGMS_CGMT; (charIdx < CY_BLE_CGMS_CHAR_COUNT) && (fFlag == 0u); charIdx++)
        {
            if(cgmscPtr->charInfo[charIdx].valueHandle == cy_ble_cgmscReqHandle[discIdx])
            {
                cy_stc_ble_cgms_char_value_t locCharValue;
                
                locCharValue.connHandle = eventParam->connHandle;
                locCharValue.charIndex  = charIdx;
                locCharValue.value      = &eventParam->value;
                
                cy_ble_cgmscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                Cy_BLE_CGMS_ApplCallback((uint32_t)CY_BLE_EVT_CGMSC_READ_CHAR_RESPONSE, &locCharValue);
                fFlag = 1u; /* instead of break */
            }
            else if(cgmscPtr->charInfo[charIdx].descrHandle[CY_BLE_CGMS_CCCD] ==
                    cy_ble_cgmscReqHandle[discIdx])
            {
                cy_stc_ble_cgms_descr_value_t locDescrValue;
                
                locDescrValue.connHandle = eventParam->connHandle;
                locDescrValue.charIndex  = charIdx;
                locDescrValue.descrIndex = CY_BLE_CGMS_CCCD;
                locDescrValue.value      = &eventParam->value;
                
                cy_ble_cgmscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                Cy_BLE_CGMS_ApplCallback((uint32_t)CY_BLE_EVT_CGMSC_READ_DESCR_RESPONSE, &locDescrValue);
                fFlag = 1u; /* instead of break */
            }
            else
            {
                /* nothing else */
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles a Write Response event.
*
*  \param eventParam: The pointer to a data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_CGMSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    /* Get pointer (with offset) to CGMS client structure with attribute handles */
    cy_stc_ble_cgmsc_t *cgmscPtr = (cy_stc_ble_cgmsc_t *)&cy_ble_cgmscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_CGMS_ApplCallback != NULL) && (cy_ble_cgmscReqHandle[discIdx] !=
                                                                                       CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        cy_en_ble_cgms_char_index_t charIdx;
        uint8_t fFlag = 0u;

        for(charIdx = CY_BLE_CGMS_CGMT; (charIdx < CY_BLE_CGMS_CHAR_COUNT) && (fFlag == 0u); charIdx++)
        {
            if(cgmscPtr->charInfo[charIdx].valueHandle == cy_ble_cgmscReqHandle[discIdx])
            {
                cy_stc_ble_cgms_char_value_t locCharValue;
                
                locCharValue.connHandle = *eventParam;
                locCharValue.charIndex  = charIdx;
                locCharValue.value      = NULL;
                
                cy_ble_cgmscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                Cy_BLE_CGMS_ApplCallback((uint32_t)CY_BLE_EVT_CGMSC_WRITE_CHAR_RESPONSE, &locCharValue);
                fFlag = 1u; /* instead of break */
            }
            else if(cgmscPtr->charInfo[charIdx].descrHandle[CY_BLE_CGMS_CCCD] == cy_ble_cgmscReqHandle[discIdx])
            {
                cy_stc_ble_cgms_descr_value_t locDescrValue;
                
                locDescrValue.connHandle = *eventParam;
                locDescrValue.charIndex  = charIdx;
                locDescrValue.descrIndex = CY_BLE_CGMS_CCCD;
                locDescrValue.value      = NULL;
                
                cy_ble_cgmscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                Cy_BLE_CGMS_ApplCallback((uint32_t)CY_BLE_EVT_CGMSC_WRITE_DESCR_RESPONSE, &locDescrValue);
                fFlag = 1u; /* instead of break */
            }
            else
            {
                /* nothing else */
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles an Error Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_CGMSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        if(cy_ble_cgmscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] == eventParam->errInfo.attrHandle)
        {
            cy_ble_cgmscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_CGMS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the CGMS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_CGMS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_CGMSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_CGMSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_CGMSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_CGMSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the CGMS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_CGMSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
                gattErr = Cy_BLE_CGMSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
                break;

        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            Cy_BLE_CGMSS_ConfirmationEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
            break;

        case CY_BLE_EVT_GATT_CONNECT_IND:
            (void)Cy_BLE_CGMSS_Init(cy_ble_cgmssConfigPtr);
            break;
                
        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_CGMSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the CGMS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_CGMSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_CGMSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_CGMSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_CGMSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_CGMSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
            break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_CGMSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_IND:
                Cy_BLE_CGMSC_IndicationEventHandler((cy_stc_ble_gattc_handle_value_ind_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_CGMSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_CGMSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
