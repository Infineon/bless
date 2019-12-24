/***************************************************************************//**
* \file cy_ble_bcs.c
* \version 3.30
*
* \brief
*  Contains the source code for the Body Composition service.
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

static cy_en_ble_gatt_err_code_t Cy_BLE_BCS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_BCSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_BCSC_EventHandler(uint32_t eventCode, void *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_BCSS_WriteEventHandler(const cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);
static void Cy_BLE_BCSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam);

static void Cy_BLE_BCSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_BCSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_BCSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_BCSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam);
static void Cy_BLE_BCSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_BCSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_BCSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_BCS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_BCSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_BCSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response for server */
static cy_ble_gatt_db_attr_handle_t cy_ble_bcssReqHandle;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_bcscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE BCS server config structure */
const cy_stc_ble_bcss_config_t *cy_ble_bcssConfigPtr = NULL;

/* The pointer to a global BLE BCS client config structure */
const cy_stc_ble_bcsc_config_t *cy_ble_bcscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_BCSS_Init
***************************************************************************//**
*
*  This function initializes server of the Body Composition service.
*
*  \param config: Configuration structure for the BCS.
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
cy_en_ble_api_result_t Cy_BLE_BCSS_Init(const cy_stc_ble_bcss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_bcssConfigPtr = config;

        /* Registers event handler for the BCS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_BCS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_BCSS_EventHandlerCallback = &Cy_BLE_BCSS_EventHandler;
        
        cy_ble_bcssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BCSC_Init
***************************************************************************//**
*
*  This function initializes client of the Body Composition service.
*
*  \param config: Configuration structure for the BCS.
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
cy_en_ble_api_result_t Cy_BLE_BCSC_Init(const cy_stc_ble_bcsc_config_t *config)
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
        cy_ble_bcscConfigPtr = config;

        /* Registers event handler for the BCS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_BCS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_BCSC_EventHandlerCallback = &Cy_BLE_BCSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 bcsServIdx = cy_ble_bcscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + bcsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to BCS client structure */
                cy_stc_ble_bcsc_t *bcscPtr = (cy_stc_ble_bcsc_t *)&cy_ble_bcscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(bcscPtr, 0, sizeof(cy_stc_ble_bcsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + bcsServIdx].uuid =
                    CY_BLE_UUID_BODY_COMPOSITION_SERVICE;
            }

            cy_ble_bcscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BCS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for BCS is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_BCSS_INDICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_bcs_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_BCS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_BCS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_BCSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a value for one of the three characteristic values of the Body Composition
*  service. The characteristic is identified by charIndex.
*
*  \param charIndex: The index of a Body Composition service characteristic of type
*                    \ref cy_en_ble_bcs_char_index_t. The valid values are,
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_FEATURE  
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT
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
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BCSS_SetCharacteristicValue(cy_en_ble_bcs_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

    if((attrValue != NULL) && (charIndex < CY_BLE_BCS_CHAR_COUNT))
    {
        /* Fill the structure */
        if(charIndex == CY_BLE_BCS_BODY_COMPOSITION_FEATURE)
        {
            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_bcssConfigPtr->
                                                        attrInfo->charInfo[CY_BLE_BCS_BODY_COMPOSITION_FEATURE].charHandle;
        }
        else
        {
            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_bcssConfigPtr->
                                                        attrInfo->charInfo[CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT].charHandle;
        }
        
        dbAttrValInfo.connHandle.bdHandle       = 0u;
        dbAttrValInfo.connHandle.attId          = 0u;
        dbAttrValInfo.handleValuePair.value.len = attrSize;
        dbAttrValInfo.handleValuePair.value.val = attrValue;
        dbAttrValInfo.offset = 0u;
        dbAttrValInfo.flags = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        
        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            /* Indicate success */
            apiResult = CY_BLE_SUCCESS;
        }
    }

    /* Return a status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BCSS_GetCharacteristicValue
***************************************************************************//**
*
*  Reads a characteristic value of the Body Composition service identified
*  by charIndex from the GATT database.
*
*  \param charIndex: The index of a Body Composition service characteristic of type
*                    \ref cy_en_ble_bcs_char_index_t. The valid values are,
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_FEATURE  
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT
*  \param attrSize:  The size of the Body Composition service characteristic 
*                    value attribute.
*  \param attrValue: The pointer to the location where characteristic value data
*                    should be stored.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BCSS_GetCharacteristicValue(cy_en_ble_bcs_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_ble_gatt_db_attr_handle_t tmpCharHandle;
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((attrValue != NULL) && (charIndex < CY_BLE_BCS_CHAR_COUNT))
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.handleValuePair.value.len = attrSize;
        dbAttrValInfo.handleValuePair.value.val = attrValue;
        dbAttrValInfo.flags                     = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        dbAttrValInfo.offset                    = 0u;

        if(charIndex == CY_BLE_BCS_BODY_COMPOSITION_FEATURE)
        {
            tmpCharHandle = cy_ble_bcssConfigPtr->attrInfo->charInfo[CY_BLE_BCS_BODY_COMPOSITION_FEATURE].charHandle;
        }
        else
        {
            tmpCharHandle = cy_ble_bcssConfigPtr->attrInfo->charInfo[CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT].charHandle;
        }

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = tmpCharHandle;

        /* Get a characteristic value from the GATT database */
        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            /* Indicate success */
            apiResult = CY_BLE_SUCCESS;
        }
    }

    /* Return a status */
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BCSS_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sets the characteristic descriptor of the specified characteristic.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of a Body Composition service characteristic of type
*                     \ref cy_en_ble_bcs_char_index_t. The valid values are,
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_FEATURE  
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_bcs_descr_index_t. The valid value is,
*                       * #CY_BLE_BCS_CCCD
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  The pointer to the descriptor value data to be stored in 
*                     the GATT database.
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
cy_en_ble_api_result_t Cy_BLE_BCSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bcs_char_index_t charIndex,
                                                               cy_en_ble_bcs_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex == CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT) && (descrIndex == CY_BLE_BCS_CCCD) && (attrValue != NULL))
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_bcssConfigPtr->attrInfo->charInfo[CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT].
                                                   descrHandle[CY_BLE_BCS_CCCD];
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.connHandle                 = connHandle;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        
        /* Read the characteristic value from the database */
        if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_SUCCESS;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BCSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Reads a a characteristic descriptor of the specified characteristic of the
*  Body Composition service from the GATT database.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of a Body Composition service characteristic of type
*                     \ref cy_en_ble_bcs_char_index_t. The valid values are,
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_FEATURE  
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_bcs_descr_index_t. The valid value is,
*                       * #CY_BLE_BCS_CCCD
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  The pointer to the location where characteristic descriptor value
*                     data should be stored.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The optional descriptor is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BCSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bcs_char_index_t charIndex,
                                                               cy_en_ble_bcs_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex == CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT) && (descrIndex == CY_BLE_BCS_CCCD) && (attrValue != NULL))
    {
        if(cy_ble_bcssConfigPtr->attrInfo->charInfo[CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT].descrHandle[CY_BLE_BCS_CCCD] !=
           CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
            
            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_bcssConfigPtr->attrInfo->
                                                       charInfo[CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT].
                                                       descrHandle[CY_BLE_BCS_CCCD];
            dbAttrValInfo.handleValuePair.value.len  = attrSize;
            dbAttrValInfo.handleValuePair.value.val  = attrValue;
            dbAttrValInfo.connHandle                 = connHandle;
            dbAttrValInfo.offset                     = 0u;
            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;

            /* Get a characteristic value from the GATT database */
            if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
            {
                /* Indicate success */
                apiResult = CY_BLE_SUCCESS;
            }
            else
            {
                apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
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
* Function Name: Cy_BLE_BCSS_WriteEventHandler
***************************************************************************//**
*
*  Handles a Write Request event for the Body Composition service.
*
*  \param void *eventParam - The pointer to the data structure specified by the event.
*
* \return
*  A return value of type cy_en_ble_gatt_err_code_t.
*
*   Error Codes                           | Description
*   ------------                          | -----------
*   CY_BLE_GATT_ERR_NONE                  | The write is successful.
*   CY_BLE_GATT_ERR_REQUEST_NOT_SUPPORTED | The request is not supported.
*   CY_BLE_GATT_ERR_INVALID_HANDLE        | 'handleValuePair.attrHandle' is not valid.
*   CY_BLE_GATT_ERR_WRITE_NOT_PERMITTED   | The write operation is not permitted on this attribute.
*   CY_BLE_GATT_ERR_INVALID_OFFSET        | The offset value is invalid.
*   CY_BLE_GATT_ERR_UNLIKELY_ERROR        | Some other error occurred.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BCSS_WriteEventHandler(const cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    uint32_t event = (uint32_t)CY_BLE_EVT_BCSS_INDICATION_DISABLED;

    if(Cy_BLE_BCS_ApplCallback != NULL)
    {
        /* Client characteristic configuration descriptor Write Request */
        if(eventParam->handleValPair.attrHandle ==
           cy_ble_bcssConfigPtr->attrInfo->charInfo[CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT].descrHandle[CY_BLE_BCS_CCCD])
        {
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
            
            dbAttrValInfo.handleValuePair = eventParam->handleValPair;
            dbAttrValInfo.connHandle      = eventParam->connHandle;
            dbAttrValInfo.offset          = 0u;
            dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;
            
            if(CY_BLE_IS_INDICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
            {
                event = (uint32_t)CY_BLE_EVT_BCSS_INDICATION_ENABLED;
            }

            gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
            if(gattErr == CY_BLE_GATT_ERR_NONE)
            {
                cy_stc_ble_bcs_char_value_t wrReqParam;
                wrReqParam.connHandle = eventParam->connHandle;
                wrReqParam.charIndex  = CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT;
                wrReqParam.value      = NULL;

                Cy_BLE_BCS_ApplCallback(event, &wrReqParam);
            }
            /* Clear a callback flag indicating that the request was handled */
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_BCSS_ConfirmationEventHandler
***************************************************************************//**
*
*  Handles a value confirmation request event from the BLE Stack.
*
*  \param eventParam: The pointer to a structure of type cy_stc_ble_conn_handle_t.
*
******************************************************************************/
static void Cy_BLE_BCSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    if((Cy_BLE_BCS_ApplCallback != NULL) && (cy_ble_bcssReqHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Only the weight measurement characteristic has the indication property. Check whether
         * the requested handle is the handle of the descriptor value change handle.
         */
        if(cy_ble_bcssReqHandle == cy_ble_bcssConfigPtr->attrInfo->charInfo[CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT].charHandle)
        {
            /* Fill in the event data and inform application about successfully
             * confirmed indication.
             */
            cy_stc_ble_bcs_char_value_t locCharValue;
            
            locCharValue.connHandle = *eventParam;
            locCharValue.charIndex  = CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT;
            locCharValue.value      = NULL;
            
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            cy_ble_bcssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_BCS_ApplCallback((uint32_t)CY_BLE_EVT_BCSS_INDICATION_CONFIRMED, &locCharValue);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BCSS_SendIndication
***************************************************************************//**
*
*  Sends indication with a characteristic value of the Body Composition service,
*  which is a value specified by charIndex to the client device.
*
*  On enabling indication successfully it sends out a handle value indication, which
*  results in \ref CY_BLE_EVT_BCSC_INDICATION or \ref CY_BLE_EVT_GATTC_HANDLE_VALUE_IND (if
*  service-specific callback function is not registered) event at the GATT client's end.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of a Body Composition service characteristic of type
*                     \ref cy_en_ble_bcs_char_index_t. The valid values are,
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_FEATURE  
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     sent to the client's device.
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
*  \events
*  If execution is successful (return value = \ref CY_BLE_SUCCESS),
*  these events can appear: <BR>
*  If a BCS service-specific callback is registered
*  (with \ref Cy_BLE_BCS_RegisterAttrCallback):
*  * \ref CY_BLE_EVT_BCSS_INDICATION_CONFIRMED - Whether the indication is
*    successfully delivered to the peer device.
*  
*  Otherwise (if a BCS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF - Whether the indication is
*    successfully delivered to the peer device.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BCSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_bcs_char_index_t charIndex,
                                                  uint8_t attrSize,
                                                  uint8_t *attrValue)
{
    /* Store new data in database */
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if(charIndex == CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT)
    {
        /* Send indication if it is enabled and connected */
        if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
        {
            apiResult = CY_BLE_ERROR_INVALID_STATE;
        }
        else if(!CY_BLE_IS_INDICATION_ENABLED(connHandle.attId,
                                              cy_ble_bcssConfigPtr->attrInfo->charInfo[CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT].
                                               descrHandle[CY_BLE_BCS_CCCD]))
        {
            apiResult = CY_BLE_ERROR_IND_DISABLED;
        }
        else
        {
            /* Fill all the fields of the Write Request structure ... */
            cy_stc_ble_gatts_handle_value_ind_t indReqParam;
            
            indReqParam.handleValPair.attrHandle = cy_ble_bcssConfigPtr->attrInfo->
                                                   charInfo[CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT].charHandle;
            indReqParam.handleValPair.value.val  = attrValue;
            indReqParam.handleValPair.value.len  = attrSize;
            indReqParam.connHandle               = connHandle;
            
            /* Send indication to the client using a previously filled structure */
            apiResult = Cy_BLE_GATTS_Indication(&indReqParam);

            /* Save a handle to support service-specific value confirmation response from the client */
            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_bcssReqHandle = indReqParam.handleValPair.attrHandle;
            }
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BCSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_BCSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    uint32_t bcsDiscIdx = cy_ble_bcscConfigPtr->serviceDiscIdx;

    /* BCS characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_bcscCharUuid[CY_BLE_BCS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_BODY_COMPOSITION_FEATURE,
        CY_BLE_UUID_CHAR_BODY_COMPOSITION_MEASUREMENT
    };

    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t i;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == bcsDiscIdx))
    {
        /* Get pointer (with offset) to BCS client structure with attribute handles */
        cy_stc_ble_bcsc_t *bcscPtr = (cy_stc_ble_bcsc_t *)&cy_ble_bcscConfigPtr->attrInfo[discIdx];

        /* Update the last characteristic endHandle to the declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        /* Search through all available characteristics */
        for(i = (uint32_t)CY_BLE_BCS_BODY_COMPOSITION_FEATURE; i < (uint32_t)CY_BLE_BCS_CHAR_COUNT; i++)
        {
            if(cy_ble_bcscCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                if(bcscPtr->charInfo[i].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    bcscPtr->charInfo[i].valueHandle = discCharInfo->valueHandle;
                    bcscPtr->charInfo[i].properties = discCharInfo->properties;
                    lastEndHandle[discIdx] = &bcscPtr->charInfo[i].endHandle;
                    break;
                }
                else
                {
                    Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo);
                }
            }
        }

        /* Initialize the characteristic endHandle to the service endHandle. The characteristic endHandle
         * will be updated to the declaration handle of the following characteristic
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


/******************************************************************************s
* Function Name: Cy_BLE_BCSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  Based on the descriptor UUID, the appropriate data structure is populated using
*  the data received as part of the callback.
*
*  \param discDescrInfo: The pointer to a descriptor information structure.
*
******************************************************************************/
static void Cy_BLE_BCSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t bcsDiscIdx = cy_ble_bcscConfigPtr->serviceDiscIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == bcsDiscIdx)
    {
        if(discDescrInfo->uuid.uuid16 == CY_BLE_UUID_CHAR_CLIENT_CONFIG)
        {
            /* Get pointer (with offset) to BCS client structure with attribute handles */
            cy_stc_ble_bcsc_t *bcscPtr = (cy_stc_ble_bcsc_t *)&cy_ble_bcscConfigPtr->attrInfo[discIdx];

            if(bcscPtr->bodyCompositionMeasurementCccdHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                bcscPtr->bodyCompositionMeasurementCccdHandle = discDescrInfo->descrHandle;
            }
            else    /* Duplication of the descriptor */
            {
                Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_DESCR_DUPLICATION, &discDescrInfo);
            }
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}

/******************************************************************************
* Function Name: Cy_BLE_BCSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_BCSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t bcsDiscIdx = cy_ble_bcscConfigPtr->serviceDiscIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == bcsDiscIdx)
    {
        /* Get pointer (with offset) to BCS client structure with attribute handles */
        cy_stc_ble_bcsc_t *bcscPtr = (cy_stc_ble_bcsc_t *)&cy_ble_bcscConfigPtr->attrInfo[discIdx];

        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        if(cy_ble_configPtr->context->discovery[discIdx].charCount == 0u)
        {
            charRangeInfo->range.startHandle = bcscPtr->charInfo[CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT].
                                                valueHandle + 1u;
            charRangeInfo->range.endHandle = bcscPtr->charInfo[CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT].
                                              endHandle;
        }
        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}

/******************************************************************************
* Function Name: Cy_BLE_BCSC_IndicationEventHandler
***************************************************************************//**
*
*  Handles an Indication event for the Body Composition service.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BCSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_BCS_ApplCallback != NULL))
    {
        /* Get pointer (with offset) to BCS client structure with attribute handles */
        cy_stc_ble_bcsc_t *bcscPtr = (cy_stc_ble_bcsc_t *)&cy_ble_bcscConfigPtr->attrInfo[discIdx];

        if(bcscPtr->charInfo[CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT].valueHandle ==
           eventParam->handleValPair.attrHandle)
        {
            cy_stc_ble_bcs_char_value_t indicationValue;
            
            indicationValue.connHandle = eventParam->connHandle;
            indicationValue.charIndex  = CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT;
            indicationValue.value      = &eventParam->handleValPair.value;
            
            Cy_BLE_BCS_ApplCallback((uint32_t)CY_BLE_EVT_BCSC_INDICATION, &indicationValue);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BCSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles a read response event for the Body Composition service.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BCSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t locReqHandle = 1u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_BCS_ApplCallback != NULL) &&
       (cy_ble_bcscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to BCS client structure with attribute handles */
        cy_stc_ble_bcsc_t *bcscPtr = (cy_stc_ble_bcsc_t *)&cy_ble_bcscConfigPtr->attrInfo[discIdx];

        if((cy_ble_bcscReqHandle[discIdx] ==
            bcscPtr->charInfo[CY_BLE_BCS_BODY_COMPOSITION_FEATURE].valueHandle) ||
           (cy_ble_bcscReqHandle[discIdx] ==
            bcscPtr->charInfo[CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT].valueHandle))
        {
            cy_stc_ble_bcs_char_value_t locCharValue = { .connHandle = eventParam->connHandle };
            if(cy_ble_bcscReqHandle[discIdx] == bcscPtr->charInfo[CY_BLE_BCS_BODY_COMPOSITION_FEATURE].
                valueHandle)
            {
                locCharValue.charIndex = CY_BLE_BCS_BODY_COMPOSITION_FEATURE;
            }
            else
            {
                locCharValue.charIndex = CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT;
            }

            locCharValue.value = &eventParam->value;
            cy_ble_bcscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_BCS_ApplCallback((uint32_t)CY_BLE_EVT_BCSC_READ_CHAR_RESPONSE, &locCharValue);
        }
        else if(cy_ble_bcscReqHandle[discIdx] == bcscPtr->bodyCompositionMeasurementCccdHandle)
        {
            cy_stc_ble_bcs_descr_value_t locDescrValue;
            
            locDescrValue.connHandle = eventParam->connHandle;
            locDescrValue.charIndex  = CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT;
            locDescrValue.descrIndex = CY_BLE_BCS_CCCD;
            locDescrValue.value      = &eventParam->value;
            
            cy_ble_bcscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_BCS_ApplCallback((uint32_t)CY_BLE_EVT_BCSC_READ_DESCR_RESPONSE, &locDescrValue);
        }
        else
        {
            locReqHandle = 0u;
        }

        if(locReqHandle != 0u)
        {
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BCSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles a Write Response event for the Body Composition service.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BCSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    /* Check whether the service handler was registered and the request handle is valid */
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_BCS_ApplCallback != NULL) &&
       (cy_ble_bcscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to BCS client structure with attribute handles */
        cy_stc_ble_bcsc_t *bcscPtr = (cy_stc_ble_bcsc_t *)&cy_ble_bcscConfigPtr->attrInfo[discIdx];

        if(cy_ble_bcscReqHandle[discIdx] == bcscPtr->bodyCompositionMeasurementCccdHandle)
        {
            cy_stc_ble_bcs_descr_value_t locDescrValue;
            
            locDescrValue.connHandle = *eventParam;
            locDescrValue.charIndex  = CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT;
            locDescrValue.descrIndex = CY_BLE_BCS_CCCD;
            locDescrValue.value      = NULL;
            
            cy_ble_bcscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            Cy_BLE_BCS_ApplCallback((uint32_t)CY_BLE_EVT_BCSC_WRITE_DESCR_RESPONSE, &locDescrValue);
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BCSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles an Error Response event for the Body Composition service.
*
*  \param eventParam - The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BCSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        if(cy_ble_bcscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] == eventParam->errInfo.attrHandle)
        {
            cy_ble_bcscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BCSC_GetCharacteristicValue
***************************************************************************//**
*
*  This function is used to read a characteristic value, which is a value
*  identified by charIndex, from the server.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of a Body Composition service characteristic of type
*                     \ref cy_en_ble_bcs_char_index_t. The valid values are,
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_FEATURE  
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT
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
*  \events
*  If execution is successful (return value = \ref CY_BLE_SUCCESS),
*  these events can appear: \n
*  If a BCS service-specific callback is registered
*  with \ref Cy_BLE_BCS_RegisterAttrCallback():
*  * \ref CY_BLE_EVT_BCSC_READ_CHAR_RESPONSE - If the requested attribute is
*    successfully read on the peer device, the details (charIndex , 
*    value, etc.) are provided with an event parameter structure
*    of type \ref cy_stc_ble_bcs_char_value_t.
*  
*  Otherwise (if a BCS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*    successfully read on the peer device, the details (handle, 
*    value, etc.) are provided with the event parameters
*    structure (cy_stc_ble_gattc_read_rsp_param_t).
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*    requested attribute on the peer device, the details are provided 
*    with the event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BCSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_bcs_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    /* Get pointer (with offset) to BCS client structure with attribute handles */
    cy_stc_ble_bcsc_t *bcscPtr = (cy_stc_ble_bcsc_t *)&cy_ble_bcscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_BCS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(bcscPtr->charInfo[charIndex].valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = bcscPtr->charInfo[charIndex].valueHandle;
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

        /* Save the handle to support a service-specific read response from the device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_bcscReqHandle[discIdx] = bcscPtr->charInfo[charIndex].valueHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BCSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  This function is used to write the characteristic descriptor to the server
*  identified by charIndex and descrIndex.
*
*  Internally, a Write Request is sent to the GATT server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*  * \ref CY_BLE_EVT_BCSS_INDICATION_ENABLED
*  * \ref CY_BLE_EVT_BCSS_INDICATION_DISABLED
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of a Body Composition service characteristic of type
*                     \ref cy_en_ble_bcs_char_index_t. The valid values are,
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_FEATURE  
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_bcs_descr_index_t. The valid value is,
*                       * #CY_BLE_BCS_CCCD
*  \param attrSize:   The size of the characteristic value attribute.
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
*   CY_BLE_ERROR_INVALID_STATE               | The state is not valid.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
*  \events
*  If execution is successful (return value = \ref CY_BLE_SUCCESS),
*  these events can appear: \n
*  If a BCS service-specific callback is registered
*  (with \ref Cy_BLE_BCS_RegisterAttrCallback):
*   * \ref CY_BLE_EVT_BCSC_WRITE_DESCR_RESPONSE - If the requested attribute is
*     successfully written on the peer device, the details 
*     (charIndex, descrIndex etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_bcs_descr_value_t.
*   
*   Otherwise (if a BCS service-specific callback is not registered):
*   * \ref CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*
*   * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure ( \ref cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BCSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bcs_char_index_t charIndex,
                                                               cy_en_ble_bcs_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    
    /* Get pointer (with offset) to BCS client structure with attribute handles */
    cy_stc_ble_bcsc_t *bcscPtr = (cy_stc_ble_bcsc_t *)&cy_ble_bcscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_BCS_CHAR_COUNT) || (descrIndex >= CY_BLE_BCS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(bcscPtr->bodyCompositionMeasurementCccdHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        /* Fill all the fields of the Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        
        writeReqParam.handleValPair.attrHandle = bcscPtr->bodyCompositionMeasurementCccdHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;
        
        /* ... and send a request to the server device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

        /* Save the handle to support a service-specific read response from the device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_bcscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BCSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to get the characteristic descriptor of the specified
*  characteristic of the service.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of a Body Composition service characteristic of type
*                     \ref cy_en_ble_bcs_char_index_t. The valid values are,
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_FEATURE  
*                       * \ref CY_BLE_BCS_BODY_COMPOSITION_MEASUREMENT
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_bcs_descr_index_t. The valid value is,
*                       * #CY_BLE_BCS_CCCD
*
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
*
*  \events
*  If execution is successful (return value = \ref CY_BLE_SUCCESS),
*  these events can appear: \n
*  If a BCS service-specific callback is registered
*  (with \ref Cy_BLE_BCS_RegisterAttrCallback):
*  * \ref CY_BLE_EVT_BCSC_READ_DESCR_RESPONSE - If the requested attribute is
*    successfully read on the peer device, the details (charIndex, descrIndex, 
*    value, etc.) are provided with an event parameter structure
*    of type \ref cy_stc_ble_bcs_descr_value_t.
*  
*  Otherwise (if a BCS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*    successfully read on the peer device, the details (handle, 
*    value, etc.) are provided with the event parameters
*    structure \ref cy_stc_ble_gattc_read_rsp_param_t.
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*    requested attribute on the peer device, the details are provided with 
*    the event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BCSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bcs_char_index_t charIndex,
                                                               cy_en_ble_bcs_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to BCS client structure with attribute handles */
    cy_stc_ble_bcsc_t *bcscPtr = (cy_stc_ble_bcsc_t *)&cy_ble_bcscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_BCS_CHAR_COUNT) || (descrIndex >= CY_BLE_BCS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(bcscPtr->bodyCompositionMeasurementCccdHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = bcscPtr->bodyCompositionMeasurementCccdHandle;
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

        /* Save the handle to support a service-specific read response from the device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_bcscReqHandle[discIdx] = bcscPtr->bodyCompositionMeasurementCccdHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BCS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the BCS.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BCS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_BCSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_BCSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_BCSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_BCSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_BCSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the BCS
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BCSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
         case CY_BLE_EVT_GATTS_WRITE_REQ:
                gattErr = Cy_BLE_BCSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
                Cy_BLE_BCSS_ConfirmationEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
                break;

        default:
        break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_BCSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the BCS
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BCSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_BCSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_BCSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_BCSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_BCSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_IND:
                Cy_BLE_BCSC_IndicationEventHandler((cy_stc_ble_gattc_handle_value_ind_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_BCSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_BCSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
