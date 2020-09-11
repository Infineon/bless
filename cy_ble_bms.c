/***************************************************************************//**
* \file cy_ble_bms.c
* \version 3.50
*
* \brief
*  This file contains the source code for the Bond Management service.
*
********************************************************************************
* \copyright
* Copyright 2017-2020, Cypress Semiconductor Corporation.  All rights reserved.
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

static cy_en_ble_gatt_err_code_t Cy_BLE_BMS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_BMSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_BMSC_EventHandler(uint32_t eventCode, void *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_BMSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);
static void Cy_BLE_BMSS_PrepareWriteRequestEventHandler(cy_stc_ble_gatts_prep_write_req_param_t *eventParam);
static void Cy_BLE_BMSS_ExecuteWriteRequestEventHandler(cy_stc_ble_gatts_exec_write_req_t *eventParam);

static void Cy_BLE_BMSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_BMSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_BMSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_BMSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_BMSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_BMSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);
static void Cy_BLE_BMSC_ExecuteWriteResponseEventHandler(const cy_stc_ble_gattc_exec_write_rsp_t *eventParam);



/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_BMS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_BMSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_BMSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_bmscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE BMS server config structure */
const cy_stc_ble_bmss_config_t *cy_ble_bmssConfigPtr = NULL;

/* The pointer to a global BLE BMS client config structure */
const cy_stc_ble_bmsc_config_t *cy_ble_bmscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_BMSS_Init
***************************************************************************//**
*
*  This function initializes server of the Bond Management service.
*
*  \param config: Configuration structure for the BMS.
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
cy_en_ble_api_result_t Cy_BLE_BMSS_Init(const cy_stc_ble_bmss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_bmssConfigPtr = config;

        /* Registers event handler for the BMS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_BMS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_BMSS_EventHandlerCallback = &Cy_BLE_BMSS_EventHandler;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BMSC_Init
***************************************************************************//**
*
*  This function initializes client of the Bond Management service.
*
*  \param config: Configuration structure for the BMS.
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
cy_en_ble_api_result_t Cy_BLE_BMSC_Init(const cy_stc_ble_bmsc_config_t *config)
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
        cy_ble_bmscConfigPtr = config;

        /* Registers event handler for the BMS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_BMS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_BMSC_EventHandlerCallback = &Cy_BLE_BMSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 bmsServIdx = cy_ble_bmscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + bmsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to BMS client structure */
                cy_stc_ble_bmsc_t *bmscPtr = (cy_stc_ble_bmsc_t *)&cy_ble_bmscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(bmscPtr, 0, sizeof(cy_stc_ble_bmsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + bmsServIdx].uuid =
                    CY_BLE_UUID_BOND_MANAGEMENT_SERVICE;
            }

            cy_ble_bmscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BMS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for BMS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback.
          (e.g. #CY_BLE_EVT_BMSS_WRITE_CHAR).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_bms_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_BMS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_BMS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_BMSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a characteristic value of Bond Management service, which is a value
*  identified by charIndex, to the local database.
*
*  \param charIndex: The index of the service characteristic of type
*                    \ref cy_en_ble_bms_char_index_t. The valid values are,
*                       * \ref CY_BLE_BMS_BMCP
*                       * \ref CY_BLE_BMS_BMFT
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
cy_en_ble_api_result_t Cy_BLE_BMSS_SetCharacteristicValue(cy_en_ble_bms_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check the parameters */
    if(charIndex >= CY_BLE_BMS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_bmssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Store a characteristic value into the GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_bmssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_BMSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets a characteristic value of Bond Management service. The value is
*  identified by charIndex.
*
*  \param charIndex: The index of the service characteristic of 
*                    \ref cy_en_ble_bms_char_index_t. The valid values are,
*                       * \ref CY_BLE_BMS_BMCP
*                       * \ref CY_BLE_BMS_BMFT
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
cy_en_ble_api_result_t Cy_BLE_BMSS_GetCharacteristicValue(cy_en_ble_bms_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Check the parameters */
    if(charIndex >= CY_BLE_BMS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_bmssConfigPtr->attrInfo->charInfo[charIndex].charHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        /* Get the characteristic value from the GATT database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_bmssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_BMSS_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sets a characteristic descriptor of the specified characteristic of the
*  Bond Management service.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
*                     \ref cy_en_ble_bms_char_index_t. The valid values are,
*                        * \ref CY_BLE_BMS_BMCP
*                        * \ref CY_BLE_BMS_BMFT
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_bms_descr_index_t. The valid value is
*                        * \ref CY_BLE_BMS_CEPD
*  \param attrSize:   The size of the characteristic descriptor attribute.
*  \param attrValue:  The pointer to the descriptor value data that should
*                     be stored to the GATT database.
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
cy_en_ble_api_result_t Cy_BLE_BMSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bms_char_index_t charIndex,
                                                               cy_en_ble_bms_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((charIndex >= CY_BLE_BMS_CHAR_COUNT) || (descrIndex >= CY_BLE_BMS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_bmssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex];
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
* Function Name: Cy_BLE_BMSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of the specified characteristic of the
*  Bond Management service.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
*                     \ref cy_en_ble_bms_char_index_t. The valid values are,
*                        * \ref CY_BLE_BMS_BMCP
*                        * \ref CY_BLE_BMS_BMFT
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_bms_descr_index_t. The valid value is
*                        * \ref CY_BLE_BMS_CEPD
*  \param attrSize:   The size of the characteristic descriptor attribute.
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
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BMSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bms_char_index_t charIndex,
                                                               cy_en_ble_bms_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((charIndex >= CY_BLE_BMS_CHAR_COUNT) || (descrIndex >= CY_BLE_BMS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
        
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_bmssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex];
        dbAttrValInfo.handleValuePair.value.len  = attrSize;
        dbAttrValInfo.handleValuePair.value.val  = attrValue;
        dbAttrValInfo.offset                     = 0u;
        dbAttrValInfo.connHandle                 = connHandle;
        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
        
        if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) != CY_BLE_GATT_ERR_NONE)
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
    }

    return(apiResult);
}

/******************************************************************************
* Function Name: Cy_BLE_BMSS_WriteEventHandler
***************************************************************************//**
*
*  Handles a Write Request event. Calls the registered BMS application
*  callback function.
*
* Note: Writing an attribute into GATT DB (if needed) is the user's responsibility
*  after the Authorization Code check in the registered BMS application
*  callback function.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
* \return
*  cy_en_ble_gatt_err_code_t - A function result state if it succeeded
*  (CY_BLE_GATT_ERR_NONE) or failed with error codes:
*   * CY_BLE_GATT_ERR_OP_CODE_NOT_SUPPORTED
*   * CY_BLE_GATT_ERR_INSUFFICIENT_AUTHORIZATION
*
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BMSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_stc_ble_bms_char_value_t locCharValue;
    
    locCharValue.connHandle    = eventParam->connHandle;
    locCharValue.gattErrorCode = CY_BLE_GATT_ERR_NONE;

    if((Cy_BLE_BMS_ApplCallback != NULL) &&
       (eventParam->handleValPair.attrHandle == cy_ble_bmssConfigPtr->attrInfo->charInfo[CY_BLE_BMS_BMCP].charHandle))
    {
        locCharValue.gattErrorCode =
            Cy_BLE_GATT_DbCheckPermission(eventParam->handleValPair.attrHandle, &eventParam->connHandle,
                                          CY_BLE_GATT_DB_WRITE | CY_BLE_GATT_DB_PEER_INITIATED);

        if(locCharValue.gattErrorCode == CY_BLE_GATT_ERR_NONE)
        {
            locCharValue.charIndex = CY_BLE_BMS_BMCP;
            locCharValue.value = &eventParam->handleValPair.value;

            Cy_BLE_BMS_ApplCallback((uint32_t)CY_BLE_EVT_BMSS_WRITE_CHAR, &locCharValue);

            if(locCharValue.gattErrorCode == CY_BLE_GATT_ERR_NONE)
            {
                cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                
                dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                dbAttrValInfo.connHandle      = eventParam->connHandle;
                dbAttrValInfo.offset          = 0u;
                dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;
                
                locCharValue.gattErrorCode = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
                if(locCharValue.gattErrorCode == CY_BLE_GATT_ERR_NONE)
                {
                    CY_BLE_GATT_DB_ATTR_SET_ATTR_GEN_LEN(cy_ble_bmssConfigPtr->attrInfo->charInfo[CY_BLE_BMS_BMCP].charHandle,
                                                         locCharValue.value->len);
                }
            }
        }

        /* Indicate that the request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }

    return(locCharValue.gattErrorCode);
}

/******************************************************************************
* Function Name: Cy_BLE_BMSS_PrepareWriteRequestEventHandler
***************************************************************************//**
*
*  Handles a Prepare Write Request event.
*
*  \param eventParam: The pointer to the data that came with a Prepare Write request.
*
******************************************************************************/
static void Cy_BLE_BMSS_PrepareWriteRequestEventHandler(cy_stc_ble_gatts_prep_write_req_param_t *eventParam)
{
    if(eventParam->baseAddr[eventParam->currentPrepWriteReqCount - 1u].handleValuePair.attrHandle ==
       cy_ble_bmssConfigPtr->attrInfo->charInfo[CY_BLE_BMS_BMCP].charHandle)
    {
        cy_stc_ble_gatt_xchg_mtu_param_t mtuParam = { .connHandle = eventParam->connHandle };
        (void)Cy_BLE_GATT_GetMtuSize(&mtuParam);

        if(((cy_ble_bmssConfigPtr->attrInfo->charInfo[CY_BLE_BMS_BMCP].descrHandle[CY_BLE_BMS_CEPD] !=
             CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE) &&
            CY_BLE_IS_RELIABLE_WRITE_ENABLED(cy_ble_bmssConfigPtr->attrInfo->
                                              charInfo[CY_BLE_BMS_BMCP].descrHandle[CY_BLE_BMS_CEPD])) ||
           (CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(cy_ble_bmssConfigPtr->attrInfo->charInfo[CY_BLE_BMS_BMCP].charHandle) >
            (mtuParam.mtu - CY_BLE_GATT_WRITE_HEADER_LEN)))
        {
            /* The first prepare write event */
        }
        else
        {
            eventParam->gattErrorCode = (uint8_t)CY_BLE_GATT_ERR_ATTRIBUTE_NOT_LONG;
        }

        /* Indicate that the request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}

/******************************************************************************
* Function Name: Cy_BLE_BMSS_ExecuteWriteRequestEventHandler
***************************************************************************//**
*
*  Handles an Execute Write Request event.
*
*  \param eventParam: The pointer to the data that came with a Write Request.
*
******************************************************************************/
static void Cy_BLE_BMSS_ExecuteWriteRequestEventHandler(cy_stc_ble_gatts_exec_write_req_t *eventParam)
{
    uint32_t locCount;
    cy_stc_ble_gatt_value_t locGattValue;
    
    locGattValue.len = 0u;
    locGattValue.val = NULL;

    for(locCount = 0u; locCount < eventParam->prepWriteReqCount; locCount++)
    {
        if(eventParam->baseAddr[locCount].handleValuePair.attrHandle ==
           cy_ble_bmssConfigPtr->attrInfo->charInfo[CY_BLE_BMS_BMCP].charHandle)
        {
            locGattValue.len = eventParam->baseAddr[locCount].offset + eventParam->baseAddr[locCount].
                                handleValuePair.value.len;

            if(locGattValue.val == NULL)
            {
                locGattValue.val = eventParam->baseAddr[locCount].handleValuePair.value.val;
            }
            else if(eventParam->baseAddr[locCount].offset == 0u)
            {
                /* The case when the client wants to rewrite the value from the start */
                locGattValue.val = eventParam->baseAddr[locCount].handleValuePair.value.val;
            }
            else
            {
                /* Do nothing */
            }
        }
    }

    if((Cy_BLE_BMS_ApplCallback != NULL) && (locGattValue.len != 0u) &&
       (locGattValue.len <= CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(cy_ble_bmssConfigPtr->attrInfo->charInfo[CY_BLE_BMS_BMCP].
                                                                      charHandle)))
    {
        /* Check the execWriteFlag before executing or canceling the Write Long operation */
        if(eventParam->execWriteFlag == CY_BLE_GATT_EXECUTE_WRITE_EXEC_FLAG)
        {
            cy_stc_ble_bms_char_value_t locCharValue = { .connHandle = eventParam->connHandle };
            locCharValue.gattErrorCode = CY_BLE_GATT_ERR_NONE;
            locCharValue.value = &locGattValue;

            Cy_BLE_BMS_ApplCallback((uint32_t)CY_BLE_EVT_BMSS_WRITE_CHAR, &locCharValue);

            if(locCharValue.gattErrorCode == CY_BLE_GATT_ERR_NONE)
            {
                CY_BLE_GATT_DB_ATTR_SET_ATTR_GEN_LEN(cy_ble_bmssConfigPtr->attrInfo->
                                                      charInfo[CY_BLE_BMS_BMCP].charHandle, locGattValue.len);
            }

            eventParam->gattErrorCode = (uint8_t)locCharValue.gattErrorCode;
        }

        /* Indicate that the request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BMSC_GetCharacteristicValue
***************************************************************************//**
*
*  Sends a request to the peer device to get a characteristic value, as
*  identified by its charIndex.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
*                     \ref cy_en_ble_bms_char_index_t. The valid values are,
*                        * \ref CY_BLE_BMS_BMCP
*                        * \ref CY_BLE_BMS_BMFT
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
*  \events
*  If execution is successful(return value = \ref CY_BLE_SUCCESS),
*  these events can appear: \n
*  If a BMS service-specific callback is registered
*  with \ref Cy_BLE_BMS_RegisterAttrCallback():
*  * \ref CY_BLE_EVT_BMSC_READ_CHAR_RESPONSE - if the requested attribute is
*    successfully read on the peer device, the details (charIndex , 
*    value, etc.) are provided with an event parameter structure
*    of type \ref cy_stc_ble_bms_char_value_t.
*  .
*  Otherwise (if a BMS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTC_READ_RSP - if the requested attribute is
*    successfully read on the peer device, the details (handle, 
*    value, etc.) are provided with the event parameters structure 
*    \ref cy_stc_ble_gattc_read_rsp_param_t.
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*    requested attribute on the peer device, the details are provided 
*    with the event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BMSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_bms_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
 
    /* Get pointer (with offset) to BMS client structure with attribute handles */
    cy_stc_ble_bmsc_t *bmscPtr = (cy_stc_ble_bmsc_t *)&cy_ble_bmscConfigPtr->attrInfo[discIdx];
    
    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex != CY_BLE_BMS_BMFT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(bmscPtr->charInfo[charIndex].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_READ & bmscPtr->charInfo[charIndex].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = bmscPtr->charInfo[CY_BLE_BMS_BMFT].valueHandle;
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_bmscReqHandle[discIdx] = bmscPtr->charInfo[charIndex].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BMSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. The function supports the Write Long
*  procedure - it depends on the attrSize parameter - if it is larger than the
*  current MTU size - 1, then the Write Long will be executed.
*  As a result, a Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, a \ref CY_BLE_EVT_BMSS_WRITE_CHAR
*  event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
*                     \ref cy_en_ble_bms_char_index_t. The valid values are,
*                        * \ref CY_BLE_BMS_BMCP
*                        * \ref CY_BLE_BMS_BMFT
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
*  \events
*    If execution is successful(return value = \ref CY_BLE_SUCCESS),
*    these events can appear: \n
*    If a BMS service-specific callback is registered
*    with \ref Cy_BLE_BMS_RegisterAttrCallback():
*    * \ref CY_BLE_EVT_BMSC_WRITE_CHAR_RESPONSE - if the requested attribute is
*      successfully written on the peer device, the details (charIndex, etc.) 
*      are provided with an event parameter structure of type 
*      \ref cy_stc_ble_bms_char_value_t.
*    .
*    Otherwise (if a BMS service-specific callback is not registered):
*    * \ref CY_BLE_EVT_GATTC_EXEC_WRITE_RSP - if the requested attribute is
*      successfully written on the peer device.
*    * #CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*      requested attribute on the peer device, the details are provided with event
*      parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BMSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_bms_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    cy_stc_ble_gatt_xchg_mtu_param_t mtuParam = { .connHandle = connHandle };

    /* Get pointer (with offset) to BMS client structure with attribute handles */
    cy_stc_ble_bmsc_t *bmscPtr = (cy_stc_ble_bmsc_t *)&cy_ble_bmscConfigPtr->attrInfo[discIdx];

    (void)Cy_BLE_GATT_GetMtuSize(&mtuParam);

    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex != CY_BLE_BMS_BMCP) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(bmscPtr->charInfo[CY_BLE_BMS_BMCP].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_WRITE & bmscPtr->charInfo[CY_BLE_BMS_BMCP].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else if((mtuParam.mtu - CY_BLE_GATT_WRITE_HEADER_LEN) < attrSize)
    {
        cy_stc_ble_gattc_prep_write_req_t prepWriteReqParam;
        
        prepWriteReqParam.handleValOffsetPair.handleValuePair.attrHandle = bmscPtr->charInfo[CY_BLE_BMS_BMCP].valueHandle;
        prepWriteReqParam.handleValOffsetPair.handleValuePair.value.val  = attrValue;
        prepWriteReqParam.handleValOffsetPair.handleValuePair.value.len  = attrSize;
        prepWriteReqParam.handleValOffsetPair.offset                     = 0u;
        prepWriteReqParam.connHandle                                     = connHandle;
        
        apiResult = Cy_BLE_GATTC_WriteLongCharacteristicValues(&prepWriteReqParam);

        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_bmscReqHandle[discIdx] = bmscPtr->charInfo[CY_BLE_BMS_BMCP].valueHandle;
        }
    }
    else
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;
        
        writeReqParam.handleValPair.attrHandle = bmscPtr->charInfo[CY_BLE_BMS_BMCP].valueHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;
        
        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_bmscReqHandle[discIdx] = bmscPtr->charInfo[CY_BLE_BMS_BMCP].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BMSC_ReliableWriteCharacteristicValue
***************************************************************************//**
*
*  Performs a Reliable Write command for the Bond Management Control Point 
*  characteristic (identified by charIndex) value attribute to the server.
*
*  The Write Response only confirms the operation success.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
*                     \ref cy_en_ble_bms_char_index_t. The valid values are,
*                        * \ref CY_BLE_BMS_BMCP
*  \param charIndex:  The index of a service characteristic.
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
*  \events
*    If execution is successful(return value = \ref CY_BLE_SUCCESS),
*    these events can appear: \n
*    If a BMS service-specific callback is registered
*    with \ref Cy_BLE_BMS_RegisterAttrCallback():
*    * \ref CY_BLE_EVT_BMSC_WRITE_CHAR_RESPONSE - if the requested attribute is
*      successfully written on the peer device, the details (charIndex, etc.) 
*      are provided with an event parameter structure of type 
*      \ref cy_stc_ble_bms_char_value_t.
*    .
*    Otherwise (if a BMS service-specific callback is not registered):
*    * \ref CY_BLE_EVT_GATTC_EXEC_WRITE_RSP - if the requested attribute is
*      successfully written on the peer device.
*    * #CY_BLE_EVT_GATTC_ERROR_RSP - In case if an error occurred with the
*      requested attribute on the peer device, the details are provided with event
*      parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_BMSC_ReliableWriteCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                                    cy_en_ble_bms_char_index_t charIndex,
                                                                    uint8_t attrSize,
                                                                    uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to BMS client structure with attribute handles */
    cy_stc_ble_bmsc_t *bmscPtr = (cy_stc_ble_bmsc_t *)&cy_ble_bmscConfigPtr->attrInfo[discIdx];

    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex != CY_BLE_BMS_BMCP) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(bmscPtr->charInfo[CY_BLE_BMS_BMCP].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_EXTENDED_PROPERTIES & bmscPtr->charInfo[CY_BLE_BMS_BMCP].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        cy_stc_ble_gattc_reliable_write_req_t prepWriteReqParam;
        
        prepWriteReqParam.connHandle = connHandle;
        
        cy_stc_ble_gatt_handle_value_offset_param_t handleValOffsetPairParam;
        
        handleValOffsetPairParam.handleValuePair.attrHandle = bmscPtr->charInfo[CY_BLE_BMS_BMCP].valueHandle;
        handleValOffsetPairParam.handleValuePair.value.val  = attrValue;
        handleValOffsetPairParam.handleValuePair.value.len  = attrSize;
        handleValOffsetPairParam.offset                     = 0u;

        prepWriteReqParam.handleValOffsetPair = &handleValOffsetPairParam;
        prepWriteReqParam.numOfRequests = 1u;

        apiResult = Cy_BLE_GATTC_ReliableWrites(&prepWriteReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_bmscReqHandle[discIdx] = bmscPtr->charInfo[CY_BLE_BMS_BMCP].valueHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BMSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to the peer device to get the characteristic descriptor of the
*  specified characteristic of Bond Management service.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic of 
*                     \ref cy_en_ble_bms_char_index_t. The valid values are,
*                        * \ref CY_BLE_BMS_BMCP
*                        * \ref CY_BLE_BMS_BMFT
*  \param descrIndex: The index of the service characteristic descriptor of type
*                     \ref cy_en_ble_bms_descr_index_t. The valid value is
*                        * \ref CY_BLE_BMS_CEPD
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
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular descriptor.
*
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the BMS service-specific callback is registered
*   with \ref Cy_BLE_BMS_RegisterAttrCallback():
*   * #CY_BLE_EVT_BMSC_READ_DESCR_RESPONSE - In case if the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     descrIndex, value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_bms_descr_value_t.
*   .
*   Otherwise (if an BMS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_BMSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_bms_char_index_t charIndex,
                                                               cy_en_ble_bms_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to BMS client structure with attribute handles */
    cy_stc_ble_bmsc_t *bmscPtr = (cy_stc_ble_bmsc_t *)&cy_ble_bmscConfigPtr->attrInfo[discIdx];

    /* Check the parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_BMS_CHAR_COUNT) || (descrIndex >= CY_BLE_BMS_DESCR_COUNT) || 
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(bmscPtr->charInfo[CY_BLE_BMS_BMCP].descrHandle[CY_BLE_BMS_CEPD] ==
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        
        readReqParam.attrHandle = bmscPtr->charInfo[CY_BLE_BMS_BMCP].descrHandle[CY_BLE_BMS_CEPD];
        readReqParam.connHandle = connHandle;
        
        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_bmscReqHandle[discIdx] = bmscPtr->charInfo[CY_BLE_BMS_BMCP].descrHandle[CY_BLE_BMS_CEPD];
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_BMSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to the characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_BMSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* BM service characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_bmscCharUuid[CY_BLE_BMS_CHAR_COUNT] =
    {
        CY_BLE_UUID_BOND_MANAGEMENT_CONTROL_POINT,
        CY_BLE_UUID_BOND_MANAGEMENT_FEATURE
    };
    
    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t bmsDiscIdx = cy_ble_bmscConfigPtr->serviceDiscIdx;
    uint32_t i;

    /* Get pointer (with offset) to BMS client structure with attribute handles */
    cy_stc_ble_bmsc_t *bmscPtr = (cy_stc_ble_bmsc_t *)&cy_ble_bmscConfigPtr->attrInfo[discIdx];
    
    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == bmsDiscIdx ))
    {
        /* Update the last characteristic endHandle to the declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        for(i = 0u; i < (uint8_t)CY_BLE_BMS_CHAR_COUNT; i++)
        {
            if(cy_ble_bmscCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                if(bmscPtr->charInfo[i].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    bmscPtr->charInfo[i].valueHandle = discCharInfo->valueHandle;
                    bmscPtr->charInfo[i].properties = discCharInfo->properties;
                    lastEndHandle[discIdx] = &bmscPtr->charInfo[i].endHandle;
                }
                else
                {
                    Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo);
                }
            }
        }

        /* Initialize the characteristic endHandle to service endHandle.
         * the characteristic endHandle will be updated to the declaration
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
* Function Name: Cy_BLE_BMSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, the appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discDescrInfo: The pointer to a descriptor information structure.
*
******************************************************************************/
static void Cy_BLE_BMSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t notSupportedDescr = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t bmsDiscIdx = cy_ble_bmscConfigPtr->serviceDiscIdx;
    uint32_t descIdx;

    /* Get pointer (with offset) to BMS client structure with attribute handles */
    cy_stc_ble_bmsc_t *bmscPtr = (cy_stc_ble_bmsc_t *)&cy_ble_bmscConfigPtr->attrInfo[discIdx];

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == bmsDiscIdx)
    {
        switch(discDescrInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_EXTENDED_PROPERTIES:
                descIdx = (uint32_t)CY_BLE_BMS_CEPD;
                break;

            default:
                /* Not supported descriptor */
                notSupportedDescr = 1u;
                break;
        }

        if(notSupportedDescr == 0u)
        {
            if(bmscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] ==
               CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                bmscPtr->charInfo[cy_ble_configPtr->context->discovery[discIdx].charCount].descrHandle[descIdx] =
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
* Function Name: Cy_BLE_BMSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_BMSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t bmsDiscIdx = cy_ble_bmscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    /* Get pointer (with offset) to BMS client structure with attribute handles */
    cy_stc_ble_bmsc_t *bmscPtr = (cy_stc_ble_bmsc_t *)&cy_ble_bmscConfigPtr->attrInfo[discIdx];

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == bmsDiscIdx)
    {
        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_BMS_CHAR_COUNT) && (exitFlag == 0u))
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            if((bmscPtr->charInfo[charIdx].endHandle - bmscPtr->charInfo[charIdx].valueHandle) != 0u)
            {
                /* Read characteristic range */
                charRangeInfo->range.startHandle = bmscPtr->charInfo[charIdx].valueHandle + 1u;
                charRangeInfo->range.endHandle = bmscPtr->charInfo[charIdx].endHandle;
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
* Function Name: Cy_BLE_BMSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles a Read Response event.
*
*  \param eventParam: The pointer to the data structure.
*
******************************************************************************/
static void Cy_BLE_BMSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    /* Get pointer (with offset) to BMS client structure with attribute handles */
    cy_stc_ble_bmsc_t *bmscPtr = (cy_stc_ble_bmsc_t *)&cy_ble_bmscConfigPtr->attrInfo[discIdx];
    
    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_BMS_ApplCallback != NULL) &&
       (cy_ble_bmscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        if(bmscPtr->charInfo[CY_BLE_BMS_BMFT].valueHandle == cy_ble_bmscReqHandle[discIdx])
        {
            cy_stc_ble_bms_char_value_t locCharValue;
            
            locCharValue.connHandle = eventParam->connHandle;
            locCharValue.charIndex  = CY_BLE_BMS_BMFT;
            locCharValue.value      = &eventParam->value;
            
            cy_ble_bmscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_BMS_ApplCallback((uint32_t)CY_BLE_EVT_BMSC_READ_CHAR_RESPONSE, &locCharValue);
        }
        else if(bmscPtr->charInfo[CY_BLE_BMS_BMCP].descrHandle[CY_BLE_BMS_CEPD] ==
                cy_ble_bmscReqHandle[discIdx])
        {
            cy_stc_ble_bms_descr_value_t locDescrValue;
            
            locDescrValue.connHandle = eventParam->connHandle;
            locDescrValue.charIndex  = CY_BLE_BMS_BMCP;
            locDescrValue.descrIndex = CY_BLE_BMS_CEPD;
            locDescrValue.value      = &eventParam->value;
            
            cy_ble_bmscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_BMS_ApplCallback((uint32_t)CY_BLE_EVT_BMSC_READ_DESCR_RESPONSE, &locDescrValue);
        }
        else
        {
            /* Do nothing */
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BMSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles a Write Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BMSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_BMS_ApplCallback != NULL) &&
       (cy_ble_bmscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        /* Get pointer (with offset) to BMS client structure with attribute handles */
        cy_stc_ble_bmsc_t *bmscPtr = (cy_stc_ble_bmsc_t *)&cy_ble_bmscConfigPtr->attrInfo[discIdx];

        if(bmscPtr->charInfo[CY_BLE_BMS_BMCP].valueHandle == cy_ble_bmscReqHandle[discIdx])
        {
            cy_stc_ble_bms_char_value_t locCharValue;
            
            locCharValue.connHandle = *eventParam;
            locCharValue.charIndex  = CY_BLE_BMS_BMCP;
            locCharValue.value      = NULL;
            
            cy_ble_bmscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
            Cy_BLE_BMS_ApplCallback((uint32_t)CY_BLE_EVT_BMSC_WRITE_CHAR_RESPONSE, &locCharValue);
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BMSC_ExecuteWriteResponseEventHandler
***************************************************************************//**
*
*  Handles a Write Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BMSC_ExecuteWriteResponseEventHandler(const cy_stc_ble_gattc_exec_write_rsp_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    /* Get pointer (with offset) to BMS client structure with attribute handles */
    cy_stc_ble_bmsc_t *bmscPtr = (cy_stc_ble_bmsc_t *)&cy_ble_bmscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_BMS_ApplCallback != NULL) &&
       (bmscPtr->charInfo[CY_BLE_BMS_BMCP].valueHandle == cy_ble_bmscReqHandle[discIdx]))
    {
        cy_stc_ble_bms_char_value_t locCharValue;
        
        locCharValue.connHandle = eventParam->connHandle;
        locCharValue.charIndex  = CY_BLE_BMS_BMCP;
        locCharValue.value      = NULL;
        
        cy_ble_bmscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        Cy_BLE_BMS_ApplCallback((uint32_t)CY_BLE_EVT_BMSC_WRITE_CHAR_RESPONSE, &locCharValue);
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BMSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles an Error Response event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_BMSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        if(cy_ble_bmscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] == eventParam->errInfo.attrHandle)
        {
            cy_ble_bmscReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_BMS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the BMS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BMS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_BMSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_BMSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_BMSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_BMSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_BMSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the BMS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BMSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_BMSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_PREP_WRITE_REQ:
            Cy_BLE_BMSS_PrepareWriteRequestEventHandler((cy_stc_ble_gatts_prep_write_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_EXEC_WRITE_REQ:
            Cy_BLE_BMSS_ExecuteWriteRequestEventHandler((cy_stc_ble_gatts_exec_write_req_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_BMSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the BMS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_BMSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_BMSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_BMSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_BMSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_BMSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_BMSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_BMSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_EXEC_WRITE_RSP:
                Cy_BLE_BMSC_ExecuteWriteResponseEventHandler((cy_stc_ble_gattc_exec_write_rsp_t*)eventParam);
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
