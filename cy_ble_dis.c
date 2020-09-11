/***************************************************************************//**
* \file cy_ble_dis.c
* \version 3.50
*
* \brief
*  Contains the source code for the Device Information service.
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
* Internal Defines / Macro Functions
*******************************************************************************/

#define Cy_BLE_DISC_CheckCharHandle(handle)                                \
do {                                                                       \
    if((handle).valueHandle == CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)      \
    {                                                                      \
        (handle).valueHandle = discCharInfo->valueHandle;                  \
        (handle).properties = discCharInfo->properties;                    \
    }                                                                      \
    else                                                                   \
    {                                                                      \
        Cy_BLE_ApplCallback((uint32_t)CY_BLE_EVT_GATTC_CHAR_DUPLICATION,   \
                            &(discCharInfo));                              \
    }                                                                      \
} while(0)


/*******************************************************************************
* Private Function Prototypes
*******************************************************************************/

static cy_en_ble_gatt_err_code_t Cy_BLE_DIS_EventHandler(uint32_t eventCode, void *eventParam);
static cy_en_ble_gatt_err_code_t Cy_BLE_DISC_EventHandler(uint32_t eventCode, void *eventParam);

static void Cy_BLE_DISC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo);
static void Cy_BLE_DISC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam);
static void Cy_BLE_DISC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam);


/*******************************************************************************
* Global Variables
*******************************************************************************/

/* The pointers to the callback functions */
static cy_ble_callback_t Cy_BLE_DIS_ApplCallback = NULL;
static cy_ble_event_handler_cb_t Cy_BLE_DISC_EventHandlerCallback = NULL;

/* The internal storage for the last request handle to check response */
static cy_ble_gatt_db_attr_handle_t cy_ble_discReqHandle[CY_BLE_MAX_CONNECTION_INSTANCES];

/* The pointer to a global BLE DIS server config structure */
const cy_stc_ble_diss_config_t *cy_ble_dissConfigPtr = NULL;

/* The pointer to a global BLE DIS client config structure */
const cy_stc_ble_disc_config_t *cy_ble_discConfigPtr = NULL;


/******************************************************************************
* Function Name: Cy_BLE_DISS_Init
***************************************************************************//**
*
*  This function initializes server of the Device Information service.
*
*  \param config: Configuration structure for the DIS.
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
cy_en_ble_api_result_t Cy_BLE_DISS_Init(const cy_stc_ble_diss_config_t *config)
{
    cy_en_ble_api_result_t apiResult;

    if(config == NULL)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Registers a pointer to config structure */
        cy_ble_dissConfigPtr = config;

        /* Registers event handler for the DIS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_DIS_EventHandler);
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_DISC_Init
***************************************************************************//**
*
*  This function initializes client of the Device Information service.
*
*  \param config: Configuration structure for the DIS.
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
cy_en_ble_api_result_t Cy_BLE_DISC_Init(const cy_stc_ble_disc_config_t *config)
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
        cy_ble_discConfigPtr = config;

        /* Registers event handler for the DIS */
        apiResult = Cy_BLE_RegisterServiceEventHandler(&Cy_BLE_DIS_EventHandler);

        /* Registers a pointer to client event handler function */
        Cy_BLE_DISC_EventHandlerCallback = &Cy_BLE_DISC_EventHandler;

        for(idx = 0u; idx < cy_ble_configPtr->params->maxClientCount; idx++)
        {
            uint32 servCnt    = cy_ble_configPtr->context->discServiCount;
            uint32 disServIdx = cy_ble_discConfigPtr->serviceDiscIdx;

            if(cy_ble_configPtr->context->serverInfo[(idx * servCnt) + disServIdx].range.startHandle ==
                CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
            {
                /* Get pointer to DIS client structure */
                cy_stc_ble_disc_t *discPtr = (cy_stc_ble_disc_t *)&cy_ble_discConfigPtr->attrInfo[idx];

                /* Clean client structure */
                (void)memset(discPtr, 0, sizeof(cy_stc_ble_disc_t));

                /* Initialize uuid  */
                cy_ble_configPtr->context->serverInfo[(idx * servCnt) + disServIdx].uuid =
                    CY_BLE_UUID_DEVICE_INFO_SERVICE;
            }

            cy_ble_discReqHandle[idx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_DIS_RegisterAttrCallback
***************************************************************************//**
*
*  Registers a callback function for service-specific attribute operations.
*  Service-specific Write Requests from the peer device will not be handled with
*  an unregistered callback function.
*
*  \param callbackFunc: An application layer event callback function to receive
*  events from the PSoC 6 BLE Middleware. The definition of \ref cy_ble_callback_t
*  for DIS service is:<br>
*  typedef void (* cy_ble_callback_t) (uint32_t eventCode, void *eventParam),
*  where:
*       * eventCode:  Indicates the event that triggered this callback
*         (e.g. #CY_BLE_EVT_DISC_READ_CHAR_RESPONSE).
*       * eventParam: Contains the parameters corresponding to the
*         current event; (e.g. pointer to \ref cy_stc_ble_dis_char_value_t
*         structure that contains details of the characteristic
*         for which the notification enabled event was triggered).
*
******************************************************************************/
void Cy_BLE_DIS_RegisterAttrCallback(cy_ble_callback_t callbackFunc)
{
    Cy_BLE_DIS_ApplCallback = callbackFunc;
}


/******************************************************************************
* Function Name: Cy_BLE_DISS_SetCharacteristicValue
***************************************************************************//**
*
*  Sets a characteristic value of the service, which is identified by charIndex,
*  to the local database.
*
*  \param charIndex: The index of a service characteristic.
*
*  \param attrSize:  The size of the characteristic value attribute.
*
*  \param attrValue: The pointer to the characteristic value data that should be
*                    stored to the GATT database.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_DISS_SetCharacteristicValue(cy_en_ble_dis_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if(charIndex >= CY_BLE_DIS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Store data in database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_dissConfigPtr->attrInfo->charHandle[charIndex];
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
* Function Name: Cy_BLE_DISS_GetCharacteristicValue
***************************************************************************//**
*
*  Gets a characteristic value of the service, which is identified by charIndex,
*  from the GATT database.
*
*  \param charIndex: The index of a service characteristic.
*
*  \param attrSize:  The size of the characteristic value attribute.
*
*  \param attrValue: The pointer to the location where characteristic value data
*                    should be stored.
*
*  \return
*   A return value of type \ref cy_en_ble_api_result_t.
*
*   Error Codes                             | Description
*   ------------                            | -----------
*   CY_BLE_SUCCESS                          | The request was handled successfully.
*   CY_BLE_ERROR_INVALID_PARAMETER          | Validation of the input parameter failed.
*
******************************************************************************/
cy_en_ble_api_result_t Cy_BLE_DISS_GetCharacteristicValue(cy_en_ble_dis_char_index_t charIndex,
                                                          uint8_t attrSize,
                                                          uint8_t *attrValue)
{
    cy_en_ble_api_result_t apiResult = CY_BLE_SUCCESS;

    if(charIndex >= CY_BLE_DIS_CHAR_COUNT)
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else
    {
        /* Read characteristic value from database */
        cy_stc_ble_gatts_db_attr_val_info_t dbAttrValInfo;

        dbAttrValInfo.connHandle.bdHandle        = 0u;
        dbAttrValInfo.connHandle.attId           = 0u;
        dbAttrValInfo.handleValuePair.attrHandle = cy_ble_dissConfigPtr->attrInfo->charHandle[charIndex];
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
* Function Name: Cy_BLE_DISC_DiscoverCharacteristicsEventHandler
***************************************************************************//**
*
*  This function is called on receiving a #CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP event.
*  Based on the service UUID, an appropriate data structure is populated using the
*  data received as part of the callback.
*
*  \param *discCharInfo:  the pointer to a characteristic information structure.
*
******************************************************************************/
static void Cy_BLE_DISC_DiscoverCharacteristicsEventHandler(cy_stc_ble_disc_char_info_t *discCharInfo)
{
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(discCharInfo->connHandle);
    uint32_t disDiscIdx = cy_ble_discConfigPtr->serviceDiscIdx;

    if((discCharInfo->uuidFormat == CY_BLE_GATT_16_BIT_UUID_FORMAT) &&
       (cy_ble_configPtr->context->discovery[discIdx].servCount == disDiscIdx))
    {
        /* Get pointer (with offset) to DIS client structure with attribute handles */
        cy_stc_ble_disc_t *discPtr = (cy_stc_ble_disc_t *)&cy_ble_discConfigPtr->attrInfo[discIdx];

        switch(discCharInfo->uuid.uuid16)
        {
            case CY_BLE_UUID_CHAR_MANUFACTURER_NAME:
                Cy_BLE_DISC_CheckCharHandle(discPtr->charInfo[CY_BLE_DIS_MANUFACTURER_NAME]);
                break;

            case CY_BLE_UUID_CHAR_MODEL_NUMBER:
                Cy_BLE_DISC_CheckCharHandle(discPtr->charInfo[CY_BLE_DIS_MODEL_NUMBER]);
                break;

            case CY_BLE_UUID_CHAR_SERIAL_NUMBER:
                Cy_BLE_DISC_CheckCharHandle(discPtr->charInfo[CY_BLE_DIS_SERIAL_NUMBER]);
                break;

            case CY_BLE_UUID_CHAR_HARDWARE_REV:
                Cy_BLE_DISC_CheckCharHandle(discPtr->charInfo[CY_BLE_DIS_HARDWARE_REV]);
                break;

            case CY_BLE_UUID_CHAR_FIRMWARE_REV:
                Cy_BLE_DISC_CheckCharHandle(discPtr->charInfo[CY_BLE_DIS_FIRMWARE_REV]);
                break;

            case CY_BLE_UUID_CHAR_SOFTWARE_REV:
                Cy_BLE_DISC_CheckCharHandle(discPtr->charInfo[CY_BLE_DIS_SOFTWARE_REV]);
                break;

            case CY_BLE_UUID_CHAR_SYSTEM_ID:
                Cy_BLE_DISC_CheckCharHandle(discPtr->charInfo[CY_BLE_DIS_SYSTEM_ID]);
                break;

            case CY_BLE_UUID_CHAR_REG_CERT_DATA:
                Cy_BLE_DISC_CheckCharHandle(discPtr->charInfo[CY_BLE_DIS_REG_CERT_DATA]);
                break;

            case CY_BLE_UUID_CHAR_PNP_ID:
                Cy_BLE_DISC_CheckCharHandle(discPtr->charInfo[CY_BLE_DIS_PNP_ID]);
                break;

            default:
                break;
        }

        /* Indicate that request was handled */
        cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
    }
}


/******************************************************************************
* Function Name: Cy_BLE_DISC_GetCharacteristicValue
***************************************************************************//**
*
*  This function is used to read the characteristic Value from a server
*  which is identified by charIndex.
*
*  The Read Response returns the characteristic value in the Attribute Value
*  parameter. The Read Response only contains the characteristic value that is
*  less than or equal to (MTU - 1) octets in length. If the characteristic value
*  is greater than (MTU - 1) octets in length, a Read Long Characteristic Value
*  procedure may be used if the rest of the characteristic value is required.
*
*  This function call can result in generation of the following events based on
*  the response from the server device.
*
*  * #CY_BLE_EVT_DISC_READ_CHAR_RESPONSE
*  * #CY_BLE_EVT_GATTC_ERROR_RSP
*
*  \param connHandle: The connection handle.
*
*  \param charIndex:  The index of a service characteristic of type
*                     \ref cy_en_ble_dis_char_index_t.
*
*  \return
*  A return value of type \ref cy_en_ble_api_result_t.

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
*  If the DIS service-specific callback is registered
*      (with \ref Cy_BLE_DIS_RegisterAttrCallback):
*  * \ref CY_BLE_EVT_DISC_READ_CHAR_RESPONSE - if the requested attribute is
*    successfully read on the peer device, the details (charIndex,
*    value, etc.) are provided with an event parameter structure
*    of type \ref cy_stc_ble_dis_char_value_t.
*  .
*  Otherwise (if a DIS service-specific callback is not registered):
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
cy_en_ble_api_result_t Cy_BLE_DISC_GetCharacteristicValue(cy_stc_ble_conn_handle_t connHandle,
                                                          cy_en_ble_dis_char_index_t charIndex)
{
    cy_en_ble_api_result_t apiResult;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(connHandle);

    /* Get pointer (with offset) to DIS client structure with attribute handles */
    cy_stc_ble_disc_t *discPtr = (cy_stc_ble_disc_t *)&cy_ble_discConfigPtr->attrInfo[discIdx];

    if(Cy_BLE_GetConnectionState(connHandle) != CY_BLE_CONN_STATE_CLIENT_DISCOVERED)
    {
        apiResult = CY_BLE_ERROR_INVALID_STATE;
    }
    else if((charIndex >= CY_BLE_DIS_CHAR_COUNT) || (discIdx >= cy_ble_configPtr->params->maxClientCount))
    {
        apiResult = CY_BLE_ERROR_INVALID_PARAMETER;
    }
    else if(discPtr[discIdx].charInfo[charIndex].valueHandle != CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE)
    {
        /* Fill Read Request parameter */
        cy_stc_ble_gattc_read_req_t readReqParam;
        readReqParam.attrHandle = discPtr[discIdx].charInfo[charIndex].valueHandle;
        readReqParam.connHandle = connHandle;

        apiResult = Cy_BLE_GATTC_ReadCharacteristicValue(&readReqParam);

        /* Save handle to support service-specific read response from device */
        if(apiResult == CY_BLE_SUCCESS)
        {
            cy_ble_discReqHandle[discIdx] = discPtr[discIdx].charInfo[charIndex].valueHandle;
        }
    }
    else
    {
        apiResult = CY_BLE_ERROR_GATT_DB_INVALID_ATTR_HANDLE;
    }

    return(apiResult);
}


/******************************************************************************
* Function Name: Cy_BLE_DISC_ReadResponseEventHandler
***************************************************************************//**
*
*  Handles the Read Response event.
*
*  \param *eventParam: the pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_DISC_ReadResponseEventHandler(cy_stc_ble_gattc_read_rsp_param_t *eventParam)
{
    cy_en_ble_dis_char_index_t locCharIndex;
    uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(eventParam->connHandle);

    if((discIdx < cy_ble_configPtr->params->maxClientCount) && (Cy_BLE_DIS_ApplCallback != NULL) &&
       (CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE != cy_ble_discReqHandle[discIdx]))
    {
        /* Get pointer (with offset) to DIS client structure with attribute handles */
        cy_stc_ble_disc_t *discPtr = (cy_stc_ble_disc_t *)&cy_ble_discConfigPtr->attrInfo[discIdx];

        for(locCharIndex = CY_BLE_DIS_MANUFACTURER_NAME; locCharIndex < CY_BLE_DIS_CHAR_COUNT; locCharIndex++)
        {
            if(cy_ble_discReqHandle[discIdx] == discPtr[discIdx].charInfo[locCharIndex].valueHandle)
            {
                cy_stc_ble_dis_char_value_t locCharValue;
                locCharValue.connHandle = eventParam->connHandle;
                locCharValue.charIndex  = locCharIndex;
                locCharValue.value      = &eventParam->value;

                cy_ble_eventHandlerFlag &= (uint8_t) ~CY_BLE_CALLBACK;
                cy_ble_discReqHandle[discIdx] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
                Cy_BLE_DIS_ApplCallback((uint32_t)CY_BLE_EVT_DISC_READ_CHAR_RESPONSE, &locCharValue);
                break;
            }
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_DISC_ErrorResponseEventHandler
***************************************************************************//**
*
*  Handles the Error Response event.
*
*  \param *eventParam: the pointer to the data structure specified by the event.
*
******************************************************************************/
static void Cy_BLE_DISC_ErrorResponseEventHandler(const cy_stc_ble_gatt_err_param_t *eventParam)
{
    if(eventParam != NULL)
    {
        if(cy_ble_discReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] == eventParam->errInfo.attrHandle)
        {
            cy_ble_discReqHandle[Cy_BLE_GetDiscoveryIdx(eventParam->connHandle)] = CY_BLE_GATT_INVALID_ATTR_HANDLE_VALUE;
        }
    }
}


/******************************************************************************
* Function Name: Cy_BLE_DIS_EventHandler
***************************************************************************//**
*
*  Handles the events from the BLE Stack for the DIS service.
*
*  \param eventCode:  the event code
*  \param eventParam:  the event parameters
*
* \return
*  A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_DIS_EventHandler(uint32_t eventCode,
                                                         void *eventParam)
{
    cy_en_ble_gatt_err_code_t gattErr = CY_BLE_GATT_ERR_NONE;

    /************************************
     * Handling GATT Client events
     ***********************************/
    if(Cy_BLE_DISC_EventHandlerCallback != NULL)
    {
        (void)Cy_BLE_DISC_EventHandlerCallback(eventCode, eventParam);
    }

    return(gattErr);
}


/******************************************************************************
* Function Name: Cy_BLE_DISC_EventHandler
***************************************************************************//**
*
*  Handles the client events from the BLE Stack for the DIS service
*
*  \param eventCode:  the event code
*  \param eventParam: the event parameters
*
* \return
*  A return value of type cy_en_ble_gatt_err_code_t.
*
******************************************************************************/
static cy_en_ble_gatt_err_code_t Cy_BLE_DISC_EventHandler(uint32_t eventCode, void *eventParam)
{
    if(eventCode > (uint32_t)CY_BLE_EVT_MAX)
    {
        /* Handling service-specific events */
        switch((cy_en_ble_evt_t)eventCode)
        {
            /* Discovery events */
            case CY_BLE_EVT_GATTC_DISC_CHAR:
                (void)Cy_BLE_DISC_DiscoverCharacteristicsEventHandler((cy_stc_ble_disc_char_info_t*)eventParam);
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
            case CY_BLE_EVT_GATTC_READ_BY_TYPE_RSP:
                /* implementation in cy_ble_event_handler.c */
                break;

            case CY_BLE_EVT_GATTC_ERROR_RSP:
            {
                uint32_t discIdx = Cy_BLE_GetDiscoveryIdx(((cy_stc_ble_gatt_err_param_t*)eventParam)->connHandle);

                if((discIdx < cy_ble_configPtr->params->maxClientCount) &&
                   (cy_ble_configPtr->context->discovery[discIdx].autoDiscoveryFlag == 0u) &&
                   (((cy_stc_ble_gatt_err_param_t*)eventParam)->errInfo.errorCode != CY_BLE_GATT_ERR_ATTRIBUTE_NOT_FOUND))
                {
                    if((cy_ble_eventHandlerFlag & CY_BLE_CALLBACK) != 0u)
                    {
                        Cy_BLE_DISC_ErrorResponseEventHandler((cy_stc_ble_gatt_err_param_t*)eventParam);
                    }
                }
            }
                break;

            case CY_BLE_EVT_GATTC_READ_RSP:
                if((cy_ble_eventHandlerFlag & CY_BLE_CALLBACK) != 0u)
                {
                    Cy_BLE_DISC_ReadResponseEventHandler((cy_stc_ble_gattc_read_rsp_param_t*)eventParam);
                }
                break;

            default:
                break;
        }
    }

    return(CY_BLE_GATT_ERR_NONE);
}

#endif /* CY_BLE_LIB_HOST_CORE */
#endif /* CY_IP_MXBLESS */


/* [] END OF FILE */
