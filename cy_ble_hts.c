/***************************************************************************//**
* \file cy_ble_hts.c
* \version 3.30
*
* \brief
*  Contains the source code for Health Thermometer service.
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

#define Cy_BLE_HTSC_CheckCharHandle(handle)                                  \
    do {                                                                     \
        if((handle).valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)    \
        {                                                                    \
            (handle).valueHandle = discCharInfo->valueHandle;                \
            (handle).properties = discCharInfo->properties;                  \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, \
                                &(discCharInfo->uuid.uuid16));               \
        }                                                                    \
    } while(0)


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_HTS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_HTSS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_HTSC_EventHandler(uint32_t eventCode, void *eventParam);

static void Cy_BLE_HTSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_HTSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam);

static void Cy_BLE_HTSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_HTSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo);
static void Cy_BLE_HTSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo);
static void Cy_BLE_HTSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam);
static void Cy_BLE_HTSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam);
static void Cy_BLE_HTSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_HTSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam);
static void Cy_BLE_HTSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_HTS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_HTSC_EventHandlerCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_HTSS_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response for server */
static cy_ble_gatt_db_attr_handle_t cy_ble_htssReqHandle;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_htscReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE HTS server config structure */
const cy_stc_ble_htss_config_t *cy_ble_htssConfigPtr = NULL;

/* The pointer to a global BLE HTS client config structure */
const cy_stc_ble_htsc_config_t *cy_ble_htscConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_HTSS_Init
***************************************************************************//**
*
*  This function initializes server of the Health Thermometer service.
*
*  \param config: Configuration structure for the HTS.
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
cy_en_ble_api_result_t Cy_BLE_HTSS_Init(const cy_stc_ble_htss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_htssConfigPtr = config;

        /* Registers event handler for the HTS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_HTS_EventHandler);

        /* Registers a pointer to server event handler function */
        Cy_BLE_HTSS_EventHandlerCallback = &Cy_BLE_HTSS_EventHandler;

        cy_ble_htssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HTSC_Init
***************************************************************************//**
*
*  This function initializes client of the Health Thermometer service.
*
*  \param config: Configuration structure for the HTS.
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
cy_en_ble_api_result_t Cy_BLE_HTSC_Init(const cy_stc_ble_htsc_config_t *config)
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
        cy_ble_htscConfigPtr = config;

        /* Registers event handler for the HTS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_HTS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_HTSC_EventHandlerCallback = &Cy_BLE_HTSC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 htsServIdx = cy_ble_htscConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + htsServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to HTS client structure */
                cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(htscPtr, 0, sizeof(cy_stc_ble_htsc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + htsServIdx].uuid =
                    CY_BLE_UUID_HEALTH_THERMOMETER_SERVICE;
            }

            cy_ble_htscReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HTS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for HTS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_HTSS_NOTIFICATION_ENABLED).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_hts_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_HTS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_HTS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_HTSS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets the characteristic value of the service in the local database.
*
*  \param charIndex:  The index of the service characteristic.
*
*  \param attrSize:   The size (in Bytes) of the characteristic value attribute.
*
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     stored in the GATT database.
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
cy_en_ble_api_result_t Cy_BLE_HTSS_SetCharacteristicValue(cy_en_ble_hts_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if(charIndex >= CY_BLE_HTS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Store data in database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_htssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_HTSS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets the characteristic value of the service, which is a value identified by
*  charIndex.
*
*  \param charIndex: The index of the service characteristic.
*
*  \param attrSize:  The size of the characteristic value attribute.
*
*  \param attrValue: The pointer to the location where characteristic value data
*                    should be stored.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HTSS_GetCharacteristicValue(cy_en_ble_hts_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if(charIndex >= CY_BLE_HTS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Read characteristic value from database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_htssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
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
* Function Name: Cy_BLE_HTSS_SetCharacteristicDescriptor
***************************************************************************//**
*
*  Sets the characteristic descriptor of the specified characteristic.
*
*  \param connHandle: The connection handle.
*
*  \param charIndex:  The index of the service characteristic.
*
*  \param descrIndex: The index of the service characteristic descriptor.
*
*  \param attrSize:   The size of the characteristic descriptor attribute.
*
*  \param attrValue:  The pointer to the descriptor value data that should
*                     be stored in the GATT database.
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
cy_en_ble_api_result_t Cy_BLE_HTSS_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hts_char_index_t charIndex,
                                                               cy_en_ble_hts_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((charIndex >= CY_BLE_HTS_CHAR_COUNT) || (descrIndex >= CY_BLE_HTS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        if(cy_ble_htssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            /* Store data in the database */
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_htssConfigPtr->attrInfo->
                                                       charInfo[charIndex].descrHandle[descrIndex];
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
        else
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HTSS_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets the characteristic descriptor of the specified characteristic.
*
*  \param connHandle: The connection handle
*
*  \param charIndex:  The index of the service characteristic.
*
*  \param descrIndex: The index of the service characteristic descriptor.
*
*  \param attrSize:   The size of the characteristic descriptor attribute.
*
*  \param attrValue:  The pointer to the location where characteristic descriptor
*                     value data should be stored.
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*
*******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HTSS_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hts_char_index_t charIndex,
                                                               cy_en_ble_hts_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if((charIndex >= CY_BLE_HTS_CHAR_COUNT) || (descrIndex >= CY_BLE_HTS_DESCR_COUNT))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        if(cy_ble_htssConfigPtr->attrInfo->charInfo[charIndex].descrHandle[descrIndex] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
        {
            cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

            dbAttrValInfo.handleValuePair.attrHandle = cy_ble_htssConfigPtr->attrInfo->
                                                       charInfo[charIndex].descrHandle[descrIndex];
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
        else
        {
            apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HTSS_WriteEventHandler
***************************************************************************//**
*
*  Handles Write Request event for HTS service.
*
*  \param void *eventParam: The pointer to the data structure specified by the event.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HTSS_WriteEventHandler(cy_stc_ble_gatts_write_cmd_req_param_t *eventParam)
{
    cy_en_ble_hts_char_index_t locCharIndex;
    cy_stc_ble_hts_char_value_t locCharValue;
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;
    uint8_t locReqHandle = 0u;


    if(Cy_BLE_HTS_ApplCallback != NULL)
    {
        locCharValue.connHandle = eventParam->connHandle;

        for(locCharIndex = CY_BLE_HTS_TEMP_MEASURE; (locCharIndex < CY_BLE_HTS_CHAR_COUNT) && (locReqHandle == 0u);
            locCharIndex++)
        {
            if((eventParam->handleValPair.attrHandle ==
                cy_ble_htssConfigPtr->attrInfo->charInfo[locCharIndex].descrHandle[CY_BLE_HTS_CCCD]) ||
               (eventParam->handleValPair.attrHandle ==
                cy_ble_htssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle))
            {
                locCharValue.charIndex = locCharIndex;
                locCharValue.value = &eventParam->handleValPair.value;
                /* Characteristic value Write Request */
                if(eventParam->handleValPair.attrHandle == cy_ble_htssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle)
                {
                    /* Validate Measure Interval value */
                    if(locCharIndex == CY_BLE_HTS_MEASURE_INTERVAL)
                    {
                        uint8_t locAttrValue[CY_BLE_HTS_VRD_LEN] = {0u};
                        uint16_t requestValue;

                        requestValue = Cy_BLE_Get16ByPtr(eventParam->handleValPair.value.val);

                        if(requestValue != 0u) /* 0 is valid interval value for no periodic measurement */
                        {
                            /* Check Valid range for Measure Interval characteristic value */
                            if(Cy_BLE_HTSS_GetCharacteristicDescriptor(eventParam->connHandle, locCharIndex,
                                CY_BLE_HTS_VRD, CY_BLE_HTS_VRD_LEN, locAttrValue) == CY_BLE_SUCCESS)
                            {
                                uint16_t lowerValue;
                                uint16_t upperValue;

                                lowerValue = Cy_BLE_Get16ByPtr(locAttrValue);
                                upperValue = Cy_BLE_Get16ByPtr(locAttrValue + sizeof(lowerValue));
                                requestValue = Cy_BLE_Get16ByPtr(eventParam->handleValPair.value.val);
                                if((requestValue != 0u) && ((requestValue < lowerValue) || (requestValue > upperValue)))
                                {
                                    gattErr = CY_BLE_GATT_ERR_HTS_OUT_OF_RANGE;
                                }
                            }
                        }
                    }
                    if(gattErr == CY_BLE_GATT_ERR_NONE)
                    {
                        /* Store value to database */
                        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

                        dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                        dbAttrValInfo.connHandle      = eventParam->connHandle;
                        dbAttrValInfo.offset          = 0u;
                        dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

                        gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
                        if(gattErr == CY_BLE_GATT_ERR_NONE)
                        {
                            Cy_BLE_HTS_ApplCallback((uint32_t)CY_BLE_EVT_HTSS_WRITE_CHAR, &locCharValue);
                        }
                    }
                }
                else /* Client Characteristic Configuration descriptor Write Request */
                {
                    /* Store value to database */
                    cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

                    dbAttrValInfo.handleValuePair = eventParam->handleValPair;
                    dbAttrValInfo.connHandle      = eventParam->connHandle;
                    dbAttrValInfo.offset          = 0u;
                    dbAttrValInfo.flags           = CY_BLE_GATT_DB_PEER_INITIATED;

                    gattErr = Cy_BLE_GATTS_WriteAttributeValueCCCD(&dbAttrValInfo);
                    if(gattErr == CY_BLE_GATT_ERR_NONE)
                    {
                        /* Check characteristic properties for Notification */
                        if(CY_BLE_IS_NOTIFICATION_SUPPORTED(cy_ble_htssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle))
                        {
                            if(CY_BLE_IS_NOTIFICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                            {
                                Cy_BLE_HTS_ApplCallback((uint32_t)CY_BLE_EVT_HTSS_NOTIFICATION_ENABLED, &locCharValue);
                            }
                            else
                            {
                                Cy_BLE_HTS_ApplCallback((uint32_t)CY_BLE_EVT_HTSS_NOTIFICATION_DISABLED, &locCharValue);
                            }
                        }
                        /* Check characteristic properties for Indication */
                        if(CY_BLE_IS_INDICATION_SUPPORTED(cy_ble_htssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle))
                        {
                            if(CY_BLE_IS_INDICATION_ENABLED_IN_PTR(eventParam->handleValPair.value.val))
                            {
                                Cy_BLE_HTS_ApplCallback((uint32_t)CY_BLE_EVT_HTSS_INDICATION_ENABLED, &locCharValue);
                            }
                            else
                            {
                                Cy_BLE_HTS_ApplCallback((uint32_t)CY_BLE_EVT_HTSS_INDICATION_DISABLED, &locCharValue);
                            }
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
* Function Name: Cy_BLE_HTSS_SendNotification
***************************************************************************//**
*
*  Sends notification with a characteristic value of the Health Thermometer
*  service, which is a value specified by charIndex, to the client device.
*
*  On enabling notification successfully for a service characteristic it sends out a
*  'Handle Value Notification' which results in #CY_BLE_EVT_HTSC_NOTIFICATION event
*  at the GATT Client's end.
*
*  \param connHandle: The connection handle.
*
*  \param charIndex:  The index of the service characteristic.
*
*  \param attrSize:   The size of the characteristic value attribute.
*
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     sent to the client's device.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
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
cy_en_ble_api_result_t Cy_BLE_HTSS_SendNotification(cy_stc_ble_conn_handle_t connHandle,
                                                    cy_en_ble_hts_char_index_t charIndex,
                                                    uint8_t attrSize,
                                                    uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send Notification if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(!CY_BLE_IS_NOTIFICATION_ENABLED(connHandle.attId, cy_ble_htssConfigPtr->attrInfo->
                                            charInfo[charIndex].descrHandle[CY_BLE_HTS_CCCD]))
    {
        apiResult = CY_BLE_ERROR_NTF_DISABLED;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
        ntfReqParam.handleValPair.attrHandle = cy_ble_htssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        ntfReqParam.handleValPair.value.val  = attrValue;
        ntfReqParam.handleValPair.value.len  = (uint16_t)attrSize;
        ntfReqParam.connHandle               = connHandle;

        /* ... end send notification to client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Notification(&ntfReqParam);
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HTSS_SendIndication
***************************************************************************//**
*
*  Sends indication with a characteristic value of the Health Thermometer
*  service, which is a value specified by charIndex, to the client device.
*
*  On enabling indication successfully it sends out a 'Handle Value Indication' which
*  results in #CY_BLE_EVT_HTSC_INDICATION or #CY_BLE_EVT_GATTC_HANDLE_VALUE_IND (if
*  service-specific callback function is not registered) event at the GATT Client's end.
*
*  \param connHandle: The connection handle.
*  \param charIndex:  The index of the service characteristic.
*  \param attrSize: The size of the characteristic value attribute.
*  \param attrValue: The pointer to the characteristic value data that should be
*               sent to the client device.
*
* \return
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
* \events
*  In case of successful execution (return value = \ref CY_BLE_SUCCESS)
*  the following events can appear: \n
*   If the HTS service-specific callback is registered
*      (with \ref Cy_BLE_HTS_RegisterAttrCallback):
*  * #CY_BLE_EVT_HTSS_INDICATION_CONFIRMED - In case if the indication is
*                                successfully delivered to the peer device.
*  .
*   Otherwise (if the HTS service-specific callback is not registered):
*  * #CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF - In case if the indication is
*                                successfully delivered to the peer device.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HTSS_SendIndication(cy_stc_ble_conn_handle_t connHandle,
                                                  cy_en_ble_hts_char_index_t charIndex,
                                                  uint8_t attrSize,
                                                  uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;

    /* Send Notification if it is enabled and connected */
    if(Cy_BLE_GetConnectionState(connHandle) < CY_BLE_CONN_STATE_CONNECTED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if(!CY_BLE_IS_INDICATION_ENABLED(connHandle.attId, cy_ble_htssConfigPtr->attrInfo->
                                          charInfo[charIndex].descrHandle[CY_BLE_HTS_CCCD]))
    {
        apiResult = CY_BLE_ERROR_IND_DISABLED;
    }
    else
    {
        /* Fill all fields of Write Request structure ... */
        cy_stc_ble_gatts_handle_value_ntf_t ntfReqParam;
        ntfReqParam.handleValPair.attrHandle = cy_ble_htssConfigPtr->attrInfo->charInfo[charIndex].charHandle;
        ntfReqParam.handleValPair.value.val  = attrValue;
        ntfReqParam.handleValPair.value.len  = attrSize;
        ntfReqParam.connHandle               = connHandle;

        /* ... and send indication to client using previously filled structure */
        apiResult = Cy_BLE_GATTS_Indication(&ntfReqParam);

        /* Save handle to support service-specific value confirmation response from client */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_htssReqHandle = ntfReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HTSS_ConfirmationEventHandler
***************************************************************************//**
*
*  Handles a Value Confirmation request event from the BLE Stack.
*
*  *eventParam - The pointer to a structure of type cy_stc_ble_conn_handle_t.
*
******************************************************************************/
static void Cy_BLE_HTSS_ConfirmationEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{

    cy_en_ble_hts_char_index_t locCharIndex;
    uint32_t locReqHandle = 0u;

    if((Cy_BLE_HTS_ApplCallback != NULL) && (cy_ble_htssReqHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        for(locCharIndex = CY_BLE_HTS_TEMP_MEASURE; (locCharIndex < CY_BLE_HTS_CHAR_COUNT) && (locReqHandle == 0u);
            locCharIndex++)
        {
            if(cy_ble_htssReqHandle == cy_ble_htssConfigPtr->attrInfo->charInfo[locCharIndex].charHandle)
            {
                cy_stc_ble_hts_char_value_t locCharValue;
                locCharValue.connHandle = *eventParam;
                locCharValue.charIndex = locCharIndex;
                locCharValue.value = NULL;

                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                cy_ble_htssReqHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_HTS_ApplCallback((uint32_t)CY_BLE_EVT_HTSS_INDICATION_CONFIRMED, &locCharValue);
                locReqHandle = 1u;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HTSC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP
*  event. Based on the service UUID, an appropriate data structure is populated
*  using the data received as part of the callback.
*
*  \param discCharInfo: The pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_HTSC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    /* HTS characteristics UUIDs */
    static const cy_ble_uuid16_t cy_ble_htscCharUuid[CY_BLE_HTS_CHAR_COUNT] =
    {
        CY_BLE_UUID_CHAR_TEMPERATURE_MSMNT,
        CY_BLE_UUID_CHAR_TEMPERATURE_TYPE,
        CY_BLE_UUID_CHAR_INTERMEDIATE_TEMP,
        CY_BLE_UUID_CHAR_MSMNT_INTERVAL
    };
    static cy_ble_gatt_db_attr_handle_t *lastEndHandle[CY_BLE_MAX_CONNECTION_INSTANCES] = { 0u };
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t htsDiscIdx = cy_ble_htscConfigPtr->serviceDiscIdx;
    uint32_t i;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == htsDiscIdx))
    {
        /* Get pointer (with offset) to HTS client structure with attribute handles */
        cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[discIdx];

        /* Update last characteristic endHandle to declaration handle of this characteristic */
        if(lastEndHandle[discIdx] != NULL)
        {
            *lastEndHandle[discIdx] = discCharInfo->charDeclHandle - 1u;
            lastEndHandle[discIdx] = NULL;
        }

        for(i = (uint32_t)CY_BLE_HTS_TEMP_MEASURE; i < (uint32_t)CY_BLE_HTS_CHAR_COUNT; i++)
        {
            if(cy_ble_htscCharUuid[i] == discCharInfo->uuid.uuid16)
            {
                if(htscPtr->charInfo[i].valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
                {
                    Cy_BLE_CheckStoreCharHandle(htscPtr->charInfo[i]);
                    lastEndHandle[discIdx] = &htscPtr->charInfo[i].endHandle;
                    break;
                }
                else
                {
                    Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION, &discCharInfo);
                }
            }
        }

        /* Init characteristic endHandle to the service endHandle. Characteristic endHandle will be updated to declaration
         * handler of the following characteristic, in the following characteristic discovery procedure. */
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
* Function Name: Cy_BLE_HTSC_DiscoverCharDescriptorsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_FIND_INFO_RSP event.
*  Based on the descriptor UUID, an appropriate data structure is populated using
*  the data received as part of the callback.
*
*  \param discDescrInfo: The pointer to a descriptor information structure.
*
******************************************************************************/
static void Cy_BLE_HTSC_DiscoverCharDescriptorsEventHandler(cy_stc_ble_disc_descr_info_t *discDescrInfo)
{
    uint32_t notSupportedDescr = 0u;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discDescrInfo->connHandle);
    uint32_t htsDiscIdx = cy_ble_htscConfigPtr->serviceDiscIdx;
    uint32_t descIdx;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == htsDiscIdx)
    {
        switch(discDescrInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_CLIENT_CONFIG:
                descIdx = (uint32_t)CY_BLE_HTS_CCCD;
                break;

            case CY_BLE_UUID_CHAR_VALID_RANGE:
                descIdx = (uint32_t)CY_BLE_HTS_VRD;
                break;

            default:
                /* Not supported descriptor */
                notSupportedDescr = 1u;
                break;
        }

        if(notSupportedDescr == 0u)
        {
            /* Get pointer (with offset) to HTS client structure with attribute handles */
            cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[discIdx];

            /* Characteristic index */
            uint32_t charInx = cy_ble_configPtr->context->discovery[discIdx].charCount;

            if(htscPtr->charInfo[charInx].descrHandle[descIdx] == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                htscPtr->charInfo[charInx].descrHandle[descIdx] = discDescrInfo->descrHandle;
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
* Function Name: Cy_BLE_HTSC_GetCharRange
***************************************************************************//**
*
* Returns a possible range of the current characteristic descriptor via
* input parameter charRangeInfo->range
*
* \param *charRangeInfo: The pointer to a descriptor range information structure.
*
******************************************************************************/
static void Cy_BLE_HTSC_GetCharRange(cy_stc_ble_disc_range_info_t *charRangeInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(charRangeInfo->connHandle);
    uint32_t htsDiscIdx = cy_ble_htscConfigPtr->serviceDiscIdx;
    uint32_t exitFlag = 0u;

    if(cy_ble_configPtr->context->discovery[discIdx].servCount == htsDiscIdx)
    {
        /* Get pointer (with offset) to HTS client structure with attribute handles */
        cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[discIdx];

        if(charRangeInfo->srviIncIdx != CY_BLE_DISCOVERY_INIT)
        {
            cy_ble_configPtr->context->discovery[discIdx].charCount++;
        }

        while((cy_ble_configPtr->context->discovery[discIdx].charCount < (uint32_t)CY_BLE_HTS_CHAR_COUNT) && (exitFlag == 0u))
        {
            uint32_t charIdx = cy_ble_configPtr->context->discovery[discIdx].charCount;
            if((htscPtr->charInfo[charIdx].endHandle - htscPtr->charInfo[charIdx].valueHandle) != 0u)
            {
                /* Read characteristic range */
                charRangeInfo->range.startHandle = htscPtr->charInfo[charIdx].valueHandle + 1u;
                charRangeInfo->range.endHandle = htscPtr->charInfo[charIdx].endHandle;
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
* Function Name: Cy_BLE_HTSC_SetCharacteristicValue
***************************************************************************//**
*
*  This function is used to write the characteristic (which is identified by
*  charIndex) value attribute in the server. As a result a Write Request is
*  sent to the GATT Server and on successful execution of the request on the
*  server side, the #CY_BLE_EVT_HTSS_WRITE_CHAR event is generated.
*  On successful request execution on the server side, the Write Response is
*  sent to the client.
*
*  \param connHandle: The connection handle.
*
*  \param charIndex:  The index of the service characteristic.
*
*  \param attrSize:   The size of the characteristic value attribute.
*
*  \param attrValue:  The pointer to the characteristic value data that should be
*                     sent to the server device.
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
*
* \events
*  If execution is successful (return value = #CY_BLE_SUCCESS),
*  these events can appear: \n
*   If a HTS service-specific callback is registered
*      with \ref Cy_BLE_HTS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HTSC_WRITE_DESCR_RESPONSE - if the requested attribute is
*     successfully written on the peer device, the details (charIndex,
*     descrIndex etc.) are provided with an event parameter structure
*     of type \ref cy_stc_ble_hts_descr_value_t.
*   .
*   Otherwise (if a HTS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is successfully
*     written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the requested
*     attribute on the peer device, the details are provided with an event
*     parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HTSC_SetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_hts_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t * attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HTS client structure with attribute handles */
    cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HTS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(htscPtr->charInfo[charIndex].valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        /* Fill all the fields of the Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle = htscPtr->charInfo[charIndex].valueHandle;
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;

        /* ... and send a request to the server device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicValue(&writeReqParam);

        /* Save handle to support service-specific Write Response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_htscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HTSC_GetCharacteristicValue
***************************************************************************//**
*
*  This function is used to read a characteristic value, which is a value
*  identified by charIndex, from the server.
*
*  \param connHandle: The connection handle.
*
*  \param charIndex:  The index of a service characteristic of type
*                     \ref cy_en_ble_hts_char_index_t.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                              | Description
*   ------------                             | -----------
*   CY_BLE_SUCCESS                           | The Read Request was sent successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER           | Validation of the input parameter failed.
*   CY_BLE_ERROR_INVALID_OPERATION           | Operation is invalid for this characteristic.
*   CY_BLE_ERROR_INVALID_STATE               | Connection with the server is not established.
*   CY_BLE_ERROR_MEMORY_ALLOCATION_FAILED    | Memory allocation failed.
*   CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE | The peer device doesn't have the particular characteristic.
*
*  \events
*  If execution is successful (return value = \ref CY_BLE_SUCCESS)
*  these events can appear: \n
*  If the HTS service-specific callback is registered
*      (with \ref Cy_BLE_HTS_RegisterAttrCallback):
*  * \ref CY_BLE_EVT_HTSC_READ_CHAR_RESPONSE - if the requested attribute is
*    successfully read on the peer device, the details (charIndex,
*    value, etc.) are provided with an event parameter structure
*    of type \ref cy_stc_ble_hts_char_value_t.
*  .
*  Otherwise (if a HTS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTC_READ_RSP - if the requested attribute is
*    successfully read on the peer device, the details (handle,
*    value, etc.) are provided with the event parameters structure
*    \ref cy_stc_ble_gattc_read_rsp_param_t.
*
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*    requested attribute on the peer device, the details are provided
*    with the event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HTSC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_hts_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HTS client structure with attribute handles */
    cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HTS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(htscPtr->charInfo[charIndex].valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        /* Fill all the fields of the Read Request structure ... */
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = htscPtr->charInfo[charIndex].valueHandle;
        readReqParam.connHandle = connHandle;

        /* ... and send a request to the server device. */
        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_htscReqHandle[discIdx] = htscPtr->charInfo[charIndex].valueHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HTSC_SetCharacteristicDescriptor
***************************************************************************//**
*
*  This function is used to write the characteristic descriptor to the server,
*  which is identified by charIndex and descrIndex.
*
*  Internally, Write Request is sent to the GATT Server and on successful
*  execution of the request on the server side, the following events can be
*  generated:
*  * #CY_BLE_EVT_HTSS_NOTIFICATION_ENABLED
*  * #CY_BLE_EVT_HTSS_NOTIFICATION_ENABLED
*  * #CY_BLE_EVT_HTSS_INDICATION_ENABLED
*  * #CY_BLE_EVT_HTSS_INDICATION_DISABLED
*
*  \param connHandle: The connection handle.
*
*  \param charIndex:  The index of the service characteristic.
*
*  \param descrIndex: The index of the service characteristic descriptor.
*
*  \param attrSize:   The size of the characteristic value attribute.
*
*  \param attrValue:  The pointer to the characteristic descriptor value data that
*                     should be sent to the server device.
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
*
* \events
*  If execution is successful (return value = #CY_BLE_SUCCESS),
*  these events can appear: \n
*   If a HTS service-specific callback is registered
*      with \ref Cy_BLE_HTS_RegisterAttrCallback():
*   * #CY_BLE_EVT_HTSC_WRITE_DESCR_RESPONSE - if the requested attribute is
*     successfully written on the peer device, the details (charIndex,
*     descrIndex etc.) are provided with an event parameter structure
*     of type \ref cy_stc_ble_hts_descr_value_t.
*   .
*   Otherwise (if a HTS service-specific callback is not registered):
*   * #CY_BLE_EVT_GATTC_WRITE_RSP - If the requested attribute is successfully
*     written on the peer device.
*
*   * #CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the requested
*     attribute on the peer device, the details are provided with an event
*     parameter structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HTSC_SetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hts_char_index_t charIndex,
                                                               cy_en_ble_hts_descr_index_t descrIndex,
                                                               uint8_t attrSize,
                                                               uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HTS client structure with attribute handles */
    cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HTS_CHAR_COUNT) || (descrIndex >= CY_BLE_HTS_DESCR_COUNT) ||
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(descrIndex != CY_BLE_HTS_CCCD)
    {
        apiResult = CY_BLE_ERROR_INVALID_OPERATION;
    }
    else
    {
        /* Fill all the fields of the Write Request structure ... */
        cy_stc_ble_gattc_write_req_t writeReqParam;
        writeReqParam.handleValPair.attrHandle = htscPtr->charInfo[charIndex].descrHandle[CY_BLE_HTS_CCCD];
        writeReqParam.handleValPair.value.val  = attrValue;
        writeReqParam.handleValPair.value.len  = attrSize;
        writeReqParam.connHandle               = connHandle;

        /* ... and send request to server device. */
        apiResult = Cy_BLE_GATTC_WriteCharacteristicDescriptors(&writeReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_htscReqHandle[discIdx] = writeReqParam.handleValPair.attrHandle;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HTSC_GetCharacteristicDescriptor
***************************************************************************//**
*
*  Gets the characteristic descriptor of the specified characteristic of the
*  service.
*
*  \param connHandle: The connection handle.
*
*  \param charIndex:  The index of the service characteristic.
*
*  \param descrIndex: The index of the service characteristic descriptor.
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
*
*  \events
*  If execution is successful (return value = \ref CY_BLE_SUCCESS),
*  these events can appear: \n
*  If a HTS service-specific callback is registered
*  (with \ref Cy_BLE_HTS_RegisterAttrCallback):
*  * \ref CY_BLE_EVT_HTSC_READ_DESCR_RESPONSE - if the requested attribute is
*    successfully read on the peer device, the details (charIndex,
*    descrIndex, value, etc.) are provided with an event parameter structure
*    of type \ref cy_stc_ble_hts_descr_value_t.
*  .
*  Otherwise (if a HTS service-specific callback is not registered):
*  * \ref CY_BLE_EVT_GATTC_READ_RSP - if the requested attribute is
*    successfully read on the peer device, the details (handle,
*    value, etc.) are provided with the event parameters structure
*    \ref cy_stc_ble_gattc_read_rsp_param_t.
*  * \ref CY_BLE_EVT_GATTC_ERROR_RSP - If an error occurred with the
*    requested attribute on the peer device, the details are provided
*    with the event parameters structure \ref cy_stc_ble_gatt_err_param_t.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_HTSC_GetCharacteristicDescriptor(cy_stc_ble_conn_handle_t connHandle,
                                                               cy_en_ble_hts_char_index_t charIndex,
                                                               cy_en_ble_hts_descr_index_t descrIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to HTS client structure with attribute handles */
    cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_HTS_CHAR_COUNT) || (descrIndex >= CY_BLE_HTS_DESCR_COUNT) ||
            (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Fill all the fields of the Write Request structure ... */
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = htscPtr->charInfo[charIndex].descrHandle[descrIndex];
        readReqParam.connHandle = connHandle;

        /* ... and send request to server device. */
        apiResult = Cy_BLE_GATTC_ReadCharacteristicDescriptors(&readReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_htscReqHandle[discIdx] = htscPtr->charInfo[charIndex].descrHandle[descrIndex];
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_HTSC_GetCharacteristicValueHandle
***************************************************************************//**
*
*  Returns the discovered peer device characteristic value handle.
*
*  \param connHandle:   The connection handle.
*  \param charIndex:    The index of a service characteristic.
*
* \return
*  Returns characteristic value handle of type cy_ble_gatt_db_attr_handle_t.
*  * CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE: when a peer device doesn't have
*                                           an optional characteristic
*
******************************************************************************/
cy_ble_gatt_db_attr_handle_t Cy_BLE_HTSC_GetCharacteristicValueHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                      cy_en_ble_hts_char_index_t charIndex)
{
    cy_ble_gatt_db_attr_handle_t tmpAttrHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (charIndex < CY_BLE_HTS_CHAR_COUNT))
    {
        /* Get pointer (with offset) to HTS client structure with attribute handles */
        cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[discIdx];

        /* Get attribute handles */
        tmpAttrHandle = htscPtr->charInfo[charIndex].valueHandle;
    }

    return (tmpAttrHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_HTSC_GetCharacteristicDescriptorHandle
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
cy_ble_gatt_db_attr_handle_t Cy_BLE_HTSC_GetCharacteristicDescriptorHandle(cy_stc_ble_conn_handle_t connHandle,
                                                                           cy_en_ble_hts_char_index_t charIndex,
                                                                           cy_en_ble_hts_descr_index_t descrIndex)
{
    cy_ble_gatt_db_attr_handle_t tmpAttrHandle = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (charIndex < CY_BLE_HTS_CHAR_COUNT) &&
       (descrIndex < CY_BLE_HTS_DESCR_COUNT))
    {
        /* Get pointer (with offset) to HTS client structure with attribute handles */
        cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[discIdx];

        /* Get attribute handles */
        tmpAttrHandle = htscPtr->charInfo[charIndex].descrHandle[descrIndex];
    }

    return (tmpAttrHandle);
}


/******************************************************************************
* Function Name: Cy_BLE_HTSC_NotificationEventHandler
***************************************************************************//**
*
*  Handles Notification event.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_HTSC_NotificationEventHandler(cy_stc_ble_gattc_handle_value_ntf_param_t *eventParam)
{
    cy_en_ble_hts_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    /* Get pointer (with offset) to HTS client structure with attribute handles */
    cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_HTS_ApplCallback != NULL))
    {
        for(locCharIndex = CY_BLE_HTS_TEMP_MEASURE; locCharIndex < CY_BLE_HTS_CHAR_COUNT; locCharIndex++)
        {
            if(htscPtr->charInfo[locCharIndex].valueHandle == eventParam->handleValPair.attrHandle)
            {
                cy_stc_ble_hts_char_value_t notifValue;
                notifValue.connHandle = eventParam->connHandle;
                notifValue.charIndex  = locCharIndex;
                notifValue.value      = &eventParam->handleValPair.value;

                Cy_BLE_HTS_ApplCallback((uint32_t)CY_BLE_EVT_HTSC_NOTIFICATION, &notifValue);
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                break;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HTSC_IndicationEventHandler
***************************************************************************//**
*
*  Handles Indication event.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_HTSC_IndicationEventHandler(cy_stc_ble_gattc_handle_value_ind_param_t *eventParam)
{
    cy_en_ble_hts_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    /* Get pointer (with offset) to HTS client structure with attribute handles */
    cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_HTS_ApplCallback != NULL))
    {
        for(locCharIndex = CY_BLE_HTS_TEMP_MEASURE; locCharIndex < CY_BLE_HTS_CHAR_COUNT; locCharIndex++)
        {
            if(htscPtr->charInfo[locCharIndex].valueHandle == eventParam->handleValPair.attrHandle)
            {
                cy_stc_ble_hts_char_value_t indicationValue;
                indicationValue.connHandle = eventParam->connHandle;
                indicationValue.charIndex  = locCharIndex;
                indicationValue.value      = &eventParam->handleValPair.value;

                Cy_BLE_HTS_ApplCallback((uint32_t)CY_BLE_EVT_HTSC_INDICATION, &indicationValue);
                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                break;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HTSC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles Read Response event.
*
*  \param eventParam: The pointer to the data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_HTSC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    cy_en_ble_hts_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
    uint32_t locReqHandle = 0u;

    /* Get pointer (with offset) to HTS client structure with attribute handles */
    cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_HTS_ApplCallback != NULL) &&
       (cy_ble_htscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        for(locCharIndex = CY_BLE_HTS_TEMP_MEASURE; (locCharIndex < CY_BLE_HTS_CHAR_COUNT) && (locReqHandle == 0u);
            locCharIndex++)
        {
            if(cy_ble_htscReqHandle[discIdx] == htscPtr->charInfo[locCharIndex].valueHandle)
            {
                cy_stc_ble_hts_char_value_t locCharValue;
                locCharValue.connHandle = eventParam->connHandle;
                locCharValue.charIndex  = locCharIndex;
                locCharValue.value      = &eventParam->value;

                cy_ble_htscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_HTS_ApplCallback((uint32_t)CY_BLE_EVT_HTSC_READ_CHAR_RESPONSE, &locCharValue);
                locReqHandle = 1u;
            }
            else
            {
                cy_en_ble_hts_descr_index_t locDescIndex;

                for(locDescIndex = CY_BLE_HTS_CCCD; (locDescIndex < CY_BLE_HTS_DESCR_COUNT) &&
                    (locReqHandle == 0u); locDescIndex++)
                {
                    if(cy_ble_htscReqHandle[discIdx] == htscPtr->charInfo[locCharIndex].
                        descrHandle[locDescIndex])
                    {
                        cy_stc_ble_hts_descr_value_t locDescrValue;
                        locDescrValue.connHandle = eventParam->connHandle;
                        locDescrValue.charIndex  = locCharIndex;
                        locDescrValue.descrIndex = locDescIndex;
                        locDescrValue.value      = &eventParam->value;

                        cy_ble_htscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                        Cy_BLE_HTS_ApplCallback((uint32_t)CY_BLE_EVT_HTSC_READ_DESCR_RESPONSE, &locDescrValue);
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
* Function Name: Cy_BLE_HTSC_WriteResponseEventHandler
***************************************************************************//**
*
*  Handles Write Response event.
*
*  \param eventParam: The pointer to a data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_HTSC_WriteResponseEventHandler(const cy_stc_ble_conn_handle_t *eventParam)
{
    cy_en_ble_hts_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(*eventParam);
    uint32_t locReqHandle = 0u;

    /* Get pointer (with offset) to HTS client structure with attribute handles */
    cy_stc_ble_htsc_t *htscPtr = (cy_stc_ble_htsc_t *)&cy_ble_htscConfigPtr->attrInfo[discIdx];

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_HTS_ApplCallback != NULL) &&
       (cy_ble_htscReqHandle[discIdx] != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE))
    {
        for(locCharIndex = CY_BLE_HTS_TEMP_MEASURE; (locCharIndex < CY_BLE_HTS_CHAR_COUNT) && (locReqHandle == 0u);
            locCharIndex++)
        {
            if(cy_ble_htscReqHandle[discIdx] == htscPtr->charInfo[locCharIndex].valueHandle)
            {
                cy_stc_ble_hts_char_value_t locCharValue;
                locCharValue.connHandle = *eventParam;
                locCharValue.charIndex  = locCharIndex;
                locCharValue.value      = NULL;

                cy_ble_htscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_HTS_ApplCallback((uint32_t)CY_BLE_EVT_HTSC_WRITE_CHAR_RESPONSE, &locCharValue);
                locReqHandle = 1u;
            }
            else if(cy_ble_htscReqHandle[discIdx] == htscPtr->charInfo[locCharIndex].descrHandle[CY_BLE_HTS_CCCD])
            {
                cy_stc_ble_hts_descr_value_t locDescrValue;
                locDescrValue.connHandle = *eventParam;
                locDescrValue.charIndex  = locCharIndex;
                locDescrValue.descrIndex = CY_BLE_HTS_CCCD;
                locDescrValue.value      = NULL;

                cy_ble_htscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_HTS_ApplCallback((uint32_t)CY_BLE_EVT_HTSC_WRITE_DESCR_RESPONSE, &locDescrValue);
                locReqHandle = 1u;
            }
            else /* No other destination for write operation is possible */
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
* Function Name: Cy_BLE_HTSC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles Error Response event.
*
*  \param eventParam: The pointer to a data structure specified by an event.
*
******************************************************************************/
static void Cy_BLE_HTSC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);
        if(eventParam->errInfo.attrHandle == cy_ble_htscReqHandle[discIdx])
        {
            cy_ble_htscReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_HTS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the HTS service.
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HTS_EventHandler(uint32_t eventCode,
                                                          void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Server events
     ***********************************/
    if(Cy_BLE_HTSS_EventHandlerCallback != NULL)
    {
        gattErr = Cy_BLE_HTSS_EventHandlerCallback(eventCode, eventParam);
    }

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_HTSC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_HTSC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_HTSS_EventHandler
***************************************************************************//**
*
*  Handles the server events from the BLE Stack for the HTS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HTSS_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /* Handling GATT Server events */
    switch((cy_en_ble_event_t)eventCode)
    {
        case CY_BLE_EVT_GATTS_WRITE_REQ:
            gattErr = Cy_BLE_HTSS_WriteEventHandler((cy_stc_ble_gatts_write_cmd_req_param_t*)eventParam);
            break;

        case CY_BLE_EVT_GATTS_HANDLE_VALUE_CNF:
            Cy_BLE_HTSS_ConfirmationEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
            break;

        default:
            break;
    }

    return (gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_HTSC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the HTS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
*  \return
*   A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_HTSC_EventHandler(uint32_t eventCode, void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                Cy_BLE_HTSC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR:
                Cy_BLE_HTSC_DiscoverCharDescriptorsEventHandler((cy_stc_ble_disc_descr_info_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_DISC_DESCR_GET_RANGE:
                Cy_BLE_HTSC_GetCharRange((cy_stc_ble_disc_range_info_t*)eventParam);
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
                    Cy_BLE_HTSC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                }
            }
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_NTF:
                Cy_BLE_HTSC_NotificationEventHandler((cy_stc_ble_gattc_handle_value_ntf_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_HANDLE_VALUE_IND:
                Cy_BLE_HTSC_IndicationEventHandler((cy_stc_ble_gattc_handle_value_ind_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                Cy_BLE_HTSC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                break;

            case CY_BLE_EVT_GATTC_WRITE_RSP:
                Cy_BLE_HTSC_WriteResponseEventHandler((cy_stc_ble_conn_handle_t*)eventParam);
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

