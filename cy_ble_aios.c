/***************************************************************************//**
* \file cy_ble_aios.c
* \version 3.30
*
* \brief
*  Contains the source code for Automation Input Output service.
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

static cy_en_ble_gatt_err_code_t Cy_BLE_AIOS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_AIOSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_AIOSC_EventHandler(uint32_t eventCode, void *eventParam);

static cy_en_ble_gatt_err_code_t Cy_BLE_AIOSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);
static void Cy_BLE_AIOSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_AIOS_SetAggregateChar(void);

static void Cy_BLE_AIOSC_DiscoverCharacteristicsEventHandler(const cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_AIOSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_AIOSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_AIOSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_AIOSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_AIOSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);
static void Cy_BLE_AIOSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_AIOSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_AIOS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_AIOSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_AIOSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_aioscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];
static cy_ble_gatt_db_attr_handle_t cy_ble_aiossReqHandle;
static uint8_t cy_ble_aioscActiveCharInstance[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };

/* The pointer to a global BLE AIOS server config structure */
const cy_stc_ble_aioss_config_t *cy_ble_aiossConfigPtr = NULL;

/* The pointer to a global BLE AIOS client config structure */
const cy_stc_ble_aiosc_config_t *cy_ble_aioscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_AIOSS_Init
***************************************************************************//**
*
*  This function initializes server of the Automation IO service.
*
*  \param config: Configuration structure for the AIOS.
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
cy_en_ble_api_result_t Cy_BLE_AIOSS_Init(const cy_stc_ble_aioss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_aiossConfigPtr = config;

        /* Registers event handler for the AIOS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_AIOS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_AIOSS_EventHandlerCallback = &Cy_BLE_AIOSS_EventHandler;
        
        /* Sets the value for Aggregate characteristic */
        cy_ble_aiossReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        (void)Cy_BLE_AIOS_SetAggregateChar();
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_AIOSC_Init
***************************************************************************//**
*
*  This function initializes client of the Automation IO service.
*
*  \param config: Configuration structure for the AIOS.
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
cy_en_ble_api_result_t Cy_BLE_AIOSC_Init(const cy_stc_ble_aiosc_config_t *config)
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
        cy_ble_aioscConfigPtr = config;

        /* Registers event handler for the AIOS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_AIOS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_AIOSC_EventHandlerCallback = &Cy_BLE_AIOSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32_t j;
            uint32 servCnt = cy_ble_configPtr->context->discServiCount;
            uint32 aiosServIdx = cy_ble_aioscConfigPtr->serviceDiscIdx;
            if(cy_ble_configPtr->context->serverInfo[idx * servCnt].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                uint32_t i;
                
                for(i = 0u; (i < (uint32_t)CY_BLE_AIOS_CHAR_COUNT); i++)
                {
                    /* Get pointer to AIOS client structure */
                    cy_stc_ble_aiosc_t *aioscPtr = (cy_stc_ble_aiosc_t *)&cy_ble_aioscConfigPtr->attrInfo[idx];
                   
                    for(j = 0u; j < cy_ble_aioscConfigPtr->charInstances[i]; j++)
                    {
                        (void)memset(&aioscPtr->charInfoAddr[i].charInfoPtr[j], 0,
                                     sizeof(cy_stc_ble_aiosc_char_t));
                    }
                }

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + aiosServIdx].uuid =
                    CY_BLE_UUID_AUTOMATION_INPUT_OUTPUT_SERVICE;
            }
            cy_ble_aioscActiveCharInstance[idx] = 0u;
            cy_ble_aioscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
    
    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_AIOS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for AIOS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_AIOSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_aios_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_AIOS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_AIOS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_AIOSS_WriteEventHandler
***************************************************************************//**
*
*  Handles the Write Request event.
*
*  \param eventParam: The pointer to the data structure specified by the event.
*
* \return
*  \ref cy_en_ble_gatt_err_code_t - A function result state if it succeeded
*  (CY_BLE_GATT_ERR_NONE) or failed with error codes:
*   * CY_BLE_GATTS_ERR_PROCEDURE_ALREADY_IN_PROGRESS - profile or service
*                           request cannot be serviced because an operation that
*                           has been previously triggered is still in progress.
*   * CY_BLE_GATTS_ERR_CCCD_IMPROPERLY_CONFIGURED - Client Characteristic Configuration
*                           descriptor is not configured according to the requirements
*                           of the profile or service.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_AIOSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    cy_ble_gatt_db_attr_handle_t tmpHandle;
    cy_stc_ble_aioss_char_t *aiosCharInfoPtr;
    cy_stc_ble_aios_descr_value_t wrDescrReqParam;
    cy_stc_ble_aios_char_value_t wrCharReqParam;
    uint32_t descInd;
    uint32_t event = 0u;
    uint32_t foundItem = CY_BLE_AIOS_UNDEFINED_ITEM;
    uint8_t i;
    uint8_t j;

    tmpHandle = eventParam->handleValPair.attrHandle;

    if(Cy_BLE_AIOS_ApplCallback != NULL)
    {
        /* Go through all possible service characteristics.
         * Exit the handler in following conditions:
         * 1) No characteristic left;
         * 2) Characteristic or descriptor was successfully written;
         * 3) Error occurred while writing characteristic or descriptor.
         */
        for(i = 0u; ((i < ((uint8_t)CY_BLE_AIOS_CHAR_COUNT)) && (foundItem == 0u) && (gattErr == CY_BLE_GATT_ERR_NONE));
            i++)
        {
            /* Check whether characteristic is enabled. If the pointer to the characteristic
             * is not "NULL", there is at least one instance of the characteristic in
             * the AIO service.
             */

            if(cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[i].charInfoPtr != NULL)
            {
                for(j = 0u; j < cy_ble_aiossConfigPtr->charInstances[i]; j++)
                {
                    aiosCharInfoPtr = (cy_stc_ble_aioss_char_t *) &cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[i].charInfoPtr[j];

                    /* Client Characteristic Write Request handling */
                    if(tmpHandle == aiosCharInfoPtr->charHandle)
                    {
                        wrDescrReqParam.descrIndex = CY_BLE_AIOS_DESCR_COUNT;
                        foundItem = CY_BLE_AIOS_CHARACTERISTIC_ITEM;
                    }

                    /* Client characteristic descriptor Write Request handling */
                    else
                    {
                        wrDescrReqParam.descrIndex = CY_BLE_AIOS_DESCR_COUNT;

                        for(descInd = 0u; (descInd < (uint32_t)CY_BLE_AIOS_DESCR_COUNT) &&
                            (foundItem != CY_BLE_AIOS_DESCRIPTOR_ITEM); descInd++)
                        {
                            if(aiosCharInfoPtr->descrHandle[descInd] == tmpHandle)
                            {
                                wrDescrReqParam.descrIndex = (cy_en_ble_aios_descr_index_t)descInd;
                                foundItem = CY_BLE_AIOS_DESCRIPTOR_ITEM;
                            }
                        }
                    }

                    /* Verify whether requested handle was found and successfully handled */
                    if((gattErr == CY_BLE_GATT_ERR_NONE) && (foundItem == CY_BLE_AIOS_DESCRIPTOR_ITEM))
                    {
                        switch(wrDescrReqParam.descrIndex)
                        {
                            case CY_BLE_AIOS_CCCD:
                                /* Verify that optional notification property is enabled for characteristic */
                                if(CY_BLE_IS_NOTIFICATION_SUPPORTED(aiosCharInfoPtr->charHandle))
                                {
                                    if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                                    {
                                        event = (uint32_t)CY_BLE_EVT_AIOSS_NOTIFICATION_ENABLED;
                                    }
                                    else
                                    {
                                        event = (uint32_t)CY_BLE_EVT_AIOSS_NOTIFICATION_DISABLED;
                                    }
                                }
                                else if(CY_BLE_IS_INDICATION_SUPPORTED(aiosCharInfoPtr->charHandle))
                                {
                                    if(CY_BLE_IS_INDICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                                    {
                                        event = (uint32_t)CY_BLE_EVT_AIOSS_INDICATION_ENABLED;
                                    }
                                    else
                                    {
                                        event = (uint32_t)CY_BLE_EVT_AIOSS_INDICATION_DISABLED;
                                    }
                                }
                                else
                                {
                                    gattErr = CY_BLE_GATT_ERR_REQUEST_NOT_SUPPORTED;
                                }
                                break;

                            case CY_BLE_AIOS_CHAR_USER_DESCRIPTION_DESCR:
                                if(CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_MAX_LEN(
                                       aiosCharInfoPtr->descrHandle[CY_BLE_AIOS_CHAR_USER_DESCRIPTION_DESCR]) >=
                                   eventParam->handleValPair.value.len)
                                {
                                    /* Set new length of the descriptor value */
                                    CY_BLE_GATT_DB_ATTR_SET_ATTR_GEN_LEN(
                                        aiosCharInfoPtr->descrHandle[CY_BLE_AIOS_CHAR_USER_DESCRIPTION_DESCR],
                                        eventParam->handleValPair.value.len);
                                }
                                else
                                {
                                    gattErr = CY_BLE_GATT_ERR_INVALID_ATTRIBUTE_LEN;
                                }
                                break;

                            case CY_BLE_AIOS_VALUE_TRIGGER_SETTINGS:
                                /* Trigger condition value not supported.*/
                                if(eventParam->handleValPair.value.val[0u] >= CY_BLE_AIOS_VALUE_TRIGGER_DESCR_MAX_VALUE)
                                {
                                    gattErr =
                                        (cy_en_ble_gatt_err_code_t)CY_BLE_GATT_ERR_TRIGGER_CODITION_VALUE_NOT_SUPPORTED;
                                }
                                break;

                            case CY_BLE_AIOS_TIME_TRIGGER_SETTINGS:
                                /* Trigger condition value not supported.*/
                                if(eventParam->handleValPair.value.val[0u] >= CY_BLE_AIOS_TIME_TRIGGER_DESCR_MAX_VALUE)
                                {
                                    gattErr =
                                        (cy_en_ble_gatt_err_code_t)CY_BLE_GATT_ERR_TRIGGER_CODITION_VALUE_NOT_SUPPORTED;
                                }
                                break;

                            default:
                                break;
                        }
                    }

                    /* Check that no error condition has happened yet */
                    if((gattErr == CY_BLE_GATT_ERR_NONE)
                       && ((foundItem == CY_BLE_AIOS_CHARACTERISTIC_ITEM) || (foundItem == CY_BLE_AIOS_DESCRIPTOR_ITEM)))
                    {
                        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                        dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                        dbAttrValInfo.connHandle      = eventParam->connHandle;
                        dbAttrValInfo.offset          = 0u;
                        dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;
                        
                        gattErr = Cy_BLE_GATT_DbCheckPermission(eventParam->handleValPair.attrHandle,
                                                                &eventParam->connHandle,
                                                                CY_BLE_GATT_DB_WRITE | CY_BLE_GATT_DB_PEER_INITIATED);
    
                        if(gattErr == CY_BLE_GATT_ERR_NONE)
                        {
                            /* Fill data and pass it to user */

                            if(foundItem == CY_BLE_AIOS_CHARACTERISTIC_ITEM)
                            {
                                wrCharReqParam.connHandle = eventParam->connHandle;
                                wrCharReqParam.charIndex = (cy_en_ble_aios_char_index_t)i;
                                wrCharReqParam.charInstance = j;
                                wrCharReqParam.gattErrorCode = CY_BLE_GATT_ERR_NONE;
                                wrCharReqParam.value = &eventParam->handleValPair.value;

                                Cy_BLE_AIOS_ApplCallback((uint32_t)CY_BLE_EVT_AIOSS_WRITE_CHAR, &wrCharReqParam);
                            }
                            else if(foundItem == CY_BLE_AIOS_DESCRIPTOR_ITEM)
                            {
                                wrDescrReqParam.connHandle = eventParam->connHandle;
                                wrDescrReqParam.charIndex = (cy_en_ble_aios_char_index_t)i;
                                wrDescrReqParam.charInstance = j;
                                wrDescrReqParam.gattErrorCode = CY_BLE_GATT_ERR_NONE;
                                wrDescrReqParam.value = &eventParam->handleValPair.value;

                                Cy_BLE_AIOS_ApplCallback((uint32_t)CY_BLE_EVT_AIOSS_DESCR_WRITE, &wrDescrReqParam);
                            }
                            else
                            {
                                /* No handle match was found */
                            }

                            /* Write value to GATT database */
                            gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo); \
                            if(event != 0u)
                            {
                                Cy_BLE_AIOS_ApplCallback(event, &wrDescrReqParam);
                            }
                            else
                            {
                                /* No event was found */
                            }
                        }
                        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                        break;
                    }
                }
            }
        }
    }

    if(gattErr != CY_BLE_GATT_ERR_NONE)
    {
        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
    else
    {
        if(foundItem == CY_BLE_AIOS_CHARACTERISTIC_ITEM)
        {
            gattErr = Cy_BLE_AIOS_SetAggregateChar();
        }
    }

    return(gattErr);
}

/******************************************************************************
* Function Name: Cy_BLE_AIOSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets the characteristic value of the service in the local database.
*
*  \param charIndex:    The index of the service characteristic. Starts with zero.
*  \param charInstance: The instance number of the characteristic specified by
*                       "charIndex".
*  \param attrSize:     The size (in bytes) of the characteristic value attribute.
*  \param attrValue:    The pointer to the characteristic value data that should be
*                       stored in the GATT database.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE| An optional characteristic is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_AIOSS_SetCharacteristicValue(cy_en_ble_aios_char_index_t charIndex,
                                                           uint8_t charInstance,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    /* Validate range of AIOS characteristics */
    if((charIndex < CY_BLE_AIOS_CHAR_COUNT) && (charInstance < cy_ble_aiossConfigPtr->charInstances[charIndex]))
    {
        /* Check whether requested characteristic is present in service.
         * There are three conditions that should be checked: 1) The pointer to
         * "CY_BLE_AIOS_CHAR_T" is not NULL, 2) The handle of the characteristic is
         * a valid handle, 3) The requested instance is a valid characteristic
         * instance.
         */
        if((cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr != NULL) &&
           (cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr[charInstance].charHandle !=
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
        {
            cy_stc_ble_gatt_handle_value_pair_t locHandleValuePair;
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            locHandleValuePair.attrHandle = cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr[charInstance].charHandle;
            locHandleValuePair.value.len  = attrSize;
            locHandleValuePair.value.val  = attrValue;

            dbAttrValInfo.connHandle.bdHandle = 0u;
            dbAttrValInfo.connHandle.attId    = 0u;
            dbAttrValInfo.handleValuePair     = locHandleValuePair;
            dbAttrValInfo.offset              = 0u;
            dbAttrValInfo.flags               = CY_BLE_GATT_DB_LOCALLY_INITIATED;
            
            /* Store data in database */
            if(Cy_BLE_GATTS_WriteAttributeValue(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
            {
                apiResult = CY_BLE_SUCCESS;
            }
        }
        else
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
    }

    if(apiResult == CY_BLE_SUCCESS)
    {
        apiResult = (cy_en_ble_api_result_t)Cy_BLE_AIOS_SetAggregateChar();
    }

    return(apiResult);
}

/******************************************************************************
* Function Name: Cy_BLE_AIOSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets the characteristic value of the service, which is a value identified by
*  charIndex.
*
*  \param charIndex:    The index of the service characteristic. Starts with zero.
*  \param charInstance: The instance number of the characteristic specified by
*                       "charIndex".
*  \param attrSize:     The size of the characteristic value attribute.
*  \param attrValue:    The pointer to the location where characteristic value
*                       data should be stored.
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
cy_en_ble_api_result_t Cy_BLE_AIOSS_GetCharacteristicValue(cy_en_ble_aios_char_index_t charIndex,
                                                           uint8_t charInstance,
                                                           uint8_t attrSize,
                                                           uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex < CY_BLE_AIOS_CHAR_COUNT) && (charInstance < cy_ble_aiossConfigPtr->charInstances[charIndex]))
    {
        /* Check whether requested characteristic is present in service. There are three
         * conditions to be checked: 1) The pointer to "cy_stc_ble_aioss_char_t" record in
         * the cy_ble_aioss struct is not NULL, 2) The handle of the characteristic is a
         * valid handle, 3) The requested instance is a valid characteristic instance.
         */
        if((cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr != NULL) &&
           (cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr[charInstance].charHandle !=
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
        {
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
            cy_stc_ble_gatt_handle_value_pair_t locHandleValuePair;
            locHandleValuePair.attrHandle = cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr[charInstance].charHandle;
            locHandleValuePair.value.len  = attrSize;
            locHandleValuePair.value.val  = attrValue;

            dbAttrValInfo.connHandle.bdHandle = 0u;
            dbAttrValInfo.connHandle.attId    = 0u;
            dbAttrValInfo.handleValuePair     = locHandleValuePair;
            dbAttrValInfo.offset              = 0u;
            dbAttrValInfo.flags               = CY_BLE_GATT_DB_LOCALLY_INITIATED;
            
            /* Read characteristic value from database */
            if(Cy_BLE_GATTS_ReadAttributeValue(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
            {
                apiResult = CY_BLE_SUCCESS;

                /* Set new length of the descriptor value */
                CY_BLE_GATT_DB_ATTR_SET_ATTR_GEN_LEN(cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].
                                                     charInfoPtr[charInstance].charHandle, attrSize);
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
* Function Name: Cy_BLE_AIOSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets a characteristic descriptor of a specified characteristic of the
*  Automation Input Output service from the local GATT database.
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of the characteristic.
*  \param charInstance: The instance number of the characteristic specified by
*                       "charIndex".
*  \param descrIndex:   The index of the characteristic descriptor.
*  \param attrSize:     The size of the characteristic descriptor attribute.
*  \param attrValue:    The pointer to the location where characteristic 
*                       descriptor value data should be stored.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | A descriptor is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_AIOSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_aios_char_index_t charIndex,
                                                                uint8_t charInstance,
                                                                cy_en_ble_aios_descr_index_t descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    
    if((charIndex < CY_BLE_AIOS_CHAR_COUNT) && (charInstance < cy_ble_aiossConfigPtr->charInstances[charIndex]) &&
       (descrIndex < CY_BLE_AIOS_DESCR_COUNT))
    {
        /* Check whether requested descriptor is present in service.
         * There are three conditions to be checked: 1) The pointer to
         * "CY_BLE_AiosS_CHAR_T" is not NULL, 2) The handle of the characteristic is
         * a valid handle, 3) The handle of the descriptor is a valid handle.
         */
        if((cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr != NULL) &&
           (cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr[charInstance].charHandle !=
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE) &&
           (cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr[charInstance].descrHandle[descrIndex] !=
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
        {
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
            cy_stc_ble_gatt_handle_value_pair_t locHandleValuePair;
            locHandleValuePair.attrHandle = cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].
                                            charInfoPtr[charInstance].descrHandle[descrIndex];
            locHandleValuePair.value.len  = attrSize;
            locHandleValuePair.value.val  = attrValue;
           
            dbAttrValInfo.handleValuePair = locHandleValuePair;
            dbAttrValInfo.connHandle      = connHandle;
            dbAttrValInfo.offset          = 0u;
            dbAttrValInfo.flags           = CY_BLE_GATT_DB_LOCALLY_INITIATED;

            /* Read characteristic value from database */
            if(Cy_BLE_GATTS_ReadAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
            {
                apiResult = CY_BLE_SUCCESS;

                /* Set new length of the descriptor value */
                CY_BLE_GATT_DB_ATTR_SET_ATTR_GEN_LEN(cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].
                                                      charInfoPtr[charInstance].descrHandle[descrIndex], attrSize);
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
* Function Name: Cy_BLE_AIOSS_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Set a characteristic descriptor of a specified characteristic of the 
*  Automation Input Output service from the local GATT database.
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of the characteristic.
*  \param charInstance: The instance number of the characteristic specified by
*                       "charIndex".
*  \param descrIndex:   The index of the characteristic descriptor.
*  \param attrSize:     The size of the characteristic descriptor attribute.
*  \param attrValue:    The pointer to the descriptor value data to
*                       be stored in the GATT database.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The characteristic descriptor value was read successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | A descriptor is absent.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_AIOSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_aios_char_index_t charIndex,
                                                                uint8_t charInstance,
                                                                cy_en_ble_aios_descr_index_t descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex < CY_BLE_AIOS_CHAR_COUNT) && (charInstance < cy_ble_aiossConfigPtr->charInstances[charIndex]) &&
       (descrIndex < CY_BLE_AIOS_DESCR_COUNT))
    {
        /* Check whether requested descriptor is present in service.
         * There are three conditions to be checked: 1) The pointer to
         * "CY_BLE_AiosS_CHAR_T" is not NULL, 2) The handle of the characteristic is
         * a valid handle, 3) The handle of the descriptor is a valid handle.
         */
        if((cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr != NULL) &&
           (cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr[charInstance].charHandle !=
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE) &&
           (cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr[charInstance].descrHandle[descrIndex] !=
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
        {
            cy_stc_ble_gatt_handle_value_pair_t locHandleValuePair;
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
            locHandleValuePair.attrHandle = cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].
                                            charInfoPtr[charInstance].descrHandle[descrIndex];
            locHandleValuePair.value.len  = attrSize;
            locHandleValuePair.value.val  = attrValue;
            
            dbAttrValInfo.handleValuePair = locHandleValuePair;
            dbAttrValInfo.connHandle      = connHandle;
            dbAttrValInfo.offset          = 0u;
            dbAttrValInfo.flags           = CY_BLE_GATT_DB_LOCALLY_INITIATED;
            
            if(Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo) == CY_BLE_GATT_ERR_NONE)
            {
                apiResult = CY_BLE_SUCCESS;
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
* Function Name: Cy_BLE_AIOSS_SendNotification
***************************************************************************//**
*
*  Sends a notification with a characteristic value of the Automation Input Output
*  service, which is a value specified by charIndex, to the client's device.
*
*  On enabling notification successfully for a service characteristic, it sends out a
*  'Handle Value Notification', which results in #CY_BLE_EVT_AIOSC_NOTIFICATION event
*  at the GATT client's end.
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of the service characteristic. Starts with zero.
*  \param charInstance: The instance number of the characteristic specified by
*                       "charIndex".
*  \param attrSize:     The size of the characteristic value attribute.
*  \param attrValue:    The pointer to the characteristic value data that should be
*                       sent to the client's device.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | This operation is not permitted.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | An optional characteristic is absent.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_NTF_DISABLED                | Notification is not enabled by the client.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_AIOSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                     cy_en_ble_aios_char_index_t charIndex,
                                                     uint8_t charInstance,
                                                     uint8_t attrSize,
                                                     uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_ERROR_INVALID_PARAMETER;

    if((charIndex < CY_BLE_AIOS_CHAR_COUNT) && (charInstance < cy_ble_aiossConfigPtr->charInstances[charIndex]))
    {
        if((cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr != NULL) &&
           (cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr[charInstance].descrHandle[CY_BLE_AIOS_CCCD] !=
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
        {
            /* Send notification if it is enabled and connected */
            if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
            {
                apiResult = CY_BLE_ERROR_INVALID_STATE;
            }

            else if(!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_aiossConfigPtr->attrInfo->
                                                     charInfoAddr[charIndex].charInfoPtr[charInstance].
                                                     descrHandle[CY_BLE_AIOS_CCCD]))
            {
                apiResult = CY_BLE_ERROR_NTF_DISABLED;
            }
            else
            {
                /* Fill all fields of Write Request structure ... */
                cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
                ntfReqParam.handleValPair.attrHandle = 
                    cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr[charInstance].charHandle;
                ntfReqParam.handleValPair.value.val  = attrValue;
                ntfReqParam.handleValPair.value.len  = attrSize;
                ntfReqParam.connHandle               = connHandle;
                
                /* Send notification to client using previously filled structure */
                apiResult = Cy_BLE_GATTS_Notification(&ntfReqParam);
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
* Function Name: Cy_BLE_AIOSS_SendIndication
***************************************************************************//**
*
*  Sends an indication with a characteristic value of the Automation Input Output
*  service, which is a value specified by charIndex, to the client's device.
*
*  On enabling indication successfully it sends out a 'Handle Value Indication', which
*  results in #CY_BLE_EVT_AIOSC_INDICATION or #CY_BLE_EVT_GATTC_HANDLE_VALUE_IND 
*  (if service-specific callback function is not registered) event
*  at the GATT client's end.
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of the service characteristic.
*  \param charInstance: The instance number of the characteristic specified by
*                       "charIndex".
*  \param attrSize:     The size of the characteristic value attribute.
*  \param attrValue:    The pointer to the characteristic value data that should be
*                       sent to the client's device.
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | This operation is not permitted.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | An optional characteristic is absent.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the client is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_IND_DISABLED                | Indication is not enabled by the client.

* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the AIOS service-specific callback is registered
*   with \ref Cy_BLE_AIOS_RegisterAttrCallback():
*   * #CY_BLE_EVT_AIOSS_INDICATION_CONFIRMED - In case the indication is
*     successfully delivered to the peer device.
*   .
*   Otherwise (if the AIOS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF - In case the indication is
*     successfully delivered to the peer device.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_AIOSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                   cy_en_ble_aios_char_index_t charIndex,
                                                   uint8_t charInstance,
                                                   uint8_t attrSize,
                                                   uint8_t *attrValue)
{
    /* Store new data in database */
    cy_en_ble_api_result_t apiResult;

    /* Send indication if it is enabled and connected */
    if(charIndex >= CY_BLE_AIOS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(charInstance >= cy_ble_aiossConfigPtr->charInstances[charIndex])
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(!CY_BLE_IS_INDICATION_ENABLED(connHandle.attId, cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].
                                           charInfoPtr[charInstance].descrHandle[CY_BLE_AIOS_CCCD]))
    {
        apiResult = CY_BLE_ERROR_IND_DISABLED;
    }
    else
    {
        if(cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr[charInstance].charHandle !=
           CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            /* Fill all fields of Write Request structure ... */
            cy_stc_ble_gatts_handle_value_ind_t indReqParam;
            indReqParam.handleValPair.attrHandle = 
                cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[charIndex].charInfoPtr[charInstance].charHandle;
            indReqParam.handleValPair.value.val  = attrValue;
            indReqParam.handleValPair.value.len  = attrSize;
            indReqParam.connHandle               = connHandle;
            
            /* Send indication to client using previously filled structure */
            apiResult = Cy_BLE_GATTS_Indication(&indReqParam);
            /* Save handle to support service-specific value confirmation response from client */
            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_aiossReqHandle = indReqParam.handleValPair.attrHandle;
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
* Function Name: Cy_BLE_AIOSS_ConfirmationEventHandler
***************************************************************************//**
*
*  Handles a Value Confirmation request event from the BLE Stack.
*
*  eventParam - The pointer to a structure of type #cy_stc_ble_conn_handle_t.
*
******************************************************************************/
static void Cy_BLE_AIOSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    cy_en_ble_aios_char_index_t locCharIndex;
    cy_stc_ble_aios_char_value_t locCharValue;
    uint32_t locReqHandle = 0u;
    uint8_t locCharInstance;

    if((Cy_BLE_AIOS_ApplCallback != NULL) && (cy_ble_aiossReqHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        for(locCharIndex = CY_BLE_AIOS_DIGITAL; (locCharIndex < CY_BLE_AIOS_CHAR_COUNT) && (locReqHandle == 0u);
            locCharIndex++)
        {
            for(locCharInstance = 0u; (locCharInstance < cy_ble_aiossConfigPtr->charInstances[locCharIndex]) &&
                (locReqHandle == 0u); locCharInstance++)
            {
                if(cy_ble_aiossReqHandle == cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[locCharIndex].
                    charInfoPtr[locCharInstance].charHandle)
                {
                    locCharValue.connHandle = *eventParam;
                    locCharValue.charIndex = locCharIndex;
                    locCharValue.charInstance = locCharInstance;
                    locCharValue.value = NULL;

                    cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                    cy_ble_aiossReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                    Cy_BLE_AIOS_ApplCallback((uint32_t)CY_BLE_EVT_AIOSS_INDICATION_CONFIRMED, &locCharValue);
                    locReqHandle = 1u;
                }
            }
        }
    }
}

/******************************************************************************
* Function Name: Cy_BLE_AIOS_SetAggregateChar
***************************************************************************//**
*
*  Sets the value for Aggregate characteristic.
*  All of the defined Digital characteristics with the Read property set are
*  part of Aggregate (if available). The length of each of the Digital characteristics
*  that are part of the Aggregate is determined from the number of signals defined in the
*  Number of Digitals descriptor of the Digital characteristic. The Digital part byte
*  aligned and trailing bits (if any) are set to 0. The order of the Digital characteristics
*  is determined by the value in the Description field of the Characteristic Presentation
*  Format descriptor with the lowest value first.
*  All of the defined Analog characteristics with Read property set part of the Aggregate
*  (if available). The order of the Analog characteristics is determined by the value in
*  the Description field of the Characteristic Presentation Format descriptor with the
*  lowest value first.
*  The first analog input follows directly after the last Digital characteristic.
*
* \return
*  A return value of type \ref cy_en_ble_gatt_err_code_t.
*  * CY_BLE_GATT_ERR_NONE - The advertisement packet collected successfully.
*  * CY_BLE_GATT_ERR_INVALID_HANDLE - 'handleValuePair.attrHandle' is not valid.
*  * Other GATT Error codes are specified in \ref cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_AIOS_SetAggregateChar(void)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    cy_en_ble_aios_char_index_t locCharIndex;
    uint16_t currentDescriptionValue;
    uint16_t minDescriptionValue;
    uint16_t minDescriptionValueTmp = 0xFFFFu;
    uint16_t maxDescriptionValue;
    uint16_t minDescriptionNumber;
    uint8_t locCharInstanceNumber;
    uint8_t aggregateChar[CY_BLE_AIOS_MAX_AGGREGATE_CHAR_LEN];
    uint8_t aggregateCharIndex = 0u;
    uint8_t aggregateDigitalBits = 0u;
    uint8_t numOfDigital;
    uint8_t i;

    if( (cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[CY_BLE_AIOS_AGGREGATE].charInfoPtr != NULL) &&
        (cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[CY_BLE_AIOS_AGGREGATE].charInfoPtr[0].charHandle !=
          CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE) &&
        (CY_BLE_GATT_DB_ATTR_GET_ATTR_GEN_LEN(cy_ble_aiossConfigPtr->attrInfo->
          charInfoAddr[CY_BLE_AIOS_AGGREGATE].charInfoPtr[0u].charHandle) < (cy_ble_configPtr->params->mtuSize - 3u)) )
    {
        for(locCharIndex = CY_BLE_AIOS_DIGITAL; (locCharIndex <= CY_BLE_AIOS_ANALOG) && (gattErr == CY_BLE_GATT_ERR_NONE);
            locCharIndex++)
        {
            minDescriptionValue = 0u;
            minDescriptionNumber = 0u;

            for(locCharInstanceNumber = 0u; (locCharInstanceNumber < cy_ble_aiossConfigPtr->charInstances[locCharIndex]) &&
                (gattErr == CY_BLE_GATT_ERR_NONE); locCharInstanceNumber++)
            {
                uint8_t locCharInstance;
                maxDescriptionValue = CY_BLE_AIOS_MAX_CHAR_DESCRIPTION_VALUE;

                for( locCharInstance = 0u; (locCharInstance < cy_ble_aiossConfigPtr->charInstances[locCharIndex]) &&
                                           (gattErr == CY_BLE_GATT_ERR_NONE); locCharInstance++)
                {
                    uint8_t locAttrValue[CY_BLE_AIOS_CHAR_PRESENTATION_FORMAT_LEN] = {0u};

                    if( (cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[locCharIndex].charInfoPtr != NULL) &&
                        (cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[locCharIndex].charInfoPtr[locCharInstance].charHandle !=
                               CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE) &&
                        (cy_ble_aiossConfigPtr->attrInfo->charInfoAddr[locCharIndex].charInfoPtr[locCharInstance].
                         descrHandle[CY_BLE_AIOS_CHAR_PRESENTATION_FORMAT] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE) )
                    {
                        /* Get characteristic value from GATT database */
                        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                        dbAttrValInfo.connHandle.bdHandle        = 0u;
                        dbAttrValInfo.connHandle.attId           = 0u;
                        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_aiossConfigPtr->attrInfo->
                                                                    charInfoAddr[locCharIndex].charInfoPtr[locCharInstance].
                                                                    descrHandle[CY_BLE_AIOS_CHAR_PRESENTATION_FORMAT];
                        dbAttrValInfo.handleValuePair.value.len  = CY_BLE_AIOS_CHAR_PRESENTATION_FORMAT_LEN;
                        dbAttrValInfo.handleValuePair.value.val  = locAttrValue;
                        dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
                        dbAttrValInfo.offset                     = 0u;                                                        
                    
                        gattErr = Cy_BLE_GATTS_ReadAttributeValue(&dbAttrValInfo);
                    }
                    else
                    {
                        gattErr = CY_BLE_GATT_ERR_INVALID_HANDLE;
                    }

                    if(gattErr == CY_BLE_GATT_ERR_NONE)
                    {
                        currentDescriptionValue =
                            ((uint16_t)(((uint16_t)locAttrValue[CY_BLE_AIOS_CHAR_PRESENTATION_FORMAT_LEN - 1u]) << 8u)) |
                            ((uint16_t)locAttrValue[CY_BLE_AIOS_CHAR_PRESENTATION_FORMAT_LEN - 2u]);

                        if( (currentDescriptionValue < maxDescriptionValue) &&
                            (currentDescriptionValue > minDescriptionValue) )
                        {
                            maxDescriptionValue = currentDescriptionValue;
                            minDescriptionNumber = locCharInstance;
                            minDescriptionValueTmp = currentDescriptionValue;
                        }
                    }
                }

                minDescriptionValue = minDescriptionValueTmp;

                if(locCharIndex == CY_BLE_AIOS_DIGITAL)
                {
                    /* Read descriptor CY_BLE_AIOS_NUM_OF_DIGITAL_DESCR value from database */
                    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                    dbAttrValInfo.connHandle.bdHandle        = 0u;
                    dbAttrValInfo.connHandle.attId           = 0u;
                    dbAttrValInfo.handleValuePair.attrHandle = cy_ble_aiossConfigPtr->attrInfo->
                                                                charInfoAddr[CY_BLE_AIOS_DIGITAL].
                                                                charInfoPtr[minDescriptionNumber].
                                                                descrHandle[CY_BLE_AIOS_NUM_OF_DIGITAL_DESCR];
                    dbAttrValInfo.handleValuePair.value.len  = CY_BLE_AIOS_NUM_OF_DIGITAL_DESCR_LEN;
                    dbAttrValInfo.handleValuePair.value.val  = &numOfDigital;
                    dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
                    dbAttrValInfo.offset                     = 0u;
                    gattErr = Cy_BLE_GATTS_ReadAttributeValue(&dbAttrValInfo);

                    if(gattErr == CY_BLE_GATT_ERR_NONE)
                    {
                        /* Read characteristic CY_BLE_AIOS_DIGITAL value from database */
                        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_aiossConfigPtr->attrInfo->
                                                                    charInfoAddr[CY_BLE_AIOS_DIGITAL].
                                                                    charInfoPtr[minDescriptionNumber].
                                                                    descrHandle[CY_BLE_AIOS_NUM_OF_DIGITAL_DESCR];

                        dbAttrValInfo.handleValuePair.value.len  = (((uint16_t)(numOfDigital) / 4u) + 1u);
                        dbAttrValInfo.handleValuePair.value.val  = &aggregateChar[aggregateCharIndex + 1u];
                        gattErr = Cy_BLE_GATTS_ReadAttributeValue(&dbAttrValInfo);

                        if(gattErr == CY_BLE_GATT_ERR_NONE)
                        {
                            for(i = 0u; i < ((numOfDigital / 4u) + 1u); i++)
                            {
                                aggregateChar[aggregateCharIndex + i] |=
                                    (uint8_t)(aggregateChar[aggregateCharIndex + i + 1u] << aggregateDigitalBits);
                                aggregateChar[aggregateCharIndex + i + 1u] >>= (8u - aggregateDigitalBits);
                            }

                            aggregateCharIndex += (((aggregateDigitalBits / 2u) + numOfDigital) / 4u);
                            aggregateDigitalBits = (((aggregateDigitalBits / 2u) + numOfDigital) % 4u) * 2u;
                        }
                    }
                }
                else if (locCharIndex == CY_BLE_AIOS_ANALOG)
                {
                    /* Read characteristic CY_BLE_AIOS_ANALOG value from database */
                    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
                    dbAttrValInfo.connHandle.bdHandle        = 0u;
                    dbAttrValInfo.connHandle.attId           = 0u;
                    dbAttrValInfo.handleValuePair.attrHandle = cy_ble_aiossConfigPtr->attrInfo->
                                                                charInfoAddr[locCharIndex].
                                                                charInfoPtr[minDescriptionNumber].charHandle;
                    dbAttrValInfo.handleValuePair.value.len  = CY_BLE_AIOS_ANALOG_LEN;
                    dbAttrValInfo.handleValuePair.value.val  = &aggregateChar[aggregateCharIndex];
                    dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
                    dbAttrValInfo.offset                     = 0u;
                    
                    gattErr = Cy_BLE_GATTS_ReadAttributeValue(&dbAttrValInfo);
                    if(gattErr == CY_BLE_GATT_ERR_NONE)
                    {
                        aggregateCharIndex += 2u;
                    }
                }
                else
                {
                    /* Do nothing */
                }
            }
            aggregateCharIndex++;
        }

        if(gattErr == CY_BLE_GATT_ERR_NONE)
        {
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;
            dbAttrValInfo.connHandle.bdHandle        = 0u;
            dbAttrValInfo.connHandle.attId           = 0u;
            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_aiossConfigPtr->attrInfo->
                                                        charInfoAddr[CY_BLE_AIOS_AGGREGATE].
                                                        charInfoPtr[0u].charHandle;
            dbAttrValInfo.handleValuePair.value.len  = (uint16_t)aggregateCharIndex - 1u;
            dbAttrValInfo.handleValuePair.value.val  = aggregateChar;
            dbAttrValInfo.flags                      = CY_BLE_GATT_DB_LOCALLY_INITIATED;
            dbAttrValInfo.offset                     = 0u;
                                                        
            /* Store data in database */
            gattErr = Cy_BLE_GATTS_WriteAttributeValue(&dbAttrValInfo);
        }
    }

    return gattErr;
}


/******************************************************************************
* Function Name: Cy_BLE_AIOSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  Based on the descriptor UUID, an appropriate data structure is populated using
*  the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_AIOSC_DiscoverCharacteristicsEventHandler(const cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* AIOS characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_AioscCharUuid[CY_BLE_AIOS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_DIGITAL,
        CY_BLE_UUID_CHAR_ANALOG,
        CY_BLE_UUID_CHAR_AGGREGATE
    };

    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t exitLoop = 0u;
    uint32_t aiosDiscIdx = cy_ble_aioscConfigPtr->serviceDiscIdx;
    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == aiosDiscIdx))
    {
         uint32_t i;
    
        /* Update last characteristic endHandle to declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        /* Search through all available characteristics */
        for(i = 0u; ((i < (uint32_t)CY_BLE_AIOS_CHAR_COUNT) && (exitLoop == 0u)); i++)
        {
            uint32_t j;
            for(j = 0u; j < cy_ble_aioscConfigPtr->charInstances[i]; j++)
            {
                if( (cy_ble_AioscCharUuid[i] == discCharInfo->uuid.uuid16) &&
                    (cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[i].charInfoPtr != NULL) &&
                    (cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[i].charInfoPtr[j].valueHandle ==
                           CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE) )
                {
                    cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[i].charInfoPtr[j].valueHandle =
                        discCharInfo->valueHandle;
                    cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[i].charInfoPtr[j].properties =
                        discCharInfo->properties;
                    lastEndHandle[discIdx] =
                        &cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[i].charInfoPtr[j].endHandle;
                    exitLoop = 1u;
                    break;
                }
            }
        }

        /* Initialize characteristic endHandle to service endHandle.
         * Characteristic endHandle will be updated to the declaration
         * handler of the following characteristic,
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
* Function Name: Cy_BLE_AIOSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  Based on the descriptor UUID, an appropriate data structure is populated using
*  the data received as part of the callback.
*
*  \param discDescrInfo: The pointer to a descriptor information structure.
*
******************************************************************************/
static void Cy_BLE_AIOSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t notSupportedDescr = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t aiosDiscIdx = cy_ble_aioscConfigPtr->serviceDiscIdx;
    uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
    uint32_t charInst = cy_ble_aioscActiveCharInstance[discIdx];
    uint32_t descIdx;

    if((cy_ble_configPtr->context->discovery[discIdx].servCount == aiosDiscIdx) &&
       (cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIdx].charInfoPtr != NULL))
    {
        switch(discDescrInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_CLIENT_CONFIG:
                descIdx = (uint32_t)CY_BLE_AIOS_CCCD;
                break;

            case CY_BLE_UUID_CHAR_FORMAT:
                descIdx = (uint32_t)CY_BLE_AIOS_CHAR_PRESENTATION_FORMAT;
                break;

            case CY_BLE_UUID_CHAR_USER_DESCRIPTION:
                descIdx = (uint32_t)CY_BLE_AIOS_CHAR_USER_DESCRIPTION_DESCR;
                break;

            case CY_BLE_UUID_CHAR_EXTENDED_PROPERTIES:
                descIdx = (uint32_t)CY_BLE_AIOS_CHAR_EXTENDED_PROPERTIES;
                break;

            case CY_BLE_UUID_CHAR_VALUE_TRIGGER_SETTING:
                descIdx = (uint32_t)CY_BLE_AIOS_VALUE_TRIGGER_SETTINGS;
                break;

            case CY_BLE_UUID_CHAR_TIME_TRIGGER_SETTING:
                descIdx = (uint32_t)CY_BLE_AIOS_TIME_TRIGGER_SETTINGS;
                break;

            case CY_BLE_UUID_CHAR_VALID_RANGE:
                descIdx = (uint32_t)CY_BLE_AIOS_VRD;
                break;

            case CY_BLE_UUID_CHAR_NUMBER_OF_DIGITALS:
                descIdx = (uint32_t)CY_BLE_AIOS_NUM_OF_DIGITAL_DESCR;
                break;

            default:
                /* Not supported descriptor */
                notSupportedDescr = 1u;
                break;
        }

        if(notSupportedDescr == 0u)
        {
            if(cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIdx].charInfoPtr[charInst].descrHandle[descIdx] ==
               CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIdx].charInfoPtr[charInst].descrHandle[descIdx] =
                    discDescrInfo->descrHandle;
            }
            else    /* Duplication of descriptor */
            {
                Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_DESCR_DUPLICATION, &discDescrInfo);
            }
        }

        /* Indicates that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_AIOSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_AIOSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t aiosDiscIdx = cy_ble_aioscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == aiosDiscIdx)
    {
        if(charRangeInfo->srviIncIdx == CY_BLE_DISCOVERY_INIT)
        {
            /* cy_ble_configPtr->context->discovery_INIT */
            cy_ble_aioscActiveCharInstance[discIdx] = 0u;
        }
        else
        {
            /* cy_ble_configPtr->context->discovery_CONTINUE */
            cy_ble_aioscActiveCharInstance[discIdx]++;
            if(cy_ble_aioscActiveCharInstance[discIdx] ==
               cy_ble_aioscConfigPtr->charInstances[cy_ble_configPtr->context->discovery[discIdx].charCount])
            {
                cy_ble_configPtr->context->discovery[discIdx].charCount++;
                cy_ble_aioscActiveCharInstance[discIdx] = 0u;
            }
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_AIOS_CHAR_COUNT) && (exitFlag == 0u))
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;

            while((cy_ble_aioscActiveCharInstance[discIdx] < cy_ble_aioscConfigPtr->charInstances[charIdx]) &&
                  (exitFlag == 0u))
            {
                uint32_t charInst = cy_ble_aioscActiveCharInstance[discIdx];
                if((cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIdx].charInfoPtr[charInst].valueHandle !=
                    CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE) &&
                   ((cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIdx].charInfoPtr[charInst].endHandle -
                     cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIdx].charInfoPtr[charInst].valueHandle) != 0u))
                {
                    charRangeInfo->range.startHandle =
                        cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIdx].charInfoPtr[charInst].valueHandle + 1u;
                    charRangeInfo->range.endHandle =
                        cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIdx].charInfoPtr[charInst].endHandle;
                    exitFlag = 1u;
                }
                else
                {
                    cy_ble_aioscActiveCharInstance[discIdx]++;
                }
            }
            if(exitFlag == 0u)
            {
                cy_ble_configPtr->context->discovery[discIdx].charCount++;
                cy_ble_aioscActiveCharInstance[discIdx] = 0u;
            }
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_AIOSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles a notification event for the Automation IO service.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_AIOSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_AIOS_ApplCallback != NULL))
    {
        cy_en_ble_aios_char_index_t locCharIndex;
        uint32_t exitLoop = 0u;
        
        for(locCharIndex = CY_BLE_AIOS_DIGITAL; (locCharIndex < CY_BLE_AIOS_CHAR_COUNT) &&
            (exitLoop == 0u); locCharIndex++)
        {
            if(cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr != NULL)
            {
                uint32_t j;
                for(j = 0u; j < cy_ble_aioscConfigPtr->charInstances[locCharIndex]; j++)
                {
                    if(cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr[j].valueHandle ==
                       eventParam->handleValPair.attrHandle)
                    {
                        cy_stc_ble_aios_char_value_t notifValue;
                        notifValue.connHandle = eventParam->connHandle;
                        notifValue.charIndex  = locCharIndex;
                        notifValue.value      = &eventParam->handleValPair.value;

                        Cy_BLE_AIOS_ApplCallback((uint32_t)CY_BLE_EVT_AIOSC_NOTIFICATION, &notifValue);
                        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                        exitLoop = 1u;
                        break;
                    }
                }
            }
        }
    }
}

/******************************************************************************
* Function Name: Cy_BLE_AIOSC_IndicationEventHandler
***************************************************************************//**
*
*  Handles an indication event for Automation IO service.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_AIOSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam)
{
    uint32_t j;
    cy_en_ble_aios_char_index_t locCharIndex;


    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_AIOS_ApplCallback != NULL))
    {
        for(locCharIndex = CY_BLE_AIOS_DIGITAL; locCharIndex < CY_BLE_AIOS_CHAR_COUNT; locCharIndex++)
        {
            if(cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr != NULL)
            {
                for(j = 0u; j < cy_ble_aioscConfigPtr->charInstances[locCharIndex]; j++)
                {
                    if(cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr[j].valueHandle ==
                       eventParam->handleValPair.attrHandle)
                    {
                        cy_stc_ble_aios_char_value_t indicationValue;
                        indicationValue.connHandle = eventParam->connHandle;
                        indicationValue.charIndex  = locCharIndex;
                        indicationValue.value      = &eventParam->handleValPair.value;

                        Cy_BLE_AIOS_ApplCallback((uint32_t)CY_BLE_EVT_AIOSC_INDICATION, &indicationValue);
                        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                        break;
                    }
                }
            }
        }
    }
}

/******************************************************************************
* Function Name: Cy_BLE_AIOSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles a Read Response event for the Automation IO service.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_AIOSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_AIOS_ApplCallback != NULL) &&
       (cy_ble_aioscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        cy_en_ble_aios_char_index_t locCharIndex;
        uint32_t locReqHandle = 0u;
        
        for(locCharIndex = CY_BLE_AIOS_DIGITAL; (locCharIndex < CY_BLE_AIOS_CHAR_COUNT) &&
            (locReqHandle == 0u); locCharIndex++)
        {
            if(cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr != NULL)
            {
                uint8_t j;
                for(j = 0u; ((j < cy_ble_aioscConfigPtr->charInstances[locCharIndex]) && (locReqHandle == 0u)); j++)
                {
                    if(cy_ble_aioscReqHandle[discIdx] ==
                       cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr[j].valueHandle)
                    {
                        cy_stc_ble_aios_char_value_t locCharValue;
                        locCharValue.connHandle   = eventParam->connHandle;
                        locCharValue.charIndex    = locCharIndex;
                        locCharValue.charInstance = j;
                        locCharValue.value        = &eventParam->value;
                        
                        cy_ble_aioscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                        Cy_BLE_AIOS_ApplCallback((uint32_t)CY_BLE_EVT_AIOSC_READ_CHAR_RESPONSE, &locCharValue);
                        locReqHandle = 1u;
                    }
                    else
                    {
                        cy_en_ble_aios_descr_index_t locDescIndex;

                        for(locDescIndex = CY_BLE_AIOS_CCCD; (locDescIndex < CY_BLE_AIOS_DESCR_COUNT) &&
                            (locReqHandle == 0u); locDescIndex++)
                        {
                            if(cy_ble_aioscReqHandle[discIdx] == cy_ble_aioscConfigPtr->attrInfo[discIdx].
                                charInfoAddr[locCharIndex].charInfoPtr[j].
                                descrHandle[locDescIndex])
                            {
                                cy_stc_ble_aios_descr_value_t locDescrValue;
                                locDescrValue.connHandle   = eventParam->connHandle;
                                locDescrValue.charIndex    = locCharIndex;
                                locDescrValue.descrIndex   = locDescIndex;
                                locDescrValue.charInstance = j;
                                locDescrValue.value        = &eventParam->value;

                                cy_ble_aioscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                                Cy_BLE_AIOS_ApplCallback((uint32_t)CY_BLE_EVT_AIOSC_READ_DESCR_RESPONSE, &locDescrValue);
                                locReqHandle = 1u;
                            }
                        }
                    }
                }
            }
        }
        if(locReqHandle != 0u)
        {
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}

/******************************************************************************
* Function Name: Cy_BLE_AIOSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles a Write Response event for the Automation IO service.
*
*  \param eventParam: The pointer to a data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_AIOSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    /* Check whether service handler was registered and request handle is
     * valid.
     */
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_AIOS_ApplCallback != NULL) &&
       (cy_ble_aioscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        cy_en_ble_aios_char_index_t locCharIndex;
        uint32_t locReqHandle = 0u;
        
        for(locCharIndex = CY_BLE_AIOS_DIGITAL; (locCharIndex < CY_BLE_AIOS_CHAR_COUNT) &&
            (locReqHandle == 0u); locCharIndex++)
        {
            /* If this condition is false, this means that the current characteristic
             * is not included in the GUI. In other words, support for the characteristic is
             * not enabled.
             */
            if(cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr != NULL)
            {
                uint8_t j;
                
                for(j = 0u; ((j < cy_ble_aioscConfigPtr->charInstances[locCharIndex]) && (locReqHandle == 0u)); j++)
                {
                    cy_stc_ble_aios_descr_value_t locDescrValue;
                    locDescrValue.connHandle   = *eventParam;
                    locDescrValue.charIndex    = locCharIndex;
                    locDescrValue.charInstance = j;
                    locDescrValue.value        = NULL;
                    
                    /* Check whether requested descriptor handle is in
                     * characteristic range.
                     */
                    if((cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr[j].valueHandle <
                        cy_ble_aioscReqHandle[discIdx]) &&
                       (cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr[j].endHandle >=
                        cy_ble_aioscReqHandle[discIdx]))
                    {
                        if(cy_ble_aioscReqHandle[discIdx] ==
                           cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr[j].
                            descrHandle[CY_BLE_AIOS_CCCD])
                        {
                            locDescrValue.descrIndex = CY_BLE_AIOS_CCCD;
                        }
                        else if(cy_ble_aioscReqHandle[discIdx] ==
                                cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr[j].
                                 descrHandle[CY_BLE_AIOS_CHAR_PRESENTATION_FORMAT])
                        {
                            locDescrValue.descrIndex = CY_BLE_AIOS_CHAR_PRESENTATION_FORMAT;
                        }
                        else if(cy_ble_aioscReqHandle[discIdx] ==
                                cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr[j].
                                 descrHandle[CY_BLE_AIOS_CHAR_USER_DESCRIPTION_DESCR])
                        {
                            locDescrValue.descrIndex = CY_BLE_AIOS_CHAR_USER_DESCRIPTION_DESCR;
                        }
                        else if(cy_ble_aioscReqHandle[discIdx] ==
                                cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr[j].
                                 descrHandle[CY_BLE_AIOS_CHAR_EXTENDED_PROPERTIES])
                        {
                            locDescrValue.descrIndex = CY_BLE_AIOS_CHAR_EXTENDED_PROPERTIES;
                        }
                        else if(cy_ble_aioscReqHandle[discIdx] ==
                                cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr[j].
                                 descrHandle[CY_BLE_AIOS_VALUE_TRIGGER_SETTINGS])
                        {
                            locDescrValue.descrIndex = CY_BLE_AIOS_VALUE_TRIGGER_SETTINGS;
                        }
                        else if(cy_ble_aioscReqHandle[discIdx] ==
                                cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr[j].
                                 descrHandle[CY_BLE_AIOS_TIME_TRIGGER_SETTINGS])
                        {
                            locDescrValue.descrIndex = CY_BLE_AIOS_TIME_TRIGGER_SETTINGS;
                        }
                        else if(cy_ble_aioscReqHandle[discIdx] ==
                                cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr[j].
                                 descrHandle[CY_BLE_AIOS_VRD])
                        {
                            locDescrValue.descrIndex = CY_BLE_AIOS_VRD;
                        }
                        else if(cy_ble_aioscReqHandle[discIdx] ==
                                cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[locCharIndex].charInfoPtr[j].
                                 descrHandle[CY_BLE_AIOS_NUM_OF_DIGITAL_DESCR])
                        {
                            locDescrValue.descrIndex = CY_BLE_AIOS_NUM_OF_DIGITAL_DESCR;
                        }
                        else
                        {
                            /* Should never enter here */
                        }

                        cy_ble_aioscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                        Cy_BLE_AIOS_ApplCallback((uint32_t)CY_BLE_EVT_AIOSC_WRITE_DESCR_RESPONSE, &locDescrValue);
                        locReqHandle = 1u;
                    }
                }
            }
        }

        if(locReqHandle != 0u)
        {
            cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
        }
    }
}

/******************************************************************************
* Function Name: Cy_BLE_AIOSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles an Error Response event for the Automation IO service.
*
*  \param eventParam: The pointer to a data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_AIOSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if(cy_ble_aioscReqHandle[discIdx] == eventParam->errInfo.attrHandle)
    {
        cy_ble_aioscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }
}

/******************************************************************************
* Function Name: Cy_BLE_AIOSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result, a Write Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the #CY_BLE_EVT_AIOSS_WRITE_CHAR event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of the service characteristic. Starts with zero.
*  \param charInstance: The instance number of the characteristic specified by
*                       "charIndex".
*  \param attrSize:     The size of the characteristic value attribute.
*  \param attrValue:    The pointer to the characteristic value data that
*                       should be sent to the server device.
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
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the AIOS service-specific callback is registered
*   with \ref Cy_BLE_AIOS_RegisterAttrCallback():
*   * #CY_BLE_EVT_AIOSC_WRITE_CHAR_RESPONSE - In case the requested attribute is
*     successfully written on the peer device, the details (charIndex, etc.) are
*     provided with event parameter structure of type \ref cy_stc_ble_aios_char_value_t.
*   .
*   Otherwise (if an AIOS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is successfully 
*     written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the requested 
*     attribute on the peer device, the details are provided with an event
*     parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_AIOSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_aios_char_index_t charIndex,
                                                           uint8_t charInstance,
                                                           uint8_t attrSize,
                                                           uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_AIOS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(charInstance >= cy_ble_aioscConfigPtr->charInstances[charIndex])
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].charInfoPtr[charInstance].valueHandle !=
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle =
            cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].charInfoPtr[charInstance].valueHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;
        
        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);

        /* Save handle to support service-specific Write Response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_aioscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_AIOSC_GetCharacteristicValue
***************************************************************************//**
*
*  This function is used to read a characteristic value, which is a value
*  identified by charIndex, from the server.
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of the service characteristic. Starts with zero.
*  \param charInstance: The instance number of the characteristic specified by
*                       "charIndex".
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
* \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the AIOS service-specific callback is registered
*   with \ref Cy_BLE_AIOS_RegisterAttrCallback():
*   * #CY_BLE_EVT_AIOSC_READ_CHAR_RESPONSE - In case the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_aios_char_value_t.
*   .
*   Otherwise (if an AIOS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_READ_RSP - If the requested attribute is
*     successfully read on the peer device, the details (handle, value, etc.) are
*     provided with an event parameter structure 
*     \ref cy_stc_ble_gattc_read_rsp_param_t.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_AIOSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                           cy_en_ble_aios_char_index_t charIndex,
                                                           uint8_t charInstance)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_AIOS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(charInstance >= cy_ble_aioscConfigPtr->charInstances[charIndex])
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].charInfoPtr[charInstance].valueHandle !=
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].
            charInfoPtr[charInstance].valueHandle;
        readReqParam.connHandle = connHandle;
      
        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_aioscReqHandle[discIdx] =
                cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].charInfoPtr[charInstance].valueHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_AIOSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result a Write Request is
*  sent to the GATT server and on successful execution of the request on the
*  server side, the #CY_BLE_EVT_AIOSS_DESCR_WRITE event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  Internally, a Write Request is sent to the GATT server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*       * #CY_BLE_EVT_AIOSS_NOTIFICATION_ENABLED
*       * #CY_BLE_EVT_AIOSS_NOTIFICATION_DISABLED
*       * #CY_BLE_EVT_AIOSS_INDICATION_ENABLED
*       * #CY_BLE_EVT_AIOSS_INDICATION_DISABLED
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of the service characteristic. Starts with zero.
*  \param descrIndex:   The index of the service characteristic descriptor.
*  \param charInstance: The instance number of the characteristic specified by
*                       "charIndex".
*  \param attrSize:     The size of the characteristic value attribute.
*  \param attrValue:    The pointer to the characteristic descriptor value
*                       data that should be sent to the server device.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*    
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this descriptor.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular descriptor.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
*  \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the AIOS service-specific callback is registered
*   with \ref Cy_BLE_AIOS_RegisterAttrCallback():
*
*   * #CY_BLE_EVT_AIOSC_WRITE_DESCR_RESPONSE - In case the requested attribute is
*     successfully written on the peer device, the details (charIndex,
*     descrIndex etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_aios_descr_value_t.
*   .
*   Otherwise (if an AIOS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is
*     successfully written on the peer device.
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*     requested attribute on the peer device, the details are provided with 
*     an event parameter structure ( \ref cy_stc_ble_gatt_err_param_t).
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_AIOSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_aios_char_index_t charIndex,
                                                                uint8_t charInstance,
                                                                cy_en_ble_aios_descr_index_t
                                                                descrIndex,
                                                                uint8_t attrSize,
                                                                uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((discIdx >= cy_ble_configPtr->params->maxClientCount) ||
            (charIndex >= CY_BLE_AIOS_CHAR_COUNT) ||
            (charInstance >= cy_ble_aioscConfigPtr->charInstances[charIndex]) ||
            (descrIndex >= CY_BLE_AIOS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].charInfoPtr[charInstance].descrHandle[descrIndex] ==
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((descrIndex == CY_BLE_AIOS_CHAR_PRESENTATION_FORMAT) ||
            (descrIndex == CY_BLE_AIOS_CHAR_EXTENDED_PROPERTIES) ||
            (descrIndex == CY_BLE_AIOS_VRD) ||
            (descrIndex == CY_BLE_AIOS_NUM_OF_DIGITAL_DESCR))
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle = cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].
                                                 charInfoPtr[charInstance].descrHandle[descrIndex];
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;
        /* ... and send request to server's device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_aioscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }
    
  
    return(apiResult);
}

/******************************************************************************
* Function Name: Cy_BLE_AIOSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Sends a request to get the characteristic descriptor of the specified
*  characteristic of the service.
*
*  \param connHandle: The connection handle.
*  \param charIndex: The index of the service characteristic. Starts with zero.
*  \param charInstance: The instance number of the characteristic specified by
*                "charIndex".
*  \param descrIndex: The index of the service characteristic descriptor.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*    
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this descriptor.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular descriptor.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
*  \events
*   In case of successful execution (return value = #CY_BLE_SUCCESS)
*   the following events can appear: \n
*   If the AIOS service-specific callback is registered
*     with \ref Cy_BLE_AIOS_RegisterAttrCallback():
*   * #CY_BLE_EVT_AIOSC_READ_DESCR_RESPONSE - In case the requested attribute is
*     successfully read on the peer device, the details (charIndex,
*     descrIndex, value, etc.) are provided with event parameter structure
*     of type \ref cy_stc_ble_aios_descr_value_t.
*   .
*   Otherwise (if an AIOS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_AIOSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                                cy_en_ble_aios_char_index_t charIndex,
                                                                uint8_t charInstance,
                                                                cy_en_ble_aios_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

        if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
        {
            apiResult = CY_BLE_ERROR_INVALID_STATE;
        }
        else if((discIdx >= cy_ble_configPtr->params->maxClientCount) ||
                (charIndex >= CY_BLE_AIOS_CHAR_COUNT) ||
                (charInstance >= cy_ble_aioscConfigPtr->charInstances[charIndex]) ||
                (descrIndex >= CY_BLE_AIOS_DESCR_COUNT))
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
        else if(cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].charInfoPtr[charInstance].descrHandle[descrIndex] ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
        }
        else
        {
            cy_stc_ble_gattc_read_req_t readReqParam;
            readReqParam .attrHandle = cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].
                                        charInfoPtr[charInstance].descrHandle[descrIndex];
            readReqParam.connHandle = connHandle;
                            
            apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

            /* Save handle to support service-specific read response from device */
            if(apiResult == CY_BLE_SUCCESS)
            {
                cy_ble_aioscReqHandle[discIdx] = cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].
                                                  charInfoPtr[charInstance].descrHandle[descrIndex];
            }
        }


    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_AIOSC_SetCharacteristicValueWithoutResponse
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server without response.
*
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of the service characteristic.
*  \param charInstance: The instance number of the characteristic specified by
*                       "charIndex".
*  \param attrSize:     The size of the characteristic value attribute.
*  \param attrValue:    The pointer to the characteristic value data that should be
*                       sent to the server device.
*
* \return
*  A return value of type \ref cy_en_ble_api_result_t.
*    
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this descriptor.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_AIOSC_SetCharacteristicValueWithoutResponse(cy_stc_ble_conn_handle_t connHandle,
                                                                          cy_en_ble_aios_char_index_t charIndex,
                                                                          uint8_t charInstance,
                                                                          uint8_t attrSize,
                                                                          uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);
    cy_stc_ble_gatt_xchg_mtu_param_t mtuParam;

    mtuParam.connHandle = connHandle;
    (void)Cy_BLE_GATT_GetMtuSize(&mtuParam);

    /* Check parameters */
    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_AIOS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(charInstance >= cy_ble_aioscConfigPtr->charInstances[charIndex])
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].charInfoPtr[charInstance].valueHandle ==
            CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }
    else if((CY_BLE_CHAR_PROP_WRITE & cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].
              charInfoPtr[charInstance].properties) == 0u)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else if((mtuParam.mtu - CY_BLE_GATT_WRITE_HEADER_LEN) < attrSize)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        cy_stc_ble_gattc_write_cmd_req_t writeCmdReqParam;
        writeCmdReqParam.handleValPair.attrHandle = cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].
                                                    charInfoPtr[charInstance].valueHandle;
        writeCmdReqParam.handleValPair.value.val  = attrValue;
        writeCmdReqParam.handleValPair.value.len  = attrSize;
        writeCmdReqParam.connHandle               = connHandle;

        apiResult = Cy_BLE_GATTC_WriteWithoutResponse(&writeCmdReqParam);
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_aioscReqHandle[discIdx] = cy_ble_aioscConfigPtr->attrInfo[discIdx].charInfoAddr[charIndex].
                                              charInfoPtr[charInstance].valueHandle;
        }
    }
    
    return(apiResult);
}

/******************************************************************************
* Function Name: Cy_BLE_AIOSC_GetCharacteristicValueHandle
***************************************************************************//**
*
*  Returns the discovered peer device characteristic value handle.
*
*  \param connHandle:   The connection handle.
*  \param serviceIndex: The index of the service instance.
*  \param charIndex:    The index of a service characteristic.
*  \param charInstance: The instance of characteristic
*  \return
*   Returns characteristic value handle of type cy_ble_gatt_db_attr_handle_t.
*   * CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE: when a peer device doesn't have an optional characteristic
*
******************************************************************************/
cy_ble_gatt_db_attr_handle_t Cy_BLE_AIOSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                       cy_en_ble_aios_char_index_t charIndex,
                                                                       uint8_t charInstance)
{
    cy_ble_gatt_db_attr_handle_t returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
   
    if((charIndex < CY_BLE_AIOS_CHAR_COUNT) && (charInstance < cy_ble_aiossConfigPtr->charInstances[charIndex]))
    {  
        returnHandle = cy_ble_aioscConfigPtr->attrInfo[Cy_BLE_GetDiscoveryIdx(connHandle)].
                            charInfoAddr[charIndex].charInfoPtr[charInstance].valueHandle;
    }
    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_AIOSC_GetCharacteristicDescriptorHandle
***************************************************************************//**
*
*  Returns the discovered peer device characteristic descriptor handle.
*
*  \param connHandle:   The connection handle.
*  \param serviceIndex: The index of the service instance.
*  \param charIndex:    The index of a service characteristic.
*  \param charInstance: The instance of characteristic
*  \param descrIndex:   The index of the descriptor.
*
* \return
*  Returns characteristic value handle of type cy_ble_gatt_db_attr_handle_t.
*  * CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE: when a peer device doesn't have an optional descriptor
*
******************************************************************************/
cy_ble_gatt_db_attr_handle_t Cy_BLE_AIOSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                            cy_en_ble_aios_char_index_t charIndex,
                                                                            uint8_t charInstance,
                                                                            cy_en_ble_aios_descr_index_t descrIndex)
{
    cy_ble_gatt_db_attr_handle_t returnHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    if((charIndex < CY_BLE_AIOS_CHAR_COUNT) && (charInstance < cy_ble_aiossConfigPtr->charInstances[charIndex]) &&
       (descrIndex < CY_BLE_AIOS_DESCR_COUNT))
    {  
        returnHandle = cy_ble_aioscConfigPtr->attrInfo[Cy_BLE_GetDiscoveryIdx(connHandle)].
                            charInfoAddr[charIndex].charInfoPtr[charInstance].descrHandle[descrIndex];
    }

    return(returnHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_AIOS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the AIOS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type \ref cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_AIOS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_AIOSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_AIOSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_AIOSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_AIOSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_AIOSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the AIOS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type \ref cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_AIOSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_AIOSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_WRITE_CMD_REQ:
            (void)Cy_BLE_AIOSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;
            
        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            Cy_BLE_AIOSS_ConfirmationEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_AIOSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the AIOS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type \ref cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_AIOSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_AIOSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_AIOSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_AIOSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_AIOSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_AIOSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_IND:
                Cy_BLE_AIOSC_IndicationEventHandler((cy_stc_ble_gattc_handle_value_ind_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_AIOSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_AIOSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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
